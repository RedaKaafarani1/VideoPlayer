#pragma once

#include "raylib.h"
#include "Common.h"

class Render {
public:
    Render (int width, int height) : isRendering(false), appWidth(width), appHeight(height)
    {
        frameTexture.width = 0;
        frameTexture.height = 0;
    }
    
    Render(const Render&) = delete;
    Render& operator=(const Render&) = delete;

    // allow move
    Render(Render&&) noexcept = default;
    Render& operator=(Render&&) noexcept = default; 

    void BeginRender() noexcept; 
    void InitializeFrameTexture(const int& width, const int& height) noexcept;
    void EndRender() noexcept; 
    void ResizeWindows(int newWidth, int newHeight) noexcept;
    bool IsStillRendering() noexcept { return isRendering; }
    void DrawFrame(const AVFrame* frame);

private:
    bool isRendering;
    int appWidth;
    int appHeight;
    Texture2D frameTexture;
};
