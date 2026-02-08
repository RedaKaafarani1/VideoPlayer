#include "Render.h"

void Render::BeginRender() noexcept
{
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);

    InitWindow(appWidth, appHeight, "RTSP Player");

    isRendering = true;
    
    while (!WindowShouldClose() && isRendering)
    {

        BeginDrawing();
        ClearBackground(GRAY);

        EndDrawing();
    }

    CloseWindow();
}

void Render::EndRender() noexcept
{
    isRendering = false;
}

void Render::ResizeWindows(int newWidth, int newHeight) noexcept
{
    appWidth = newWidth;
    appHeight = newHeight;
}
