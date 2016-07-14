/**
 * << detailed description >>
 *
 * @file ParticleData.cpp
 * @brief << brief description >>
 * @author clonker
 * @date 14.07.16
 */


#include <readdy/kernel/cpu/model/ParticleData.h>
#include <readdy/common/make_unique.h>
#include <numeric>
#include <algorithm>
#include <boost/log/trivial.hpp>

namespace readdy {
    namespace kernel {
        namespace cpu {
            namespace model {
                ParticleData::ParticleData() : ParticleData(0) {
                }

                ParticleData::ParticleData(unsigned int capacity) {
                    ids = std::make_unique<std::vector<boost::uuids::uuid>>(capacity);
                    positions = std::make_unique<std::vector<readdy::model::Vec3>>(capacity);
                    forces = std::make_unique<std::vector<readdy::model::Vec3>>(capacity);
                    type = std::make_unique<std::vector<unsigned int>>(capacity);
                    markedForDeactivation = std::make_unique<std::set<size_t>>();
                    deactivated = std::make_unique<std::vector<bool>>(capacity);
                    std::fill(deactivated->begin(), deactivated->end(), true);
                    n_deactivated = capacity;
                    deactivated_index = 0;
                }

                void ParticleData::swap(ParticleData &rhs) {
                    std::swap(ids, rhs.ids);
                    std::swap(positions, rhs.positions);
                    std::swap(forces, rhs.forces);
                    std::swap(type, rhs.type);
                    std::swap(deactivated, rhs.deactivated);
                    std::swap(deactivated_index, rhs.deactivated_index);
                    std::swap(n_deactivated, rhs.n_deactivated);
                    std::swap(markedForDeactivation, rhs.markedForDeactivation);
                }

                size_t ParticleData::size() const {
                    auto s = markedForDeactivation->size();
                    return s <= deactivated_index ? deactivated_index - s : 0;
                }

                size_t ParticleData::max_size() const {
                    return ids->max_size();
                }

                bool ParticleData::empty() const {
                    return size() == 0;
                }

                void ParticleData::addParticle(const readdy::model::Particle &particle) {
                    addParticles({particle});
                };

                void ParticleData::addParticles(const std::vector<readdy::model::Particle> &particles) {
                    auto added = particles.cbegin();
                    auto ids_it = ids->begin() + deactivated_index;
                    auto positions_it = positions->begin() + deactivated_index;
                    auto forces_it = forces->begin() + deactivated_index;
                    auto type_it = type->begin() + deactivated_index;
                    auto deactivated_it = deactivated->begin() + deactivated_index;
                    while (added != particles.cend()) {
                        if (n_deactivated > 0) {

                            *ids_it = added->getId();
                            *positions_it = added->getPos();
                            *forces_it = {0, 0, 0};
                            *type_it = added->getType();
                            *deactivated_it = false;

                            --n_deactivated;
                            ++deactivated_index;

                            ++ids_it; ++positions_it; ++forces_it; ++type_it; ++deactivated_it;
                        } else {
                            ids->push_back(added->getId());
                            positions->push_back(added->getPos());
                            forces->push_back({0, 0, 0});
                            type->push_back(added->getType());
                            deactivated->push_back(false);
                            ++deactivated_index;
                        }
                        ++added;
                    }
                }

                void ParticleData::removeParticle(const size_t index) {
                    (*deactivated)[index] = true;

                    std::swap((*ids)[index], (*ids)[deactivated_index-1]);
                    std::swap((*positions)[index], (*positions)[deactivated_index-1]);
                    std::swap((*forces)[index], (*forces)[deactivated_index-1]);
                    std::swap((*type)[index], (*type)[deactivated_index-1]);
                    std::swap((*deactivated)[index], (*deactivated)[deactivated_index-1]);

                    ++n_deactivated;
                    if(deactivated_index == 0) throw std::runtime_error("hier sollte man aber nicht hinkommen!1");
                    --deactivated_index;
                }

                void ParticleData::markForDeactivation(size_t index) {
                    (*deactivated)[index] = true;
                    markedForDeactivation->emplace(index);
                }

                void ParticleData::deactivateMarked() {
                    // if we havent marked anything, return
                    if(markedForDeactivation->size() == 0) return;
                    // sanity check: the deactivated_index is pointing to the
                    // first (real) deactivated particle, i.e., marks the end of the
                    // active data structure. "deactivated" is a vector<bool>
                    // that is as long as the data, thus the deactivated_index
                    // can be at most deactivated->begin() - deactivated->end().
                    if(deactivated->size() < deactivated_index-1) {
                        throw std::runtime_error("this should not happen");
                    }
                    // if we have active particles
                    if (deactivated_index > 0) {
                        // we now are going backwards through the active part of the data structure,
                        // starting with the first _active_ (but possible marked) particle
                        auto deactivatedIt = deactivated->begin() + deactivated_index - 1;
                        // for each index in the markedForDeactivation data structure
                        // (which is a set and thus sorted)
                        for (auto &&idx : *markedForDeactivation) {
                            // if there are marked particles at the very end,
                            // just shift the deactivated_index and increase n_deactivated
                            while (*deactivatedIt && deactivatedIt != deactivated->begin()) {
                                --deactivated_index;
                                ++n_deactivated;
                                --deactivatedIt;
                            }
                            // since the deactivated_index might have decreased
                            // so that we already have deactivated "idx", we check
                            // if it has been deactivated already (by the above loop)
                            if (idx < deactivated_index) {
                                // performs swapping of this particle with the last active
                                // particle
                                removeParticle(idx);
                                // if we are not at the begin already,
                                // we want to decrease the current particle considered in
                                // deactivatedIt
                                if(deactivatedIt != deactivated->begin()) --deactivatedIt;
                            } else {
                                // since the set is sorted and we start with the smallest idx,
                                // we can stop here
                                break;
                            }
                        }
                    }
                    markedForDeactivation->clear();
                }

                void ParticleData::removeParticle(const readdy::model::Particle &particle) {
                    auto &&beginIt = begin_ids();
                    auto &&endIt = end_ids();
                    auto &&it = std::find(beginIt, endIt, particle.getId());
                    if (it != endIt) {
                        removeParticle(it-beginIt);
                    } else {
                        BOOST_LOG_TRIVIAL(warning) << "Could not find and thus remove particle";
                    }
                }

                std::vector<boost::uuids::uuid>::iterator ParticleData::begin_ids() {
                    return ids->begin();
                }

                std::vector<boost::uuids::uuid>::iterator ParticleData::end_ids() {
                    return ids->begin() + deactivated_index;
                }

                std::vector<readdy::model::Vec3>::iterator ParticleData::begin_positions() {
                    return positions->begin();
                }

                std::vector<readdy::model::Vec3>::iterator ParticleData::end_positions() {
                    return positions->begin()+deactivated_index;
                }

                std::vector<readdy::model::Vec3>::iterator ParticleData::begin_forces() {
                    return forces->begin();
                }

                std::vector<readdy::model::Vec3>::iterator ParticleData::end_forces() {
                    return forces->begin()+deactivated_index;
                }

                std::vector<unsigned int>::iterator ParticleData::begin_types() {
                    return type->begin();
                }

                std::vector<unsigned int>::iterator ParticleData::end_types() {
                    return type->begin()+deactivated_index;
                }

                readdy::model::Particle ParticleData::operator[](const size_t index) const{
                    return readdy::model::Particle(*(begin_positions() + index), *(begin_types() + index), *(begin_ids() + index));
                }


                ParticleData &ParticleData::operator=(ParticleData &&rhs) = default;

                ParticleData::ParticleData(ParticleData &&rhs) = default;

                ParticleData::~ParticleData() {
                }

                std::vector <boost::uuids::uuid>::const_iterator ParticleData::begin_ids() const {
                    return cbegin_ids();
                }

                std::vector<boost::uuids::uuid>::const_iterator ParticleData::cbegin_ids() const {
                    return ids->cbegin();
                }

                std::vector<boost::uuids::uuid>::const_iterator ParticleData::end_ids() const {
                    return cend_ids();
                }

                std::vector<boost::uuids::uuid>::const_iterator ParticleData::cend_ids() const {
                    return ids->begin()+deactivated_index;
                }

                std::vector<readdy::model::Vec3>::const_iterator ParticleData::begin_positions() const {
                    return cbegin_positions();
                }

                std::vector<readdy::model::Vec3>::const_iterator ParticleData::cbegin_positions() const {
                    return positions->cbegin();
                }

                std::vector<readdy::model::Vec3>::const_iterator ParticleData::end_positions() const {
                    return cend_positions();
                }

                std::vector<readdy::model::Vec3>::const_iterator ParticleData::cend_positions() const {
                    return positions->cbegin()+deactivated_index;
                }

                std::vector<readdy::model::Vec3>::const_iterator ParticleData::begin_forces() const {
                    return cbegin_forces();
                }

                std::vector<readdy::model::Vec3>::const_iterator ParticleData::cbegin_forces() const {
                    return forces->cbegin();
                }

                std::vector<readdy::model::Vec3>::const_iterator ParticleData::end_forces() const {
                    return cend_forces();
                }

                std::vector<readdy::model::Vec3>::const_iterator ParticleData::cend_forces() const {
                    return forces->cbegin()+deactivated_index;
                }

                std::vector<unsigned int>::const_iterator ParticleData::begin_types() const {
                    return cbegin_types();
                }

                std::vector<unsigned int>::const_iterator ParticleData::cbegin_types() const {
                    return type->cbegin();
                }

                std::vector<unsigned int>::const_iterator ParticleData::end_types() const {
                    return cend_types();
                }

                std::vector<unsigned int>::const_iterator ParticleData::cend_types() const {
                    return type->cbegin()+deactivated_index;
                }

                bool ParticleData::isMarkedForDeactivation(const size_t index) {
                    return (*deactivated)[index];
                }

                size_t ParticleData::getDeactivatedIndex() const {
                    return deactivated_index;
                }

                size_t ParticleData::getNDeactivated() const {
                    return n_deactivated;
                }

                void ParticleData::setParticleData(const readdy::model::Particle& particle, const size_t& index) {
                    (*ids)[index] = particle.getId();
                    (*positions)[index] = particle.getPos();
                    (*type)[index] = particle.getType();
                }

                void ParticleData::clear() {
                    deactivated_index = 0;
                    n_deactivated = ids->size();
                }


            }
        }
    }
}
