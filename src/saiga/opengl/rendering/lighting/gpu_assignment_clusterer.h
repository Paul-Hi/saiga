/**
 * Copyright (c) 2020 Paul Himmler
 * Licensed under the MIT License.
 * See LICENSE file for more information.
 */

#pragma once
#include "saiga/opengl/rendering/lighting/clusterer.h"

namespace Saiga
{

class SAIGA_OPENGL_API LightAssignmentComputeShader : public Shader
{
   public:
    GLint location_lightInfoBlock;

    virtual void checkUniforms() override
    {
        Shader::checkUniforms();

        location_lightInfoBlock = getUniformBlockLocation("lightInfoBlock");
        setUniformBlockBinding(location_lightInfoBlock, LIGHT_INFO_BINDING_POINT);
    }
};

class SAIGA_OPENGL_API GPUAssignmentClusterer : public Clusterer
{
   public:
    GPUAssignmentClusterer(GLTimerSystem* timer);
    GPUAssignmentClusterer& operator=(GPUAssignmentClusterer& c) = delete;
    ~GPUAssignmentClusterer();

   private:
    void clusterLightsInternal(Camera* cam, const ViewPort& viewPort) override;

    void buildClusters(Camera* cam);

    //_d
    // Structures for AABB cluster boundaries.
    //

    struct clusterBounds
    {
        vec3 center;
        float pad0;
        vec3 extends;
        float pad1;
    };

    int allowedItemsPerCluster = 1024;
    std::vector<clusterBounds> cullingCluster;

    int allowedLights = 263000;
    ShaderStorageBuffer lightClusterDataBuffer;
    ShaderStorageBuffer clusterStructuresBuffer;

    float gridCount[3];

    std::shared_ptr<LightAssignmentComputeShader> lightAssignmentShader;

    const char* assignmentShaderString = "lighting/light_assignment.glsl";
};
}  // namespace Saiga
