#include "App.h"

int App::InitializeInternals() noexcept
{
    const auto& codec = _decoder.getCodecByType(CodecType::VideoCodec);
    if (!codec)
    {
        gLogger.error("Could not get Codec of type CodecType::VideoCodec");
        return -1;
    }

    //We assume codec context is available since the codec was returned
    const auto& codecCtx = codec->get().GetCodecContext();
    gLogger.info("Video width = {}, Video height = {}", codecCtx->width, codecCtx->height);
    _appRender.InitializeFrameTexture(codecCtx->width, codecCtx->height);
    return 0;
}

void App::Update(const double& timeBase)
{
    PlayerState currState = _playerState.load(std::memory_order_acquire);
    if (currState == PlayerState::DecoderDoneSeeking)
    {
        //consume state
        _playerState.store(PlayerState::None, std::memory_order_release);
        _playbackController.ResetInternalState();
        _playbackController.StartPlaybackTimer();
    }
    if (currState == PlayerState::DecoderLoading ||
        currState == PlayerState::DecoderWaiting)
    {
        //Decoder is re-initializing after adding video file
        //or the app just started, the state will change on
        //decoder side to DecoderReady
        _appRender.DrawFrame(nullptr);
        return;
    }

    if (currState == PlayerState::DecoderReady)
    {
        //We just added a video file and the decoder 
        //has started the decoding process
        gLogger.info("Decoder is ready");
        ReinitializeState();
    }

    if (!_playbackController.IsPlaybackFinished() && !_playbackController.IsPlaybackPaused() && !(currState == PlayerState::DecoderFailed))
    {
        //This waits on the frame
        auto frame = _decoder.getFrame();
        //after getFrame, we are sure the first frame pts is set (if all is good), we can safely use it
        auto firstFramePTS = _decoder.getFirstFramePTS();
        if (frame)
        {
            // This function accounts for paused time
            double currTime = _playbackController.GetEffectivePlaybackTime(); 
            //ensure we have a valid timestamp if possible
            auto currPTS = (frame->pts != AV_NOPTS_VALUE) ? frame->pts : frame->best_effort_timestamp;
            // calculate frame time based on offset from first frame pts, in case it's not equal to 0
            double frameTime = (currPTS - firstFramePTS) * timeBase;
            double delay = frameTime - currTime;
            if (delay > 0)
            {
                // wait before displaying next frame
                std::this_thread::sleep_for(std::chrono::duration<double>(delay));
            }
            //conserve last frame to display it indefinitely at the end
            _lastFrame = std::move(frame);
            _appRender.DrawFrame(_lastFrame.get()->data[0]);
        }
        else {
            //Playback is done
            _playbackController.SetPlaybackFinished(true); 
            gLogger.info("Playback finished");
        }
    }
    else
    {
        // This is used in case we reach end of video or we had an error, so
        // we display last frame or gray BG 
        if (_lastFrame)
            _appRender.DrawFrame(_lastFrame.get()->data[0]);
        else
            _appRender.DrawFrame(nullptr);
    }
}

void App::RunAppLoop() noexcept
{
    // Initializes raylib related stuff 
    _appRender.BeginRender();

    gLogger.info("Starting decoding process in separate thread");
    _decoder.startDecoderThread(); 
    //start the decoder in waiting state

    while (!WindowShouldClose() && _appRender.IsStillRendering())
    {
        HandleVideoFileChange();
        if (IsWindowResized())
            _appRender.UpdateRLWindowSize();
        if (IsKeyPressed(KEY_R))
        {
            //pause video while waiting for seek
            _playbackController.TogglePause();
            _decoder.pushCommand({DecoderCommandType::Seek, "", 0});
        }
        // if playback is finished, logically we cannot pause
        if (IsKeyPressed(KEY_SPACE) && !_playbackController.IsPlaybackFinished())
            _playbackController.TogglePause();
        Update(_timeBase);
    }
    
    _appRender.EndRender();
}

void App::HandleVideoFileChange() noexcept
{
   if (IsFileDropped())
   {
        auto filePathList = LoadDroppedFiles();
        if (filePathList.count <= 0) return;
        //we take only the first file if there are multiple ones. 
        //we only decode one video and display it
        std::string videoFileName = filePathList.paths[0];
        gLogger.info("New video file dropped: ", videoFileName);
        //we assume the dropped file is a video file, ffmpeg will tell us anyway
        _playerState.store(PlayerState::DecoderLoading, std::memory_order_release);
        _decoder.pushCommand({DecoderCommandType::DecodeVideo, videoFileName, 0});
        UnloadDroppedFiles(filePathList);
   }
}

void App::ReinitializeState() noexcept
{
    _playbackController.ResetInternalState();
    int err = InitializeInternals();
    if (err != 0)
        _playbackController.SetPlaybackFinished(true); 
    else
     _appRender.AdjustRenderSize();

    _timeBase = _decoder.getVideoTimeBase();
    gLogger.debug("Video time base = {}", _timeBase);
    _playbackController.StartPlaybackTimer();
    _lastFrame.reset();
    //consume the state
    _playerState.store(PlayerState::None, std::memory_order_release);
}
