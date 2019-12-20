#include "randomplayer.h"

using namespace K273;
using namespace MuZero::Agents::Player;

int Player::onNextMove(double end_time) {
    SM::LegalState* ls = this->sm->getLegalState(this->our_role_index);
    return ls->getLegal(this->random.getWithMax(ls->getCount()));
}
