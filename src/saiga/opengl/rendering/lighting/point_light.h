﻿/**
 * Copyright (c) 2017 Darius Rückert
 * Licensed under the MIT License.
 * See LICENSE file for more information.
 */

#pragma once

#include "saiga/core/camera/camera.h"
#include "saiga/opengl/rendering/lighting/attenuated_light.h"

namespace Saiga
{
class SAIGA_OPENGL_API PointLightShader : public AttenuatedLightShader
{
   public:
    GLint location_shadowPlanes;

    virtual void checkUniforms();

    void uploadShadowPlanes(float f, float n);
};



class SAIGA_OPENGL_API PointLight : public AttenuatedLight
{
    friend class DeferredLighting;

   protected:
    std::shared_ptr<CubeShadowmap> shadowmap;

   public:
    vec3 position;

    void setPosition(const vec3& p) { position = p; }
    vec3 getPosition() { return position; }
    float shadowNearPlane = 0.1f;
    PerspectiveCamera shadowCamera;


    PointLight();
    virtual ~PointLight() {}
    PointLight& operator=(const PointLight& light) = delete;



    void bindUniforms(std::shared_ptr<PointLightShader> shader, Camera* shadowCamera);


    float getRadius() const;
    virtual void setRadius(float value);

    mat4 ModelMatrix();


    void createShadowMap(int w, int h, ShadowQuality quality = ShadowQuality::LOW);

    void bindFace(int face);
    void calculateCamera(int face);


    bool cullLight(Camera* camera);
    bool renderShadowmap(DepthFunction f, UniformBuffer& shadowCameraBuffer);
    void renderImGui();
};

}  // namespace Saiga
