#pragma once

#include "Codec.h"
#include "Stream.h"
#include <thread>
#include <condition_variable>
#include <queue>
#include <mutex>
#include <unordered_map>

extern "C" {
#include <libswscale/swscale.h>
}

enum class DecoderCommandType {
    None,
    DecodeVideo,
    Stop,
    Seek,
};

struct DecoderCommand {
    DecoderCommandType type;
    std::string videoFilename;
    int64_t seekPTS = 0;
};

class DecoderCore {
public:
    DecoderCore(std::atomic<PlayerState>& playerState) : _state(playerState) {}
    ~DecoderCore() { 
        if (swsCtx) sws_freeContext(swsCtx);
        // object is being destroyed, notify all conditions so the thread can stop
        stopRequested.store(true, std::memory_order_release);
        queueCondition.notify_all();
        controlCondition.notify_all();
    };

    AVFormatContext* getFormatContext() const   { return FormatContextPtr.get(); }
    double getVideoTimeBase() const;

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
    void changeVideoFile(const std::string& filename);
    AVFrame* decodeNextFrame(const Codec& codec);
    AVFrame* convertFrameToRGB(const AVFrame* const yuvFrame);
    std::unique_ptr<AVFrame, CustomDeleter> getFrame();
    std::optional<std::reference_wrapper<const Codec>> getCodecByType(CodecType codecType) const;
    void seekFrame(int64_t pts) noexcept;
    int64_t getFirstFramePTS() const noexcept { return _firstFramePTS; }
    void pushCommand(const DecoderCommand& decoderCommand);

private:
    int readFrame(AVPacket& packet, const unsigned int codecIdx, AVCodecContext& codecCtx) const;
    void setCodecParameters();
    void decodeVideoStream(std::stop_token stopToken);

    std::unique_ptr<AVFormatContext, CustomDeleter> FormatContextPtr{nullptr};
    std::unordered_map<CodecType, Codec> codecsMap;
    Stream stream{};
    bool doneDecoding = false;
    int64_t _firstFramePTS{ AV_NOPTS_VALUE };
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
    std::mutex commandMutex;
    std::condition_variable commandCondition;
    std::queue<DecoderCommand> commandQueue;
};
