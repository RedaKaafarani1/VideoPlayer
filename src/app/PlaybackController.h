#pragma once

#include <chrono>

class PlaybackController {
public:
    using Clock = std::chrono::steady_clock;

    void StartPlaybackTimer() noexcept { _playbackStartTime = Clock::now(); }; 
    double GetEffectivePlaybackTime() const noexcept {
        double elapsed = std::chrono::duration<double>(Clock::now() - _playbackStartTime).count();
        return elapsed - _totalPausedTime;
    }
    bool IsPlaybackFinished() const noexcept { return _playbackFinished; }
    void SetPlaybackFinished(const bool playbackState) noexcept { _playbackFinished = playbackState; }
    void TogglePause() noexcept {
        _playbackPaused = !_playbackPaused;
        if (_playbackPaused)
            PausePlayBack();
        else ContinuePlayBack();
    }
    bool IsPlaybackPaused() const noexcept { return _playbackPaused; }
    void ResetInternalState() noexcept {
        _totalPausedTime = 0;
        _playbackFinished = false;
        _playbackPaused = false;
    }

private:
    void PausePlayBack() noexcept { _playbackPauseTime = Clock::now(); }
    void ContinuePlayBack() noexcept {
        _totalPausedTime += std::chrono::duration<double>(Clock::now() - _playbackPauseTime).count();
    }

    Clock::time_point _playbackStartTime;
    Clock::time_point _playbackPauseTime;
    double _totalPausedTime{0};
    bool _playbackFinished = false;
    bool _playbackPaused = false;

};
