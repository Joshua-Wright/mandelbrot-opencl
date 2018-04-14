//
// Created by j0sh on 4/13/18.
//

#include "pre_include.h"

#include <iostream>
#include <sstream>
#include <string>

#include <GL/glew.h>
#include <GL/gl.h>
#include <CL/cl2.hpp>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "util.h"

#if defined (__APPLE__) || defined(MACOSX)
#define CL_GL_SHARING_EXT "cl_APPLE_gl_sharing"
#else
#define CL_GL_SHARING_EXT "cl_khr_gl_sharing"
#endif

#define NVIDIA_PLATFORM "NVIDIA"
#define AMD_PLATFORM    "AMD"
#define MESA_PLATFORM   "Clover"
#define INTEL_PLATFORM  "Intel"
#define APPLE_PLATFORM  "Apple"

using std::cout;
using std::endl;


GLuint makeTexture(int width, int height, void *data) {
    GLuint ret = 0;
    glGenTextures(1, &ret);
    glBindTexture(GL_TEXTURE_2D, ret);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    return ret;
}

GLuint makeBuffer(int size, const float *data, GLenum usage) {
    GLuint ret = 0;
    glGenBuffers(1, &ret);
    glBindBuffer(GL_ARRAY_BUFFER, ret);
    glBufferData(GL_ARRAY_BUFFER, size * sizeof(float), data, usage);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return ret;
}

GLuint makeShaderProgram(const std::string &vertex_shader, const std::string &fragment_shader) {
    GLuint v = glCreateShader(GL_VERTEX_SHADER);
    GLuint f = glCreateShader(GL_FRAGMENT_SHADER);

    const GLchar *v_str = vertex_shader.c_str();
    auto v_len = static_cast<GLint>(vertex_shader.length());
    glShaderSource(v, 1, &v_str, &v_len);

    const GLchar *f_str = fragment_shader.c_str();
    auto f_len = static_cast<GLint>(fragment_shader.length());
    glShaderSource(f, 1, &f_str, &f_len);

    GLint success;
    glCompileShader(v);
    glGetShaderiv(v, GL_COMPILE_STATUS, &success);
    if (!success) { abort(); }

    glCompileShader(f);
    glGetShaderiv(f, GL_COMPILE_STATUS, &success);
    if (!success) { abort(); }

    GLuint program = glCreateProgram();
    glAttachShader(program, v);
    glAttachShader(program, f);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) { abort(); }

    return program;
}

bool checkClGlSharingExtensionAvailability(const cl::Device &pDevice) {
    bool ret = true;
    // find extensions required
    std::string exts = pDevice.getInfo<CL_DEVICE_EXTENSIONS>();
    std::stringstream ss(exts);
    std::string item;
    int found = -1;
    while (std::getline(ss, item, ' ')) {
        if (item == CL_GL_SHARING_EXT) {
            found = 1;
            break;
        }
    }
    if (found == 1) {
        std::cout << "Found CL_GL_SHARING extension: " << item << std::endl;
        ret = true;
    } else {
        std::cout << "CL_GL_SHARING extension not found\n";
        ret = false;
    }
    return ret;
}

cl::Platform getPlatform(const std::string &pName, cl_int &error) {
    using cl::Platform;
    using PlatformIter = std::vector<Platform>::iterator;

    Platform ret_val;
    error = 0;
    // Get available platforms
    std::vector<Platform> platforms;
    Platform::get(&platforms);
    int found = -1;
    for (PlatformIter it = platforms.begin(); it < platforms.end(); ++it) {
        std::string temp = it->getInfo<CL_PLATFORM_NAME>();
        if (temp.find(pName) != std::string::npos) {
            found = it - platforms.begin();
            std::cout << "Found platform: " << temp << std::endl;
            break;
        }
    }
    if (found == -1) {
        // Going towards + numbers to avoid conflict with OpenCl error codes
        error = +1; // requested platform not found
    } else {
        ret_val = platforms[found];
    }
    return ret_val;
}

cl::Platform findOpenClPlatform() {
    cl_int errCode;
    cl::Platform plat;

    auto platformNames = {
            NVIDIA_PLATFORM,
            AMD_PLATFORM,
            INTEL_PLATFORM,
            APPLE_PLATFORM,
            MESA_PLATFORM,
    };

    for (auto &name: platformNames) {
        plat = getPlatform(name, errCode);
        if (errCode == CL_SUCCESS)
            return plat;
    }
    // If no platforms are found
    cout << "no supported platform found" << endl;
    exit(252);
}


cl::Device findOpenClDevice(const cl::Platform &platform, GLFWwindow *window) {

    std::vector<cl::Device> devices;
    platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);
    // Get a list of devices on this platform
    for (const auto &device : devices) {
        if (checkClGlSharingExtensionAvailability(device)) {
            return device;
        }
    }
    std::cout << "you need a GPU" << std::endl;
    exit(1);
}

cl::Program makeOpenClProgram(const cl::Context &pContext, const std::string &sourceCode, cl_int &error) {
    cl::Program program;
    error = 0;
    cl::Program::Sources source(1, sourceCode);
    // Make program of the source code in the context
    program = cl::Program(pContext, source);
    return program;
}

cl::Context makeOpenCLContext(const cl::Platform &platform, cl::Device &device, GLFWwindow *window) {
#ifdef OS_LNX
    cl_context_properties cps[] = {
            CL_GL_CONTEXT_KHR, (cl_context_properties) glfwGetGLXContext(window),
            CL_GLX_DISPLAY_KHR, (cl_context_properties) glfwGetX11Display(),
            CL_CONTEXT_PLATFORM, (cl_context_properties) platform(),
            0};
#endif
#ifdef OS_WIN
    cl_context_properties cps[] = {
            CL_GL_CONTEXT_KHR, (cl_context_properties)glfwGetWGLContext(window),
            CL_WGL_HDC_KHR, (cl_context_properties)GetDC(glfwGetWin32Window(window)),
            CL_CONTEXT_PLATFORM, (cl_context_properties)lPlatform(),
            0};
#endif
    return cl::Context(device, cps);
}
