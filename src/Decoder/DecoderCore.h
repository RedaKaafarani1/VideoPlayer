#pragma once

#include "Codec.h"
#include "Stream.h"
#include <thread>
#include <condition_variable>
#include <queue>
#include <mutex>
#include <unordered_map>
#include <optional>
#include <atomic>

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
    DecoderCore() = default;
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

    std::unique_ptr<AVFrame, CustomDeleter> getFrame();
    std::optional<std::reference_wrapper<const Codec>> getCodecByType(CodecType codecType) const;
    int64_t getFirstFramePTS() const noexcept { return _firstFramePTS.load(std::memory_order_acquire); }
    double getVideoTimeBase() const;
    double getVideoDurationSeconds() const;
    void pushCommand(const DecoderCommand& decoderCommand);

    DecoderState getState() const noexcept { return _state.load(std::memory_order_acquire); }
    void clearState() noexcept { _state.store(DecoderState::None, std::memory_order_release); }


private:
    int openStream(const std::string& filename); 
    int readFrame(AVPacket& packet, const unsigned int codecIdx, AVCodecContext& codecCtx) const;
    AVFrame* decodeNextFrame(const Codec& codec);
    void setCodecParameters();
    void runDecoderLoop(std::stop_token stopToken);
    void handleSeek(const int64_t seekPTS) noexcept;
    void handleNewVideoFile(const std::string& filename);
    void computeFrameDuration(const Codec& codec) noexcept;
    AVFrame* convertFrameToRGB(const AVFrame* const yuvFrame);

    std::unique_ptr<AVFormatContext, CustomDeleter> _formatContextPtr{nullptr};
    std::unordered_map<CodecType, Codec> _codecsMap;
    Stream _stream;
    bool doneDecoding = false;
    std::atomic<int64_t> _firstFramePTS{ AV_NOPTS_VALUE };
    int64_t _frameDuration;
    int64_t _lastPTS = 0;
    std::atomic<DecoderState> _state{DecoderState::DecoderWaiting};
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
