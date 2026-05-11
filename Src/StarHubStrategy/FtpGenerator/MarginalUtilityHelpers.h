#pragma once

#include <algorithm>
#include <cmath>
#include <limits>

#include "StarTopologyEmulator/StarHubStrategy/FtpGenerator/F1LinearUtilityConfig.h"
#include "StarTopologyEmulator/StarHubStrategy/FtpGenerator/F2LogarithmicUtilityConfig.h"
#include "StarTopologyEmulator/StarHubStrategy/FtpGenerator/F3AlphaFairUtilityConfig.h"
#include "StarTopologyEmulator/StarHubStrategy/FtpGenerator/F4SigmoidalUtilityConfig.h"
#include "StarTopologyEmulator/StarHubStrategy/FtpGenerator/F5HardDeadlineUtilityConfig.h"
#include "StarTopologyEmulator/StarHubStrategy/FtpGenerator/F6CostOfDelayUtilityConfig.h"
#include "StarTopologyEmulator/StarHubStrategy/FtpGenerator/F7QuadraticBacklogUtilityConfig.h"
#include "StarTopologyEmulator/StarHubStrategy/FtpGenerator/F8CesUtilityConfig.h"

namespace starTopologyEmulator::detail
{

inline double channelG(double r, double nHat, double ptx)
{
    return nHat * ptx / r;
}

inline double muRa(double r, double nHat, double ptx)
{
    return nHat * ptx * std::exp(-channelG(r, nHat, ptx));
}

inline double collRa(double r, double nHat, double ptx)
{
    const double g = channelG(r, nHat, ptx);
    return r * (1.0 - std::exp(-g) * (1.0 + g));
}

inline double delayRa(double r, double nHat, double ptx, double wb, double d0)
{
    const double g = channelG(r, nHat, ptx);
    const double a = 2.0 * ptx / (wb + 1.0);
    return d0 + std::exp(g) / a;
}

inline double throughputDa(double d, double qDaSlots)
{
    return std::min(qDaSlots, d);
}

inline double delayDa(double d, double qDaSlots)
{
    return qDaSlots / d;
}

inline double alphaFairValue(double x, double alpha, double eps)
{
    const double xe = x + eps;
    if (std::abs(alpha - 1.0) < 1e-9)
        return std::log(xe);
    return std::pow(xe, 1.0 - alpha) / (1.0 - alpha);
}

inline double computeUtility(const F1LinearUtilityConfig& cfg, double r, double d,
                              double nHat, double ptx, double, double qDaSlots)
{
    if (r < 1.0 || d < 1.0)
        return -std::numeric_limits<double>::max();
    return cfg.wAcq * muRa(r, nHat, ptx)        - cfg.wColl * collRa(r, nHat, ptx)
         + cfg.wB  * throughputDa(d, qDaSlots)  - cfg.wD    * delayDa(d, qDaSlots);
}

inline double computeUtility(const F2LogarithmicUtilityConfig& cfg, double r, double d,
                              double nHat, double ptx, double, double qDaSlots)
{
    if (r < 1.0 || d < 1.0)
        return -std::numeric_limits<double>::max();
    return cfg.wAcq * std::log(1.0 + muRa(r, nHat, ptx))           - cfg.wColl * collRa(r, nHat, ptx)
         + cfg.wB  * std::log(1.0 + throughputDa(d, qDaSlots))     - cfg.wD    * delayDa(d, qDaSlots);
}

inline double computeUtility(const F3AlphaFairUtilityConfig& cfg, double r, double d,
                              double nHat, double ptx, double, double qDaSlots)
{
    if (r < 1.0 || d < 1.0)
        return -std::numeric_limits<double>::max();
    return cfg.wAcq * alphaFairValue(muRa(r, nHat, ptx),        cfg.alphaFair, cfg.epsilon)
         - cfg.wColl * collRa(r, nHat, ptx)
         + cfg.wB   * alphaFairValue(throughputDa(d, qDaSlots), cfg.alphaFair, cfg.epsilon)
         - cfg.wD   * delayDa(d, qDaSlots);
}

inline double computeUtility(const F4SigmoidalUtilityConfig& cfg, double r, double d,
                              double nHat, double ptx, double, double qDaSlots)
{
    if (r < 1.0 || d < 1.0)
        return -std::numeric_limits<double>::max();
    const auto sig = [&](double w, double x) {
        return w / (1.0 + std::exp(-cfg.k * (x - cfg.x0)));
    };
    return sig(cfg.wAcq, muRa(r, nHat, ptx))         - cfg.wColl * collRa(r, nHat, ptx)
         + sig(cfg.wB,  throughputDa(d, qDaSlots))   - cfg.wD    * delayDa(d, qDaSlots);
}

inline double computeUtility(const F5HardDeadlineUtilityConfig& cfg, double r, double d,
                              double nHat, double ptx, double, double qDaSlots)
{
    if (r < 1.0 || d < 1.0)
        return -std::numeric_limits<double>::max();
    const double excess = std::max(0.0, delayDa(d, qDaSlots) - cfg.Dtar);
    return cfg.wAcq * std::log(1.0 + muRa(r, nHat, ptx)) - cfg.wColl * collRa(r, nHat, ptx)
         - cfg.B   * excess * excess + cfg.wB * std::log(d);
}

inline double computeUtility(const F6CostOfDelayUtilityConfig& cfg, double r, double d,
                              double nHat, double ptx, double wb, double qDaSlots)
{
    if (r < 1.0 || d < 1.0)
        return -std::numeric_limits<double>::max();
    return -cfg.cJ * delayRa(r, nHat, ptx, wb, cfg.d0) * nHat
           -cfg.cD * delayDa(d, qDaSlots) * qDaSlots;
}

inline double computeUtility(const F7QuadraticBacklogUtilityConfig& cfg, double r, double d,
                              double nHat, double ptx, double, double qDaSlots)
{
    if (r < 1.0 || d < 1.0)
        return -std::numeric_limits<double>::max();
    return cfg.wAcq * std::log(1.0 + muRa(r, nHat, ptx)) - cfg.wColl * collRa(r, nHat, ptx)
         + cfg.wB  * std::log(1.0 + d)                   - cfg.wS   * qDaSlots * qDaSlots;
}

inline double computeUtility(const F8CesUtilityConfig& cfg, double r, double d,
                              double nHat, double ptx, double, double qDaSlots)
{
    if (r < 1.0 || d < 1.0)
        return -std::numeric_limits<double>::max();
    constexpr double eps = 1e-9;
    const double xRa = muRa(r, nHat, ptx);
    const double xDa = throughputDa(d, qDaSlots);
    if (std::abs(cfg.rho) < 1e-9)
        return cfg.wRa * std::log(xRa + eps) + cfg.wDa * std::log(xDa + eps);
    const double inner = cfg.wRa * std::pow(std::max(0.0, xRa), cfg.rho)
                       + cfg.wDa * std::pow(std::max(0.0, xDa), cfg.rho);
    if (inner <= 0.0)
        return -std::numeric_limits<double>::max();
    return std::pow(inner, 1.0 / cfg.rho);
}

} // namespace starTopologyEmulator::detail
