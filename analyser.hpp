/* Created by Darren Otgaar on 2016/12/04. http://www.github.com/otgaard/zap */
#ifndef ZAPPLAYER_ANALYSER_HPP
#define ZAPPLAYER_ANALYSER_HPP

#include <memory>

/*
 * The spectral analyser.
 *
 * The following outputs are hoped for:
 *   a) General Pitch where possible
 *   b) The underlying beat
 *   c) Volume level
 *   d) Isolated signals (outliers)
 *
 * These are then mapped as parameters to the modules which can use them as sources for
 * random generation.
 *
 * General Process:
 *
 * 1) Converts an audio sample window into the frequency domain (using FFT)
 * 2) Applies any number of filters to the FFT window (low-pass, noise reduction, convolution)
 * 3) Builds several models for pitch detection:
 *    a) Autocorrelation
 *    b) Cepstral
 *    c) Wavelets
 * 4) Beat detection
 *    a) Envelope extraction
 *    b) Comb-filter bank
 *    c) Correlation
 * 5) Determine over-all volume level, noise, bands, envelope, etc.
 *
 * Soft real-time.
 *
 */

class analyser {
public:
    analyser();
    ~analyser();

protected:

private:
    struct state_t;
    std::unique_ptr<state_t> state_;
    state_t& s;
};

#endif //ZAPPLAYER_ANALYSER_HPP
