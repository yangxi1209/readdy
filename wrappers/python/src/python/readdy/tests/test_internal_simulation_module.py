import unittest

import numpy as np
from readdy._internal.simulation import Vec
from readdy._internal.simulation import Pot2
from readdy._internal.simulation import Simulation
from readdy._internal.simulation import KernelProvider

from readdy.util import platform_utils


class TestInternalSimulationModule(unittest.TestCase):
    def py_harmonic_repulsion_energy(self, x_i, x_j):
        dist = (x_i - x_j) * (x_i - x_j)
        # if dist < sqrt(25): return energy with force constant 1
        if dist < 25:
            return (np.sqrt(dist) - 5) ** 2
        else:
            return 0

    def py_harmonic_repulsion_force(self, x_i, x_j):
        r_ij = x_i - x_j
        dist = r_ij * r_ij
        if dist < 25:
            dist = np.sqrt(dist)
            return (2 * (dist - 5) / dist) * r_ij
        else:
            return Vec(0, 0, 0)

    @classmethod
    def setUpClass(cls):
        cls.kernel_provider = KernelProvider.get()
        cls.kernel_provider.load_from_dir(platform_utils.get_readdy_plugin_dir())
        cls.simulation = Simulation()

    def test_properties(self):
        if not self.simulation.isKernelSelected():
            self.simulation.setKernel('SingleCPU')
        np.testing.assert_equal(self.simulation.isKernelSelected(), True)
        np.testing.assert_equal(self.simulation.getSelectedKernelType(), "SingleCPU")
        self.simulation.kbt = 5.0
        np.testing.assert_equal(self.simulation.kbt, 5.0)
        self.simulation.periodic_boundary = [True, False, True]
        np.testing.assert_equal(self.simulation.periodic_boundary, (True, False, True))
        self.simulation.box_size = Vec(1, 3.6, 7)
        np.testing.assert_equal(self.simulation.box_size, Vec(1, 3.6, 7))

    def test_potentials(self):
        if not self.simulation.isKernelSelected():
            self.simulation.setKernel("SingleCPU")

        pot = Pot2("mypot", self.py_harmonic_repulsion_energy, self.py_harmonic_repulsion_force)
        self.simulation.registerParticleType("ParticleTypeA", 1.0)
        self.simulation.registerParticleType("ParticleTypeB", 3.0)
        self.simulation.registerParticleType("ParticleTypeA_internal", 1.0)
        self.simulation.registerParticleType("ParticleTypeB_internal", 3.0)
        self.simulation.registerPotentialOrder2(pot, "ParticleTypeA", "ParticleTypeB")
        self.simulation.registerPotentialOrder2("HarmonicRepulsion", "ParticleTypeA_internal", "ParticleTypeB_internal")
        self.simulation.run(100, 1)

if __name__ == '__main__':
    unittest.main()