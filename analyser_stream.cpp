/* Created by Darren Otgaar on 2016/11/19. http://www.github.com/otgaard/zap */
#include <cmath>
#include <zap/maths/maths.hpp>
#include "analyser_stream.hpp"

#define LOGGING_ENABLED
#include <zap/tools/log.hpp>

/*
 * Note: This FFT implementation is based on the excellent FFT tutorial at:
 * http://blogs.zynaptiq.com/bernsee/dft-a-pied/
 */

constexpr float s16_inv = 1.f/std::numeric_limits<short>::max();

analyser_stream::analyser_stream(audio_stream<sample_t>* parent, size_t frame_size, size_t bins)
        : audio_stream<sample_t>(parent), frame_size_(frame_size), bins_(bins), transform_buffer_(frame_size_*2),
          bin_buffer_(bins_), prev_(frame_size_*2, 0.f), curr_(frame_size*2, 0.f) {
}

size_t analyser_stream::read(buffer_t& buffer, size_t len) {
    if(!parent()) return 0;

    size_t ret = parent()->read(buffer, len);
    if(len != 2*frame_size_) {
        LOG("Error, frame_size mismatch");
        return ret;
    }

    /*
    for(int i = 0; i != frame_size_; ++i) {
        auto idx = 2*i;
        transform_buffer_[idx] = buffer[idx] * s16_inv;
        transform_buffer_[idx+1] = 0.f;
    }

    fourier_transform(transform_buffer_, frame_size_, false);
    */

    process_samples(buffer);

    const float inv_transform_size = 1.f/transform_buffer_.size();

    {
        std::unique_lock<std::mutex> lock(bin_buffer_mtx_);
        for(int i = 0; i != bins_; ++i) {
            const auto idx = 2*i;
            float mag = 20.f * std::log10(2.f * inv_transform_size
                                              * std::sqrt(transform_buffer_[idx] * transform_buffer_[idx]
                                                          + transform_buffer_[idx+1] * transform_buffer_[idx+1]));
            bin_buffer_[i] = (zap::maths::clamp(mag, -100.f, 0.f) + 100.f)*0.01f;
        }
    }

    return ret;
}

size_t analyser_stream::write(const buffer_t& buffer, size_t len) {
    return 0;
}

void analyser_stream::process_samples(const buffer_t& samples) {
    const size_t sample_count = frame_size_*2;
    const size_t fft_window = 2 * sample_count;
    if(transform_buffer_.size() < fft_window) transform_buffer_.resize(fft_window);

    for(size_t i = 0; i < sample_count; ++i) {
        const auto idx = 2*i;
        if(i < frame_size_) {
            transform_buffer_[idx]     = prev_[idx] * hamming_window(i, sample_count);
            transform_buffer_[idx + 1] = 0;
        } else {
            const auto offset = idx - sample_count;
            transform_buffer_[idx]     = s16_inv*samples[offset] * hamming_window(i, sample_count);
            transform_buffer_[idx + 1] = 0;
            curr_[offset] = s16_inv*samples[offset];
        }
    }

    fourier_transform(transform_buffer_, sample_count, false);
    prev_ = curr_;
}

void analyser_stream::fourier_transform(fft_buffer_t& fft_buffer, int window, bool inverse) {
    int sign = inverse ? -1 : 1;
    float wr, wi, arg, temp;
    int p1, p2;
    float tr, ti, ur, ui;
    int p1r, p1i, p2r, p2i;
    int i, bitm, j, le, le2, k;
    int fftFrameSize2 = window * 2;

    for(i = 2; i < fftFrameSize2 - 2; i += 2) {
        for(bitm = 2, j = 0; bitm < fftFrameSize2; bitm <<= 1) {
            if((i & bitm) != 0) j++;
            j <<= 1;
        }
        if(i < j) {
            p1 = i; p2 = j;
            temp = fft_buffer[p1];
            fft_buffer[p1++] = fft_buffer[p2];
            fft_buffer[p2++] = temp;
            temp = fft_buffer[p1];
            fft_buffer[p1] = fft_buffer[p2];
            fft_buffer[p2] = temp;
        }
    }

    int kmax = (int)(std::log(window) / std::log(2.0) + 0.5);
    for (k = 0, le = 2; k < kmax; k++) {
        le <<= 1;
        le2 = le >> 1;
        ur = 1.0f;
        ui = 0.0f;
        arg = (float)(zap::maths::PI / (le2 >> 1));
        wr = std::cos(arg);
        wi = sign * std::sin(arg);
        for (j = 0; j < le2; j += 2) {
            p1r = j; p1i = p1r + 1;
            p2r = p1r + le2; p2i = p2r + 1;
            for (i = j; i < fftFrameSize2; i += le) {
                float p2rVal = fft_buffer[p2r];
                float p2iVal = fft_buffer[p2i];
                tr = p2rVal * ur - p2iVal * ui;
                ti = p2rVal * ui + p2iVal * ur;
                fft_buffer[p2r] = fft_buffer[p1r] - tr;
                fft_buffer[p2i] = fft_buffer[p1i] - ti;
                fft_buffer[p1r] += tr;
                fft_buffer[p1i] += ti;
                p1r += le;
                p1i += le;
                p2r += le;
                p2i += le;
            }
            tr = ur * wr - ui * wi;
            ui = ur * wi + ui * wr;
            ur = tr;
        }
    }
}