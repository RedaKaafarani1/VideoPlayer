#pragma once

#include "../render/Render.h"
#include "../decoder/DecoderCore.h"
#include "../constants.h"
#include "Common.h"
#include <libavutil/frame.h>

class App
{
public:
    App() : _appRender(WIDTH, HEIGHT) {}

    void RunAppLoop() noexcept;
    
    Render& GetAppRender() noexcept { return _appRender; }
    DecoderCore& GetDecoder() noexcept { return _decoder; }
private:
    void RunDecoder() noexcept;
    void update(const double& timeBase);

    Render _appRender;
    DecoderCore _decoder;
    std::unique_ptr<AVFrame, CustomDeleter> _lastFrame;
    std::string _videoFileName = "resources/example_video_720p.mp4";
    double playbackStartTime;
    bool playbackFinished = false;
};
