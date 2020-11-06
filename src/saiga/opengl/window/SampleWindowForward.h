﻿/**
 * Copyright (c) 2017 Darius Rückert
 * Licensed under the MIT License.
 * See LICENSE file for more information.
 */

#pragma once

#include "saiga/config.h"
#ifdef SAIGA_USE_SDL

#    include "saiga/core/sdl/all.h"
#    include "saiga/opengl/assets/all.h"
#    include "saiga/opengl/assets/objAssetLoader.h"
#    include "saiga/opengl/rendering/forwardRendering/forwardRendering.h"
#    include "saiga/opengl/rendering/renderer.h"
#    include "saiga/opengl/window/WindowTemplate.h"
#    include "saiga/opengl/window/sdl_window.h"
#    include "saiga/opengl/world/LineSoup.h"
#    include "saiga/opengl/world/pointCloud.h"
#    include "saiga/opengl/world/proceduralSkybox.h"



namespace Saiga
{
/**
 * This is the class from which most saiga samples inherit from.
 * It's a basic scene with a camera, a skybox and a ground plane.
 *
 * @brief The SampleWindowForward class
 */
class SAIGA_OPENGL_API SampleWindowForward : public StandaloneWindow<WindowManagement::SDL, ForwardRenderer>,
                                             public SDL_KeyListener
{
   public:
    SampleWindowForward();
    ~SampleWindowForward() {}

    void update(float dt) override;
    void interpolate(float dt, float interpolation) override;

    virtual void render(Camera* camera, RenderPass render_pass) override;

    void keyPressed(SDL_Keysym key) override;
    void keyReleased(SDL_Keysym key) override;

   protected:
    SDLCamera<PerspectiveCamera> camera;
    SimpleAssetObject groundPlane;
    ProceduralSkybox skybox;

    bool showSkybox = true;
    bool showGrid   = true;
};

}  // namespace Saiga

#endif
