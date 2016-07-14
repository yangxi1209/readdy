/**
 * << detailed description >>
 *
 * @file CalculateForces.h
 * @brief << brief description >>
 * @author clonker
 * @date 14.07.16
 */


#ifndef READDY_MAIN_CALCULATEFORCES_H
#define READDY_MAIN_CALCULATEFORCES_H

#include <readdy/model/programs/Programs.h>
#include <readdy/kernel/cpu/CPUKernel.h>

namespace readdy {
    namespace kernel {
        namespace cpu {
            namespace programs {
                class CalculateForces : public readdy::model::programs::CalculateForces {

                public:

                    CalculateForces(CPUKernel* kernel) : kernel(kernel) {}

                    virtual void execute() override {
                        kernel->getKernelStateModel().calculateForces();
                    }

                protected:
                    CPUKernel* kernel;
                };
            }
        }
    }
}
#endif //READDY_MAIN_CALCULATEFORCES_H
