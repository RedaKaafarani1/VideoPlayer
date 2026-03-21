#include "Render.h"
#include "Logger/GLogger.h"
#include "UI/UICommon.h"
#include <raylib.h>

Render::Render(const int width, const int height, const int unusableHeight) 
{
    renderWindow.SetRLWindowSize(width, height, unusableHeight);
    isRendering = false;
}

void Render::LoadTextureMap() noexcept
{
    //load all used textures here
    _textureMap["SeekCircle"] = LoadTexture("resources/seek.png");
    _textureMap["PlayButton"] = LoadTexture("resources/play.png");
    _textureMap["StopButton"] = LoadTexture("resources/stop.png");
    _textureMap["ForwardButton"] = LoadTexture("resources/forward.png");
    _textureMap["RewindButton"] = LoadTexture("resources/rewind.png");

    for (auto& tex : _textureMap)
        SetTextureFilter(tex.second, TEXTURE_FILTER_BILINEAR);
}

void Render::InitializeFrameTexture(const int& width, const int& height) noexcept
{
    Image img;
    img.data = nullptr;
    img.width = width;
    img.height = height;
    img.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
    img.mipmaps = 1;

    //These will be used for resizing calculations
    renderWindow.videoSize.width  = width;
    renderWindow.videoSize.height = height;
    // Rectangle of source (video) which has original resolution of the video
    // and won't change
    renderWindow.source = {0.0, 0.0, width*1.0f, height*1.0f};
    //destination rectangle, it uses GetVideoDrawingRectangle, so it assumes
    //that windowSize is already set.
    renderWindow.destination = renderWindow.GetVideoDrawingRectangle();

    //Reuse texture, if we had a previous video, unload first
    if (renderWindow.frameTexture.id != 0)
        UnloadTexture(renderWindow.frameTexture);
    renderWindow.frameTexture = LoadTextureFromImage(img);
}

//helper function to get monitor size
static Size GetMonitorSize()
{
    int monitor = GetCurrentMonitor();
    return {
        GetMonitorWidth(monitor),
        GetMonitorHeight(monitor)
    };
}

void Render::BeginRender() noexcept
{
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI | FLAG_WINDOW_RESIZABLE);

    InitWindow(renderWindow.windowSize.width, renderWindow.windowSize.height, "Video Player");

    Size monitor = GetMonitorSize();
    SetWindowMinSize(renderWindow.minWindowSize.width, renderWindow.minWindowSize.height);
    SetWindowMaxSize(monitor.width, monitor.height);

    LoadTextureMap();
    isRendering = true;
}

void Render::EndRender() noexcept
{
    UnloadTexture(renderWindow.frameTexture); 
    for(auto& tex : _textureMap)
        UnloadTexture(tex.second);
    isRendering = false;
    CloseWindow();
}

void Render::DrawFrame(const uint8_t* frameData)
{
    BeginDrawing();
    ClearBackground(BLACK);
    if (frameData)
    {
        UpdateTexture(renderWindow.frameTexture, frameData);
        DrawTexturePro(renderWindow.frameTexture,
                       renderWindow.source,
                       renderWindow.destination,
                       {0,0},
                       0,
                       WHITE
        );
    }
    EndDrawing();
}

void Render::UpdateRLWindowSize() noexcept
{
    Size newSize;
    newSize.width  = GetScreenWidth();
    newSize.height = GetScreenHeight();

    renderWindow.SetRLWindowSize(newSize.width, newSize.height, renderWindow.unusableHeight);
    //window size changed, update destination to scale video if needed
    renderWindow.destination = renderWindow.GetVideoDrawingRectangle();
}

void Render::RenderWindow::AdjustRenderSize() noexcept
{
    Size monitorSize = GetMonitorSize();

    if (videoSize.width  <= monitorSize.width &&
        videoSize.height <= monitorSize.height)
    {
        //update internal memory with window size
        SetRLWindowSize(videoSize.width, videoSize.height, unusableHeight);
        //change raylib window size
        SetWindowSize(videoSize.width, videoSize.height); 
        return;
    }

    // if video is larger, we need to scale down to fit our monitor
    double scale = std::min(
            monitorSize.width / ((1.0)*videoSize.width),
            monitorSize.height / ((1.0)*videoSize.height)
    );

    Size newSize {static_cast<int>(videoSize.width*scale),
                  static_cast<int>(videoSize.height*scale)
                 };

    SetRLWindowSize(newSize.width, newSize.height, unusableHeight);
    SetWindowSize(newSize.width, newSize.height);
}

Rectangle Render::RenderWindow::GetVideoDrawingRectangle() noexcept
{
    const int availableHeight = windowSize.height - unusableHeight;

    float scale = std::min(
            windowSize.width / ((1.0f) * videoSize.width),
            availableHeight / ((1.0f) * videoSize.height)
    );
    //only scale down
    scale = std::min(scale, 1.0f);
    float drawWidth = videoSize.width * scale;
    float drawHeight = videoSize.height * scale;

    return {(windowSize.width*1.0f - drawWidth)/2.0f,
            (availableHeight*1.0f - drawHeight)/2.0f,
            drawWidth,
            drawHeight
    };
}

inline Rectangle ToRLRectangle(const UI::Rect& rect)
{
    return Rectangle{rect.x, rect.y, rect.width, rect.height};
}

inline Color ToRLColor(const UI::Color& color)
{
    return Color{color.r, color.g, color.b, color.a};
}

void Render::DrawScaledElement(const UI::UIElement& elem, const bool isAsset) noexcept
{
    float scale = elem.GetScale();
    float width = elem.GetWidth();
    float height = elem.GetHeight();
    float x = elem.GetXPosition();
    float y = elem.GetYPosition();

    bool isRounded = elem.GetRoundness() != 0.0f ? true : false;

    if (!isAsset)
    {
        Color color = ToRLColor(elem.GetColor());

        Rectangle raylibRect {
            x,
            y,
            width,
            height
        };

        if (isRounded)
        {
            DrawRectangleRounded(raylibRect,
                                 elem.GetRoundness(),
                                 elem.GetSegments(),
                                 color
            );
        }
        else
        {
            DrawRectangleRec(raylibRect, color);
        }
    }
    else
    {
        Rectangle src {0, 0, width, height};
        Rectangle dst {
            x + width  / 2.0f,
            y + height / 2.0f,
            width * scale,
            height * scale,
        };

        Vector2 origin { dst.width / 2.0f, dst.height / 2.0f };

        DrawTexturePro(_textureMap.at(elem.GetAsset()),
                       src,
                       dst,
                       origin, 
                       0.0f, 
                       WHITE
        );
    }
}

void Render::DrawUIElement(const UI::UIElement& elem) noexcept
{
    Rectangle r = ToRLRectangle(elem.GetRectangle());
    Color c = ToRLColor(elem.GetColor());

    auto elemAsset = elem.GetAsset();
    // if we have no texture, draw shape
    if (_textureMap.find(elemAsset) == _textureMap.end())
    {
        switch (elem.GetType())
        {
            case UI::UIElementType::Container:
            case UI::UIElementType::Element:
            {
                DrawScaledElement(elem, false);
                break;
            }
            case UI::UIElementType::Text:
            {
                auto& uiTextElement = dynamic_cast<const UI::UIText&>(elem);
                DrawText(uiTextElement.GetText().c_str(), r.x, r.y, uiTextElement.GetFontSize(), c);
                break;
            }
            default: gLogger.error("Could not determine type of UIElement in Draw function");
        }
    }
    else {
        DrawScaledElement(elem, true);
    }

    // Draw children of a ui element if it got any
    if (elem.GetType() == UI::UIElementType::Container)
    {
        auto& container = dynamic_cast<const UI::UIContainer&>(elem);
        for (auto& c : container.GetChildren())
           DrawUIElement(*c);
    }
}
