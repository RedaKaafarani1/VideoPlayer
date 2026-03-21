#pragma once

#include "PlaybackController.h"
#include "UI/Timeline.h"
#include "UI/Controls.h"
#include "Render/Render.h"
#include "Decoder/DecoderCore.h"
#include "UI/UICommon.h"

class App
{
public:
    using Clock = std::chrono::steady_clock;
    App() : _appRender(WIDTH, HEIGHT, UI::TIMELINE_SIZE), _decoder(_playerState) 
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
    void HandleUIStates() noexcept;
    void ReinitializeState() noexcept;

    std::atomic<DecoderState> _playerState = DecoderState::DecoderWaiting;
    Render _appRender;
    DecoderCore _decoder;
    std::unique_ptr<AVFrame, CustomDeleter> _lastFrame {nullptr};
    PlaybackController _playbackController;
    double _timeBase;
    double _totalVideoDuration;
    int _lastVideoProgressUpdateTime = -1;
    UI::Timeline _timeline;
    UI::Controls _controls;
};
