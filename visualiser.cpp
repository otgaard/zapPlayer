/* Created by Darren Otgaar on 2016/11/19. http://www.github.com/otgaard/zap */
#include "visualiser.hpp"
#include <zap/engine/engine.hpp>
#include "module/histogram.hpp"
#include <zap/renderer/camera.hpp>
#include <module/spectrogram.hpp>
#include <module/surface.hpp>
#include <module/texture_mod.hpp>

using camera = zap::renderer::camera;

struct visualiser::state_t {
    std::vector<float> bins;
    bool is_initialised;
    camera cam;

    std::vector<std::unique_ptr<module>> modules;
    std::vector<std::string> module_names;
    int active;

    state_t(size_t bins) : bins(bins, 0.f), is_initialised(false), cam(false), active(-1) { }
};

visualiser::visualiser(size_t bins) : state_(new state_t(bins)), s(*state_.get()) {
}

visualiser::~visualiser() = default;

void visualiser::set_frequency_bins(const std::vector<float>& bins) {
    s.bins = bins;
}

bool visualiser::initialise() {
    auto histo = std::make_unique<histogram>();
    if(!histo->initialise()) {
        LOG_ERR("Failed to initialise histogram visualisation");
        return false;
    }

    auto spectro = std::make_unique<spectrogram>();
    if(!spectro->initialise()) {
        LOG_ERR("Failure initialising spectrogram visualisation");
        return false;
    }

    auto surf = std::make_unique<surface>();
    if(!surf->initialise()) {
        LOG_ERR("Failure initialising surface visualisation");
        return false;
    }

    auto texmod = std::make_unique<texture_mod>();
    if(!texmod->initialise()) {
        LOG_ERR("Failure initialising texture_mod visualisation");
        return false;
    }

    s.modules.emplace_back(std::move(histo));
    s.modules.emplace_back(std::move(spectro));
    s.modules.emplace_back(std::move(surf));
    s.modules.emplace_back(std::move(texmod));
    s.module_names.push_back("Histogram");
    s.module_names.push_back("Spectrogram");
    s.module_names.push_back("Surface");
    s.module_names.push_back("Texture Module");

    s.active = 0;

    s.is_initialised = true;
    return true;
}

void visualiser::resize(int width, int height) {
    s.cam.viewport(0, 0, width, height);
    s.cam.frustum(0, width, 0, height, 0, 10.f);
    if(s.active > -1) s.modules[s.active]->resize(width, height);
}

void visualiser::update(double t, float dt) {
    if(s.active > -1) s.modules[s.active]->update(dt, s.bins);
}

void visualiser::draw() {
    if(s.active > -1) s.modules[s.active]->draw(s.cam);
}

bool visualiser::is_initialised() const {
    return s.is_initialised;
}

std::vector<std::string> visualiser::get_visualisations() const {
    return s.module_names;
}

void visualiser::set_visualisation(const std::string& name) {
    auto it = std::find(s.module_names.begin(), s.module_names.end(), name);
    if(it != s.module_names.end()) {
        s.active = it - s.module_names.begin();
        s.modules[s.active]->resize(s.cam.width(), s.cam.height());
        s.modules[s.active]->update(0.f, std::vector<float>(128));
    }
}
