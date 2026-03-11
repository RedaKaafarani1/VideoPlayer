#include "Controls.h"

namespace UI {

    void Controls::AdjustSizePosition(const int width, const int height) noexcept
    { 
        _rect.x = 0;
        _rect.y = height - TIMELINE_SIZE;
        _rect.width = CONTROLS_WIDTH;
        _rect.height = TIMELINE_SIZE;

        auto& children = GetChildren();

        auto playButton = children[0].get();
        playButton->AdjustSizePosition(32, 32);
        playButton->SetXPosition((_rect.width - _rect.x - playButton->GetWidth()*2.0f)/2.0f);
        playButton->SetYPosition(_rect.y);

        auto stopButton = children[1].get();
        stopButton->AdjustSizePosition(32, 32);
        stopButton->SetXPosition((_rect.width - _rect.x)/2.0f);
        stopButton->SetYPosition(_rect.y);
        
        auto forwardButton = children[2].get();
        forwardButton->AdjustSizePosition(32, 32);
        forwardButton->SetXPosition((_rect.width - _rect.x + forwardButton->GetWidth()*2.0f)/2.0f);
        forwardButton->SetYPosition(_rect.y);

        auto rewindButton = children[3].get();
        rewindButton->AdjustSizePosition(32, 32);
        rewindButton->SetXPosition((_rect.width - _rect.x - rewindButton->GetWidth()*4.0f)/2.0f);
        rewindButton->SetYPosition(_rect.y);
    }

    void Button::AdjustSizePosition(const int width, const int height) noexcept
    {
        _rect.width = width;
        _rect.height = height;
    }

}
