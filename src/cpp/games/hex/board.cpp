// local includes
#include "sm.h"
#include "desc.h"
#include "board.h"
#include "statemachine/statemachine.h"

// k273 includes
#include <k273/strutils.h>
#include <k273/logging.h>
#include <k273/exception.h>
#include <k273/util.h>

#include <ctype.h>
#include <stdio.h>

using namespace MuZero;
using namespace MuZero::HexGame;

///////////////////////////////////////////////////////////////////////////////

void Board::swapFirstCell() {
    const int board_size = this->board_desc->getBoadSize();

    for (int ii=0; ii<board_size; ii++) {
        for (int jj=0; jj<board_size; jj++) {
            int pos = ii * board_size + jj;
            Cell cell = this->cell_states[pos];

            if (!cell.empty()) {
                // zero current cell
                this->cell_states[pos] = Cell();
                int new_pos = jj * board_size + ii;
                this->cell_states[new_pos] = this->board_desc->getCell(Role::White, new_pos);
                return;
            }
        }
    }

    ASSERT_MSG(false, "Cannot get here!");
}

void Board::playMove(const SM::JointMove* move) {

    MetaCell* meta = this->getMeta();
    Role role = meta->whosTurn();

    int legal;
    if (role == Role::Black) {
        ASSERT(move->get(1) == this->board_desc->getNoopLegal());
        legal = move->get(0);

    } else {
        ASSERT(move->get(0) == this->board_desc->getNoopLegal());
        legal = move->get(1);

        if (legal == this->board_desc->getSwapLegal()) {
            ASSERT(this->board_desc->canSwap() && meta->canSwap());
            this->swapFirstCell();
            meta->removeSwap();
            meta->switchTurn();
            return;
        }

        meta->removeSwap();
    }

    int pos = this->board_desc->legalToPos(role, legal);

    // set the cell
    Cell updated_cell = this->board_desc->getCell(role, pos);

    // merge
    if (role == Role::Black) {
        // check neighbours
        for (int neighbour : this->board_desc->getNeighouringPositions(pos)) {
            updated_cell.mergeBlack(this->cell_states[neighbour]);
        }
    } else {
        // check neighbours
        for (int neighbour : this->board_desc->getNeighouringPositions(pos)) {
            updated_cell.mergeWhite(this->cell_states[neighbour]);
        }
    }

    this->cell_states[pos] = updated_cell;

    // is terminal?
    if (updated_cell.blackWins()) {
        meta->setBlackConnected();
        return;

    } else if (updated_cell.whiteWins()) {
        meta->setWhiteConnected();
        return;
    }


    // Propagate edge-connected groups to maintain that all edge connected nodes are known about.

    if (updated_cell.connected()) {

        // very subtle:
        // (a) updated_cell, can never be fully connected - or game is already won
        // (b) updated_cell had already inherited its neighbours connections
        // (c) with a and b, only unconnected neighbouring cells need to be propagated to

        // will always be empty on entry
        this->cells_to_check.push(pos);

        while (!this->cells_to_check.empty()) {

            int current = this->cells_to_check.top();
            this->cells_to_check.pop();

            for (int neighbour : this->board_desc->getNeighouringPositions(current)) {

                Cell& ncell = this->cell_states[neighbour];
                if (ncell.unconnected(updated_cell)) {
                    // Update the cell before pushing to stack, avoids repeating
                    this->cell_states[neighbour] = updated_cell;
                    this->cells_to_check.push(neighbour);
                }
            }
        }
    }

    meta->switchTurn();
}

void Board::moveGen(SM::LegalState* ls_black, SM::LegalState* ls_white) {
    const MetaCell* meta = this->getMeta();

    // clear the legal states
    ls_black->clear();
    ls_white->clear();

    const int num_points = this->board_desc->numberPoints();

    Role role = meta->whosTurn();
    if (role == Role::Black) {
        // add noop for white
        ls_white->insert(this->board_desc->getNoopLegal());

        // add any points that are valid/legal
        for (int ii=0; ii<num_points; ii++) {
            Cell cur = this->cell_states[ii];
            if (cur.empty()) {
                ls_black->insert(this->board_desc->blackPosToLegal(ii));
            }
        }

    } else {
        // add noop for black
        ls_black->insert(this->board_desc->getNoopLegal());

        // add swap?
        if (this->board_desc->canSwap() && meta->canSwap()) {
            ls_white->insert(this->board_desc->getSwapLegal());
        }

        // add any points that are valid/legal
        for (int ii=0; ii<num_points; ii++) {
            Cell cur = this->cell_states[ii];
            if (cur.empty()) {
                ls_white->insert(this->board_desc->whitePosToLegal(ii));
            }
        }
    }
}

bool Board::finished() const {
    return this->getMeta()->done();
}

int Board::score(Role role) const {
    ASSERT(this->finished());

    if (role == Role::Black) {
        if (this->getMeta()->blackConnected()) {
            return 100;
        } else {
            return 0;
        }
    } else {
        if (this->getMeta()->whiteConnected()) {
            return 100;
        } else {
            return 0;
        }
    }
}


