#pragma once

#include "../render/Render.h"
#include "../decoder/DecoderCore.h"
#include "../constants.h"

class App
{
public:
    using Clock = std::chrono::steady_clock;
    App() : _appRender(WIDTH, HEIGHT) {}

    void RunAppLoop() noexcept;
    int InitializeInternals() noexcept;
    
    Render& GetAppRender() noexcept { return _appRender; }
    DecoderCore& GetDecoder() noexcept { return _decoder; }
private:
    int RunDecoder() noexcept;
    void Update(const double& timeBase);

    Render _appRender;
    DecoderCore _decoder;
    std::unique_ptr<AVFrame, CustomDeleter> _lastFrame {nullptr};
    std::string _videoFileName = "resources/example_video_720p.mp4";
    Clock::time_point playbackStartTime;
    bool playbackFinished = false;
};
