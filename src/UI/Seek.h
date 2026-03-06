#pragma once

#include "UICommon.h"

namespace UI {

    class Seek : public UIElement {
    public:
        Seek(const UIElementShape& shape, const Color& color) : 
            UIElement(shape, color) {}

        void AdjustSizePosition(const int width, const int height) noexcept override; 
    };
}
