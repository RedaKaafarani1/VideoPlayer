#include "Codec.h"
#include "Common.h"
#include "../Logger/GLogger.h"

Codec::Codec(AVFormatContext& formatCtx, const unsigned int streamIdx)
{
    gLogger.info("Initializing codec with index {}", streamIdx);
    auto stream = formatCtx.streams[streamIdx];
   
   _codecParams.reset(avcodec_parameters_alloc());
    if (!_codecParams)
    {
        gLogger.error("Could not allocate codec parameters for codec with index {}", streamIdx);
        throw std::runtime_error("Could not allocate codec parameters");
    }
    if (avcodec_parameters_copy(_codecParams.get(), stream->codecpar) < 0)
    {
        gLogger.error("Could not copy parameters for codec with index {}", streamIdx);
        throw std::runtime_error("Could not copy coded parameters");
    }
    
    //TODO: add support for more types
    if (_codecParams->codec_type == AVMEDIA_TYPE_VIDEO)
    {
        _codecType = CodecType::VideoCodec;
        gLogger.info("Video Codec: resolution {}x{}", _codecParams->width, _codecParams->height);
    }
    else if (_codecParams->codec_type == AVMEDIA_TYPE_AUDIO)
    {
        _codecType = CodecType::AudioCodec;
        gLogger.info("Audio Codec: {} channels, sample rate {}", _codecParams->ch_layout.nb_channels, _codecParams->sample_rate);
    }
    _codecIdx = streamIdx;

    const AVCodec* avCodec = avcodec_find_decoder(_codecParams.get()->codec_id);
    if (!avCodec)
    {
        gLogger.error("Could not find codec for given codec id {}", static_cast<int>(_codecParams.get()->codec_id));
        throw std::runtime_error("Could not find codec for given parameters");
    }
    _codec.reset(avCodec);

    AVCodecContext* codecContext = avcodec_alloc_context3(_codec.get());
    if (!codecContext) 
    {
        gLogger.error("Could not allocate codec context for codec with index {}", streamIdx);
        throw std::runtime_error("Could not allocate codec context");
    }
    _codecCtx.reset(codecContext);

    if (avcodec_parameters_to_context(_codecCtx.get(), _codecParams.get()) < 0)
    {
        gLogger.error("Could not get context from parameters for codec with index {}", streamIdx);
        throw std::runtime_error("Could not get context from parameters");
    }
    gLogger.info("Codec {} ID {} bitrate {}", _codec->long_name, static_cast<int>(_codec->id), _codecParams->bit_rate); 
    gLogger.info("Finalized setting up codec with index {}", streamIdx);
}

int Codec::OpenCodec() const
{
    gLogger.info("Opening codec with index {}", _codecIdx);
    return avcodec_open2(_codecCtx.get(), _codec.get(), nullptr);
}
