/**
 * << detailed description >>
 *
 * @file FilteredGillespieParallel.h
 * @brief << brief description >>
 * @author clonker
 * @date 20.10.16
 */

#ifndef READDY_CPUKERNEL_FILTEREDGILLESPIEPARALLEL_H
#define READDY_CPUKERNEL_FILTEREDGILLESPIEPARALLEL_H

#include "GillespieParallel.h"

namespace readdy {
namespace kernel {
namespace cpu {
namespace programs {
namespace reactions {

class FilteredGillespieParallel : GillespieParallel {
public:
    FilteredGillespieParallel(const kernel_t *const kernel);

};

}}}}
}
#endif //READDY_CPUKERNEL_FILTEREDGILLESPIEPARALLEL_H
