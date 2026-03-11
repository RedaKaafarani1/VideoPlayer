#include "App.h"
#include "Common.h"
#include "UI/Controls.h"

inline void AdjustUISizePosition(UI::Timeline& timeline, UI::Controls& controls, const Size newSize)
{
   timeline.AdjustSizePosition(newSize.width, newSize.height); 
   controls.AdjustSizePosition(newSize.width, newSize.height); 
}

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
    DecoderState currState = _playerState.load(std::memory_order_acquire);
    if (currState == DecoderState::DecoderDoneSeeking)
    {
        //consume state
        _playerState.store(DecoderState::None, std::memory_order_release);
        _playbackController.ResetInternalState();
        _playbackController.StartPlaybackTimer();
        _lastVideoProgressUpdateTime = -1;
    }
    if (currState == DecoderState::DecoderLoading ||
        currState == DecoderState::DecoderWaiting)
    {
        //Decoder is re-initializing after adding video file
        //or the app just started, the state will change on
        //decoder side to DecoderReady
        _appRender.DrawFrame(nullptr);
        return;
    }

    if (currState == DecoderState::DecoderReady)
    {
        //We just added a video file and the decoder 
        //has started the decoding process
        gLogger.info("Decoder is ready");
        ReinitializeState();
        //consume the state
        _playerState.store(DecoderState::None, std::memory_order_release);
    }

    if (!_playbackController.IsPlaybackFinished() && !_playbackController.IsPlaybackPaused() && !(currState == DecoderState::DecoderFailed))
    {
        //This waits on the frame
        auto frame = _decoder.getFrame();
        //after getFrame, we are sure the first frame pts is set (if all is good), we can safely use it
        auto firstFramePTS = _decoder.getFirstFramePTS();
        if (frame)
        {
            // This function accounts for paused time
            double currTime = _playbackController.GetEffectivePlaybackTime(); 
            auto currPTS = frame->pts;
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

            // This updates seek bar
            _timeline.UpdateVideoProgess(currTime, _totalVideoDuration);

            //This updates the time/totaltime text display, we only
            //want it to happen every second
            int iCurrTime = static_cast<int>(currTime);
            if (_lastVideoProgressUpdateTime != iCurrTime)
            {
                _timeline.UpdateVideoTime(currTime, _totalVideoDuration, true);
                _lastVideoProgressUpdateTime = iCurrTime;
            }
        }
        else {
            //Playback is done
            _playbackController.SetPlaybackFinished(true); 
            _timeline.UpdateVideoProgess(_totalVideoDuration, _totalVideoDuration);
            _timeline.UpdateVideoTime(_totalVideoDuration, _totalVideoDuration, true);
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

    _appRender.DrawUIElement(_timeline);
    _appRender.DrawUIElement(_controls);
}

void App::RunAppLoop() noexcept
{
    // Initializes raylib related stuff 
    _appRender.BeginRender();
    _timeline.AdjustSizePosition(WIDTH, HEIGHT);
    _controls.AdjustSizePosition(WIDTH, HEIGHT);

    gLogger.info("Starting decoding process in separate thread");
    _decoder.startDecoderThread(); 
    //start the decoder in waiting state

    while (!WindowShouldClose() && _appRender.IsStillRendering())
    {
        HandleVideoFileChange();
        if (IsWindowResized())
        {
            _appRender.UpdateRLWindowSize();
            AdjustUISizePosition(_timeline, _controls, _appRender.GetWindowSize());
        }
        if (IsKeyPressed(KEY_R))
        {
            //pause video while waiting for seek
            _playbackController.TogglePause();
            //DecoderState will be DecoderDoneSeeking after command
            //is processed
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
        _playerState.store(DecoderState::DecoderLoading, std::memory_order_release);
        _decoder.pushCommand({DecoderCommandType::DecodeVideo, videoFileName, 0});
        UnloadDroppedFiles(filePathList);
   }
}

void App::ReinitializeState() noexcept
{
    //reset this here since playback state can change if we have an 
    //error later
    _playbackController.ResetInternalState();
    int err = InitializeInternals();
    if (err != 0)
        _playbackController.SetPlaybackFinished(true); 
    else
    {
        //player size adjusts to that of the video
        _appRender.AdjustRenderSize(); 
        AdjustUISizePosition(_timeline, _controls, _appRender.GetWindowSize());
    }

    _timeBase = _decoder.getVideoTimeBase();
    gLogger.debug("Video time base = {}", _timeBase);
    _totalVideoDuration = _decoder.getVideoDurationSeconds();
    gLogger.debug("Video duration: {}", _totalVideoDuration);
    _playbackController.StartPlaybackTimer();
    _lastFrame.reset();
    _lastVideoProgressUpdateTime = -1;
}
