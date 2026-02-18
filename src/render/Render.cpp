#include "Render.h"
#include <libavutil/frame.h>
#include <raylib.h>

void Render::InitializeFrameTexture(const int& width, const int& height) noexcept
{
    Image img;
    img.data = nullptr;
    img.width = width;
    img.height = height;
    img.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
    img.mipmaps = 1;

    frameTexture = LoadTextureFromImage(img);
}

void Render::BeginRender() noexcept
{
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);

    InitWindow(appWidth, appHeight, "RTSP Player");

    isRendering = true;
}

void Render::EndRender() noexcept
{
    UnloadTexture(frameTexture); 
    isRendering = false;
    CloseWindow();
}

void Render::ResizeWindows(int newWidth, int newHeight) noexcept
{
    appWidth = newWidth;
    appHeight = newHeight;
}

void Render::DrawFrame(const AVFrame* frame)
{
    BeginDrawing();
    ClearBackground(GRAY);
    if (frameTexture.width == 0 && frameTexture.height == 0)
        InitializeFrameTexture(frame->width, frame->height);
    UpdateTexture(frameTexture, frame->data[0]);
    DrawTexture(frameTexture, 0, 0, WHITE);
    EndDrawing();
}
