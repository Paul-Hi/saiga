/**
 * Copyright (c) 2017 Darius Rückert
 * Licensed under the MIT License.
 * See LICENSE file for more information.
 */

#pragma once

#include "saiga/core/util/quality.h"
#include "saiga/opengl/framebuffer.h"

namespace Saiga
{
struct SAIGA_OPENGL_API GBufferParameters
{
    Quality colorQuality  = Quality::MEDIUM;
    Quality normalQuality = Quality::MEDIUM;
    Quality dataQuality   = Quality::LOW;
    Quality depthQuality  = Quality::HIGH;
};

class SAIGA_OPENGL_API GBuffer : public Framebuffer
{
   public:
    GBuffer();
    GBuffer(int w, int h, GBufferParameters params);
    void init(int w, int h, GBufferParameters params);

    framebuffer_texture_t getTextureColor() { return this->colorBuffers[0]; }
    framebuffer_texture_t getTextureNormal() { return this->colorBuffers[1]; }
    framebuffer_texture_t getTextureData() { return this->colorBuffers[2]; }

    void sampleNearest();
    void sampleLinear();

    void clampToEdge();

   protected:
    GBufferParameters params;
};

}  // namespace Saiga
