/**
 * Copyright (c) 2021 Darius Rückert
 * Licensed under the MIT License.
 * See LICENSE file for more information.
 */


#include "AssetRenderSystem.h"

#include "saiga/opengl/shader/shaderLoader.h"

namespace Saiga
{
AssetRenderSystem::AssetRenderSystem()
{
    {
        const char* shaderStr = "asset/ColoredAsset.glsl";
        shader_colored_deferred =
            shaderLoader.load<MVPColorShader>(shaderStr, {{GL_FRAGMENT_SHADER, "#define DEFERRED", 1}});
        shader_colored_forward =
            shaderLoader.load<MVPColorShader>(shaderStr, {{GL_FRAGMENT_SHADER, "#define DEPTH", 1}});
        shader_colored_depth = shaderLoader.load<MVPColorShader>(shaderStr);
    }
    {
        const char* shaderStr = "asset/texturedAsset.glsl";
        shader_textured_deferred =
            shaderLoader.load<MVPTextureShader>(shaderStr, {{GL_FRAGMENT_SHADER, "#define DEFERRED", 1}});
        shader_textured_forward =
            shaderLoader.load<MVPTextureShader>(shaderStr, {{GL_FRAGMENT_SHADER, "#define DEPTH", 1}});
        shader_textured_depth = shaderLoader.load<MVPTextureShader>(shaderStr);
    }
}

void AssetRenderSystem::Clear()
{
    colored_assets.clear();
    textured_assets.clear();
}
void AssetRenderSystem::Render(Camera* cam, RenderPass render_pass)
{
    if (render_pass == RenderPass::Deferred)
    {
        // Colored assets
        if (!colored_assets.empty() && shader_colored_deferred->bind())
        {
            for (auto& data : colored_assets)
            {
                if (data.flags & RENDER_DEFAULT)
                {
                    shader_colored_deferred->uploadModel(data.model);
                    data.asset->renderRaw();
                }
            }
            shader_colored_deferred->unbind();
        }

        // Textured assets
        if (!textured_assets.empty() && shader_textured_deferred->bind())
        {
            for (auto& data : textured_assets)
            {
                if (data.flags & RENDER_DEFAULT)
                {
                    shader_textured_deferred->uploadModel(data.model);
                    data.asset->renderRaw(shader_textured_deferred.get());
                }
            }
            shader_textured_deferred->unbind();
        }
    }
    else if (render_pass == RenderPass::Shadow || render_pass == RenderPass::DepthPrepass)
    {
        // Colored assets
        if (!colored_assets.empty() && shader_colored_depth->bind())
        {
            for (auto& data : colored_assets)
            {
                if (data.flags & RENDER_SHADOW || render_pass == RenderPass::DepthPrepass)
                {
                    shader_colored_depth->uploadModel(data.model);
                    data.asset->renderRaw();
                }
            }
            shader_colored_depth->unbind();
        }
    }
}
void AssetRenderSystem::Add(Asset* asset, const mat4& transformation, int render_flags)
{
    if (auto a = dynamic_cast<ColoredAsset*>(asset))
    {
        colored_assets.push_back({a, transformation, render_flags});
    }

    if (auto a = dynamic_cast<TexturedAsset*>(asset))
    {
        textured_assets.push_back({a, transformation, render_flags});
    }
}

}  // namespace Saiga
