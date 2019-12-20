#pragma once

// local includes
#include <statemachine/basestate.h>

// k273 includes
#include <k273/util.h>
#include <k273/strutils.h>
#include <k273/exception.h>

// std includes
#include <string>
#include <vector>
#include <cstdint>
#include <type_traits>

///////////////////////////////////////////////////////////////////////////////

namespace MuZero {
namespace HexGame {

enum class Role : int { Black = 0, White = 1 };
using Legal = int;

class Cell {
    // State of a cell.
    // Describes if a cell is
    //   empty or black or white
    //   - unconnected or connected to N/S edges if black
    //   - unconnected or connected to E/W edges if white

  public:
    Cell() : data(0) {
    }

    Cell(int board_size, Role role, int pos);

  private:
    constexpr static uint8_t BLACK = 1 << 0;
    constexpr static uint8_t BLACK_NORTH = 1 << 1;
    constexpr static uint8_t BLACK_SOUTH = 1 << 2;

    constexpr static uint8_t WHITE = 1 << 3;
    constexpr static uint8_t WHITE_WEST = 1 << 4;
    constexpr static uint8_t WHITE_EAST = 1 << 5;
    constexpr static uint8_t ALL_CONNECTS = BLACK_NORTH + BLACK_SOUTH + WHITE_WEST + WHITE_EAST;

  public:
    bool blackWins() const {
        return this->data & BLACK_NORTH && this->data & BLACK_SOUTH;
    }

    bool whiteWins() const {
        return this->data & WHITE_WEST && this->data & WHITE_EAST;
    }

    bool empty() const {
        return this->data == 0;
    }

    void update(uint8_t what) {
        this->data |= what;
    }

    void mergeBlack(const Cell other) {
        this->data |= (other.data & (Cell::BLACK_NORTH + Cell::BLACK_SOUTH));
    }

    void mergeWhite(const Cell other) {
        this->data |= (other.data & (Cell::WHITE_WEST + Cell::WHITE_EAST));
    }

    bool connected() const {
        return (this->data & ALL_CONNECTS) > 0;
    }

    bool unconnected(Cell other) const {
        const uint8_t mask = (BLACK + WHITE) & other.data;
        return !this->connected() && this->data & mask;
    }

    bool operator==(const Cell& other) const {
        return this->data == other.data;
    }

    std::string repr() const {
        if (this->empty()) {
            return "empty";
        }

        if (this->data & BLACK) {
            if (this->data & BLACK_NORTH && this->data & BLACK_SOUTH) {
                return "B*";
            } else if (this->data & BLACK_NORTH) {
                return "B+";
            } else if (this->data & BLACK_SOUTH) {
                return "B-";
            } else {
                return "B";
            }
        } else {
            ASSERT(this->data & WHITE);
            if (this->data & WHITE_WEST && this->data & WHITE_EAST) {
                return "W*";
            } else if (this->data & WHITE_WEST) {
                return "W+";
            } else if (this->data & WHITE_EAST) {
                return "W-";
            } else {
                return "W";
            }
        }
    }

  private:
    uint8_t data;
};

class MetaCell {
  private:
    constexpr static uint8_t BLACK_TURN = 1 << 0;
    constexpr static uint8_t WHITE_TURN = 1 << 1;

    constexpr static uint8_t BLACK_CONNECTED = 1 << 2;
    constexpr static uint8_t WHITE_CONNECTED = 1 << 3;

    constexpr static uint8_t CAN_SWAP = 1 << 4;

  public:
    MetaCell(bool can_swap) : data(BLACK_TURN) {
        if (can_swap) {
            data += CAN_SWAP;
        }
    }

  public:
    bool canSwap() const {
        return this->data & CAN_SWAP;
    }

    bool blackConnected() const {
        return this->data & BLACK_CONNECTED;
    }

    bool whiteConnected() const {
        return this->data & WHITE_CONNECTED;
    }

    bool done() const {
        return this->data & (BLACK_CONNECTED + WHITE_CONNECTED);
    }

    Role whosTurn() const {
        if (this->data & BLACK_TURN) {
            return Role::Black;
        } else {
            return Role::White;
        }
    }

    void switchTurn() {
        if (this->data & BLACK_TURN) {
            this->data &= ~BLACK_TURN;
            this->data |= WHITE_TURN;
        } else {
            this->data &= ~WHITE_TURN;
            this->data |= BLACK_TURN;
        }
    }

    void setBlackConnected() {
        this->data |= BLACK_CONNECTED;
    }

    void setWhiteConnected() {
        this->data |= WHITE_CONNECTED;
    }

    void removeSwap() {
        this->data &= ~CAN_SWAP;
    }

    std::string repr() const {
        return K273::fmtString("turn %s [%s,%s,%s]", this->whosTurn() == Role::Black ? "B" : "W",
                               this->canSwap() ? "swap" : "", this->blackConnected() ? "B" : "",
                               this->whiteConnected() ? "W" : "");
    }

  private:
    uint8_t data;
};


class Description {
  public:
    Description(int board_size, bool swap_pie_rule);
    ~Description();

  private:
    void initBoard();
    std::vector<int> adjacentCells(int pos) const;

  public:
    int getBoadSize() const {
        return this->board_size;
    }

    int canSwap() const {
        return this->can_swap;
    }

    int numberPoints() const {
        return this->board_size * this->board_size;
    }

    Legal getNoopLegal() const {
        return 0;
    }

    Legal getSwapLegal() const {
        return 1;
    }

    int legalToPos(Role role, int legal) const {
        int pos = legal - 1;
        if (role == Role::White) {
            pos--;
        }

        return pos;
    }

    Legal blackPosToLegal(int pos) const {
        // no conditionals for speed!
        return pos + 1;
    }

    Legal whitePosToLegal(int pos) const {
        // no conditionals for speed!
        return pos + 2;
    }

    const char* legalToMove(Role role, Legal legal) const {
        if (role == Role::Black) {
            return this->black_legal_moves[legal].c_str();
        } else {
            return this->white_legal_moves[legal].c_str();
        }
    }

    int legalsSize(Role role) const {
        if (role == Role::Black) {
            return this->black_legal_moves.size();
        } else {
            return this->white_legal_moves.size();
        }
    }

    const std::vector<int> getNeighouringPositions(int pos) const {
        return this->all_adj_cells[pos];
    }

    Cell getCell(Role role, int pos) const {
        if (role == Role::Black) {
            return this->cells_black[pos];
        } else {
            return this->cells_white[pos];
        }
    }

  private:
    const int board_size;
    const int can_swap;

    std::vector<std::string> white_legal_moves;
    std::vector<std::string> black_legal_moves;

    std::vector<bool> initial_state;

    std::vector<Cell> cells_black;
    std::vector<Cell> cells_white;

    std::vector<std::vector<int>> all_adj_cells;
};

}  // namespace HexGame
}  // namespace MuZero
