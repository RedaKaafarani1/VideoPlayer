#pragma once

#include "UICommon.h"

namespace UI {

    class Seek : public UIElement {
    public:
        Seek(const Color& color) : 
            UIElement(color) {}
        Seek(const Color& color, const std::string& asset) : 
            UIElement(color, asset) {}

        void AdjustSizePosition(const int width, const int height) noexcept override; 
    };
}
