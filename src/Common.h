#pragma once

#include <cstdint>

constexpr const int WIDTH  = 1280;
constexpr const int HEIGHT = 720;

enum class CodecType {
    VideoCodec,
    AudioCodec
};

enum class PlayerState : uint8_t {
    None,
    DecoderFailed,
    DecoderLoading,
    DecoderReady,
    DecoderWaiting,
    DecoderDoneSeeking
};
