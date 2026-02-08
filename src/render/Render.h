#pragma once

#include "raylib.h"

class Render {
public:
    Render (int width, int height) : isRendering(false), appWidth(width), appHeight(height) {}
    void BeginRender() noexcept; 
    void EndRender() noexcept; 
    void ResizeWindows(int newWidth, int newHeight) noexcept;

private:
    bool isRendering;
    int appWidth;
    int appHeight;
};
