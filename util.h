//
// Created by j0sh on 4/13/18.
//

#ifndef MANDELBROT_OPENCL_UTIL_H
#define MANDELBROT_OPENCL_UTIL_H

#ifdef OS_WIN
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#endif

#ifdef OS_LNX
#define GLFW_EXPOSE_NATIVE_X11
#define GLFW_EXPOSE_NATIVE_GLX
#endif

#define CL_HPP_ENABLE_EXCEPTIONS

#include <string>
#include <GL/glew.h>
#include <GL/gl.h>
#include <CL/cl.h>
#include <CL/cl2.hpp>

using std::string;
using cl::Device;

GLuint makeTexture(int width, int height, void *data = nullptr);

GLuint makeBuffer(int size, const float *data, GLenum usage);

GLuint makeShaderProgram(const std::string &vertex_shader, const std::string &fragment_shader);

Device findOpenClDevice();

#endif //MANDELBROT_OPENCL_UTIL_H
