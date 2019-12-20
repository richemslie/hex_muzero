
// local includes
#include "desc.h"

// k273 includes
#include <k273/logging.h>
#include <k273/strutils.h>
#include <k273/exception.h>

using namespace MuZero::HexGame;

///////////////////////////////////////////////////////////////////////////////

Cell::Cell(int board_size, Role role, int pos) {
    if (role == Role::Black) {

        this->data = BLACK;

        // base cases: first row & last row
        if (pos < board_size) {
            this->update(BLACK_NORTH);

        } else if (pos >= board_size * (board_size - 1)) {
            this->update(BLACK_SOUTH);
        }

    } else {
        this->data = WHITE;

        // base cases: first column & last column
        if (pos % board_size == 0) {
            this->update(WHITE_WEST);
        }

        // Last row
        if (pos % board_size == (board_size - 1)) {
            this->update(WHITE_EAST);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

Description::Description(int sz, bool swap_pie_rule) :
    board_size(sz),
    can_swap(swap_pie_rule) {

    K273::l_warning("HexGame size %d", this->board_size);

    this->initBoard();
}

Description::~Description() {
    K273::l_warning("In HexHame ~Description()");
}

///////////////////////////////////////////////////////////////////////////////

void Description::initBoard() {
    const char* alpha = "abcdefghijklmnopqrstuvwxyz";
    ASSERT(this->board_size <= 26);

    // put the noop/swap legals first
    this->black_legal_moves.emplace_back("noop");
    this->white_legal_moves.emplace_back("noop");
    this->white_legal_moves.emplace_back("swap");

    // set the cells and neighbours
    for (int ii=0; ii<this->board_size; ii++) {
        for (int jj=0; jj<this->board_size; jj++) {

            // pos is defined from west to east along north axis,
            // so goes a1 b1 c1... b2 b2 c2...
            int pos = ii * board_size + jj;

            // cels
            this->cells_black.emplace_back(this->board_size, Role::Black, pos);
            this->cells_white.emplace_back(this->board_size, Role::White, pos);

            // neighbours
            this->all_adj_cells.push_back(this->adjacentCells(pos));

            // legal strings
            std::string legal_str = K273::fmtString("(place %c %d)",
                                                    alpha[jj],
                                                    ii + 1);
            black_legal_moves.push_back(legal_str);
            white_legal_moves.push_back(legal_str);
        }
    }
}

std::vector <int> Description::adjacentCells(int pos) const {
    const int sz = this->getBoadSize();

    int x = pos % this->board_size;
    int y = pos / this->board_size;

    // Add only valid neighbours
    std::vector <int> result;

    auto test_point = [sz, x, y, &result](int adj_x, int adj_y){
       int new_x = x + adj_x;
       int new_y = y + adj_y;

       if (new_x >= 0 && new_x < sz &&
           new_y >= 0 && new_y < sz) {
           result.emplace_back(new_y * sz + new_x);
       }
    };

    test_point(-1, 0);
    test_point(1, 0);

    test_point(1, -1);
    test_point(0, -1);

    test_point(0, 1);
    test_point(-1, 1);

    return result;
}
