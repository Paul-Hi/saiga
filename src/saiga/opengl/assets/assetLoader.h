/**
 * Copyright (c) 2017 Darius Rückert
 * Licensed under the MIT License.
 * See LICENSE file for more information.
 */

#pragma once

#include "saiga/core/util/color.h"
#include "saiga/opengl/animation/boneShader.h"

#include "animatedAsset.h"
#include "coloredAsset.h"

namespace Saiga
{
class SAIGA_OPENGL_API AssetLoader
{
   public:
    AssetLoader();
    virtual ~AssetLoader();

    /**
     * Creates a plane with a checker board texture.
     * The plane lays in the x-z plane with a normal pointing to positve y.
     * size[0] and size[1] are the dimensions of the plane.
     * quadSize is the size of one individual quad of the checkerboard.
     */

    std::shared_ptr<TexturedAsset> loadDebugPlaneAsset(vec2 size, float quadSize = 1.0f,
                                                       Color color1 = Colors::lightgray, Color color2 = Colors::gray);
    std::shared_ptr<ColoredAsset> loadDebugPlaneAsset2(ivec2 size, float quadSize = 1.0f,
                                                       Color color1 = Colors::lightgray, Color color2 = Colors::gray);

    std::shared_ptr<TexturedAsset> loadDebugTexturedPlane(std::shared_ptr<Texture> texture, vec2 size);

    std::shared_ptr<ColoredAsset> loadDebugGrid(int numX, int numY, float quadSize = 1.0f, Color color = Colors::gray);

    std::shared_ptr<ColoredAsset> loadDebugArrow(float radius, float length, vec4 color = vec4(1, 0, 0, 1));

    std::shared_ptr<ColoredAsset> assetFromMesh(TriangleMesh<VertexNC, GLuint>& mesh);
    std::shared_ptr<ColoredAsset> assetFromMesh(TriangleMesh<VertexNT, GLuint>& mesh,
                                                const vec4& color = vec4(1, 1, 1, 1));

    std::shared_ptr<ColoredAsset> nonTriangleMesh(std::vector<vec3> vertices, std::vector<GLuint> indices,
                                                  GLenum mode = GL_TRIANGLES, const vec4& color = vec4(1, 1, 1, 1));

    std::shared_ptr<ColoredAsset> frustumMesh(const mat4& proj, const vec4& color = vec4(1, 1, 1, 1));
};

}  // namespace Saiga
