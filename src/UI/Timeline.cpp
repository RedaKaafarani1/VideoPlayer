#include "Timeline.h"
#include "UI/UICommon.h"

namespace UI {

    void Timeline::AdjustSizePosition(const int width, const int height) noexcept
    {
        _rect.x = 0;
        _rect.y = height - TIMELINE_SIZE;
        _rect.width = width;
        _rect.height = TIMELINE_SIZE;

        auto children = GetChildren();

        //seek rectangle where knob moves
        Seek* seek = dynamic_cast<Seek*>(children[0]);
        seek->AdjustSizePosition(_rect.width - CONTROLS_WIDTH*2.0f, _rect.height/5.0f);
        auto seekHeight = seek->GetHeight();
        seek->SetXPosition(_rect.x + CONTROLS_WIDTH);
        seek->SetYPosition(_rect.y + (_rect.height - seekHeight)/2.0f);

        Seek* seekKnob = dynamic_cast<Seek*>(children[1]);
        //20 here is the size of the seek circle, this is bad, I need a way to 
        //get the size of the asset
        seekKnob->AdjustSizePosition(20, 20);
        UpdateKnobPosition();
    }

    void Timeline::UpdateKnobPosition() noexcept {
        auto children = GetChildren();

        Seek* seek = dynamic_cast<Seek*>(children[0]);
        auto seekX = seek->GetXPosition(); 
        auto seekY = seek->GetYPosition();
        auto seekWidth = seek->GetWidth();
        auto seekHeight = seek->GetHeight();

        Seek* seekKnob = dynamic_cast<Seek*>(children[1]);
        seekKnob->SetXPosition(seekX - seekKnob->GetWidth()/2 + _videoProgress*seekWidth);
        seekKnob->SetYPosition(seekY + (seekHeight - seekKnob->GetHeight())/2.0f);
    }
}
