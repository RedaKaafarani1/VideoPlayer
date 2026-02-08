#pragma once

#include <algorithm>
#include <memory>
#include <string>
#include <stdexcept>
#include <type_traits>
#include <vector>
#include <iostream>

#include "logger/Logger.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavcodec/packet.h>
#include <libavcodec/codec_par.h>
#include <libavformat/avformat.h>
}

inline Logger& gLogger = Logger::instance("videoPlayer.log");

//This is verbose, yes, but I would like to simplify working with ffmpeg api
struct CustomDeleter {
    template <typename T>
    void operator() (T* ctx) const {
        if (ctx) {
            if constexpr (std::is_same_v<T, AVFormatContext>)
                avformat_close_input(&ctx); //we free it this way since avformat_open_input is used 
            if constexpr (std::is_same_v<T, AVCodecParameters>)
                avcodec_parameters_free(&ctx);
            if constexpr (std::is_same_v<T, AVCodec>) { /*do nothing*/ }
            if constexpr (std::is_same_v<T, AVCodecContext>)
                avcodec_free_context(&ctx);
            if constexpr (std::is_same_v<T, AVPacket>)
                av_packet_free(&ctx);
            if constexpr (std::is_same_v<T, AVFrame>)
                av_frame_free(&ctx);
        }
    }
};

enum class CodecType {
    VideoCodec,
    AudioCodec
};
