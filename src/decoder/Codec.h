#pragma once

#include "../Common.h"
#include <memory>

//forward declarations
struct AVCodecParameters;
struct AVCodecContext;
struct AVCodec;
struct AVFormatContext;

#include "FFMpegDeleter.h"

class Codec {
public:
    // all the logic is done here. This is called from DecoderCore when filling
    // codecs vector
    Codec() {};
    Codec (AVFormatContext& formatCtx, const unsigned int streamIdx);

    //We should never be able to modify the codec, so disable copying of all sorts
    Codec(const Codec&) = delete;
    Codec& operator=(const Codec&) = delete;

    Codec(Codec&&) = default;
    Codec& operator=(Codec&&) = default;

    //These are here if ever needed for access to codec information
    //we don't really abstract away ffmpeg, but it's not really the goal
    AVCodecParameters* GetCodecParams()  const { return _codecParams.get(); }
    AVCodecContext*    GetCodecContext() const { return _codecCtx.get();    }
    const AVCodec*     GetCodec()        const { return _codec.get();       }
    CodecType          GetCodecType()    const { return _codecType;         }
    unsigned int       GetCodecIndex()   const { return _codecIdx;          }

    int OpenCodec() const;       
    void ResetInternalState() {
        _codecParams.reset();
        _codecCtx.reset();
        _codec.reset();
    }

private:
    std::unique_ptr<AVCodecParameters, CustomDeleter> _codecParams{nullptr};
    std::unique_ptr<AVCodecContext, CustomDeleter> _codecCtx{nullptr};
    std::unique_ptr<const AVCodec, CustomDeleter> _codec{nullptr};
    CodecType _codecType;
    unsigned int _codecIdx;
};

