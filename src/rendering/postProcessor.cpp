#include "saiga/rendering/postProcessor.h"
#include "saiga/geometry/triangle_mesh_generator.h"
#include "saiga/opengl/shader/shaderLoader.h"
#include "saiga/util/error.h"

void PostProcessingShader::checkUniforms(){
    Shader::checkUniforms();
    location_texture = Shader::getUniformLocation("image");
    location_screenSize = Shader::getUniformLocation("screenSize");
}


void PostProcessingShader::uploadTexture(raw_Texture *texture){
    texture->bind(0);
    Shader::upload(location_texture,0);
}

void PostProcessingShader::uploadScreenSize(vec4 size){
    Shader::upload(location_screenSize,size);
}






void LightAccumulationShader::checkUniforms(){
    DeferredShader::checkUniforms();
    location_lightAccumulationtexture = Shader::getUniformLocation("lightAccumulationtexture");
}


void LightAccumulationShader::uploadLightAccumulationtexture(raw_Texture *texture){
    texture->bind(4);
    Shader::upload(location_lightAccumulationtexture,4);
}




void PostProcessor::init(int width, int height)
{
    this->width=width;this->height=height;
    createFramebuffers();

    auto qb = TriangleMeshGenerator::createFullScreenQuadMesh();
    qb->createBuffers(quadMesh);

    timer.create();

    computeTest = ShaderLoader::instance()->load<Shader>("computeTest.glsl");
}

void PostProcessor::nextFrame(Framebuffer *gbuffer)
{
    gbuffer->blitDepth(framebuffers[0].id);
    currentBuffer = 0;
    lastBuffer = 1;

}

void PostProcessor::createFramebuffers()
{
    for(int i = 0 ;i <2 ;++i){
        framebuffers[i].create();

        if(i==0){
            Texture* depth_stencil = new Texture();
            depth_stencil->createEmptyTexture(width,height,GL_DEPTH_STENCIL, GL_DEPTH24_STENCIL8,GL_UNSIGNED_INT_24_8);
            framebuffers[i].attachTextureDepthStencil(depth_stencil);
        }


        textures[i] = new Texture();
        textures[i]->createEmptyTexture(width,height,GL_RGBA,GL_RGBA16,GL_UNSIGNED_SHORT);
        framebuffers[i].attachTexture(textures[i]);
        glDrawBuffer( GL_COLOR_ATTACHMENT0);
        framebuffers[i].check();
        framebuffers[i].unbind();
    }
}

void PostProcessor::bindCurrentBuffer()
{
    framebuffers[currentBuffer].bind();
}

void PostProcessor::switchBuffer()
{
    lastBuffer = currentBuffer;
    currentBuffer = (currentBuffer +1) %2;
}

void PostProcessor::render()
{
    int effects = postProcessingEffects.size();

    if(effects==0){
        cout<<"Warning no post processing effects specified. The screen will probably be black!"<<endl;
        return;
    }


    timer.startTimer();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    for(int i = 0 ; i < effects-1; ++i){
        applyShader(postProcessingEffects[i]);
        switchBuffer();
    }

    applyShaderFinal(postProcessingEffects[effects-1]);

    glEnable(GL_DEPTH_TEST);


    timer.stopTimer();

    //    std::cout<<"Time spent on the GPU: "<< timer.getTimeMS() <<std::endl;

}

void PostProcessor::resize(int width, int height)
{
    glDispatchCompute(512/16, 512/16, 1);
    this->width=width;this->height=height;
    framebuffers[0].resize(width,height);
    framebuffers[1].resize(width,height);
}

void PostProcessor::applyShader(PostProcessingShader *postProcessingShader)
{

    framebuffers[currentBuffer].bind();


    postProcessingShader->bind();
    vec4 screenSize(width,height,1.0/width,1.0/height);
    postProcessingShader->uploadScreenSize(screenSize);
    postProcessingShader->uploadTexture(textures[lastBuffer]);
    postProcessingShader->uploadAdditionalUniforms();
    quadMesh.bindAndDraw();
    postProcessingShader->unbind();

    framebuffers[currentBuffer].unbind();



}

void PostProcessor::applyShaderFinal(PostProcessingShader *postProcessingShader)
{



    auto textureId = textures[lastBuffer]->getId();


    computeTest->bind();

//    auto loc = computeTest->getUniformLocation("destTex");
//    computeTest->upload(loc,textures[lastBuffer],0);

    const GLint location = glGetUniformLocation(computeTest->program, "destTex");
     glUniform1i(location, 0);
    glBindImageTexture(0,textureId , 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16);


    GLint work_size[3];
    glGetProgramiv(computeTest->program, GL_COMPUTE_WORK_GROUP_SIZE, work_size);
    int res_x = 50, res_y = 50;
//    int res_x = width, res_y = height;
    int call_x = (res_x / work_size[0]) + (res_x % work_size[0] ? 1 : 0);
    int call_y = (res_y / work_size[1]) + (res_y % work_size[1] ? 1 : 0);
    glUniform2i(glGetUniformLocation(computeTest->program, "res"), res_x, res_y);
    glDispatchCompute(call_x, call_y, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    cout<<"dispatch compute "<<call_x<<" "<<call_y<<endl;


    computeTest->unbind();

    Error::quitWhenError("compute shader stuff");


    //shader post process + gamma correction
    glEnable(GL_FRAMEBUFFER_SRGB);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    postProcessingShader->bind();
    vec4 screenSize(width,height,1.0/width,1.0/height);
    postProcessingShader->uploadScreenSize(screenSize);
    postProcessingShader->uploadTexture(textures[lastBuffer]);
    postProcessingShader->uploadAdditionalUniforms();
    quadMesh.bindAndDraw();
    postProcessingShader->unbind();

    glDisable(GL_FRAMEBUFFER_SRGB);
}