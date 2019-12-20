#pragma once

// local includes
#include "desc.h"
#include "board.h"

#include "statemachine/statemachine.h"


namespace MuZero::HexGame {

class HexStateMachine : public SM::StateMachineInterface {
  public:
    HexStateMachine(const Description* board_desc);
    virtual ~HexStateMachine();

  public:
    // SM interface:

    StateMachineInterface* dupe() const;
    SM::BaseState* newBaseState() const;
    const SM::BaseState* getCurrentState() const;

    void setInitialState(const SM::BaseState* bs);
    const SM::BaseState* getInitialState() const;

    SM::LegalState* getLegalState(int role_index);

    void updateBases(const SM::BaseState* bs);
    const char* legalToMove(int role_index, int choice) const;
    SM::JointMove* getJointMove();

    bool isTerminal() const;
    void nextState(const SM::JointMove* move, SM::BaseState* bs);

    int getGoalValue(int role_index);
    void reset();

    int getRoleCount() const;

  private:
    const Description* board_desc;
    Board* board;

    SM::BaseState* cur_state;
    SM::BaseState* initial_state;

    SM::LegalState* legal_states[2];
};

}  // namespace MuZero::HexGame
