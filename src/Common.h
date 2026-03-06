#pragma once

#include <cstdint>

constexpr int WIDTH  = 1280;
constexpr int HEIGHT = 720;

enum class CodecType {
    VideoCodec,
    AudioCodec
};

enum class DecoderState : uint8_t {
    None,
    DecoderFailed,
    DecoderLoading,
    DecoderReady,
    DecoderWaiting,
    DecoderDoneSeeking
};
