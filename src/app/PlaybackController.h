#pragma once

#include <chrono>

class PlaybackController {
public:
    using Clock = std::chrono::steady_clock;

    void StartPlaybackTimer() noexcept { _playbackStartTime = Clock::now(); }; 
    double GetTimeSinceStart() const noexcept { 
        return std::chrono::duration<double>(Clock::now() - _playbackStartTime).count();
    }
    bool IsPlaybackFinished() const noexcept { return _playbackFinished; }
    void SetPlaybackFinished(const bool playbackState) noexcept { _playbackFinished = playbackState; }

private:
    Clock::time_point _playbackStartTime;
    bool _playbackFinished = false;
};
