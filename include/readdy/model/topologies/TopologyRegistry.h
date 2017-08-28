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
 * << detailed description >>
 *
 * @file TopologyTypeRegistry.h
 * @brief << brief description >>
 * @author clonker
 * @date 24.08.17
 * @copyright GNU Lesser General Public License v3.0
 */

#pragma once

#include <readdy/common/common.h>
#include <readdy/model/topologies/reactions/StructuralTopologyReaction.h>
#include <unordered_set>
#include <unordered_map>
#include <readdy/model/topologies/reactions/SpatialTopologyReaction.h>
#include <readdy/common/ParticleTypeTuple.h>
#include <readdy/model/ParticleTypeRegistry.h>
#include <readdy/api/PotentialConfiguration.h>


NAMESPACE_BEGIN(readdy)
NAMESPACE_BEGIN(model)
NAMESPACE_BEGIN(top)

struct TopologyTypeInfo {
    using structural_reaction = reactions::StructuralTopologyReaction;
    using structural_reaction_vector = std::vector<structural_reaction>;

    std::string name {""};
    topology_type_type type {0};
    structural_reaction_vector structural_reactions {};
};

class TopologyRegistry {
public:

    using spatial_reaction = reactions::SpatialTopologyReaction;
    using spatial_reaction_map = util::particle_type_pair_unordered_map<std::vector<spatial_reaction>>;
    using spatial_reactions = spatial_reaction_map::mapped_type;
    using spatial_reaction_types = std::unordered_set<particle_type_type>;

    using structural_reaction = TopologyTypeInfo::structural_reaction;
    using structural_reactions = TopologyTypeInfo::structural_reaction_vector;

    using type_registry = std::unordered_map<topology_type_type, TopologyTypeInfo>;

    explicit TopologyRegistry(const ParticleTypeRegistry &typeRegistry);
    TopologyRegistry(const TopologyRegistry&) = delete;
    TopologyRegistry& operator=(const TopologyRegistry&) = delete;
    TopologyRegistry(TopologyRegistry&&) = delete;
    TopologyRegistry& operator=(TopologyRegistry&&) = delete;
    ~TopologyRegistry() = default;

    topology_type_type add_type(const std::string& name, const structural_reactions& reactions = {});

    void add_structural_reaction(topology_type_type type, const reactions::StructuralTopologyReaction &reaction);

    void add_structural_reaction(topology_type_type type, reactions::StructuralTopologyReaction &&reaction);

    void add_structural_reaction(const std::string &type, const reactions::StructuralTopologyReaction &reaction);

    void add_structural_reaction(const std::string &type, reactions::StructuralTopologyReaction &&reaction);

    const TopologyTypeInfo &info_of(topology_type_type type) const;

    const std::string &name_of(topology_type_type type) const;

    topology_type_type id_of(const std::string& name) const;

    bool empty();

    bool contains_structural_reactions();

    const TopologyTypeInfo::structural_reaction_vector &reactions_of(topology_type_type type) const;

    const structural_reactions &reactions_of(const std::string &type) const;

    void configure();

    void debug_output() const;

    void add_spatial_reaction(const std::string &name, const std::string &typeFrom1,
                              const std::string &typeFrom2, const std::string &typeTo1,
                              const std::string &typeTo2, scalar rate, scalar radius, reactions::STRMode mode);

    void add_spatial_reaction(const std::string &name, const util::particle_type_pair &types,
                              const util::particle_type_pair &types_to, scalar rate, scalar radius,
                              reactions::STRMode mode);

    void validate_spatial_reaction(const spatial_reaction &reaction) const;

    const spatial_reaction_map &spatial_reaction_registry() const;

    const spatial_reactions &spatial_reactions_by_type(particle_type_type t1, particle_type_type t2) const;

    bool is_spatial_reaction_type(const std::string &name) const;

    bool is_spatial_reaction_type(particle_type_type type) const;


    /*
     * Potentials
     */

    void configure_bond_potential(const std::string &type1, const std::string &type2, const api::Bond &bond);

    void configure_angle_potential(const std::string &type1, const std::string &type2, const std::string &type3,
                                   const api::Angle &angle);

    void configure_torsion_potential(const std::string &type1, const std::string &type2, const std::string &type3,
                                     const std::string &type4, const api::TorsionAngle &torsionAngle);

    api::PotentialConfiguration &potential_configuration();

    const api::PotentialConfiguration &potential_configuration() const;

private:
    static unsigned short counter;

    TopologyTypeInfo defaultInfo;
    bool _contains_structural_reactions {false};
    type_registry _registry {};

    spatial_reaction_map _spatial_reactions {};
    spatial_reactions defaultTopologyReactions{};
    spatial_reaction_types _topology_reaction_types{};

    std::reference_wrapper<const ParticleTypeRegistry> _type_registry;

    api::PotentialConfiguration potentialConfiguration_{};
};

NAMESPACE_END(top)
NAMESPACE_END(model)
NAMESPACE_END(readdy)