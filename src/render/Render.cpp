#include "Render.h"
#include <raylib.h>

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
static Render::RenderWindow::Size GetMonitorSize()
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

    RenderWindow::Size monitor = GetMonitorSize();
    SetWindowMinSize(renderWindow.minWindowSize.width, renderWindow.minWindowSize.height);
    SetWindowMaxSize(monitor.width, monitor.height);

    isRendering = true;
}

void Render::EndRender() noexcept
{
    UnloadTexture(renderWindow.frameTexture); 
    isRendering = false;
    CloseWindow();
}

void Render::DrawFrame(const uint8_t* frameData)
{
    BeginDrawing();
    ClearBackground(GRAY);
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
    RenderWindow::Size newSize;
    newSize.width  = GetScreenWidth();
    newSize.height = GetScreenHeight();

    renderWindow.SetRLWindowSize(newSize.width, newSize.height);
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
        SetRLWindowSize(videoSize.width, videoSize.height);
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

    SetRLWindowSize(newSize.width, newSize.height);
    SetWindowSize(newSize.width, newSize.height);
}

Rectangle Render::RenderWindow::GetVideoDrawingRectangle() noexcept
{
    float scale = std::min(
            windowSize.width / ((1.0f) * videoSize.width),
            windowSize.height / ((1.0f) * videoSize.height)
    );
    //only scale down
    scale = std::min(scale, 1.0f);
    float drawWidth = videoSize.width * scale;
    float drawHeight = videoSize.height* scale;

    //we want to center the video and draw it scaled down to drawWidth and drawHeight
    return {(windowSize.width*1.0f - drawWidth)/2.0f,
            (windowSize.height*1.0f - drawHeight)/2.0f,
            drawWidth,
            drawHeight
    };
}
