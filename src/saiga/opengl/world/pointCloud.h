/**
 * Copyright (c) 2017 Darius Rückert
 * Licensed under the MIT License.
 * See LICENSE file for more information.
 */

#pragma once

#include "saiga/core/geometry/object3d.h"
#include "saiga/opengl/indexedVertexBuffer.h"
#include "saiga/opengl/shader/basic_shaders.h"
#include "saiga/opengl/texture/CubeTexture.h"
#include "saiga/opengl/texture/Texture.h"
#include "saiga/opengl/vertex.h"

namespace Saiga
{
struct SAIGA_OPENGL_API PointVertex
{
    vec3 position;
    vec3 color;
};



class SAIGA_OPENGL_API GLPointCloud : public Object3D
{
   public:
    bool splat_geometry = false;
    float screen_point_size = 3;
    float world_point_size = 0.1;
    std::vector<PointVertex> points;

    GLPointCloud();
    void render(Camera* cam);
    void updateBuffer();

    std::shared_ptr<MVPShader> shader_simple, shader_geometry;
    VertexBuffer<PointVertex> buffer;

    void imgui();
};


template <>
SAIGA_OPENGL_API void VertexBuffer<PointVertex>::setVertexAttributes();

}  // namespace Saiga
