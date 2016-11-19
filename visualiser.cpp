/* Created by Darren Otgaar on 2016/11/19. http://www.github.com/otgaard/zap */
#include "visualiser.hpp"

struct visualiser::state_t {

};

visualiser::visualiser() : state_(new state_t()), s(*state_.get()) {

}

visualiser::~visualiser() = default;

