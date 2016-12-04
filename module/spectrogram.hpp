/* Created by Darren Otgaar on 2016/12/04. http://www.github.com/otgaard/zap */
#ifndef ZAPPLAYER_SPECTROGRAM_HPP
#define ZAPPLAYER_SPECTROGRAM_HPP

#include "module.hpp"

/*
 * Creates a line-plot of the input FFT bins and a 2D spectrogram image with each frame
 */

class spectrogram : public module {
public:
    spectrogram();
    virtual ~spectrogram();


    bool initialise() override;
    void resize(int width, int height) override;

    void update(float dt, const std::vector<float>& samples) override;
    void draw(const zap::renderer::camera& cam) override;

protected:

private:
    struct state_t;
    std::unique_ptr<state_t> state_;
    state_t& s;
};


#endif //ZAPPLAYER_SPECTROGRAM_HPP
