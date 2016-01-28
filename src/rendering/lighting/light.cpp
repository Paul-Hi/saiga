#include "saiga/rendering/lighting/light.h"

void LightShader::checkUniforms(){
    DeferredShader::checkUniforms();
    location_color = getUniformLocation("color");
    location_depthBiasMV = getUniformLocation("depthBiasMV");
    location_depthTex = getUniformLocation("depthTex");
    location_readShadowMap = getUniformLocation("readShadowMap");
    location_shadowMapSize = getUniformLocation("shadowMapSize");
    location_invProj = getUniformLocation("invProj");
}


void LightShader::uploadColor(vec4 &color){
    Shader::upload(location_color,color);
}

void LightShader::uploadColor(vec3 &color, float intensity){
    vec4 c = vec4(color,intensity);
    Shader::upload(location_color,c);
}

void LightShader::uploadDepthBiasMV(mat4 &mat){
    Shader::upload(location_depthBiasMV,mat);
}

void LightShader::uploadInvProj(mat4 &mat){
    Shader::upload(location_invProj,mat);
}

void LightShader::uploadDepthTexture(raw_Texture* texture){

    texture->bind(5);
    Shader::upload(location_depthTex,5);
}

void LightShader::uploadShadow(float shadow){
    Shader::upload(location_readShadowMap,shadow);
}

void LightShader::uploadShadowMapSize(float w, float h)
{
    Shader::upload(location_shadowMapSize,vec4(w,h,1.0f/w,1.0f/h));
}

void Light::createShadowMap(int resX, int resY){

//    cout<<"Light::createShadowMap"<<endl;

    shadowmap.createFlat(resX,resY);

}

void Light::bindShadowMap(){
    shadowmap.bind();
}

void Light::unbindShadowMap(){

    shadowmap.unbind();

}

void Light::bindUniformsStencil(MVPShader& shader){
    shader.uploadModel(model);
}
