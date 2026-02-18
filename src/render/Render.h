#pragma once

#include "raylib.h"
#include "Common.h"

class Render {
public:
    Render (int width, int height) : isRendering(false), appWidth(width), appHeight(height) {}
    
    Render(const Render&) = delete;
    Render& operator=(const Render&) = delete;

    Render(Render&&) noexcept = default;            // allow move
    Render& operator=(Render&&) noexcept = default; // allow move

    void BeginRender(const AVRational& timeBase, const double& fps) noexcept; 
    void InitializeFrameTexture(const int& width, const int& height) noexcept;
    void EndRender() noexcept; 
    void ResizeWindows(int newWidth, int newHeight) noexcept;

    std::vector<std::unique_ptr<AVFrame, CustomDeleter>> rgbFrames;

private:
    bool isRendering;
    int appWidth;
    int appHeight;
    Texture2D frameTexture;
};
