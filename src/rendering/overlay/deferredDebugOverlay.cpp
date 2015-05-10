#include "rendering/overlay/deferredDebugOverlay.h"
#include "libhello/opengl/basic_shaders.h"
#include "libhello/geometry/triangle_mesh.h"
#include "libhello/opengl/framebuffer.h"



DeferredDebugOverlay::DeferredDebugOverlay(int width, int height):width(width),height(height){
    proj = glm::ortho(0.0f,(float)width,0.0f,(float)height,1.0f,-1.0f);

    TriangleMesh<VertexNT, GLuint> tm;


      VertexNT verts[4];
      //bottom left
      verts[0] = VertexNT(vec3(-1,-1,0),
                          vec3(0,0,1),
                          vec2(0,0));
      //bottom right
      verts[1] = VertexNT(vec3(1,-1,0),
                          vec3(0,0,1),
                          vec2(1,0));
      //top right
      verts[2] = VertexNT(vec3(1,1,0),
                          vec3(0,0,1),
                          vec2(1,1));
      //top left
      verts[3] = VertexNT(vec3(-1,1,0),
                          vec3(0,0,1),
                          vec2(0,1));
    tm.addQuad(verts);


    tm.createBuffers(buffer);


    setScreenPosition(&color,0);
    setScreenPosition(&normal,1);
    setScreenPosition(&depth,2);
    setScreenPosition(&data,3);

}

void DeferredDebugOverlay::setScreenPosition(DeferredDebugOverlay::GbufferTexture *gbt, int id)
{
    float images = 4;

    float s = 1.0f/images;
    gbt->setScale(vec3(s));

    float dy = -s*2.0f;
    float y = id*dy+dy*0.5f+1.0f;
    gbt->translateGlobal(vec3(1.0f-s,y,0));
    gbt->calculateModel();
}

void DeferredDebugOverlay::render(){
    shader->bind();

    shader->uploadModel(color.model);
    shader->uploadTexture(color.texture);
    buffer.bindAndDraw();

    shader->uploadModel(data.model);
    shader->uploadTexture(data.texture);
    buffer.bindAndDraw();

    shader->unbind();


     normalShader->bind();
    normalShader->uploadModel(normal.model);
    normalShader->uploadTexture(normal.texture);
    buffer.bindAndDraw();
    normalShader->unbind();

    depthShader->bind();

    depthShader->uploadModel(depth.model);
    depthShader->uploadTexture(depth.texture);
    buffer.bindAndDraw();

    depthShader->unbind();


}

void DeferredDebugOverlay::setDeferredFramebuffer(Framebuffer *fb)
{
    color.texture = fb->colorBuffers[0];
    normal.texture = fb->colorBuffers[1];
    depth.texture = fb->depthBuffer;
    data.texture = fb->colorBuffers[2];
}