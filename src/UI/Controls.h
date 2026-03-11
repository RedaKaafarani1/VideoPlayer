#pragma once

#include "UICommon.h"

namespace UI {

    class Button : public UIElement {
    public:
        Button(const Color& color) : 
            UIElement(color) {}
        Button(const Color& color, const std::string& asset) : 
            UIElement(color, asset) {}

        void AdjustSizePosition(const int width, const int height) noexcept override; 
    };

    class Controls : public UIContainer{
    public:
        Controls() { 
            _color = {0, 0, 0, 0};
            AddChild(std::make_unique<Button>(Color{}, "PlayButton"));
            AddChild(std::make_unique<Button>(Color{}, "StopButton"));
            AddChild(std::make_unique<Button>(Color{}, "ForwardButton"));
            AddChild(std::make_unique<Button>(Color{}, "RewindButton"));
        };
        void AdjustSizePosition(const int width, const int height) noexcept override; 
    };

}
