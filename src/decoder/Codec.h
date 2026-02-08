#pragma once

#include "../Common.h"

class Codec {
public:
    Codec (AVFormatContext& formatCtx, const unsigned int streamIdx);

    Codec(const Codec&) = delete;
    Codec& operator=(const Codec&) = delete;

    Codec(Codec&&) = default;
    Codec& operator=(Codec&&) = default;

    AVCodecParameters* getCodecParams()  const { return _codecParams.get(); }
    AVCodecContext*    getCodecContext() const { return _codecCtx.get();    }
    const AVCodec*     getCodec()        const { return _codec.get();       }
    CodecType          getCodecType()    const { return _codecType;         }
    unsigned int       getCodecIndex()   const { return _codecIdx;          }

    int openCodec() const;       

private:
    std::unique_ptr<AVCodecParameters, CustomDeleter> _codecParams{nullptr};
    std::unique_ptr<AVCodecContext, CustomDeleter> _codecCtx{nullptr};
    std::unique_ptr<const AVCodec, CustomDeleter> _codec{nullptr};
    CodecType _codecType;
    unsigned int _codecIdx;
};

