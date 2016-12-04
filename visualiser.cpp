/* Created by Darren Otgaar on 2016/11/19. http://www.github.com/otgaard/zap */
#include "visualiser.hpp"
#include <zap/engine/engine.hpp>
#include "module/histogram.hpp"
#include <zap/renderer/camera.hpp>
#include <module/spectrogram.hpp>

using camera = zap::renderer::camera;

struct visualiser::state_t {
    std::vector<float> bins;
    bool is_initialised;
    camera cam;

    std::unique_ptr<module> histogram_;
    std::unique_ptr<module> spectrogram_;

    state_t(size_t bins) : bins(bins, 0.f), is_initialised(false), cam(false) { }
};

visualiser::visualiser(size_t bins) : state_(new state_t(bins)), s(*state_.get()) {
}

visualiser::~visualiser() = default;

void visualiser::set_frequency_bins(const std::vector<float>& bins) {
    s.bins = bins;
}

bool visualiser::initialise() {
    zap::engine::init();

    s.histogram_ = std::make_unique<histogram>();
    if(!s.histogram_->initialise()) {
        LOG_ERR("Failure initialising histogram visualisation");
        return false;
    }

    s.spectrogram_ = std::make_unique<spectrogram>();
    if(!s.spectrogram_->initialise()) {
        LOG_ERR("Failure initialising spectrogram visualisation");
        return false;
    }

    s.is_initialised = true;
    return true;
}

void visualiser::resize(int width, int height) {
    s.cam.viewport(0, 0, width, height);
    s.cam.frustum(0, width, 0, height, 0, 10.f);
    if(s.histogram_) s.histogram_->resize(width, height);
    if(s.spectrogram_) s.spectrogram_->resize(width, height);
}

void visualiser::update(double t, float dt) {
    if(s.histogram_) s.histogram_->update(dt, s.bins);
    if(s.spectrogram_) s.spectrogram_->update(dt, s.bins);
}

void visualiser::draw() {
    if(s.histogram_) s.histogram_->draw(s.cam);
    if(s.spectrogram_) s.spectrogram_->draw(s.cam);
}

bool visualiser::is_initialised() const {
    return s.is_initialised;
}