/**
 * Copyright (c) 2020 Paul Himmler
 * Licensed under the MIT License.
 * See LICENSE file for more information.
 */

#include "forward_lighting.h"

#include "saiga/opengl/imgui/imgui_opengl.h"
#include "saiga/opengl/rendering/lighting/cpu_plane_clusterer.h"
#include "saiga/opengl/rendering/lighting/directional_light.h"
#include "saiga/opengl/rendering/lighting/gpu_assignment_clusterer.h"
#include "saiga/opengl/rendering/lighting/point_light.h"
#include "saiga/opengl/rendering/lighting/six_plane_clusterer.h"
#include "saiga/opengl/rendering/lighting/spot_light.h"

namespace Saiga
{
using namespace uber;

ForwardLighting::ForwardLighting(GLTimerSystem* timer) : RendererLighting(timer)
{
    int maxSize = ShaderStorageBuffer::getMaxShaderStorageBlockSize();

    maximumNumberOfDirectionalLights =
        std::clamp(maximumNumberOfDirectionalLights, 0, maxSize / (int)sizeof(DirectionalLight::ShaderData));
    maximumNumberOfPointLights =
        std::clamp(maximumNumberOfPointLights, 0, maxSize / (int)sizeof(PointLight::ShaderData));
    maximumNumberOfSpotLights = std::clamp(maximumNumberOfSpotLights, 0, maxSize / (int)sizeof(SpotLight::ShaderData));

    lightInfoBuffer.createGLBuffer(nullptr, sizeof(LightInfo), GL_DYNAMIC_DRAW);
}

ForwardLighting::~ForwardLighting() {}

void ForwardLighting::init(int _width, int _height, bool _useTimers)
{
    RendererLighting::init(_width, _height, _useTimers);
    if (clustererType) lightClusterer->resize(_width, _height);
}

void ForwardLighting::resize(int _width, int _height)
{
    RendererLighting::resize(_width, _height);
    if (clustererType) lightClusterer->resize(_width, _height);
}

void ForwardLighting::initRender()
{
    auto tim = timer->Measure("Lightinit");
    RendererLighting::initRender();
    LightInfo li;
    li.pointLightCount       = 0;
    li.spotLightCount        = 0;
    li.directionalLightCount = 0;

    li.clusterEnabled = clustererType > 0;

    // Point Lights
    li.pointLightCount = std::min((int)active_point_lights.size(), maximumNumberOfPointLights);

    // Spot Lights
    li.spotLightCount = std::min((int)active_spot_lights.size(), maximumNumberOfSpotLights);

    // Directional Lights
    li.directionalLightCount = std::min((int)active_directional_lights.size(), maximumNumberOfDirectionalLights);

    lightInfoBuffer.updateBuffer(&li, sizeof(LightInfo), 0);
    visibleLights = li.pointLightCount + li.spotLightCount + li.directionalLightCount;

    lightInfoBuffer.bind(LIGHT_INFO_BINDING_POINT);
}

void ForwardLighting::cluster(Camera* cam, const ViewPort& viewPort)
{
    if (clustererType)
    {
        lightClusterer->clusterLights(cam, viewPort, active_point_lights, active_spot_lights);
        // At this point we can use clustering information in the lighting uber shader with the right binding points.
    }
}

void ForwardLighting::render(Camera* cam, const ViewPort& viewPort)
{
    auto tim = timer->Measure("Light Render");
    // Does nothing
    RendererLighting::render(cam, viewPort);

    if (drawDebug)
    {
        //        glDepthMask(GL_TRUE);
        renderDebug(cam);
        //        glDepthMask(GL_FALSE);
    }
    if (clustererType) lightClusterer->renderDebug(cam);
    assert_no_glerror();
}

void ForwardLighting::setLightMaxima(int maxDirectionalLights, int maxPointLights, int maxSpotLights)
{
    maxDirectionalLights = std::max(0, maxDirectionalLights);
    maxPointLights       = std::max(0, maxPointLights);
    maxSpotLights        = std::max(0, maxSpotLights);

    int maxSize = ShaderStorageBuffer::getMaxShaderStorageBlockSize();

    maximumNumberOfDirectionalLights =
        std::clamp(maximumNumberOfDirectionalLights, 0, maxSize / (int)sizeof(DirectionalLight::ShaderData));
    maximumNumberOfPointLights =
        std::clamp(maximumNumberOfPointLights, 0, maxSize / (int)sizeof(PointLight::ShaderData));
    maximumNumberOfSpotLights = std::clamp(maximumNumberOfSpotLights, 0, maxSize / (int)sizeof(SpotLight::ShaderData));

    maximumNumberOfDirectionalLights = maxDirectionalLights;
    maximumNumberOfPointLights       = maxPointLights;
    maximumNumberOfSpotLights        = maxSpotLights;
}


void ForwardLighting::renderImGui()
{
    if (!showLightingImgui) return;

    if (!editor_gui.enabled)
    {
        int w = 340;
        int h = 240;
        ImGui::SetNextWindowPos(ImVec2(680, height - h), ImGuiCond_Once);
        ImGui::SetNextWindowSize(ImVec2(w, h), ImGuiCond_Once);
    }

    if (ImGui::Begin("Lighting", &showLightingImgui))
    {
        const char* const clustererTypes[4] = {"None", "CPU SixPlanes", "CPU PlaneArrays", "GPU AABB Light Assignment"};

        bool changed = ImGui::Combo("Mode", &clustererType, clustererTypes, 4);

        if (changed)
        {
            setClusterType(clustererType);
        }
        ImGui::Separator();
    }
    ImGui::End();

    RendererLighting::renderImGui();

    if (clustererType) lightClusterer->imgui();
}

void ForwardLighting::setClusterType(int tp)
{
    clustererType = tp;
    if (clustererType > 0)
    {
        switch (clustererType)
        {
            case 1:
                lightClusterer = std::static_pointer_cast<Clusterer>(std::make_shared<SixPlaneClusterer>(timer));
                break;
            case 2:
                lightClusterer = std::static_pointer_cast<Clusterer>(std::make_shared<CPUPlaneClusterer>(timer));
                break;
            case 3:
                lightClusterer = std::static_pointer_cast<Clusterer>(std::make_shared<GPUAssignmentClusterer>(timer));
                break;
            default:
                lightClusterer = nullptr;
                return;
        }

        lightClusterer->resize(width, height);
    }
}

}  // namespace Saiga
