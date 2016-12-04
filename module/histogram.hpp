/* Created by Darren Otgaar on 2016/12/04. http://www.github.com/otgaard/zap */
#ifndef ZAPPLAYER_HISTOGRAM_HPP
#define ZAPPLAYER_HISTOGRAM_HPP

#include "module.hpp"

/*
 * A very basic histogram for directly displaying the frequency bins output by the FFT.
 */

class histogram : public module {
public:
    histogram();
    virtual ~histogram();

    bool initialise() override final;
    void resize(int width, int height) override final;

    void update(float dt, const std::vector<float>& samples) override final;
    void draw(const zap::renderer::camera& cam) override final;

protected:

private:
    struct state_t;
    std::unique_ptr<state_t> state_;
    state_t& s;
};

#endif //ZAPPLAYER_HISTOGRAM_HPP
