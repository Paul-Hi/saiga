﻿/**
 * Copyright (c) 2017 Darius Rückert
 * Licensed under the MIT License.
 * See LICENSE file for more information.
 */

#pragma once

#include "saiga/config.h"

#ifndef SAIGA_USE_SDL
#    error Saiga was compiled without SDL2.
#endif

#include "saiga/core/sdl/saiga_sdl.h"
#include "saiga/core/sdl/sdl_eventhandler.h"
#include "saiga/opengl/window/OpenGLWindow.h"

#undef main

namespace Saiga
{
class SAIGA_OPENGL_API SDLWindow : public OpenGLWindow, public SDL_ResizeListener
{
   public:
    SDL_Window* window = nullptr;

   protected:
    SDL_GLContext gContext;

    virtual bool initWindow() override;
    virtual bool initInput() override;
    virtual bool shouldClose() override;
    virtual void checkEvents() override;
    virtual void swapBuffers() override;
    virtual void freeContext() override;
    virtual void loadGLFunctions() override;

    virtual bool resizeWindow(Uint32 windowId, int width, int height) override;

   public:
    SDLWindow(WindowParameters windowParameters, OpenGLParameters openglParameter);
    ~SDLWindow();

    virtual std::shared_ptr<ImGui_GL_Renderer> createImGui() override;
};

}  // namespace Saiga
