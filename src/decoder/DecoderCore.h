#pragma once

#include "Codec.h"
#include "Stream.h"
#include <thread>
#include <condition_variable>
#include <queue>
#include <mutex>
#include <unordered_map>
#include <optional>

extern "C" {
#include <libswscale/swscale.h>
}

enum class DecoderCommandType {
    None,
    DecodeVideo,
    Stop,
    Seek,
    Wait
};

struct DecoderCommand {
    DecoderCommandType type = DecoderCommandType::None;
    std::string videoFilename;
    int64_t seekPTS = 0;
};

class DecoderCore {
public:
    DecoderCore(std::atomic<PlayerState>& playerState) : _state(playerState) {}
    ~DecoderCore() { 
        if (swsCtx) sws_freeContext(swsCtx);
        // object is being destroyed, notify all conditions so the thread can stop
        pushCommand({DecoderCommandType::Stop, "", 0});
        commandCondition.notify_all();
        queueCondition.notify_all();
    };

    AVFormatContext* getFormatContext() const   { return FormatContextPtr.get(); }
    double getVideoTimeBase() const;

    void startDecoderThread() 
    {
        //run dedicated decoder thread
        decoderThread = std::jthread(
                [this](std::stop_token stopToken)
                {
                    runDecoderLoop(stopToken);
                }
        );
    }

    int openStream(const std::string& filename); 
    void handleNewVideoFile(const std::string& filename);
    AVFrame* decodeNextFrame(const Codec& codec);
    AVFrame* convertFrameToRGB(const AVFrame* const yuvFrame);
    std::unique_ptr<AVFrame, CustomDeleter> getFrame();
    std::optional<std::reference_wrapper<const Codec>> getCodecByType(CodecType codecType) const;
    int64_t getFirstFramePTS() const noexcept { return _firstFramePTS; }
    void pushCommand(const DecoderCommand& decoderCommand);

private:
    int readFrame(AVPacket& packet, const unsigned int codecIdx, AVCodecContext& codecCtx) const;
    void setCodecParameters();
    void runDecoderLoop(std::stop_token stopToken);
    void handleSeek(const int seekPTS) noexcept;

    std::unique_ptr<AVFormatContext, CustomDeleter> FormatContextPtr{nullptr};
    std::unordered_map<CodecType, Codec> codecsMap;
    Stream stream;
    bool doneDecoding = false;
    int64_t _firstFramePTS{ AV_NOPTS_VALUE };
    std::atomic<PlayerState>& _state;

    SwsContext* swsCtx = nullptr;
    std::mutex queueMutex;
    std::condition_variable queueCondition;
    std::queue<std::unique_ptr<AVFrame, CustomDeleter>> decodedRGBFrames;
    std::jthread decoderThread;

    std::mutex commandMutex;
    std::condition_variable commandCondition;
    std::queue<DecoderCommand> commandQueue;
};
