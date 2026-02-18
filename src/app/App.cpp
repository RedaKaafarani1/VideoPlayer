#include "App.h"
#include <ctime>

void App::RunDecoder() noexcept
{
    if (_videoFileName.empty())
    {
        gLogger.error("No video file to open");
        return;
    }
    gLogger.info("Starting application");
    // TODO: this should be used to display error messages to user
    int ret = _decoder.openStream(_videoFileName); 
    if (ret!=0)
    {
        gLogger.error("Could not open video file {}", _videoFileName);
        return;
    }
    //separate thread for decoding
    _decoder.startDecoding();
}

void App::update(const double& timeBase)
{
    double currTime = GetTime() - playbackStartTime; 
    
    if (!playbackFinished)
    {
        //This waits on the frame
        auto frame = _decoder.getFrame();
        if (frame)
        {
            double frameTime = frame->pts * timeBase;
            double delay = frameTime - currTime;
            if (delay > 0)
            {
                struct timespec ts{0, (long)(delay * 1e9)};
                nanosleep(&ts, nullptr);
            }
            _lastFrame = std::move(frame);
            _appRender.DrawFrame(_lastFrame.get());
        }
        else {
            // we are done decoding
            playbackFinished = true;
            gLogger.info("Playback finished");
        }
    }
    else
    {
        if (_lastFrame)
            _appRender.DrawFrame(_lastFrame.get());
    }
}

void App::RunAppLoop() noexcept
{
    _appRender.BeginRender();
    if (!_videoFileName.empty())
        RunDecoder();

    playbackStartTime = GetTime();
    double timeBase = _decoder.getVideoTimeBase(); 
    while (!WindowShouldClose() && _appRender.IsStillRendering())
    {
        update(timeBase);
    }
    
    _appRender.EndRender();
}
