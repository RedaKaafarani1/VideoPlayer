#pragma once

#include "Codec.h"
#include "Common.h"
#include "Stream.h"
#include <stop_token>

    class DecoderCore {
    public:
        ~DecoderCore() { if (swsCtx) sws_freeContext(swsCtx); };
        int openStream(const std::string& filename); 

        AVFormatContext* getFormatContext()         { return FormatContextPtr.get(); }
        const std::vector<Codec>& getCodecs() const { return codecs; } 
        const Stream& getStream()             const { return stream; }
        double getVideoTimeBase();

        void startDecoding() 
        { 
            decoderThread = std::jthread(
                    [this](std::stop_token stopToken)
                    {
                        decodeVideoStream(stopToken);
                    }
            );
        }
    AVFrame* decodeNextFrame(const Codec& codec);
    AVFrame* convertFrameToRGB(const AVFrame* const yuvFrame);
    std::unique_ptr<AVFrame, CustomDeleter> getFrame();
    
    std::optional<std::reference_wrapper<const Codec>> getCodecByType(CodecType codecType) const;

private:
    int readFrame(AVPacket& packet, const unsigned int codecIdx, AVCodecContext& codecCtx) const;
    void setCodecParameters();
    void decodeVideoStream(std::stop_token stopToken);

    std::unique_ptr<AVFormatContext, CustomDeleter> FormatContextPtr{nullptr};
    std::vector<Codec> codecs;
    Stream stream{};
    bool doneDecoding = false;

    SwsContext* swsCtx = nullptr;
    std::mutex queueMutex;
    std::condition_variable queueCondition;
    std::jthread decoderThread;
    std::queue<std::unique_ptr<AVFrame, CustomDeleter>> decodedRGBFrames;
};
