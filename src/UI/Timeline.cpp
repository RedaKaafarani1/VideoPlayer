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
        seek->AdjustSizePosition(_rect.width - _rect.width/8.0f, _rect.height/4.0f);
        auto seekWidth = seek->GetWidth();
        auto seekHeight = seek->GetHeight();
        seek->SetXPosition(_rect.x + (_rect.width - seekWidth)/2.0f);
        seek->SetYPosition(_rect.y + (_rect.height - seekHeight)/2.0f);

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
        seekKnob->AdjustSizePosition(TIMELINE_SIZE/2.0f, TIMELINE_SIZE/2.0f);
        seekKnob->SetXPosition(seekX + _videoProgress*seekWidth);
        seekKnob->SetYPosition(seekY + (seekHeight - seekKnob->GetHeight())/2.0f);
    }
}
