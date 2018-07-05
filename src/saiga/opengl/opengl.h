/**
 * Copyright (c) 2017 Darius Rückert 
 * Licensed under the MIT License.
 * See LICENSE file for more information.
 */

#pragma once

#include <saiga/config.h>
#include <vector>

#ifdef SAIGA_USE_GLEW
#include <GL/glew.h>
typedef int MemoryBarrierMask;
#endif

#ifdef SAIGA_USE_GLBINDING
#include <glbinding/gl/gl.h>
#include <glbinding/Binding.h>
//make sure nobody else includes gl.h after this
#define __gl_h_
//todo: fix glbinding support
using namespace gl;
#define GLFW_INCLUDE_NONE
#endif

namespace Saiga {

SAIGA_GLOBAL void initOpenGL();
SAIGA_GLOBAL void terminateOpenGL();
SAIGA_GLOBAL bool OpenGLisInitialized();

SAIGA_GLOBAL int getVersionMajor();
SAIGA_GLOBAL int getVersionMinor();
SAIGA_GLOBAL void printOpenGLVersion();

SAIGA_GLOBAL int getExtensionCount();
SAIGA_GLOBAL bool hasExtension(const std::string &ext);
SAIGA_GLOBAL std::vector<std::string> getExtensions();


enum class OpenGLVendor{
    Nvidia,
    Ati,
    Intel,
    Mesa,
    Unknown
};

SAIGA_GLOBAL OpenGLVendor getOpenGLVendor();

struct SAIGA_GLOBAL OpenGLParameters
{
    enum class Profile{
        ANY,
        CORE,
        COMPATIBILITY
    };
    Profile profile = Profile::CORE;

    bool debug = true;

    //all functionality deprecated in the requested version of OpenGL is removed
    bool forwardCompatible = false;

    int versionMajor = 3;
    int versionMinor = 2;

    /**
     *  Reads all paramters from the given config file.
     *  Creates the file with the default values if it doesn't exist.
     */
    void fromConfigFile(const std::string& file);
};

}

#define SAIGA_OPENGL_INCLUDED
