#pragma once

#include "statemachine/statemachine.h"
#include "statemachine/basestate.h"
#include "statemachine/legalstate.h"
#include "statemachine/jointmove.h"

#include <k273/util.h>

namespace MuZero::Testing {

class DepthChargeTest {
  public:
    DepthChargeTest(SM::StateMachineInterface* sm) :
        msecs_taken(0), rollouts(0), num_state_changes(0),
        sm(sm) {
        this->joint_move = this->sm->getJointMove();
        this->next_state = this->sm->newBaseState();
    }

    ~DepthChargeTest() {
        free(this->joint_move);
        free(this->next_state);
    }

  public:
    void doRollouts(int second);

  public:
    int msecs_taken;
    int rollouts;
    int num_state_changes;

  private:
    SM::StateMachineInterface* sm;
    SM::JointMove* joint_move;
    SM::BaseState* next_state;

    K273::Random random;
};

}  // namespace MuZero::Testing
