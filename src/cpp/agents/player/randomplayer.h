#pragma once

#include "agents/player/base.h"
#include "statemachine/statemachine.h"

#include <k273/util.h>

namespace MuZero::Agents::Player {

class Player : public Agents::Player::PlayerBase {
    /* Plays - randomly. */

  public:
    Player(SM::StateMachineInterface* sm, int our_role_index) :
        PlayerBase(sm, our_role_index) {
    }

    virtual ~Player() {
    }

  public:
    virtual int onNextMove(double end_time);

  private:
    K273::Random random;
};

}  // namespace MuZero::Agents::Player
