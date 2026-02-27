#pragma once

#include "raylib.h"
#include "../logger/GLogger.h"

class Render {
public:
    struct RenderWindow {
        struct Size {
            int width;
            int height;
        };

        //constant
        constexpr static Size minWindowSize {640, 360}; 

        // called when a video is first loaded
        void AdjustRenderSize() noexcept;
        // this should be called on every windows resize to keep 
        // internal state up to date
        void SetRLWindowSize(const int& width, const int& height) noexcept
        {
            gLogger.debug("Raylib window {}x{}", width, height);
            windowSize.width = width;
            windowSize.height = height;
        }

        Rectangle GetVideoDrawingRectangle() noexcept;

        // actual window size (raylib screen)
        Size windowSize;
        // video size
        Size videoSize;
        //texture to hold frame data, it has the size of the video
        Texture2D frameTexture;
        // source rectangle having resolution of video
        Rectangle source;
        // destination rectangle having resolution of screen
        Rectangle destination;
    };

    Render (const int& width, const int& height) 
    {
        renderWindow.SetRLWindowSize(width, height);
        isRendering = false;
    }
    
    Render(const Render&) = delete;
    Render& operator=(const Render&) = delete;

    // allow move
    Render(Render&&) noexcept = default;
    Render& operator=(Render&&) noexcept = default; 

    void BeginRender() noexcept; 
    void InitializeFrameTexture(const int& width, const int& height) noexcept;
    void EndRender() noexcept; 
    bool IsStillRendering() noexcept { return isRendering; }
    void DrawFrame(const uint8_t* frameData);
    void AdjustRenderSize() noexcept { renderWindow.AdjustRenderSize(); } 
    void UpdateRLWindowSize() noexcept; 

private:
    bool isRendering;
    RenderWindow renderWindow;
};
