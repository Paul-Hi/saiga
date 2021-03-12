/**
 * Copyright (c) 2020 Paul Himmler
 * Licensed under the MIT License.
 * See LICENSE file for more information.
 */


#pragma once
#include "saiga/core/camera/camera.h"
#include "saiga/core/window/Interfaces.h"
#include "saiga/opengl/framebuffer.h"
#include "saiga/opengl/indexedVertexBuffer.h"
#include "saiga/opengl/query/gpuTimer.h"
#include "saiga/opengl/rendering/lighting/light_clusterer.h"
#include "saiga/opengl/rendering/lighting/renderer_lighting.h"
#include "saiga/opengl/shader/basic_shaders.h"
#include "saiga/opengl/shaderStorageBuffer.h"
#include "saiga/opengl/uniformBuffer.h"
#include "saiga/opengl/vertex.h"

#include <set>

namespace Saiga
{
class SAIGA_OPENGL_API UberDeferredLightingShader : public DeferredShader
{
   public:
    GLint location_lightInfoBlock;

    GLint location_invProj;

    virtual void checkUniforms() override
    {
        DeferredShader::checkUniforms();

        location_lightInfoBlock = getUniformBlockLocation("lightInfoBlock");
        setUniformBlockBinding(location_lightInfoBlock, LIGHT_INFO_BINDING_POINT);


        location_invProj = getUniformLocation("invProj");
    }

    inline void uploadInvProj(const mat4& mat) { Shader::upload(location_invProj, mat); }
};

class SAIGA_OPENGL_API UberDeferredLighting : public RendererLighting
{
   public:
    ShaderStorageBuffer lightDataBufferPoint;
    ShaderStorageBuffer lightDataBufferSpot;
    ShaderStorageBuffer lightDataBufferDirectional;

    UniformBuffer lightInfoBuffer;

    UberDeferredLighting(GBuffer& gbuffer);
    UberDeferredLighting& operator=(UberDeferredLighting& l) = delete;
    ~UberDeferredLighting();

    void loadShaders() override;
    void init(int _width, int _height, bool _useTimers) override;
    void resize(int width, int height) override;

    void initRender() override;
    void render(Camera* cam, const ViewPort& viewPort) override;

    void renderImGui() override;

    void setLightMaxima(int maxDirectionalLights, int maxPointLights, int maxSpotLights) override;

    void setClusterType(int tp);

   public:
    std::shared_ptr<UberDeferredLightingShader> lightingShader;
    GBuffer& gbuffer;
    IndexedVertexBuffer<VertexNT, uint32_t> quadMesh;
    std::shared_ptr<Clusterer> lightClusterer;
    int clustererType = 0;
};

}  // namespace Saiga
