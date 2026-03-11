#include "Timeline.h"
#include <format>

namespace UI {

    void Seek::AdjustSizePosition(const int width, const int height) noexcept {
        _rect.width = width; 
        _rect.height = height;
        //x and y will be in parent Timeline
    }

    void VideoTime::AdjustSizePosition(const int width, const int height) noexcept {
        _rect.width = width; 
        _rect.height = height;
    }
    
    void Timeline::AdjustSizePosition(const int width, const int height) noexcept
    {
        _rect.x = 0;
        _rect.y = height - TIMELINE_SIZE;
        _rect.width = width;
        _rect.height = TIMELINE_SIZE;

        //seek rectangle where knob moves
        auto seek = _children[0].get();
        seek->AdjustSizePosition(_rect.width - CONTROLS_WIDTH*2.0f, _rect.height/5.0f);
        auto seekHeight = seek->GetHeight();
        seek->SetXPosition(_rect.x + CONTROLS_WIDTH);
        seek->SetYPosition(_rect.y + (_rect.height - seekHeight)/2.0f);
        seek->SetRoundness(1.0f);
        seek->SetSegments(8);

        //seek progress which fills the seek 
        auto seekProgress = _children[1].get();
        seekProgress->AdjustSizePosition(0, seekHeight);
        seekProgress->SetRoundness(1.0f);
        seekProgress->SetSegments(8);
        UpdateSeekProgress();

        auto seekKnob = _children[2].get();
        //20 here is the size of the seek circle, this is bad, we need a way to 
        //get the size of the asset
        seekKnob->AdjustSizePosition(20, 20);
        UpdateKnobPosition();

        auto videoTime = dynamic_cast<UIText*>(_children[3].get()); 
        videoTime->AdjustSizePosition(100, videoTime->GetFontSize());
        UpdateVideoTime(0.0, 0.0, false);
    }

    void Timeline::UpdateKnobPosition() noexcept 
    {
        auto seek = _children[0].get();
        auto seekX = seek->GetXPosition(); 
        auto seekY = seek->GetYPosition();
        auto seekWidth = seek->GetWidth();
        auto seekHeight = seek->GetHeight();

        auto seekKnob = _children[2].get();
        seekKnob->SetXPosition(seekX - seekKnob->GetWidth()/2 + static_cast<float>(_videoProgress)*seekWidth);
        seekKnob->SetYPosition(seekY + (seekHeight - seekKnob->GetHeight())/2.0f);
    }

    void Timeline::UpdateSeekProgress() noexcept 
    {
        auto seek = _children[0].get();
        auto seekX = seek->GetXPosition(); 
        auto seekY = seek->GetYPosition();
        auto seekWidth = seek->GetWidth();

        auto seekProgess = _children[1].get();
        seekProgess->SetXPosition(seekX);
        seekProgess->SetWidth(seekWidth*_videoProgress);
        seekProgess->SetYPosition(seekY);
    }

    std::string FormatTime(int totalSeconds)
    {
        const int minutes = totalSeconds / 60;
        const int seconds = totalSeconds % 60;
        return std::format("{:02}:{:02}", minutes, seconds);
    }

    void Timeline::UpdateVideoTime(const double progress, const double videoDuration, bool updateTime) noexcept
    {
        auto seek = _children[0].get();
        auto seekX = seek->GetXPosition();
        auto seekY = seek->GetYPosition();
        auto seekWidth = seek->GetWidth();
        auto seekHeight = seek->GetHeight();
        
        auto videoTime = dynamic_cast<UIText*>(_children[3].get()); 
        videoTime->SetXPosition(seekX + seekWidth + 10.0f);
        if (updateTime)
        {
            int seconds = static_cast<int>(progress);
            int totalSeconds = static_cast<int>(videoDuration);
            std::string text = FormatTime(seconds) + "/" + FormatTime(totalSeconds);
            videoTime->SetText(text);
        }
        videoTime->SetYPosition(seekY + (seekHeight - videoTime->GetHeight())/2.0f);
    }
}
