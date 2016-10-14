/**
 * The random provider can provide normal and uniform distributed random numbers. The choice of random generator is
 * hidden within the implementation and can currently not be altered.
 *
 * @file Random.h
 * @brief Header file containing the definitions for readdy::model::RandomProvider.
 * @author clonker
 * @date 19.04.16
 */

#ifndef READDY_MAIN_RANDOMPROVIDER_H
#define READDY_MAIN_RANDOMPROVIDER_H

#include <memory>
#include <random>
#include "Vec3.h"

namespace readdy {
namespace model {
namespace rnd {

template<typename Generator = std::mt19937>
double normal(const double mean = 0.0, const double variance = 1.0) {
    static thread_local Generator generator;
    std::normal_distribution<double> distribution(mean, variance);
    return distribution(generator);
}

template<typename Generator = std::mt19937>
double uniform(const double a = 0.0, const double b = 1.0) {
    static thread_local std::mt19937 generator;
    std::normal_distribution<double> distribution(a,b);
    return distribution(generator);
}

template<typename Generator = std::mt19937>
Vec3 normal3(const double mean = 0.0, const double variance = 1.0) {
    return {normal<Generator>(mean, variance), normal<Generator>(mean, variance), normal<Generator>(mean, variance)};
}


}
}
}
#endif //READDY_MAIN_RANDOMPROVIDER_H
