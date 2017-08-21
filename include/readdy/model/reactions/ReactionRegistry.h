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
 * @file ReactionRegistry.h
 * @brief << brief description >>
 * @author clonker
 * @date 29.03.17
 * @copyright GNU Lesser General Public License v3.0
 */

#pragma once

#include <readdy/common/common.h>
#include <readdy/common/ParticleTypeTuple.h>
#include <readdy/model/ParticleTypeRegistry.h>
#include <readdy/model/topologies/reactions/ExternalTopologyReaction.h>
#include <unordered_set>
#include "Reaction.h"

NAMESPACE_BEGIN(readdy)
NAMESPACE_BEGIN(model)
NAMESPACE_BEGIN(reactions)

class ReactionRegistry {
    using particle = readdy::model::Particle;
    using rea_ptr_vec1 = std::vector<std::unique_ptr<reactions::Reaction<1>>>;
    using rea_ptr_vec2 = std::vector<std::unique_ptr<reactions::Reaction<2>>>;
    using reaction_o1_registry_internal = std::unordered_map<particle::type_type, rea_ptr_vec1>;
    using reaction_o2_registry_internal = util::particle_type_pair_unordered_map<rea_ptr_vec2>;

public:
    using topology_reaction = top::reactions::ExternalTopologyReaction;
    using topology_reaction_registry = util::particle_type_pair_unordered_map<std::vector<topology_reaction>>;
    using topology_reactions = topology_reaction_registry::mapped_type;
    using topology_reaction_types = std::unordered_set<particle_type_type>;

    using reaction_o1_registry = std::unordered_map<particle::type_type, std::vector<reactions::Reaction<1> *>>;
    using reaction_o2_registry = util::particle_type_pair_unordered_map<std::vector<reactions::Reaction<2> *>>;
    using reaction_o2_types = std::unordered_set<particle_type_type>;

    using reactions_o1 = reaction_o1_registry::mapped_type;
    using reactions_o2 = reaction_o2_registry::mapped_type;
    using reaction_o1 = reactions_o1::value_type;
    using reaction_o2 = reactions_o2::value_type;

    explicit ReactionRegistry(std::reference_wrapper<const ParticleTypeRegistry> ref);

    ReactionRegistry(const ReactionRegistry &) = delete;

    ReactionRegistry &operator=(const ReactionRegistry &) = delete;

    ReactionRegistry(ReactionRegistry &&) = delete;

    ReactionRegistry &operator=(ReactionRegistry &&) = delete;

    ~ReactionRegistry() = default;

    const std::size_t &n_order1() const;

    const reactions_o1 order1_flat() const;

    const reaction_o1 order1_by_name(const std::string &name) const;

    const reactions_o1 &order1_by_type(const particle::type_type type) const;

    const std::size_t &n_order2() const;

    const reaction_o2_registry &order2() const;

    const reactions_o2 order2_flat() const;

    const reaction_o2 order2_by_name(const std::string &name) const;

    const reactions_o2 &order2_by_type(const particle::type_type type1, const particle::type_type type2) const;

    const reactions_o1 &order1_by_type(const std::string &type) const;

    const reactions_o2 &order2_by_type(const std::string &type1, const std::string &type2) const;

    bool is_reaction_order2_type(particle_type_type type) const;

    template<typename R>
    const short add(std::unique_ptr<R> r,
                    typename std::enable_if<std::is_base_of<reactions::Reaction<1>, R>::value>::type * = 0) {
        log::trace("registering reaction {}", *r);
        const auto id = r->getId();
        const auto type = r->getEducts()[0];
        if (one_educt_registry_internal.find(type) == one_educt_registry_internal.end()) {
            one_educt_registry_internal.emplace(type, rea_ptr_vec1());
        }
        one_educt_registry_internal[type].push_back(std::move(r));
        n_order1_ += 1;
        return id;
    }

    template<typename R>
    const short add(std::unique_ptr<R> r,
                    typename std::enable_if<std::is_base_of<reactions::Reaction<2>, R>::value>::type * = 0) {
        log::trace("registering reaction {}", *r);
        const auto id = r->getId();
        const auto t1 = r->getEducts()[0];
        const auto t2 = r->getEducts()[1];

        const auto pp = std::make_tuple(t1, t2);
        if (two_educts_registry_internal.find(pp) == two_educts_registry_internal.end()) {
            two_educts_registry_internal.emplace(pp, rea_ptr_vec2());
        }
        two_educts_registry_internal[pp].push_back(std::move(r));
        n_order2_ += 1;
        return id;
    }

    const short add_external(reaction_o1 r);

    const short add_external(reaction_o2 r);

    bool is_topology_reaction_type(particle_type_type type) const;

    void add_external_topology_reaction(const std::string &name, const std::string& typeFrom1,
                                        const std::string& typeFrom2, const std::string& typeTo1,
                                        const std::string& typeTo2, scalar rate, scalar radius);

    void add_external_topology_reaction(const std::string &name, const util::particle_type_pair &types,
                                        const util::particle_type_pair &types_to, scalar rate, scalar radius);

    const topology_reaction_registry &external_topology_reactions() const;

    const topology_reactions &external_top_reactions_by_type(particle_type_type t1, particle_type_type t2) const;

    void configure();

    void debug_output() const;

private:
    using reaction_o1_registry_external = reaction_o1_registry;
    using reaction_o2_registry_external = reaction_o2_registry;

    std::size_t n_order1_ {0};
    std::size_t n_order2_ {0};

    const ParticleTypeRegistry &typeRegistry;

    reaction_o1_registry one_educt_registry{};
    reaction_o1_registry_internal one_educt_registry_internal{};
    reaction_o1_registry_external one_educt_registry_external{};
    reaction_o2_registry two_educts_registry{};
    reaction_o2_registry_internal two_educts_registry_internal{};
    reaction_o2_registry_external two_educts_registry_external{};
    reaction_o2_types _reaction_o2_types {};

    topology_reaction_registry _topology_reactions {};
    topology_reactions defaultTopologyReactions{};
    topology_reaction_types _topology_reaction_types{};

    reactions_o1 defaultReactionsO1{};
    reactions_o2 defaultReactionsO2{};
};

NAMESPACE_END(reactions)
NAMESPACE_END(model)
NAMESPACE_END(readdy)