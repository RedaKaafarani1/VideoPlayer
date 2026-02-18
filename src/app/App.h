#pragma once

#include "../render/Render.h"
#include "../constants.h"

class App
{
public:
    App() : appRender(WIDTH, HEIGHT) {}
    void InitializeApp() noexcept;
    Render& GetAppRender() noexcept { return appRender; }
private:
    Render appRender;
};
