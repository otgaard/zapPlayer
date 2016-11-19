#ifndef ZAPPLAYER_VISUALISER_HPP
#define ZAPPLAYER_VISUALISER_HPP

/*
 * The visualiser is the class that processes the audio stream into graphics in the OpenGL context.
 */

#include <memory>

class visualiser {
public:
    visualiser();
    ~visualiser();

    void resize(int width, int height);

    void update(double t, float dt);
    void draw();

protected:

private:
    struct state_t;
    std::unique_ptr<state_t> state_;
    state_t& s;
};

#endif //ZAPPLAYER_VISUALISER_HPP
