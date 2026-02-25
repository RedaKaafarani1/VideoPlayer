#include "App.h"
#include "Common.h"
#include <atomic>
#include <raylib.h>

int App::InitializeInternals() noexcept
{
    const auto& codec = _decoder.getCodecByType(CodecType::VideoCodec);
    if (!codec)
    {
        gLogger.error("Could not get Codec of type CodecType::VideoCodec");
        return -1;
    }

    //We assume codec context is available since the codec was returned
    const auto& codecCtx = codec->get().getCodecContext();
    gLogger.info("Video width = {}, Video height = {}", codecCtx->width, codecCtx->height);
    _appRender.InitializeFrameTexture(codecCtx->width, codecCtx->height);
    return 0;
}

int App::RunDecoder() noexcept
{
    if (_videoFileName.empty())
    {
        gLogger.error("No video file to open");
        return -1;
    }
    gLogger.info("Starting application");
    // TODO: this should be used to display error messages to user
    int ret = _decoder.openStream(_videoFileName); 
    if (ret!=0)
    {
        gLogger.error("Could not open video file {}", _videoFileName);
        return -1;
    }
    //separate thread for decoding
    gLogger.info("Starting decoding process in separate thread");
    _decoder.startDecoding();
    return 0;
}

void App::Update(const double& timeBase)
{
    PlayerState currState = _playerState.load(std::memory_order_acquire);
    if (!playbackFinished && !(currState == PlayerState::DecoderFailed))
    {
        //This waits on the frame
        auto frame = _decoder.getFrame();
        if (frame)
        {
            double currTime = std::chrono::duration<double>(Clock::now() - playbackStartTime).count();
            double frameTime = frame->pts * timeBase;
            double delay = frameTime - currTime;
            if (delay > 0)
            {
                // wait before displaying next frame
                std::this_thread::sleep_for(std::chrono::duration<double>(delay));
            }
            //conserve last frame to display it indefinitely at the end
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
        // This is used in case we reach end of video or we had an error, so
        // we display last frame or gray BG 
        if (_lastFrame)
            _appRender.DrawFrame(_lastFrame.get());
        else
            _appRender.DrawFrame(nullptr);
    }
}

void App::RunAppLoop() noexcept
{
    // Initializes raylib related stuff 
    _appRender.BeginRender();
     
    //TODO: this is temporary, we should be able to change video file
    if (!_videoFileName.empty())
    {
        // opens stream and runs the decoding loop in a separate thread
        int err = RunDecoder();
        if (err != 0)
            playbackFinished = true; //will display gray BG
        
        //Initialize renderer mostly for now
        err = InitializeInternals();
        if (err != 0)
            playbackFinished = true;
        else
         _appRender.AdjustRenderSize();
    }

    // time base used to calculate frame time for display
    playbackStartTime = Clock::now(); 
    double timeBase = _decoder.getVideoTimeBase(); 
    gLogger.debug("Video time base = {}", timeBase);


    while (!WindowShouldClose() && _appRender.IsStillRendering())
    {
        if (IsWindowResized())
            _appRender.UpdateRLWindowSize();
        Update(timeBase);
    }
    
    _appRender.EndRender();
}


