/* Created by Darren Otgaar on 2016/11/24. http://www.github.com/otgaard/zap */
#include "directory_stream.hpp"
#include "tools/os.hpp"
#include <regex>
#define LOGGING_ENABLED
#include <tools/log.hpp>

bool directory_stream::start() {
    static const std::regex mp3_substr(".mp3$|.MP3$");
    if(zap::is_dir(path_)) {
        auto files = zap::get_files(path_);
        for(const auto& file : files) {
            LOG(file);
            file_queue_.push(file);
        }
    }

    if(file_queue_.size() > 1) {
        file_streams_[0] = std::make_unique<mp3_stream>(file_queue_.front(), frame_size_, nullptr);
        file_streams_[0]->start();
        file_queue_.pop();
    }

    if(file_queue_.size() > 1) {
        file_streams_[1] = std::make_unique<mp3_stream>(file_queue_.front(), frame_size_, nullptr);
        file_streams_[1]->start();
        file_queue_.pop();
    }

    return true;
}

size_t directory_stream::read(buffer_t& buffer, size_t len) {
    if(file_streams_[0]) {
        auto ret = file_streams_[0]->read(buffer, len);

        if(skip_track_) {
            ret = 0;
            skip_track_ = false;
        }

        if(ret < len) {
            // This stream is exhausted.  Continue on the next
            file_streams_[0].swap(file_streams_[1]);
            std::vector<short> remainder(len - ret);
            auto rem = file_streams_[0]->read(remainder, len - ret);

            if(file_queue_.size() > 1) {
                file_streams_[1] = std::make_unique<mp3_stream>(file_queue_.front(), frame_size_, nullptr);
                file_streams_[1]->start();
                file_queue_.pop();
            }

            std::copy(remainder.begin(), remainder.end(), buffer.begin()+ret);
            return ret + rem;
        }
        return ret;
    }

    return 0;
}

size_t directory_stream::write(const buffer_t& buffer, size_t len) {
    return 0;
}
