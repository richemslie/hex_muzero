#include "testing/perf_test.h"
#include "games/hex/desc.h"
#include "games/hex/sm.h"

#include <k273/util.h>
#include <k273/logging.h>
#include <k273/exception.h>

using namespace MuZero;
using namespace MuZero::HexGame;

static void logExceptionWrapper(const std::string& name) {
    try {
        K273::l_critical("an exception was thrown in in %s:", name.c_str());
        throw;

    } catch (const K273::Exception& exc) {
        K273::l_critical("K273::Exception Message : %s", exc.getMessage().c_str());
        K273::l_critical("K273::Exception Stacktrace : \n%s", exc.getStacktrace().c_str());

    } catch (std::exception& exc) {
        K273::l_critical("std::exception What : %s", exc.what());

    } catch (...) {
        K273::l_critical("Unknown exception");
    }
}

static void testDescription() {
    Description* desc = new Description(7, true);

    ASSERT(desc->getBoadSize() == 7);
    ASSERT(desc->canSwap());
    ASSERT(desc->numberPoints() == 49);

    ASSERT(desc->legalsSize(Role::Black) == desc->numberPoints() + 1);
    ASSERT(desc->legalsSize(Role::White) == desc->numberPoints() + 2);

    K273::l_debug("legals!");
    for (int ii = 0; ii < desc->numberPoints(); ii++) {
        K273::l_debug("%s %s", desc->legalToMove(Role::Black, desc->blackPosToLegal(ii)),
                      desc->legalToMove(Role::White, desc->whitePosToLegal(ii)));
    }

    K273::l_debug("get cells black!");
    for (int ii = 0; ii < desc->numberPoints(); ii++) {
        Cell cell = desc->getCell(Role::Black, ii);
        K273::l_debug("%d %s", ii, cell.repr().c_str());
    }

    K273::l_debug("get cells white!");
    for (int ii = 0; ii < desc->numberPoints(); ii++) {
        Cell cell = desc->getCell(Role::White, ii);
        K273::l_debug("%d %s", ii, cell.repr().c_str());
    }

    K273::l_debug("get neighbours!");
    for (int ii : {0, 6, 7, 32, 41, 48}) {
        for (int jj : desc->getNeighouringPositions(ii)) {
            K273::l_debug("%d n:%d", ii, jj);
        }
    }
}

static void testBoard() {
    Description* desc = new Description(7, true);
    SM::StateMachineInterface* sm = new HexStateMachine(desc);

    Cell* cells = (Cell*) sm->getInitialState()->data;

    for (int ii = 0; ii < desc->numberPoints(); ii++) {
        K273::l_debug("%d %s", ii, cells->repr().c_str());
        cells++;
    }

    MetaCell* meta = (MetaCell*) cells;
    K273::l_debug(meta->repr());
}


static void test_walkAcross() {
    Description* desc = new Description(5, true);
    SM::StateMachineInterface* sm = new HexStateMachine(desc);
    sm->reset();

    SM::JointMove* joint_move = sm->getJointMove();
    SM::BaseState* next_state = sm->newBaseState();

    // noop
    joint_move->set(1, 0);

    for (int ii = 0; ii < 5; ii++) {
        int pos = 3 + ii * 5;
        joint_move->set(0, pos + 1);

        K273::l_debug("%d %s %s", ii, desc->legalToMove(Role::Black, joint_move->get(0)),
                      desc->legalToMove(Role::White, joint_move->get(1)));

        sm->nextState(joint_move, next_state);
        sm->updateBases(next_state);

        Cell* cells = (Cell*) sm->getCurrentState()->data;
        for (int ii = 0; ii < desc->numberPoints(); ii++) {
            K273::l_debug("%d %s", ii, cells->repr().c_str());
            cells++;
        }

        MetaCell* meta = (MetaCell*) cells;
        meta->switchTurn();
    }

    ASSERT(sm->isTerminal());
    ASSERT(sm->getGoalValue(0) == 100);
    ASSERT(sm->getGoalValue(1) == 0);
}

void tests() {
    try {
        testDescription();
    } catch (...) {
        logExceptionWrapper(__PRETTY_FUNCTION__);
        throw;
    }

    try {
        testBoard();
    } catch (...) {
        logExceptionWrapper(__PRETTY_FUNCTION__);
        throw;
    }

    try {
        test_walkAcross();
    } catch (...) {
        logExceptionWrapper(__PRETTY_FUNCTION__);
        throw;
    }
}

void depthCharges(int board_size) {
    const int seconds = 10;

    K273::l_info("Creating statemachine of board size %d", board_size);
    Description* desc = new Description(board_size, true);
    SM::StateMachineInterface* sm = new HexStateMachine(desc);

    K273::l_info("Running rollouts/depth charges for %d seconds", seconds);
    MuZero::Testing::DepthChargeTest depth_charges(sm);
    depth_charges.doRollouts(seconds);

    auto depth_charges_per_second = depth_charges.rollouts / (depth_charges.msecs_taken / 1000.0);
    auto changes_per_second =
        depth_charges.num_state_changes / (depth_charges.msecs_taken / 1000.0);

    K273::l_debug("rollouts per second %.1f, changes per second %.1f", depth_charges_per_second,
                  changes_per_second);
}

void run(std::vector<std::string>& args) {
    const int args_count = args.size();
    if (args_count == 2 && args[1] == "-t") {
        tests();

    } else if (args_count == 3 && args[1] == "-p") {
        const int board_size = K273::toInt(args[2]);
        depthCharges(board_size);

    } else {
        K273::l_error("-t for simple tests, '-p sz' for perfomance test of board size 'sz'");
    }
}

///////////////////////////////////////////////////////////////////////////////

#include <k273/runner.h>
const std::string bin_name = "test_hex";

int main(int argc, char** argv) {
    K273::Runner::Config config(argc, argv);
    config.log_filename = K273::fmtString("%s.log", bin_name.c_str());
    return K273::Runner::Main(run, config);
}
