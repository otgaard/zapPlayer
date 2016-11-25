/* Created by Darren Otgaar on 2016/11/24. http://www.github.com/otgaard/zap */
#ifndef ZAPPLAYER_DIRECTORY_STREAM_HPP
#define ZAPPLAYER_DIRECTORY_STREAM_HPP

#include <array>
#include <queue>
#include <string>
#include "zapAudio/base/mp3_stream.hpp"

class directory_stream : public audio_stream<short> {
public:
    directory_stream(const std::string& path, size_t frame_size) : path_(path), frame_size_(frame_size) { }
    virtual ~directory_stream() { }

    bool start();

    virtual size_t read(buffer_t& buffer, size_t len);
    virtual size_t write(const buffer_t& buffer, size_t len);

private:
    std::string path_;
    size_t frame_size_;
    std::queue<std::string> file_queue_;
    std::array<std::unique_ptr<mp3_stream>, 2> file_streams_;
};

#endif //ZAPPLAYER_DIRECTORY_STREAM_HPP
