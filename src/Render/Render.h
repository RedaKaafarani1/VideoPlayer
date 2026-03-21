#pragma once

#include "raylib.h"
#include "../Logger/GLogger.h"
#include "UI/UICommon.h"
#include <unordered_map>

struct Size {
    int width;
    int height;
};

class Render {
    
public:
    Render(const int width, const int height, const int uHeight);  
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
    void DrawUIElement(const UI::UIElement& elem) noexcept;
    void AdjustRenderSize() noexcept { renderWindow.AdjustRenderSize(); } 
    void UpdateRLWindowSize() noexcept; 
    Size GetWindowSize() noexcept { return renderWindow.windowSize; }

private:
    struct RenderWindow {
        //constant
        constexpr static Size minWindowSize {640, 360};
        // called when a video is first loaded
        void AdjustRenderSize() noexcept;
        // this should be called on every windows resize to keep 
        // internal state up to date
        void SetRLWindowSize(const int width, const int height, const int uHeight) noexcept
        {
            gLogger.debug("Raylib window {}x{}", width, height);
            windowSize.width = width;
            windowSize.height = height;
            unusableHeight = uHeight;
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
        // available video height, to account for UI
        int unusableHeight;
    };
        
    void DrawScaledElement(const UI::UIElement& elem, const bool isAsset) noexcept;
    void LoadTextureMap() noexcept;
    
    bool isRendering;
    RenderWindow renderWindow;
    std::unordered_map<std::string, Texture2D> _textureMap;
};
