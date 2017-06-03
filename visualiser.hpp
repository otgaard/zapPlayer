#ifndef ZAPPLAYER_VISUALISER_HPP
#define ZAPPLAYER_VISUALISER_HPP

/*
 * The visualiser is the class that processes the audio stream into graphics in the OpenGL context.
 */

#include <memory>
#include <vector>

class visualiser {
public:
    visualiser(size_t bins);
    ~visualiser();

    bool initialise();
    bool is_initialised() const;

    std::vector<std::string> get_visualisations() const;
    void set_visualisation(const std::string& name);

    void set_frequency_bins(const std::vector<float>& bins);

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
