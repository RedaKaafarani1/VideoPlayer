#pragma once

#include "Codec.h"
#include "Stream.h"
#include <libswscale/swscale.h>

class DecoderCore {
public:
    ~DecoderCore() { if (swsCtx) sws_freeContext(swsCtx); };
    void openStream(std::string filename); 

    AVFormatContext* getFormatContext()         { return FormatContextPtr.get(); }
    const std::vector<Codec>& getCodecs() const { return codecs; } 
    const Stream& getStream()             const { return stream; }

    AVFrame* decodeNextFrame(const Codec& codec);
    AVFrame* convertFrameToRGB(const AVFrame* const yuvFrame);
    
    std::optional<std::reference_wrapper<const Codec>> getCodecByType(CodecType codecType) const;

private:
    int readFrame(AVPacket& packet, const unsigned int codecIdx, AVCodecContext& codecCtx) const;
    void setCodecParameters();

    std::unique_ptr<AVFormatContext, CustomDeleter> FormatContextPtr{nullptr};
    std::vector<Codec> codecs;
    Stream stream{};
    bool doneDecoding = false;

    SwsContext* swsCtx = nullptr;
};
