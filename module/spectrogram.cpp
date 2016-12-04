/* Created by Darren Otgaar on 2016/12/04. http://www.github.com/otgaard/zap */
#include "spectrogram.hpp"
#include <zap/maths/algebra.hpp>
#include <zap/graphics2/plotter/plotter.hpp>

#define LOGGING_ENABLED
#include <zap/tools/log.hpp>

using namespace zap;
using namespace zap::maths;
using namespace zap::graphics;

struct spectrogram::state_t {
    plotter plot;

    sampler1D<vec3b, decltype(interpolators::nearest<vec3b>)> colour_sampler_;

    state_t() : plot(vec2f(0.f, 1.f), vec2f(0.f, 1.f), .1f) { }
};

spectrogram::spectrogram() : state_(new state_t()), s(*state_.get()) {
}

spectrogram::~spectrogram() = default;

bool spectrogram::initialise() {
    if(!s.plot.initialise()) {
        LOG_ERR("Error initialising plotter");
        return false;
    }

    s.colour_sampler_.data.resize(8);
    s.colour_sampler_.data[0] = vec3b(255, 255, 255);
    s.colour_sampler_.data[1] = vec3b(255, 0, 255);
    s.colour_sampler_.data[2] = vec3b(100, 100, 255);
    s.colour_sampler_.data[3] = vec3b(0, 255, 255);
    s.colour_sampler_.data[4] = vec3b(0, 255, 0);
    s.colour_sampler_.data[5] = vec3b(255, 255, 0);
    s.colour_sampler_.data[6] = vec3b(255, 0, 0);
    s.colour_sampler_.data[7] = vec3b(255, 255, 255);
    s.colour_sampler_.inv_u = 1.f/7.f;
    s.colour_sampler_.fnc = interpolators::linear<vec3b>;

    return true;
}

void spectrogram::resize(int width, int height) {
    auto hwidth = width - 20, hheight = height/4;
    s.plot.world_transform.scale(vec2f(hwidth, hheight));
    s.plot.world_transform.translate(vec2f(10, height - hheight - 10));
}

void spectrogram::update(float dt, const std::vector<float>& samples) {
    sampler1D<float, decltype(interpolators::cubic<float>)> sampler(samples, interpolators::cubic<float>);
    s.plot.live_plot(sampler, s.colour_sampler_, 2000);
}

void spectrogram::draw(const zap::renderer::camera& cam) {
    s.plot.draw(cam);
}
