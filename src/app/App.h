#pragma once

#include "PlaybackController.h"
#include "../render/Render.h"
#include "../decoder/DecoderCore.h"

class App
{
public:
    using Clock = std::chrono::steady_clock;
    App() : _appRender(WIDTH, HEIGHT), _decoder(_playerState) 
    {
        _decoder.pushCommand({DecoderCommandType::Wait, "", 0});
    }

    void RunAppLoop() noexcept;
    int InitializeInternals() noexcept;
    
    Render& GetAppRender() noexcept { return _appRender; }
    DecoderCore& GetDecoder() noexcept { return _decoder; }
private:
    void Update(const double& timeBase);
    void HandleVideoFileChange() noexcept;
    void ReinitializeState() noexcept;

    std::atomic<DecoderState> _playerState = DecoderState::DecoderWaiting;
    Render _appRender;
    DecoderCore _decoder;
    std::unique_ptr<AVFrame, CustomDeleter> _lastFrame {nullptr};
    PlaybackController _playbackController;
    double _timeBase;
};
