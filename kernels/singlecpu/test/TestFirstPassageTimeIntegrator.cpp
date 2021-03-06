/********************************************************************
 * Copyright © 2019 Computational Molecular Biology Group,          *
 *                  Freie Universität Berlin (GER)                  *
 *                                                                  *
 * Redistribution and use in source and binary forms, with or       *
 * without modification, are permitted provided that the            *
 * following conditions are met:                                    *
 *  1. Redistributions of source code must retain the above         *
 *     copyright notice, this list of conditions and the            *
 *     following disclaimer.                                        *
 *  2. Redistributions in binary form must reproduce the above      *
 *     copyright notice, this list of conditions and the following  *
 *     disclaimer in the documentation and/or other materials       *
 *     provided with the distribution.                              *
 *  3. Neither the name of the copyright holder nor the names of    *
 *     its contributors may be used to endorse or promote products  *
 *     derived from this software without specific                  *
 *     prior written permission.                                    *
 *                                                                  *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND           *
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,      *
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF         *
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE         *
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR            *
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,     *
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,         *
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; *
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER *
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,      *
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)    *
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF      *
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                       *
 ********************************************************************/


/**
 * @file TestFirstPassageTimeIntegrator.cpp
 * @brief Tests for MDGFRD stuff
 * @author luigisbailo
 * @author chrisfroe
 * @date 15.02.19
 */

#include <catch2/catch.hpp>

#include <readdy/model/Kernel.h>
#include <readdy/plugin/KernelProvider.h>
#include <readdy/model/actions/Actions.h>
#include <readdy/kernel/singlecpu/actions/SCPUMdgfrdIntegrator.h>

TEST_CASE("Test single cpu MDGFRD", "[scpu]") {
    auto kernel = readdy::plugin::KernelProvider::getInstance().create("SingleCPU");
    kernel->context().boxSize() = {{10, 10, 10}};
    kernel->context().particleTypes().add("X", .25);
    readdy::scalar timestep = 0.1;
    auto&& integrator = kernel->actions().mdgfrdIntegrator(timestep);
}
