/**
 * Copyright (c) 2017 Darius Rückert
 * Licensed under the MIT License.
 * See LICENSE file for more information.
 */


#if defined(DEFERRED)
#include "geometry/geometry_helper_fs.glsl"
#elif defined(DEPTH)
#else
layout(location=0) out vec4 out_color;
#if defined(FORWARD_LIT)
#include "lighting/uber_lighting_helpers.glsl"
#endif
#endif


struct AssetMaterial
{
    vec4 color;
    vec4 data;
};

void render(AssetMaterial material, vec3 position, vec3 normal)
{
#if defined(DEFERRED)
    setGbufferData(vec3(material.color),normal,material.data);
#elif defined(DEPTH)
#else
#if defined(FORWARD_LIT)
    vec3 lighting = vec3(0);

    lighting += calculatePointLights(material, position, normal, gl_FragCoord.z);
    lighting += calculateSpotLights(material, position, normal, gl_FragCoord.z);
    lighting += calculateDirectionalLights(material, position, normal);

    out_color = vec4(lighting, 1);
#else
    out_color = material.color;
#endif
#endif
}

