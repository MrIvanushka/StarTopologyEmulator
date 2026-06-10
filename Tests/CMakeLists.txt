cmake_minimum_required(VERSION 3.25)

project(StarTopologyEmulatorTests)

include(FetchContent)

FetchContent_Declare(
	googletest
	GIT_REPOSITORY https://github.com/google/googletest.git
	GIT_TAG v1.14.0
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

set(TEST_SOURCES
	Helpers.h
	Mocks/MockBacklogAccumulator.h
	Mocks/MockDynamicFrameSettings.h
	Mocks/MockEmulator.h
	Mocks/MockFrameCalculator.h
	Mocks/MockFtpGenerator.h
	Mocks/MockIncomeLoadController.h
	Mocks/MockIncomeLoadEstimator.h
	Mocks/MockIncomeStationsPredictor.h
	Mocks/MockScenarioEvent.h
	Mocks/MockStarHub.h
	Mocks/MockStarHubStrategy.h
	Mocks/MockStarStation.h
	BacklogAccumulatorTest.cpp
	DynamicFrameSettingsTest.cpp
	FrameCalculatorsTest.cpp
	IncomeLoadEstimatorsTest.cpp
	FtpGeneratorTest.cpp
	IncomeLoadControllersTest.cpp
	IncomeStationsPredictorsTest.cpp
	ScenarioRunnerTest.cpp
	StarHubStrategiesTest.cpp
	StarStationTest.cpp
	StarHubTest.cpp
	EmulatorTest.cpp
	CrdsaFrameModelTest.cpp
	Aff3ctPhysicalLinkTest.cpp
	PhysicalLinkTest.cpp
	TimeSeriesMetricSinkTest.cpp
	TrafficProfileTest.cpp
)

add_executable(${PROJECT_NAME} ${TEST_SOURCES})

add_subdirectory(../ ${CMAKE_CURRENT_BINARY_DIR}/StarTopologyEmulator)

target_link_libraries(${PROJECT_NAME} PRIVATE
	StarTopologyEmulator
	GTest::gtest
	GTest::gmock
	GTest::gtest_main
)

target_include_directories(${PROJECT_NAME} PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
)

if(STE_ENABLE_AFF3CT)
	target_compile_definitions(${PROJECT_NAME} PRIVATE
		STE_AFF3CT_TEST_MATRIX="${CMAKE_CURRENT_SOURCE_DIR}/../External/aff3ct/conf/dec/LDPC/AR4JA_4096_8192.qc")
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
	CXX_STANDARD 23
	CXX_STANDARD_REQUIRED ON
)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})
