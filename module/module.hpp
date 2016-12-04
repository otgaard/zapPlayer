/* Created by Darren Otgaar on 2016/12/04. http://www.github.com/otgaard/zap */
#ifndef ZAPPLAYER_MODULE_HPP
#define ZAPPLAYER_MODULE_HPP

#include <vector>
#include <zap/maths/transform.hpp>
#include <zap/renderer/renderer_fwd.hpp>

/*
 * A module is just a wrapper to swap visualisations in and out.
 */

class analyser;

class module {
public:
    module() = default;
    virtual ~module() = default;

    virtual bool initialise() = 0;

    virtual void resize(int width, int height) = 0;
    virtual void update(float dt, const std::vector<float>& samples) = 0;
    virtual void draw(const zap::renderer::camera& cam) = 0;

    zap::maths::transform4f world_transform;

protected:

private:

};


#endif //ZAPPLAYER_MODULE_HPP
