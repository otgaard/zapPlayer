/* Created by Darren Otgaar on 2017/06/03. http://www.github.com/otgaard/zap */
#ifndef ZAPPLAYER_TEXTURE_MOD_HPP
#define ZAPPLAYER_TEXTURE_MOD_HPP

#include "module.hpp"

class texture_mod : public module {
public:
    texture_mod();
    virtual ~texture_mod();

    bool initialise() override;
    void resize(int width, int height) override;
    void update(float dt, const std::vector<float>& samples) override;
    void draw(const zap::renderer::camera& cam) override;

private:
    struct state_t;
    std::unique_ptr<state_t> state_;
    state_t& s;
};


#endif //ZAPPLAYER_TEXTURE_MOD_HPP
