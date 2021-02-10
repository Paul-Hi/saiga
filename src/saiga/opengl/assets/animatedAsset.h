/**
 * Copyright (c) 2017 Darius Rückert
 * Licensed under the MIT License.
 * See LICENSE file for more information.
 */

#pragma once

#include "saiga/opengl/assets/asset.h"
#include "saiga/opengl/texture/Texture.h"
#include "saiga/opengl/uniformBuffer.h"

namespace Saiga
{
class AnimatedModel : public TriangleMesh<BoneVertexCD, uint32_t>
{
   public:
};

/**
 *
 * Select Armature and click Rest Position.
 *
 * Blender Collada animation export options: X = checked, O = unchecked
 *
 * X Apply Modifiers
 * X Selection Only
 * X Include Children
 * X Include Armature
 * X Include Shape Keys
 *
 * 0 Deform Bones only
 * 0 Export for Open Sim
 *
 * Note: The blender animation speed is equal to the loaded animation speed.
 * You can change blender's frame rate in the render options.
 * So if you set blender's frame rate to 30 fps and have your animation start at frame 0 and end at frame 30
 * the animation will play for exactly one second.
 *
 */

class SAIGA_OPENGL_API AnimatedAsset : public BasicAsset<AnimatedModel, MVPShader>
{
   public:
    // Default shaders
    // If you want to use your own load them and override the shader memebers in BasicAsset.
    static constexpr const char* shaderStr = "asset/AnimatedAsset.glsl";
    void loadDefaultShaders() override;

    using BasicAsset<AnimatedModel, MVPShader>::render;
    using BasicAsset<AnimatedModel, MVPShader>::renderDepth;

    int boneCount;

    std::map<std::string, int> boneMap;
    std::map<std::string, int> nodeindexMap;

    AlignedVector<mat4> boneOffsets;
    AlignedVector<mat4> inverseBoneOffsets;

    std::vector<Animation> animations;


    void render(Camera* cam, const mat4& model, UniformBuffer& boneMatrices);
    void renderDepth(Camera* cam, const mat4& model, UniformBuffer& boneMatrices);
};

}  // namespace Saiga
