//
// Created by clonker on 08.04.16.
//

#ifndef READDY_MAIN_PROGRAMFACTORY_H
#define READDY_MAIN_PROGRAMFACTORY_H

#include <type_traits>
#include <readdy/model/Program.h>

namespace readdy {
    namespace model {
        class ProgramFactory {
            virtual std::unique_ptr<Program> createProgram(const std::string& name) const = 0;
        };
    }
}

#endif //READDY_MAIN_PROGRAMFACTORY_H