#pragma once

#include "UI/Seek.h"
#include "UICommon.h"

namespace UI {

    class Timeline : public UIContainer{
    public:
        Timeline() { 
            _color = {50, 50, 50, 255};
            AddChild(std::make_unique<Seek>(Color{90, 90, 90, 255}));
            AddChild(std::make_unique<Seek>(Color{80, 80, 80, 255}, "SeekCircle"));
        };
        void AdjustSizePosition(const int width, const int height) noexcept override; 
        void UpdateVideoProgess(const double progress) noexcept
        {
            _videoProgress = progress; 
            UpdateKnobPosition();
        };

    private:
        void UpdateKnobPosition() noexcept;
        //This is percentage for now 
        double _videoProgress = 0;
    };


}
