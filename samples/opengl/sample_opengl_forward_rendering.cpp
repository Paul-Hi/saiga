﻿/**
 * Copyright (c) 2017 Darius Rückert
 * Licensed under the MIT License.
 * See LICENSE file for more information.
 */



#include "saiga/core/imgui/imgui.h"
#include "saiga/core/math/random.h"
#include "saiga/core/model/model_from_shape.h"
#include "saiga/opengl/shader/shaderLoader.h"
#include "saiga/opengl/window/SampleWindowForward.h"
#include "saiga/opengl/world/skybox.h"


using namespace Saiga;

ImGui::IMTable test_table("Fancy Table", {10, 10}, {"First", "Second"});


class Sample : public SampleWindowForward
{
    using Base = SampleWindowForward;

   public:
    Sample()
    {
        for (int i = 0; i < 10000; ++i)
        {
            PointVertex v;
            v.position = linearRand(make_vec3(-3), make_vec3(3));
            v.color    = linearRand(make_vec3(0), make_vec3(1));

            pointCloud.points.push_back(v);
        }
        pointCloud.updateBuffer();

        {
            // let's just draw the coordiante axis
            PointVertex v;

            // x
            v.color    = vec3(1, 0, 0);
            v.position = vec3(-1, 0, 0);
            lineSoup.lines.push_back(v);
            v.position = vec3(1, 0, 0);
            lineSoup.lines.push_back(v);

            // y
            v.color    = vec3(0, 1, 0);
            v.position = vec3(0, -1, 0);
            lineSoup.lines.push_back(v);
            v.position = vec3(0, 1, 0);
            lineSoup.lines.push_back(v);

            // z
            v.color    = vec3(0, 0, 1);
            v.position = vec3(0, 0, -1);
            lineSoup.lines.push_back(v);
            v.position = vec3(0, 0, 1);
            lineSoup.lines.push_back(v);

            lineSoup.translateGlobal(vec3(5, 5, 5));
            lineSoup.calculateModel();

            lineSoup.lineWidth = 3;
            lineSoup.updateBuffer();
        }


        //    frustum.vertices.resize(2);
        //    frustum.vertices[0].position = vec4(0, 0, 0, 0);
        //    frustum.vertices[1].position = vec4(10, 10, 10, 0);
        //    frustum.fromLineList();

        //    frustum.createGrid({100, 100}, {0.1, 0.1});
        frustum = LineVertexColoredAsset(FrustumLineMesh(camera.proj, 1, false));
        //        frustum.setColor(vec4{0, 1, 0, 1});

        //        frustum.create();



        std::cout << "Program Initialized!" << std::endl;
    }



    void render(Camera* camera, RenderPass render_pass) override
    {
        Base::render(camera, render_pass);

        if (render_pass == RenderPass::Forward)
        {
            pointCloud.render(camera);
            lineSoup.render(camera);
            frustum.renderForward(camera, mat4::Identity());
        }
        else if (render_pass == RenderPass::GUI)
        {
            if (add_values_to_console)
            {
                console << Random::sampleDouble(0, 100000) << std::endl;
                test_table << Random::sampleDouble(0, 100000) << Random::sampleDouble(0, 100000);
            }

            ImGui::Begin("Saiga Sample");

            ImGui::Checkbox("add_values_to_console", &add_values_to_console);
            if (ImGui::Button("add"))
            {
                console << "asdf " << 234 << std::endl;
            }

            if (ImGui::Button("Screenshot"))
            {
                window->ScreenshotDefaultFramebuffer().save("screenshot.png");
            }

            ImGui::End();



            //            console.render();
            test_table.Render();
        }
    }


   private:
    Skybox skybox;
    bool add_values_to_console = false;
    GLPointCloud pointCloud;
    LineSoup lineSoup;
    LineVertexColoredAsset frustum;
};


int main(const int argc, const char* argv[])
{
    initSaigaSample();
    Sample example;
    example.run();
    return 0;
}
