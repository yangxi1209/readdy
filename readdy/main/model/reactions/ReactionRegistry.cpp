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
 * @file ReactionRegistry.cpp
 * @brief << brief description >>
 * @author clonker
 * @date 29.03.17
 * @copyright GNU Lesser General Public License v3.0
 */

#include <readdy/model/reactions/ReactionRegistry.h>
#include <readdy/common/Utils.h>

namespace readdy {
namespace model {
namespace reactions {

const ReactionRegistry::reactions_o1 ReactionRegistry::order1_flat() const {
    reaction_o1_registry::mapped_type result;
    for (const auto &mapEntry : one_educt_registry) {
        for (const auto reaction : mapEntry.second) {
            result.push_back(reaction);
        }
    }
    return result;
}

const ReactionRegistry::reaction_o1 ReactionRegistry::order1_by_name(const std::string &name) const {
    for (const auto &mapEntry : one_educt_registry) {
        for (const auto &reaction : mapEntry.second) {
            if (reaction->getName() == name) return reaction;
        }
    }

    return nullptr;
}

const ReactionRegistry::reactions_o2 ReactionRegistry::order2_flat() const {
    reaction_o2_registry::mapped_type result;
    for (const auto &mapEntry : two_educts_registry) {
        for (const auto reaction : mapEntry.second) {
            result.push_back(reaction);
        }
    }
    return result;
}

const ReactionRegistry::reactions_o1 &ReactionRegistry::order1_by_type(const Particle::type_type type) const {
    return util::collections::getOrDefault(one_educt_registry, type, defaultReactionsO1);
}

const ReactionRegistry::reaction_o2 ReactionRegistry::order2_by_name(const std::string &name) const {
    for (const auto &mapEntry : two_educts_registry) {
        for (const auto &reaction : mapEntry.second) {
            if (reaction->getName() == name) return reaction;
        }
    }
    return nullptr;
}

const ReactionRegistry::reactions_o2& ReactionRegistry::order2_by_type(const Particle::type_type type1,
                                                                       const Particle::type_type type2) const {
    auto it = two_educts_registry.find(std::tie(type1, type2));
    if(it != two_educts_registry.end()) {
        return it->second;
    }
    return defaultReactionsO2;
}

void ReactionRegistry::configure() {
    namespace coll = readdy::util::collections;
    using pair = util::particle_type_pair;
    using reaction1ptr = std::unique_ptr<reactions::Reaction<1>>;
    using reaction2ptr = std::unique_ptr<reactions::Reaction<2>>;

    one_educt_registry.clear();
    two_educts_registry.clear();
    _reaction_o2_types.clear();

    coll::for_each_value(one_educt_registry_internal,
                         [&](const particle_type_type type, const reaction1ptr &ptr) {
                             (one_educt_registry)[type].push_back(ptr.get());
                         });
    coll::for_each_value(two_educts_registry_internal, [&](const pair &type, const reaction2ptr &r) {
        (two_educts_registry)[type].push_back(r.get());
        _reaction_o2_types.emplace(std::get<0>(type));
        _reaction_o2_types.emplace(std::get<1>(type));
    });
    coll::for_each_value(one_educt_registry_external,
                         [&](const particle_type_type type, reactions::Reaction<1> *ptr) {
                             (one_educt_registry)[type].push_back(ptr);
                         });
    coll::for_each_value(two_educts_registry_external, [&](const pair &type, reactions::Reaction<2> *r) {
        (two_educts_registry)[type].push_back(r);
        _reaction_o2_types.emplace(std::get<0>(type));
        _reaction_o2_types.emplace(std::get<1>(type));
    });

}

void ReactionRegistry::debug_output() const {
    if (!one_educt_registry.empty()) {
        log::debug(" - reactions of order 1:");
        for(const auto& entry : one_educt_registry) {
            for(const auto reaction : entry.second) {
                log::debug("     * reaction {}", *reaction);
            }
        }
    }
    if (!two_educts_registry.empty()) {
        log::debug(" - reactions of order 2:");
        for(const auto& entry : two_educts_registry) {
            for(const auto reaction : entry.second) {
                log::debug("     * reaction {}", *reaction);
            }
        }
    }
}

const std::size_t &ReactionRegistry::n_order1() const {
    return n_order1_;
}

const std::size_t &ReactionRegistry::n_order2() const {
    return n_order2_;
}

const short ReactionRegistry::add_external(reactions::Reaction<1> *r) {
    one_educt_registry_external[r->getEducts()[0]].push_back(r);
    n_order1_ += 1;
    return r->getId();
}

ReactionRegistry::ReactionRegistry(std::reference_wrapper<const ParticleTypeRegistry> ref) : typeRegistry(ref) {}

const ReactionRegistry::reactions_o1 &ReactionRegistry::order1_by_type(const std::string &type) const {
    return order1_by_type(typeRegistry.id_of(type));
}

const ReactionRegistry::reactions_o2& ReactionRegistry::order2_by_type(const std::string &type1,
                                                                       const std::string &type2) const {
    return order2_by_type(typeRegistry.id_of(type1), typeRegistry.id_of(type2));
}

const ReactionRegistry::reaction_o2_registry &ReactionRegistry::order2() const {
    return two_educts_registry;
}

bool ReactionRegistry::is_reaction_order2_type(particle_type_type type) const {
    return _reaction_o2_types.find(type) != _reaction_o2_types.end();
}

const short ReactionRegistry::add_external(reactions::Reaction<2> *r) {
    two_educts_registry_external[std::tie(r->getEducts()[0], r->getEducts()[1])].push_back(r);
    n_order2_ += 1;
    return r->getId();
}

}
}
}