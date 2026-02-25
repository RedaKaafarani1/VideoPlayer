#pragma once

#include "../render/Render.h"
#include "../decoder/DecoderCore.h"
#include "../constants.h"
#include "Common.h"

class App
{
public:
    using Clock = std::chrono::steady_clock;
    App() : _appRender(WIDTH, HEIGHT), _decoder(_playerState) {}

    void RunAppLoop() noexcept;
    int InitializeInternals() noexcept;
    
    Render& GetAppRender() noexcept { return _appRender; }
    DecoderCore& GetDecoder() noexcept { return _decoder; }
private:
    int RunDecoder() noexcept;
    void Update(const double& timeBase);

    std::atomic<PlayerState> _playerState = PlayerState::None;
    Render _appRender;
    DecoderCore _decoder;
    std::unique_ptr<AVFrame, CustomDeleter> _lastFrame {nullptr};
    std::string _videoFileName = "resources/bbb_1080p.mp4";
    Clock::time_point playbackStartTime;
    bool playbackFinished = false;
};
