//
// Created by j0sh on 4/13/18.
//

#include <iostream>
#include <sstream>
#include <string>
#include "util.h"



#if defined (__APPLE__) || defined(MACOSX)
static const std::string CL_GL_SHARING_EXT = "cl_APPLE_gl_sharing";
#else
static const std::string CL_GL_SHARING_EXT = "cl_khr_gl_sharing";
#endif

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

bool checkExtnAvailability(const Device &pDevice) {
    using cl::Error;
    bool ret_val = true;
    try {
        // find extensions required
        std::string exts = pDevice.getInfo<CL_DEVICE_EXTENSIONS>();
        std::stringstream ss(exts);
        std::string item;
        int found = -1;
        while (std::getline(ss,item,' ')) {
            if (item==CL_GL_SHARING_EXT) {
                found=1;
                break;
            }
        }
        if (found==1) {
            std::cout<<"Found CL_GL_SHARING extension: "<<item<<std::endl;
            ret_val = true;
        } else {
            std::cout<<"CL_GL_SHARING extension not found\n";
            ret_val = false;
        }
    } catch (Error err) {
        std::cout << err.what() << "(" << err.err() << ")" << std::endl;
    }
    return ret_val;
}


static const std::string NVIDIA_PLATFORM = "NVIDIA";
static const std::string AMD_PLATFORM = "AMD";
static const std::string MESA_PLATFORM = "Clover";
static const std::string INTEL_PLATFORM = "Intel";
static const std::string APPLE_PLATFORM = "Apple";

cl::Platform getPlatform(std::string pName, cl_int &error)
{
    using cl::Platform;
    using cl::Error;
    using PlatformIter = std::vector<Platform>::iterator;

    Platform ret_val;
    error = 0;
    try {
        // Get available platforms
        std::vector<Platform> platforms;
        Platform::get(&platforms);
        int found = -1;
        for(PlatformIter it=platforms.begin(); it<platforms.end(); ++it) {
            std::string temp = it->getInfo<CL_PLATFORM_NAME>();
            if (temp.find(pName)!=std::string::npos) {
                found = it - platforms.begin();
                std::cout<<"Found platform: "<<temp<<std::endl;
                break;
            }
        }
        if (found==-1) {
            // Going towards + numbers to avoid conflict with OpenCl error codes
            error = +1; // requested platform not found
        } else {
            ret_val = platforms[found];
        }
    } catch(Error err) {
        std::cout << err.what() << "(" << err.err() << ")" << std::endl;
        error = err.err();
    }
    return ret_val;
}


#define FIND_PLATFORM(name)             \
    plat = getPlatform(name, errCode);  \
    if (errCode == CL_SUCCESS)          \
        return plat;

cl::Platform getPlatform()
{
    cl_int errCode;
    cl::Platform plat;

    FIND_PLATFORM(NVIDIA_PLATFORM)
    FIND_PLATFORM(AMD_PLATFORM)
    FIND_PLATFORM(INTEL_PLATFORM)
    FIND_PLATFORM(APPLE_PLATFORM)
    FIND_PLATFORM(MESA_PLATFORM)

    // If no platforms are found
    exit(252);
}


Device findOpenClDevice() {
    using cl::Platform;
    Platform lPlatform = getPlatform();

    cl_context_properties cps[] = {
            CL_CONTEXT_PLATFORM, (cl_context_properties)lPlatform(),
            0};
    std::vector<Device> devices;
    lPlatform.getDevices(CL_DEVICE_TYPE_GPU, &devices);
    // Get a list of devices on this platform
    for (unsigned d = 0; d < devices.size(); ++d) {
        if (checkExtnAvailability(devices[d])) {
            return devices[d];
        }
    }
    std::cout << "you need a GPU" << std::endl;
    exit(1);
}
