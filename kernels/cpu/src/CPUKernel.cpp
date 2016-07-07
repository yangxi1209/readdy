/**
 * << detailed description >>
 *
 * @file CPUKernel.cpp
 * @brief << brief description >>
 * @author clonker
 * @date 23.06.16
 */

#include <readdy/kernel/cpu/CPUKernel.h>
#include <readdy/kernel/cpu/programs/CPUProgramFactory.h>
#include <thread>

namespace readdy {
    namespace kernel {
        namespace cpu {
            const std::string CPUKernel::name = "CPU";

            struct CPUKernel::Impl {
                std::unique_ptr<programs::CPUProgramFactory> programFactory;
            };

            std::unique_ptr<CPUKernel> CPUKernel::create() {
                return std::make_unique<CPUKernel>();
            }

            readdy::model::programs::ProgramFactory &CPUKernel::getProgramFactory() const {
                return *pimpl->programFactory;
            }

            CPUKernel::CPUKernel() : pimpl(std::make_unique<Impl>()){
                pimpl->programFactory = std::make_unique<programs::CPUProgramFactory>(this);
            }

            unsigned int CPUKernel::getNCores() {
                return std::thread::hardware_concurrency();
            }


        }
    }
}