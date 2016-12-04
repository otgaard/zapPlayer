/* Created by Darren Otgaar on 2016/12/04. http://www.github.com/otgaard/zap */
#include "analyser.hpp"

struct analyser::state_t {
};

analyser::analyser() : state_(new state_t()), s(*state_.get()) {
}

analyser::~analyser() = default;

