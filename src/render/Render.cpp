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

    frameTexture = LoadTextureFromImage(img);
}

void Render::BeginRender(const AVRational& timeBase, const double& fps) noexcept
{
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);

    InitWindow(appWidth, appHeight, "RTSP Player");

    const auto& firstFrame = rgbFrames.at(0).get();

    InitializeFrameTexture(firstFrame->width, firstFrame->height);

    isRendering = true;

    SetTargetFPS(fps);

    double startTime = GetTime();
    int currFrameIdx = 0;

    while (!WindowShouldClose() && isRendering && currFrameIdx < static_cast<int>(rgbFrames.size()))
    {
        BeginDrawing();
        ClearBackground(GRAY);
        const auto& currFrame = rgbFrames.at(currFrameIdx).get();
        double frameTime = currFrame->pts * av_q2d(timeBase);
        if (GetTime() - startTime > frameTime)
        {
            UpdateTexture(frameTexture, currFrame->data[0]);
            DrawTexture(frameTexture, 0, 0, WHITE);
            currFrameIdx++;
        }

        EndDrawing();
    }

    EndRender();
    CloseWindow();
}

void Render::EndRender() noexcept
{
    UnloadTexture(frameTexture); 
    isRendering = false;
}

void Render::ResizeWindows(int newWidth, int newHeight) noexcept
{
    appWidth = newWidth;
    appHeight = newHeight;
}
