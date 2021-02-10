/**
 * Copyright (c) 2017 Darius Rückert
 * Licensed under the MIT License.
 * See LICENSE file for more information.
 */

#include "saiga/opengl/rendering/renderer.h"

#include "saiga/core/camera/camera.h"
#include "saiga/core/util/ini/ini.h"
#include "saiga/opengl/shader/basic_shaders.h"
#include "saiga/opengl/window/OpenGLWindow.h"

namespace Saiga
{
OpenGLRenderer::OpenGLRenderer(OpenGLWindow& window) : outputWidth(window.getWidth()), outputHeight(window.getHeight())
{
    cameraBuffer.createGLBuffer(nullptr, sizeof(CameraDataGLSL), GL_DYNAMIC_DRAW);


    window.setRenderer(this);

    // ImGUI
    imgui = window.createImGui();
}

OpenGLRenderer::~OpenGLRenderer() {}

void OpenGLRenderer::resize(int windowWidth, int windowHeight)
{
    if (windowWidth <= 0 || windowHeight <= 0)
    {
        std::cerr << "Warning: The window size must be greater than zero." << std::endl;
        windowWidth  = std::max(windowWidth, 1);
        windowHeight = std::max(windowHeight, 1);
    }
    outputWidth  = windowWidth;
    outputHeight = windowHeight;
    //    std::cout << "resize to " << windowWidth << "x" << windowHeight << std::endl;
}


void OpenGLRenderer::bindCamera(Camera* cam)
{
    CameraDataGLSL cd(cam);
    cameraBuffer.updateBuffer(&cd, sizeof(CameraDataGLSL), 0);
    cameraBuffer.bind(CAMERA_DATA_BINDING_POINT);
}

void RenderingParameters::fromConfigFile(const std::string& file)
{
    Saiga::SimpleIni ini;
    ini.LoadFile(file.c_str());

    auto section = "Rendering";
    INI_GETADD_BOOL(ini, section, srgbWrites);

    if (ini.changed()) ini.SaveFile(file.c_str());
}


}  // namespace Saiga
