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
 * The KernelProvider gives means to access kernels that are in shared libs. The can be indexed by calling
 * loadKernelsFromDirectory(directory).
 *
 * @file KernelProvider.h
 * @brief This file contains the KernelProvider class. It is used to make kernels that reside in shared libs accessible.
 * @author clonker
 * @date 02.05.16
 */

#ifndef READDY_MAIN_KERNELPROVIDER_H
#define READDY_MAIN_KERNELPROVIDER_H

#include <readdy/model/Kernel.h>
#include "PluginProvider.h"

namespace readdy {
namespace plugin {

/**
 * The KernelProvider is a singleton which can be accessed by getInstance()
 * and provides Kernels that can be added directly or loaded from directories.
 * If loaded from directories (with loadKernelsFromDirectory(string), the
 * specified directory will be scanned for shared libraries with the required
 * symbols, i.e., with an implementation of the Kernel class.
 */
class KernelProvider : public PluginProvider<readdy::model::Kernel> {
protected:
    /**
     * The constructor of KernelProvider. As it is a singleton, it is protected.
     */
    KernelProvider();

    /**
     * The destructor of KernelProvider.
     */
    virtual ~KernelProvider();

    /**
     * A protected method that determines if a path points to a shared library.
     *
     * @param path the path
     * @return True if the path points to a shared library, otherwise false.
     */
    bool isSharedLibrary(const std::string &path) const;

public:
    /**
     * Method that returns the singleton KernelProvider.
     *
     * @return The KernelProvider.
     */
    static KernelProvider &getInstance();

    /**
     * Method to load kernels (non-recursively) in shared libraries from a directory.
     *
     * @param directory the directory in which the shared libraries are located.
     */
    void loadKernelsFromDirectory(const std::string &directory);

    /**
     * Sinking method that allows to move a kernel into the KernelProvider and thus make it available.
     *
     * @param kernel the kernel that should be moved
     * @todo update docs
     */
    void add(const std::string name, const std::function<readdy::model::Kernel *()> &creator);

    /**
     * Method that allows to add a kernel to the KernelProvider by providing a path to a shared lib (containing an implementation of a kernel).
     *
     * @param sharedLib the path to the shared lib
     */
    void add(const std::string &sharedLib);

    /**
     * Method that gives the default kernel directory, i.e., where the kernel implementations are usually to be found.
     * First it is checked, if the environment variable 'READDY_PLUGIN_DIR' is set. In that case, the default kernel directory is the contents
     * of that environment variable.
     * Otherwise, the default kernel directory on unix systems is
     * \code{.unparsed}
     * /usr/local/readdy/lib/readdy_plugins
     * \endcode
     * and the default kernel directory on windows systems is
     * \code{.unparsed}
     * C:\\Program Files\ReaDDy\lib\readdy_plugins
     * \endcode
     *
     * @return the default kernel directory.
     */
    static const std::string getDefaultKernelDirectory();

private:
    const std::string loadKernelName(const std::string &sharedLib);


    // prevent that copies can be created
    KernelProvider(KernelProvider const &) = delete;

    // prevent that copies can be created
    void operator=(KernelProvider const &) = delete;

    struct Impl;
    std::unique_ptr<Impl> pimpl;

};
}
}
#endif //READDY_MAIN_KERNELPROVIDER_H
