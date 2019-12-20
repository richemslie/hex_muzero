#pragma once

// local includes
#include "desc.h"
#include "statemachine/statemachine.h"

// stl includes
#include <stack>

namespace MuZero::HexGame {

// State of an in-play game.
class Board {
  public:
    Board(const Description* board_desc) : board_desc(board_desc), cell_states(nullptr) {
    }

    ~Board() {
    }

  private:
    MetaCell* getMeta() {
        return (MetaCell*) (this->cell_states + this->board_desc->numberPoints());
    }

    const MetaCell* getMeta() const {
        MetaCell* meta = (MetaCell*) (this->cell_states + this->board_desc->numberPoints());
        return meta;
    }

    void swapFirstCell();

  public:
    void setCells(SM::BaseState* bs) {
        this->cell_states = (Cell*) bs->data;
    }

    void playMove(const SM::JointMove* move);

    void moveGen(SM::LegalState* ls_black, SM::LegalState* ls_white);

    bool finished() const;
    int score(Role role) const;

  private:
    const Description* board_desc;

    Cell* cell_states;
    std::stack<int> cells_to_check;
};

}  // namespace MuZero::HexGame
