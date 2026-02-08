#pragma once

#include "Codec.h"
#include "Stream.h"

class DecoderCore {
public:
    void openStream(std::string filename); 
    void setCodecParameters();

    AVFormatContext*   getFormatContext() { return FormatContextPtr.get(); }
    const std::vector<Codec>& getCodecs() const { return codecs; } 
    const Stream& getStream()             const { return stream; }

private:
    std::unique_ptr<AVFormatContext, CustomDeleter> FormatContextPtr{nullptr};
    std::vector<Codec> codecs;
    Stream stream; 
};
