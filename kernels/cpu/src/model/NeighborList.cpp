/**
 * << detailed description >>
 *
 * @file NeighborList.cpp
 * @brief << brief description >>
 * @author clonker
 * @date 08.09.16
 */

#include <readdy/kernel/cpu/model/NeighborList.h>
#include <sstream>

namespace readdy {
namespace kernel {
namespace cpu {
namespace model {

NeighborList::NeighborList(const readdy::model::KernelContext *const context, util::Config const *const config)
        : ctx(context), config(config), cells(std::vector<Cell>()), simBoxSize(ctx->getBoxSize()), maps(config->nThreads) {}

void NeighborList::setupCells() {
    if (cells.empty()) {
        double maxCutoff = 0;
        for (auto &&e : ctx->getAllOrder2RegisteredPotentialTypes()) {
            for (auto &&p : ctx->getOrder2Potentials(std::get<0>(e), std::get<1>(e))) {
                maxCutoff = maxCutoff < p->getCutoffRadius() ? p->getCutoffRadius() : maxCutoff;
            }
        }
        for (auto &&e : ctx->getAllOrder2Reactions()) {
            maxCutoff = maxCutoff < e->getEductDistance() ? e->getEductDistance() : maxCutoff;
        }
        NeighborList::maxCutoff = maxCutoff;
        if (maxCutoff > 0) {
            const auto desiredCellWidth = .5 * maxCutoff;

            for (unsigned short i = 0; i < 3; ++i) {
                nCells[i] = static_cast<cell_index>(floor(simBoxSize[i] / desiredCellWidth));
                if (nCells[i] == 0) nCells[i] = 1;
                cellSize[i] = simBoxSize[i] / static_cast<double>(nCells[i]);
            }
            for (cell_index i = 0; i < nCells[0]; ++i) {
                for (cell_index j = 0; j < nCells[1]; ++j) {
                    for (cell_index k = 0; k < nCells[2]; ++k) {
                        cells.push_back({i, j, k, nCells});
                    }
                }
            }
            for (cell_index i = 0; i < nCells[0]; ++i) {
                for (cell_index j = 0; j < nCells[1]; ++j) {
                    for (cell_index k = 0; k < nCells[2]; ++k) {
                        setupNeighboringCells(i, j, k);
                    }
                }
            }
        } else {
            nCells = {{1, 1, 1}};
            cells.push_back({0, 0, 0, nCells});
            setupNeighboringCells(0, 0, 0);
        }
    }
}

void
NeighborList::setupNeighboringCells(const signed_cell_index i, const signed_cell_index j, const signed_cell_index k) {
    auto me = getCell(i, j, k);
    for (signed_cell_index _i = -2; _i < 3; ++_i) {
        for (signed_cell_index _j = -2; _j < 3; ++_j) {
            for (signed_cell_index _k = -2; _k < 3; ++_k) {
                // don't add me as neighbor to myself
                if (!(_i == 0 && _j == 0 && _k == 0)) {
                    me->addNeighbor(getCell(i + _i, j + _j, k + _k));
                }
            }
        }
    }
}

void NeighborList::clear() {
    cells.clear();
    maps.clear();
}

void NeighborList::fillCells(data_t &data) {
    if (maxCutoff > 0) {

        for (auto &cell : cells) {
            cell.particleIndices.clear();
        }
        std::for_each(maps.begin(), maps.end(), [](container_t &map) { map.clear(); });
        maps.resize(config->nThreads);

        auto it = data.entries.begin();
        while (it != data.entries.end()) {
            if (!it->is_deactivated()) {
                auto cell = getCell(it->pos);
                if (cell) {
                    auto ptr = &(*it);
                    getPairs(cell).emplace(ptr, std::vector<neighbor_t>());
                    cell->particleIndices.push_back(ptr);
                }
            }
            ++it;
        }
        // todo: in order to avoid concurrent modification, each cell has its own neighbor list map
        {
            using cell_it_t = decltype(cells.begin());
            const auto size = cells.size();
            const std::size_t grainSize = size / config->nThreads;
            const auto cutoffSquared = maxCutoff * maxCutoff;
            auto worker = [&data, cutoffSquared, this](const cell_it_t begin, const cell_it_t end, container_t &map) {
                const auto &d2 = ctx->getDistSquaredFun();
                for (auto _b = begin; _b != end; ++_b) {
                    auto &cell = *_b;
                    for (const auto &pI : cell.particleIndices) {
                        try {
                            auto &pI_vec = map.at(pI);
                            for (const auto &pJ : cell.particleIndices) {
                                if (pI != pJ) {
                                    const auto distSquared = d2(pI->pos, pJ->pos);
                                    if (distSquared < cutoffSquared) {
                                        Neighbor neighbor{pJ, distSquared};
                                        pI_vec.push_back(std::move(neighbor));
                                    }
                                }
                            }
                            for (auto &neighboringCell : cell.neighbors) {
                                for (const auto pJ : neighboringCell->particleIndices) {
                                    const auto distSquared = d2(pI->pos, pJ->pos);
                                    if (distSquared < cutoffSquared) {
                                        Neighbor neighbor{pJ, distSquared};
                                        pI_vec.push_back(std::move(neighbor));
                                    }
                                }
                            }
                        } catch(const std::out_of_range& e) {
                            log::console()->error("got out of range! {}", e.what());
                        }
                    }
                }
            };

            std::vector<util::scoped_thread> threads;
            threads.reserve(config->nThreads);

            auto it_cells = cells.begin();
            auto it_maps = maps.begin();
            while(it_maps != maps.end()-1) {
                auto i = it_maps - maps.begin();
                threads.push_back(util::scoped_thread(std::thread(worker, it_cells, it_cells + grainSize, std::ref(*it_maps))));
                it_cells += grainSize;
                ++it_maps;
            }
            threads.push_back(util::scoped_thread(std::thread(worker, it_cells, cells.end(), std::ref(*it_maps))));
        }
    }
}

void NeighborList::create(data_t &data) {
    simBoxSize = ctx->getBoxSize();
    setupCells();
    fillCells(data);
}

NeighborList::Cell *NeighborList::getCell(signed_cell_index i, signed_cell_index j, signed_cell_index k) {
    const auto &periodic = ctx->getPeriodicBoundary();
    if (periodic[0]) i = readdy::util::numeric::positive_modulo(i, nCells[0]);
    else if (i < 0 || i >= nCells[0]) return nullptr;
    if (periodic[1]) j = readdy::util::numeric::positive_modulo(j, nCells[1]);
    else if (j < 0 || j >= nCells[1]) return nullptr;
    if (periodic[2]) k = readdy::util::numeric::positive_modulo(k, nCells[2]);
    else if (k < 0 || k >= nCells[2]) return nullptr;
    return &cells[k + j * nCells[2] + i * nCells[2] * nCells[1]];
}

// todo the particledata update needs to be propagated (or funneled) through here

void NeighborList::remove(const particle_index idx) {
    auto cell = getCell(idx->pos);
    if (cell != nullptr) {
        std::set<container_t *> affectedMaps;
        for (auto neighborCell : getCell(idx->pos)->neighbors) {
            affectedMaps.insert(&getPairs(neighborCell));
        }
        try {
            for (auto &neighbor : neighbors(idx)) {
                auto &&neighborCell = getCell(neighbor.idx->pos);
                for (auto map : affectedMaps) {
                    const auto it = map->find(neighbor.idx);
                    if (it != map->end()) {
                        std::remove_if(it->second.begin(), it->second.end(),
                                       [idx](const neighbor_t &n) {
                                           return n.idx == idx;
                                       });
                    }

                }
            }
            getPairs(cell).erase(idx);
        } catch(const std::out_of_range&) {
            log::console()->error("tried to remove particle with index {} but it was not in the neighbor list");
        }
    }
}

void NeighborList::insert(const data_t &data, const particle_index idx) {
    const auto d2 = ctx->getDistSquaredFun();
    const auto pos = idx->pos;
    const auto cutoffSquared = maxCutoff * maxCutoff;
    auto cell = getCell(pos);
    if (cell) {
        auto &map = getPairs(cell);
        cell->particleIndices.push_back(idx);
        map.emplace(idx, std::vector<neighbor_t>());

        for (const auto pJ : cell->particleIndices) {
            if (idx != pJ) {
                const auto distSquared = d2(pos, pJ->pos);
                if (distSquared < cutoffSquared) {
                    map.at(idx).push_back({pJ, distSquared});
                    map.at(pJ).push_back({idx, distSquared});
                }
            }
        }
        for (auto &neighboringCell : cell->neighbors) {
            auto &neighboring_map = getPairs(neighboringCell);
            for (const auto &pJ : neighboringCell->particleIndices) {
                const auto distSquared = d2(pos, pJ->pos);
                if (distSquared < cutoffSquared) {
                    map.at(idx).push_back({pJ, distSquared});
                    neighboring_map.at(pJ).push_back({idx, distSquared});
                }
            }
        }
    } else {
        //log::console()->error("could not assign particle (index={}) to any cell!", data.getEntryIndex(idx));
    }
}

NeighborList::Cell *NeighborList::getCell(const readdy::model::Particle::pos_type &pos) {
    const cell_index i = static_cast<const cell_index>(floor((pos[0] + .5 * simBoxSize[0]) / cellSize[0]));
    const cell_index j = static_cast<const cell_index>(floor((pos[1] + .5 * simBoxSize[1]) / cellSize[1]));
    const cell_index k = static_cast<const cell_index>(floor((pos[2] + .5 * simBoxSize[2]) / cellSize[2]));
    return getCell(i, j, k);
}

void NeighborList::updateData(NeighborList::data_t &data, ParticleData::update_t update) {
    if(maxCutoff > 0) {
        for (const auto &p : std::get<1>(update)) {
            remove(p);
        }
    }

    auto newEntries = data.update(std::move(update));
    if(maxCutoff > 0) {
        for (const auto &p : newEntries) {
            insert(data, p);
        }
    }
}

const std::vector<NeighborList::neighbor_t> &NeighborList::neighbors(NeighborList::particle_index const entry) const {
    if(maxCutoff > 0) {
        const auto cell = getCell(entry->pos);
        if (cell != nullptr) {
            return getPairs(cell).at(entry);
        } else {
            std::stringstream stream;
            stream << "the particle position " << entry->pos << " was in no neighbor list cell" << std::endl;
            throw std::out_of_range(stream.str());
        }
    }
    return no_neighbors;
}

const NeighborList::Cell *const NeighborList::getCell(const readdy::model::Particle::pos_type &pos) const {
    const cell_index i = static_cast<const cell_index>(floor((pos[0] + .5 * simBoxSize[0]) / cellSize[0]));
    const cell_index j = static_cast<const cell_index>(floor((pos[1] + .5 * simBoxSize[1]) / cellSize[1]));
    const cell_index k = static_cast<const cell_index>(floor((pos[2] + .5 * simBoxSize[2]) / cellSize[2]));
    return getCell(i, j, k);
}

const NeighborList::Cell *const NeighborList::getCell(NeighborList::signed_cell_index i,
                                                      NeighborList::signed_cell_index j,
                                                      NeighborList::signed_cell_index k) const {

    const auto &periodic = ctx->getPeriodicBoundary();
    if (periodic[0]) i = readdy::util::numeric::positive_modulo(i, nCells[0]);
    else if (i < 0 || i >= nCells[0]) return nullptr;
    if (periodic[1]) j = readdy::util::numeric::positive_modulo(j, nCells[1]);
    else if (j < 0 || j >= nCells[1]) return nullptr;
    if (periodic[2]) k = readdy::util::numeric::positive_modulo(k, nCells[2]);
    else if (k < 0 || k >= nCells[2]) return nullptr;
    return &cells.at(k + j * nCells[2] + i * nCells[2] * nCells[1]);
}

NeighborList::container_t &NeighborList::getPairs(const NeighborList::Cell *const cell) {
    return maps.at(getMapsIndex(cell));
}

const NeighborList::container_t &NeighborList::getPairs(const NeighborList::Cell *const cell) const {
    return maps.at(getMapsIndex(cell));
}

util::Config::n_threads_t NeighborList::getMapsIndex(const NeighborList::Cell *const cell) const {
    return static_cast<util::Config::n_threads_t>(floor(cell->id / floor(cells.size() / config->nThreads )));
}

NeighborList::~NeighborList() = default;

Neighbor::Neighbor(const index_t idx, const double d2) : idx(idx), d2(d2) {}

Neighbor::Neighbor(Neighbor &&rhs) : idx(rhs.idx), d2(std::move(rhs.d2)) {
}

Neighbor &Neighbor::operator=(Neighbor &&rhs) {
    idx = rhs.idx;
    d2 = std::move(rhs.d2);
    return *this;
}

void NeighborList::Cell::addNeighbor(NeighborList::Cell *cell) {
    if (cell && cell->id != id
        && (enoughCells || std::find(neighbors.begin(), neighbors.end(), cell) == neighbors.end())) {
        neighbors.push_back(cell);
    }
}

NeighborList::Cell::Cell(cell_index i, cell_index j, cell_index k,
                         const std::array<NeighborList::cell_index, 3> &nCells)
        : id(k + j * nCells[2] + i * nCells[2] * nCells[1]),
          enoughCells(nCells[0] >= 5 && nCells[1] >= 5 && nCells[2] >= 5) {
}
}
}
}
}
