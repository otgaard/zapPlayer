/* Created by Darren Otgaar on 2017/05/28. http://www.github.com/otgaard/zap */
#ifndef ZAPPLAYER_SURFACE_HPP
#define ZAPPLAYER_SURFACE_HPP

#include "module.hpp"

// A surface rendered in response to frequency variations

class surface : public module {
public:
    surface();
    virtual ~surface();

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


#endif //ZAPPLAYER_SURFACE_HPP
