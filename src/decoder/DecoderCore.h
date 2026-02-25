#pragma once

#include "Codec.h"
#include "Common.h"
#include "Stream.h"

class DecoderCore {
public:
    DecoderCore(std::atomic<PlayerState>& playerState) : _state(playerState) {}
    ~DecoderCore() { 
        stopRequested.store(true, std::memory_order_release);
        queueCondition.notify_all();
        controlCondition.notify_all();
        if (swsCtx) sws_freeContext(swsCtx);
    };

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

    int openStream(const std::string& filename); 
    AVFrame* decodeNextFrame(const Codec& codec);
    AVFrame* convertFrameToRGB(const AVFrame* const yuvFrame);
    std::unique_ptr<AVFrame, CustomDeleter> getFrame();
    std::optional<std::reference_wrapper<const Codec>> getCodecByType(CodecType codecType) const;
    void seekFrame(int64_t pts) noexcept;

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

    std::atomic<bool> seekRequested{false};
    std::atomic<bool> stopRequested{false};
    int64_t seekPTS;
    std::mutex controlMutex;
    std::condition_variable controlCondition;
};
