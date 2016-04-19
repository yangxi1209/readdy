//
// Created by clonker on 08.04.16.
//

#include "SingleCPUProgramFactory.h"
#include "programs/SingleCPUTestProgram.h"
#include "programs/SingleCPUAddParticleProgram.h"

std::shared_ptr<readdy::plugin::Program> readdy::kernel::singlecpu::SingleCPUProgramFactory::createProgram(const std::string name) {
    namespace prog = readdy::kernel::singlecpu::programs;
    if(name == prog::SingleCPUTestProgram::getName()) {
        return std::make_shared<prog::SingleCPUTestProgram>();
    }
    if(name == prog::SingleCPUAddParticleProgram::getName()) {
        return std::make_shared<prog::SingleCPUAddParticleProgram>(kernel);
    }
    return nullptr;
}

readdy::kernel::singlecpu::SingleCPUProgramFactory::SingleCPUProgramFactory(SingleCPUKernel &kernel) : kernel(kernel) {
}



