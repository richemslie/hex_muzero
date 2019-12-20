
#pragma once

// local includes
#include "statemachine/basestate.h"
#include "statemachine/legalstate.h"
#include "statemachine/roleinfo.h"
#include "statemachine/jointmove.h"

namespace MuZero::SM {

class StateMachineInterface {
  public:
    StateMachineInterface() {
    }

    virtual ~StateMachineInterface() {
    }

  public:
    virtual StateMachineInterface* dupe() const = 0;

    virtual BaseState* newBaseState() const = 0;
    virtual const BaseState* getCurrentState() const = 0;

    virtual void setInitialState(const BaseState* bs) = 0;

    // get the initial state without changing the state of sm
    virtual const BaseState* getInitialState() const = 0;

    virtual void updateBases(const BaseState* bs) = 0;
    virtual LegalState* getLegalState(int role_index) = 0;

    virtual const char* legalToMove(int role_index, int choice) const = 0;

    virtual JointMove* getJointMove() = 0;
    virtual bool isTerminal() const = 0;
    virtual void nextState(const JointMove* move, BaseState* bs) = 0;
    virtual int getGoalValue(int role_index) = 0;

    virtual void reset() = 0;
    virtual int getRoleCount() const = 0;
};

}  // namespace MuZero::SM
