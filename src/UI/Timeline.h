#pragma once

#include "UI/Seek.h"
#include "UICommon.h"

namespace UI {

    class Timeline : public UIContainer{
    public:
        Timeline() { 
            _shape = UIElementShape::Rectangle;
            _color = {130, 130, 130, 255};
            AddChild(std::make_unique<Seek>(UIElementShape::Rectangle, Color{200, 200, 200, 255}));
            AddChild(std::make_unique<Seek>(UIElementShape::Rectangle, Color{80, 80, 80, 255}));
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
