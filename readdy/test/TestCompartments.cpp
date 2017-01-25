/********************************************************************
 * Copyright © 2016 Computational Molecular Biology Group,          *
 *                  Freie Universität Berlin (GER)                  *
 *                                                                  *
 * This file is part of ReaDDy.                                     *
 *                                                                  *
 * ReaDDy is free software: you can redistribute it and/or modify   *
 * it under the terms of the GNU Lesser General Public License as   *
 * published by the Free Software Foundation, either version 3 of   *
 * the License, or (at your option) any later version.              *
 *                                                                  *
 * This program is distributed in the hope that it will be useful,  *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of   *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the    *
 * GNU Lesser General Public License for more details.              *
 *                                                                  *
 * You should have received a copy of the GNU Lesser General        *
 * Public License along with this program. If not, see              *
 * <http://www.gnu.org/licenses/>.                                  *
 ********************************************************************/


/**
 * @file TestCompartments.cpp
 * @brief Test implementation-independent execution of program 'Compartments'
 * @author chrisfroe
 * @date 18.10.16
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <readdy/testing/Utils.h>
#include <readdy/testing/KernelTest.h>

namespace m = readdy::model;

namespace {

class TestCompartments : public KernelTest {

};

TEST_P(TestCompartments, OneCompartmentOneConversionOneParticle) {
    auto &ctx = kernel->getKernelContext();
    ctx.setDiffusionConstant("A", 1.);
    ctx.setDiffusionConstant("B", 1.);
    auto &&comp = kernel->createAction<m::actions::Compartments>();
    auto fun = [](m::Vec3 position) {
        return true;
    };
    comp->registerCompartment(fun);
    comp->registerConversion(0, "A", "B");
    kernel->addParticle("A", m::Vec3(1, 0, 2));

    std::vector<std::string> typesToCount = {"A", "B"};
    auto &&obs = kernel->createObservable<m::observables::NParticles>(1, typesToCount);
    obs->evaluate();
    const auto &resultBefore = obs->getResult();
    EXPECT_THAT(resultBefore, ::testing::ElementsAre(1, 0)) << "Expect one A particle before program execution";

    comp->perform();

    obs->evaluate();
    const auto &resultAfter = obs->getResult();
    EXPECT_THAT(resultAfter, ::testing::ElementsAre(0, 1)) << "Expect zero A particle after program execution";
}

TEST_P(TestCompartments, TwoCompartments) {
    // two compartments, four species A,B,C and D, in the end there should only be C and D particles
    auto &ctx = kernel->getKernelContext();
    ctx.setBoxSize(10, 10, 10);
    ctx.setDiffusionConstant("A", 1.);
    ctx.setDiffusionConstant("B", 1.);
    ctx.setDiffusionConstant("C", 1.);
    ctx.setDiffusionConstant("D", 1.);
    auto &&comp = kernel->createAction<m::actions::Compartments>();
    auto funXPos = [](m::Vec3 position) {
        return position[0] >= 0;
    };
    auto funXNeg = [](m::Vec3 position) {
        return position[0] < 0;
    };
    comp->registerCompartment(funXPos);
    comp->registerCompartment(funXNeg);
    comp->registerConversion(0, "A", "C");
    comp->registerConversion(0, "B", "C");
    comp->registerConversion(1, "A", "D");
    comp->registerConversion(1, "B", "D");

    for (auto i = 0; i < 100; ++i) {
        kernel->addParticle("A", readdy::model::rnd::normal3());
        kernel->addParticle("B", readdy::model::rnd::normal3());
    }

    comp->perform();

    std::vector<std::string> typesToCount = {"A", "B", "C", "D"};
    auto &&obs = kernel->createObservable<m::observables::NParticles>(1, typesToCount);
    obs->evaluate();
    const auto &result = obs->getResult();
    EXPECT_EQ(result[0], 0) << "Expect no As";
    EXPECT_EQ(result[1], 0) << "Expect no Bs";

    const std::vector<std::string> typesC = {"C"};
    const std::vector<std::string> typesD = {"D"};
    auto &&posC = kernel->createObservable<m::observables::Positions>(1, typesC);
    auto &&posD = kernel->createObservable<m::observables::Positions>(1, typesD);
    posC->evaluate();
    posD->evaluate();
    const auto &resultC = posC->getResult();
    const auto &resultD = posD->getResult();
    for (auto pos : resultC) {
        EXPECT_GE(pos[0], 0) << "x position of Cs must be > 0";
    }
    for (auto pos : resultD) {
        EXPECT_LT(pos[0], 0) << "x position of Ds must be < 0";
    }
}

INSTANTIATE_TEST_CASE_P(TestCompartments, TestCompartments, ::testing::ValuesIn(readdy::testing::getKernelsToTest()));

}