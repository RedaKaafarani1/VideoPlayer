#include "Render.h"
#include "UI/UICommon.h"
#include <raylib.h>

Render::Render(const int width, const int height, const int unusableHeight) 
{
    renderWindow.SetRLWindowSize(width, height, unusableHeight);
    isRendering = false;
}

void Render::LoadTextureMap() noexcept
{
    //load all used texture here
    _textureMap["SeekCircle"] = LoadTexture("resources/seek.png");
    _textureMap["PlayButton"] = LoadTexture("resources/play.png");
    _textureMap["StopButton"] = LoadTexture("resources/stop.png");
    _textureMap["ForwardButton"] = LoadTexture("resources/forward.png");
    _textureMap["RewindButton"] = LoadTexture("resources/rewind.png");
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

void Render::DrawUIElement(const UI::UIElement& elem) noexcept
{
    Rectangle r = ToRLRectangle(elem.GetRectangle());
    Color c = ToRLColor(elem.GetColor());

    auto elemAsset = elem.GetAsset();
    // if we have no texture, draw shape
    if (_textureMap.find(elemAsset) == _textureMap.end())
    {
        DrawRectangle(r.x, r.y, r.width, r.height, c);
    }
    else {
       DrawTexture(_textureMap.at(elemAsset), r.x, r.y, WHITE);
    }

    // Draw children of a ui element if it got any
    for (auto* c : elem.GetChildren())
       DrawUIElement(*c);
}
