#pragma once

#include "Codec.h"
#include "Common.h"
#include "Stream.h"

    class DecoderCore {
    public:
        DecoderCore(std::atomic<PlayerState>& playerState) : _state(playerState) {}
        ~DecoderCore() { if (swsCtx) sws_freeContext(swsCtx); };
        int openStream(const std::string& filename); 

        AVFormatContext* getFormatContext()         { return FormatContextPtr.get(); }
        const std::vector<Codec>& getCodecs() const { return codecs; } 
        const Stream& getStream()             const { return stream; }
        double getVideoTimeBase();

        void startDecoding() 
        {
            //run dedicated decoder thread
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
    std::atomic<PlayerState>& _state;

    SwsContext* swsCtx = nullptr;
    std::mutex queueMutex;
    std::condition_variable queueCondition;
    std::jthread decoderThread;
    std::queue<std::unique_ptr<AVFrame, CustomDeleter>> decodedRGBFrames;
};
