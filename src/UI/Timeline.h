#pragma once

#include "UICommon.h"

namespace UI {

    class Seek : public UIElement {
    public:
        Seek(const Color& color) : 
            UIElement(color) { SetType(UIElementType::Element); }
        Seek(const Color& color, const std::string& asset) : 
            UIElement(color, asset) { SetType(UIElementType::Element); }

        void AdjustSizePosition(const int width, const int height) noexcept override; 
    };

    class VideoTime : public UIText {
    public:
        VideoTime(const Color& color, const int fontSize ) :
            UIText(color) 
        {
            SetType(UIElementType::Text); 
            SetFontSize(fontSize);
        }

        void AdjustSizePosition(const int width, const int height) noexcept override; 
    };

    class Timeline : public UIContainer{
    public:
        Timeline() { 
            _color = {50, 50, 50, 255};
            SetType(UIElementType::Container);
            //seek rectangle
            AddChild(std::make_unique<Seek>(Color{90, 90, 90, 255}));
            //seek progress
            AddChild(std::make_unique<Seek>(Color{7, 38, 236, 255}));
            //seek knob
            AddChild(std::make_unique<Seek>(Color{80, 80, 80, 255}, "SeekCircle"));
            //Video time formatted as 00:00/00:00
            AddChild(std::make_unique<VideoTime>(Color{255, 255, 255, 255}, 16));
        };
        void AdjustSizePosition(const int width, const int height) noexcept override; 
        void UpdateVideoTime(const double progress, const double videoDuration, bool updateTime) noexcept;
        void UpdateVideoProgess(const double progress, const double videoDuration) noexcept
        {
            _videoProgress = progress/videoDuration; 
            UpdateKnobPosition();
            UpdateSeekProgress();
        };

    private:
        void UpdateKnobPosition() noexcept;
        void UpdateSeekProgress() noexcept;
        //This is percentage for now 
        double _videoProgress = 0;
        int _lastDisplayedSecond = -1;
    };


}
