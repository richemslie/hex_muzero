// local includes
#include "sm.h"
#include "desc.h"
#include "board.h"
#include "statemachine/statemachine.h"

// k273 includes
#include <k273/logging.h>
#include <k273/exception.h>

using namespace MuZero;
using namespace MuZero::HexGame;

///////////////////////////////////////////////////////////////////////////////

HexStateMachine::HexStateMachine(const Description* board_desc) :
    board_desc(board_desc),
    board(new Board(board_desc)) {

    this->cur_state = this->newBaseState();
    this->initial_state = this->newBaseState();

    this->legal_states[0] = new SM::LegalState(board_desc->legalsSize(Role::Black));
    this->legal_states[1] = new SM::LegalState(board_desc->legalsSize(Role::White));

    // set the initial state
    uint8_t* bytes = (uint8_t*) this->initial_state->data;
    memset(bytes, 0, this->board_desc->numberPoints());
    bytes += this->board_desc->numberPoints();
    MetaCell* meta = (MetaCell*) bytes;
    *meta = MetaCell(this->board_desc->canSwap());

    // sm.reset() will be called in interface.StateMachine()
}

HexStateMachine::~HexStateMachine() {
    K273::l_warning("In HexGame HexStateMachine::~HexStateMachine()");
    delete this->board;

    ::free(this->cur_state);
    ::free(this->initial_state);
}

SM::StateMachineInterface* HexStateMachine::dupe() const {
    HexStateMachine* duped = new HexStateMachine(this->board_desc);
    duped->cur_state->assign(this->cur_state);
    duped->updateBases(duped->cur_state);
    duped->reset();
    return duped;
}

void HexStateMachine::reset() {
    this->updateBases(this->getInitialState());
}

void HexStateMachine::updateBases(const SM::BaseState* bs) {
    this->cur_state->assign(bs);

    this->board->setCells(this->cur_state);
    this->board->moveGen(this->legal_states[0], this->legal_states[1]);
}

SM::BaseState* HexStateMachine::newBaseState() const {
    int num_bases = 8 * (this->board_desc->numberPoints() + 1);

    void* mem = ::malloc(SM::BaseState::mallocSize(num_bases));
    SM::BaseState* bs = static_cast <SM::BaseState*>(mem);
    bs->init(num_bases);
    return bs;
}

const SM::BaseState* HexStateMachine::getCurrentState() const {
    return this->cur_state;
}

void HexStateMachine::setInitialState(const SM::BaseState* bs) {
    ASSERT_MSG(false, "not supported");
}

const SM::BaseState* HexStateMachine::getInitialState() const {
    return this->initial_state;
}

SM::LegalState* HexStateMachine::getLegalState(int role_index) {
    return this->legal_states[role_index];
}

SM::JointMove* HexStateMachine::getJointMove() {
    // zero array size malloc
    void* mem = malloc(SM::JointMove::mallocSize(this->getRoleCount()));
    SM::JointMove* move = static_cast <SM::JointMove*>(mem);
    move->setSize(this->getRoleCount());
    return move;
}

void HexStateMachine::nextState(const SM::JointMove* move, SM::BaseState* bs) {
    // we'll act on current state in playMove().
    // we'll assign it to bs when done, and then revert current_state.  Huge
    // hack, but get's the job done.

    bs->assign(this->cur_state);
    this->board->setCells(bs);

    // important: ensure this doesn't effect legal state
    this->board->playMove(move);

    this->board->setCells(this->cur_state);
}

const char* HexStateMachine::legalToMove(int role_index, int choice) const {
    // get from board_desc (same for both bt/non bt variants)
    if (role_index == 0) {
        return this->board_desc->legalToMove(Role::Black, choice);
    } else {
        return this->board_desc->legalToMove(Role::White, choice);
    }
}

bool HexStateMachine::isTerminal() const {
    return this->board->finished();
}

int HexStateMachine::getGoalValue(int role_index) {
    // undefined, call getGoalValue() in a non terminal state at your peril
    if (!this->isTerminal()) {
        return -1;
    }

    if (role_index == 0) {
        return this->board->score(Role::Black);
    } else {
        return this->board->score(Role::White);
    }
}

int HexStateMachine::getRoleCount() const {
    return 2;
}
