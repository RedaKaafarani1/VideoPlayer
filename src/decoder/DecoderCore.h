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
    std::string videoFilename; //only used with DecodeVideo
    int64_t seekPTS = 0; //only used with Seek
};

class DecoderCore {
public:
    DecoderCore(std::atomic<PlayerState>& playerState) : _state(playerState) {}
    ~DecoderCore() { 
        if (_swsCtx) sws_freeContext(_swsCtx);
        // object is being destroyed, notify all conditions so the thread can stop
        pushCommand({DecoderCommandType::Stop, "", 0});
        _commandCondition.notify_all();
        _queueCondition.notify_all();
    };

    void startDecoderThread() 
    {
        //run dedicated decoder thread
        _decoderThread = std::jthread(
                [this](std::stop_token stopToken)
                {
                    runDecoderLoop(stopToken);
                }
        );
    }

    AVFrame* convertFrameToRGB(const AVFrame* const yuvFrame);
    std::unique_ptr<AVFrame, CustomDeleter> getFrame();
    std::optional<std::reference_wrapper<const Codec>> getCodecByType(CodecType codecType) const;
    int64_t getFirstFramePTS() const noexcept { return _firstFramePTS; }
    double getVideoTimeBase() const;
    void pushCommand(const DecoderCommand& decoderCommand);

private:
    int openStream(const std::string& filename); 
    int readFrame(AVPacket& packet, const unsigned int codecIdx, AVCodecContext& codecCtx) const;
    AVFrame* decodeNextFrame(const Codec& codec);
    void setCodecParameters();
    void runDecoderLoop(std::stop_token stopToken);
    void handleSeek(const int seekPTS) noexcept;
    void handleNewVideoFile(const std::string& filename);

    std::unique_ptr<AVFormatContext, CustomDeleter> FormatContextPtr{nullptr};
    std::unordered_map<CodecType, Codec> codecsMap;
    Stream stream;
    bool doneDecoding = false;
    int64_t _firstFramePTS{ AV_NOPTS_VALUE };
    std::atomic<PlayerState>& _state;
    std::jthread _decoderThread;
    SwsContext* _swsCtx = nullptr;

    // variables for decoded RGB frames control
    std::mutex _queueMutex;
    std::condition_variable _queueCondition;
    std::queue<std::unique_ptr<AVFrame, CustomDeleter>> _decodedRGBFrames;

    // variables for decoder command control
    std::mutex _commandMutex;
    std::condition_variable _commandCondition;
    std::queue<DecoderCommand> _commandQueue;
};
