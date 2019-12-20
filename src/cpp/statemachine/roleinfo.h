#pragma once

// local includes
#include "statemachine/legalstate.h"

// std includes
#include <string>

namespace MuZero::SM {

struct RoleInfo {
    std::string name;
    int input_start_index;
    int legal_start_index;
    int goal_start_index;
    int num_inputs_legals;
    int num_goals;
    LegalState legal_state;
};

}  // namespace MuZero::SM
