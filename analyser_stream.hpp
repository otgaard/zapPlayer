/* Created by Darren Otgaar on 2016/11/19. http://www.github.com/otgaard/zap */
#ifndef ZAPPLAYER_ANALYSER_STREAM_HPP
#define ZAPPLAYER_ANALYSER_STREAM_HPP

/*
 * Implements the spectral analyser stream.  The stream is plugged into the playback stream just before the data
 * is sent to the audio device.  This allows the current frame to be synced with the FFT for that frame.  It may be
 * necessary to build a delay line to sync the FFT with the audio output as the output may be a frame or two behind.
 */

#include <zapAudio/base/audio_stream.hpp>
#include <zap/maths/maths.hpp>
#include <mutex>

class analyser_stream : public audio_stream<short> {
public:
    using sample_t = short;
    using buffer_t = typename audio_stream<sample_t>::buffer_t;
    using fft_buffer_t = std::vector<float>;

    analyser_stream(audio_stream<sample_t>* parent, size_t frame_size=512, size_t bins=128);
    virtual ~analyser_stream() = default;

    virtual size_t read(buffer_t& buffer, size_t len);
    virtual size_t write(const buffer_t& buffer, size_t len);

    size_t copy_bins(fft_buffer_t& output, size_t bins) {
        std::unique_lock<std::mutex> lock(bin_buffer_mtx_);
        size_t size = std::min(bins_, bins);
        if(output.size() != bins) output.resize(bins);
        std::copy(bin_buffer_.begin(), bin_buffer_.begin()+size, output.begin());
        return size;
    }

protected:
    void process_samples(const  buffer_t& samples);

    inline float hamming_window(size_t n, size_t N) {
        return 0.54f - 0.46f * std::sin(2.0f * (float)zap::maths::TWO_PI * n)/(N - 1);
    }

    size_t frame_size_;
    size_t bins_;
    fft_buffer_t transform_buffer_;
    fft_buffer_t bin_buffer_;
    std::mutex bin_buffer_mtx_;
    fft_buffer_t prev_;
    fft_buffer_t curr_;
    fft_buffer_t smoothing_;

    void fourier_transform(fft_buffer_t& fft_buffer, int window, bool inverse);

private:

};


#endif //ZAPPLAYER_ANALYSER_STREAM_HPP
