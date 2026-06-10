# StarTopologyEmulator

Библиотека содержит эмулятор спутниковой сети TDMA с MAC-уровнем случайного доступа для входящих в сеть абонентских станций и физически достоверным разрешением коллизий по схеме CRDSA + SIC. Поддерживает подмену реализаций ключевых алгоритмов через интерфейсы и публичные фабрики.

## Требования

- Компилятор с поддержкой C++23
- CMake ≥ 3.25

### Использование эмулятора


Библиотека может быть подключена к внешнему процету следующим образом:

```cmake
add_subdirectory(path/to/StarTopologyEmulator)

add_executable(MyApp main.cpp)
target_link_libraries(MyApp PRIVATE StarTopologyEmulator)
```

Заголовки автоматически становятся доступны (`#include "StarTopologyEmulator/..."`).

Минимальный `main.cpp`, который создаёт эмулятор на 50 абонентских станций со стратегией хаба по умолчанию и прокручивает время до горизонта 1 000 000 единиц:

```cpp
#include <memory>
#include <random>

#include "StarTopologyEmulator/BacklogAccumulator/BacklogAccumulatorFactory.h"
#include "StarTopologyEmulator/DynamicFrameSettingsFactory.h"
#include "StarTopologyEmulator/EmulatorFactory.h"
#include "StarTopologyEmulator/FrameCalculatorFactory.h"
#include "StarTopologyEmulator/IncomeLoadEstimator/EmaIncomeLoadEstimatorConfig.h"
#include "StarTopologyEmulator/IncomeLoadEstimator/IncomeLoadEstimatorFactory.h"
#include "StarTopologyEmulator/StarHubStrategy/StarHubStrategyConfig.h"
#include "StarTopologyEmulator/StarHubStrategy/StarHubStrategyFactory.h"
#include "StarTopologyEmulator/Stations/StarHubFactory.h"
#include "StarTopologyEmulator/Stations/StarStationFactory.h"

using namespace starTopologyEmulator;

int main()
{
	constexpr int       kStationCount = 50;
	constexpr Timestamp kTts          = 100;
	constexpr Timestamp kHorizon      = 1'000'000;

	FrameConfig frameCfg{};
	frameCfg.slotCountInFrame = 100;
	frameCfg.slotDuration     = 100;
	frameCfg.epoch            = 0;
	frameCfg.bitsPerSlot      = 1024;

	std::mt19937 rng(42);

	EmulatorInitData init;
	init.abonentFrameCalculator = FrameCalculatorFactory::abonentCalculator(frameCfg, kTts);
	init.hubFrameCalculator     = FrameCalculatorFactory::hubCalculator(frameCfg, kTts);
	init.stationCount           = kStationCount;

	//сборка абонентской станции
	init.stationFactory = [frameCfg, &rng](EmulatorInitData::SendFunc sendFunc, StationID id,
	                                       std::shared_ptr<IStationStatsCollector> stats) {
		StarStationInitData data{
			.sendFunc             = std::move(sendFunc),
			.frameCalculator      = FrameCalculatorFactory::abonentCalculator(frameCfg, kTts),
			.dynamicFrameSettings = DynamicFrameSettingsFactory::make(),
			.id                   = id,
			.messagesNeeded       = 3,
			.tts                  = kTts,
			.rng                  = rng,
			.statsCollector       = std::move(stats),
		};
		return StarStationFactory::make(std::move(data));
	};

	//сборка центральной станции
	init.hubFactory = [frameCfg](EmulatorInitData::SendFunc sendFunc) {
		auto frameCalc = std::shared_ptr<IFrameCalculator>(
			FrameCalculatorFactory::hubCalculator(frameCfg, kTts));
		auto dyn       = std::shared_ptr<IDynamicFrameSettings>(DynamicFrameSettingsFactory::make());
		auto estimator = std::shared_ptr<IIncomeLoadEstimator>(
			IncomeLoadEstimatorFactory::make(EmaIncomeLoadEstimatorConfig{}));
		auto strategy  = StarHubStrategyFactory::make(estimator, StarHubStrategyConfig{});
		auto backlog   = std::shared_ptr<IBacklogAccumulator>(
			BacklogAccumulatorFactory::make(frameCfg.bitsPerSlot));

		StarHubInitData data{
			.sendFunc             = std::move(sendFunc),
			.incomeLoadEstimator  = std::move(estimator),
			.frameCalculator      = std::move(frameCalc),
			.dynamicFrameSettings = std::move(dyn),
			.strategy             = std::move(strategy),
			.backlogAccumulator   = std::move(backlog),
			.tts                  = kTts,
		};
		return StarHubFactory::make(std::move(data));
	};

	//сборка эмулятора
	auto emu = EmulatorFactory::make(std::move(init));

	//прогон теста
	for (auto now = 0; now <= kHorizon; ++now)
		emu->update(now);

	return 0;
}
```

### Тесты

```bash
cmake -S Tests -B _buildTests
cmake --build _buildTests --config Debug
_buildTests/Debug/StarTopologyEmulatorTests.exe
```

### Тесты с физическим бэкендом AFF3CT (опционально)

Библиотека AFF3CT используется для реализации физически досторерного CRDSA+SIC. Активирует `Aff3ctPhysicalLink` (LDPC + BPSK + AWGN + SPA-декодер).

```bash
git submodule update --init --recursive
cmake -S Tests -B _buildTests -DSTE_ENABLE_AFF3CT=ON
cmake --build _buildTests --config Debug
_buildTests/Debug/StarTopologyEmulatorTests.exe
```

## Алгоритмы планировщика центральной станции

Все алгоритмические компоненты доступны через публичные интерфейсы (в `Include/StarTopologyEmulator/IFaces/`) и создаются через одноимённые фабрики (`<Feature>Factory`). Конфиги «по варианту» — у каждой реализации свой структ.

### IIncomeStationsPredictor — оценка числа готовых к передаче станций

Контракт: `double estimateReadyUsers(currentFrame, targetFrame)`. Возвращает ожидаемое число активных абонентов, которые попытаются вещать в RA-сегменте `targetFrame`. Используется генератором FTP и контроллером нагрузки. Фабрика — `IncomeStationsPredictorFactory`.

| Реализация | Идея алгоритма |
|---|---|
| **StaticIncomeStationsPredictor** | Прямая оценка из истории нагрузки: `n̂ = g · raSlots / aggressiveness`, где `aggressiveness` — функция от `pTx` и `baseWindow`. Без собственного состояния между кадрами. |
| **LinearRegressionIncomeStationsPredictor** | Экспоненциально-взвешенная МНК-регрессия по скользящему окну истории нагрузки с фактором забывания `w = exp(−age/τ)`. Конфиг: `regressionWindow`, `forgettingHorizonSec`. |
| **GreyModelIncomeStationsPredictor** | Серая модель GM(1,1): аккумулированная последовательность (AGO) + МНК-подгонка экспоненты, прогноз `(1−eᵃ)(x₀ − b/a)e⁻ᵃᵏ`. Конфиг: `windowSize`, `minHistory`. |
| **CogorthyIncomeStationsPredictor** | Суммирует вклад исторических кадров, где «выживаемость» затухает множителем `(1 − pTx)` на каждый предыдущий кадр. Конфиг отсутствует (только конструктор). |
| **BackoffAwareCogorthyIncomeStationsPredictor** | Усиленный вариант предыдущего: учитывает глубину backoff-окна — перебирает возможные задержки повторных попыток и взвешивает по вероятности коллизии на каждом шаге backoff. |

Все варианты, кроме `Cogorthy*`, ограничивают результат через `minProbability`/`maxProbability` (числовая стабилизация).

### IFtpGenerator — генератор плана кадра

Контракт: `FtpConfig generate(currentFrame, targetFrame)`. Возвращает распределение слотов кадра по сегментам: онлайн (`online`), жёлтые (`yellow`) и случайного доступа (`raSlots`). Фабрика — `FtpGeneratorFactory`.

| Реализация | Идея алгоритма |
|---|---|
| **StaticFtpGenerator** | Возвращает фиксированный план без адаптации (заданные в конструкторе количества слотов). |
| **BacklogFeedbackFtpGenerator** | PI-обратная связь по очередям: `rawRa = R₀ + kJ·(jRa − jStar) − kQ·(qDa − qStar)`, ограничение скорости изменения через `clamp(rawRa − lastRa, −ΔR, +ΔR)`. |
| **ServiceDelayFtpGenerator** | Перебирает кандидатов `r ∈ [raMin, raMax]`, оценивает задержки RA-канала (`d₀ + exp(n̂·aggr/r)/aggr`) и DA-канала (`qDa/dSlots`), выбирает `r`, минимизирующее `\|dRa − λ·dDa\|`. |
| **LyapunovFtpGenerator** | Drift-plus-penalty: ищет `r`, минимизирующее `−qRa·μRa(r) − qDa·dSlots + V·penalty(r)`, где `μRa` и `penalty` отражают успешные передачи и коллизионные потери. |
| **MarginalUtilityFtpGenerator** | Обобщающий вариантный генератор: перебирает `r` и максимизирует одну из 8 утилит ниже (см. `F1`–`F8`). Конфиг — `std::variant` поверх F1…F8. |
| **UtilityFtpGenerator&lt;F1LinearUtility&gt;** | Линейная утилита: взвешенная сумма успеха acquisition, авторизации, коллизий, бэклога, задержки. |
| **UtilityFtpGenerator&lt;F2LogarithmicUtility&gt;** | Пропорциональная справедливость Kelly (1998): `U(x) = w·ln(1+x)`. |
| **UtilityFtpGenerator&lt;F3AlphaFairUtility&gt;** | α-fair: `Uᵅ(x) = (x+ε)^(1−α)/(1−α)`; `α=0` — max-throughput, `α=1` — Kelly, `α→∞` — max-min. |
| **UtilityFtpGenerator&lt;F4SigmoidalUtility&gt;** | Несвыпуклая сигмоида `U(x) = w/(1 + exp(−k(x − x₀)))`: моделирует «порог полезности», требует полного перебора, а не градиента. |
| **UtilityFtpGenerator&lt;F5HardDeadlineUtility&gt;** | Жёсткий дедлайн DA: штраф `−B·max(0, D^DA − Dtar)²`; для RA — Kelly-логарифм. |
| **UtilityFtpGenerator&lt;F6CostOfDelayUtility&gt;** | Правило `cμ`: `U^DA = −cD·D^DA·Q^DA`, `U^RA = −cJ·D^RA·J^RA`. |
| **UtilityFtpGenerator&lt;F7QuadraticBacklogUtility&gt;** | Квадратичный штраф бэклога: `U^DA = wB·ln(1+d) − wS·(Q^DA)²` (эквивалентно Lyapunov при `V = wS/wB`). |
| **UtilityFtpGenerator&lt;F8CesUtility&gt;** | CES-функция (постоянная эластичность замещения) с параметром `ρ` для агрегации RA- и DA-вкладов. |

Каждый F1…F8 имеет собственный конфиг (`F1LinearUtilityConfig`, …, `F8CesUtilityConfig`) с общими полями `raMin`, `raMax`, `yellowSlots` плюс параметрами своей утилиты.

### ITrafficProfile — модель трафика абонента

Контракт: `uint64_t generateBits(Timestamp duration)`. Возвращает число бит, сгенерированных за интервал; реализации сохраняют дробные остатки между вызовами для точности. Фабрика — `TrafficProfileFactory`.

| Реализация | Идея алгоритма |
|---|---|
| **CbrTrafficProfile** | Constant Bit Rate: `bits = ⌊bitsPerTimestamp·duration + residual⌋`, остаток переносится в следующий вызов. Конфиг: `bitsPerTimestamp`. |
| **PoissonTrafficProfile** | Пуассоновский поток пакетов: число пакетов ~ `Poisson(λ·duration)`, итог = `packets·bitsPerPacket`. Конфиг: `packetsPerTimestamp`, `bitsPerPacket`, `seed`. |
| **BurstTrafficProfile** | On-off с экспоненциальными длительностями фаз: в On — пуассон, в Off — ноль; средние длительности `meanOnDuration`/`meanOffDuration` задают burstiness. |
| **ParetoBurstTrafficProfile** | On-off с тяжёлохвостовыми длительностями фаз: `xₘ / (1−U)^(1/α)` при `α > 1`, `U ~ Uniform(0,1)`. Моделирует self-similar / long-range-dependent трафик. |

### IIncomeLoadController — контроллер вероятности передачи

Контракт: `BackoffConfig generate(plannedRaSlots, currentFrame, targetFrame)`. Возвращает параметры backoff'а абонента (`pTx`, `baseWindow`, `maxWindow`, `backoffType`), регулирующие нагрузку под текущее распределение слотов. Фабрика — `IncomeLoadControllerFactory`.

| Реализация | Идея алгоритма |
|---|---|
| **StaticIncomeLoadController** | Возвращает фиксированный `BackoffConfig` без адаптации. |
| **PiLoadController** | PI-регулятор: `error = gTarget − ĝ`, `u = kP·error + ∫kI·error`, `p ← clamp(p + α·u)` с anti-windup и утечкой интеграла. Конфиг богатый: `gTarget`, `kP`, `kI`, `integralWindowFrames`, `α`, `antiWindup`, …. |
| **PseudoBayesianLoadController** | Классическая стабилизация slotted ALOHA: целевое `pTx = 1/n`, где `n = estimatedStations / plannedRaSlots`, с ограничением скорости (`maxStepUp`/`maxStepDown`). |

Все «градиентные» контроллеры разделяют общий набор гиперпараметров: `gradientStep` (шаг η), `maxProbabilityStep` (антискачок), `minProbability`/`maxProbability` (диапазон). Названный за ними `epsilon` — это численная стабилизация делителей.

---