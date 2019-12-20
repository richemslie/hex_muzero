#include "perf_test.h"

#include "statemachine/statemachine.h"

#include <k273/logging.h>
#include <k273/exception.h>

using namespace K273;
using namespace MuZero::Testing;

void DepthChargeTest::doRollouts(int seconds_to_run) {
    const int role_count = this->sm->getRoleCount();

    const double start_time = get_time();
    const double end_time = start_time + seconds_to_run;

    while (true) {
        this->sm->reset();
        ASSERT (!this->sm->isTerminal());

        const double cur_time = get_time();
        if (cur_time > end_time) {
            this->msecs_taken = 1000 * (cur_time - start_time);
            break;
        }

        int depth = 0;

        while (true) {
            if (this->sm->isTerminal()) {
                break;
            }

            // populate joint move
            for (int ii=0; ii<role_count; ii++) {
                const SM::LegalState* ls = this->sm->getLegalState(ii);
                int x = this->random.getWithMax(ls->getCount());
                int choice = ls->getLegal(x);
                this->joint_move->set(ii, choice);
            }

            this->sm->nextState(this->joint_move, this->next_state);
            this->sm->updateBases(this->next_state);

            depth++;
        }

        // for heating the cpu side effect only
        for (int ii=0; ii<this->sm->getRoleCount(); ii++) {
            this->sm->getGoalValue(ii);
        }

        this->rollouts++;
        this->num_state_changes += depth;
    }
}
