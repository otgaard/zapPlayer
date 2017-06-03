//
// Created by Darren Otgaar on 2016/11/28.
//

#ifndef ZAPPLAYER_CONTROLLER_STREAM_HPP
#define ZAPPLAYER_CONTROLLER_STREAM_HPP

#include <zapAudio/streams/audio_stream.hpp>
#include <zap/maths/maths.hpp>

template <typename SampleT>
class controller_stream : public audio_stream<SampleT> {
public:
    using stream_t = audio_stream<SampleT>;
    using buffer_t = typename stream_t::buffer_t;

    controller_stream(stream_t* input, size_t sample_rate, size_t channels, size_t frame_size) : stream_t(input),
        volume_(0.6f), sample_rate_(sample_rate), channels_(channels), frame_size_(frame_size) { }

    void set_volume(float v) { volume_ = zap::maths::clamp(v, 0.f, 1.f); }
    float get_volume(float v) const { return volume_; }

    virtual size_t read(buffer_t& buffer, size_t len) {
        auto ret = this->parent()->read(buffer, len);
        for(int i = 0; i != len; ++i) {
            buffer[i] = (SampleT)(std::round(buffer[i] * volume_));
        }
        return ret;
    }

    virtual size_t write(const buffer_t& buffer, size_t len) {
        return 0;
    }

private:
    float volume_;
    size_t sample_rate_;
    size_t channels_;
    size_t frame_size_;
};

#endif //ZAPPLAYER_CONTROLLER_STREAM_HPP
