#pragma once

#include <type_traits>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

struct CustomDeleter {
    template <typename T>
    void operator() (T* ctx) const noexcept {
        if (ctx) { //ffmpeg normally checks if null anyway
            if constexpr (std::is_same_v<T, AVFormatContext>)
                avformat_close_input(&ctx); //we free it this way since avformat_open_input is used 
            else if constexpr (std::is_same_v<T, AVCodecParameters>)
                avcodec_parameters_free(&ctx);
            else if constexpr (std::is_same_v<T, AVCodec>) { /*do nothing*/ }
            else if constexpr (std::is_same_v<T, AVCodecContext>)
                avcodec_free_context(&ctx);
            else if constexpr (std::is_same_v<T, AVPacket>)
                av_packet_free(&ctx);
            else if constexpr (std::is_same_v<T, AVFrame>)
                av_frame_free(&ctx);
        }
    }
};
