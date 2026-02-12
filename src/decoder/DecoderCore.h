#pragma once

#include "Codec.h"
#include "Stream.h"

class DecoderCore {
public:
    void openStream(std::string filename); 

    AVFormatContext* getFormatContext()         { return FormatContextPtr.get(); }
    const std::vector<Codec>& getCodecs() const { return codecs; } 
    const Stream& getStream()             const { return stream; }

    AVFrame* decodeNextFrame(const Codec& codec);
    
    std::optional<std::reference_wrapper<const Codec>> getCodecByType(CodecType codecType) const;

private:
    int readFrame(AVPacket& packet, const unsigned int codecIdx, AVCodecContext& codecCtx) const;
    void setCodecParameters();

    std::unique_ptr<AVFormatContext, CustomDeleter> FormatContextPtr{nullptr};
    std::vector<Codec> codecs;
    Stream stream{};
    bool doneDecoding = false;

};
