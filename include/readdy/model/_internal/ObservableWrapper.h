/**
 * << detailed description >>
 *
 * @file ObservableWrapper.h
 * @brief << brief description >>
 * @author clonker
 * @date 10.05.16
 */

#ifndef READDY2_MAIN_OBSERVABLEWRAPPER_H
#define READDY2_MAIN_OBSERVABLEWRAPPER_H
#include <readdy/model/Observable.h>

namespace readdy {
    namespace model {
        class ObservableWrapper : public ObservableBase {
        public:
            ObservableWrapper(readdy::model::Kernel *const kernel, const readdy::model::ObservableType &observable, unsigned int stride = 1);

            void operator()(readdy::model::time_step_type t);

            virtual void evaluate() override;

        protected:
            const readdy::model::ObservableType observable;
        };

    }
}
#endif //READDY2_MAIN_OBSERVABLEWRAPPER_H
