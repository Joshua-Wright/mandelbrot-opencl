//
// Created by j0sh on 4/13/18.
//

#ifndef MANDELBROT_OPENCL_UTIL_H
#define MANDELBROT_OPENCL_UTIL_H

#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#define GLFW_EXPOSE_NATIVE_X11
#define GLFW_EXPOSE_NATIVE_GLX

#include <string>
#include <GL/glew.h>
#include <GL/gl.h>

using std::string;

GLuint makeTexture(int width, int height, void *data);

GLuint makeBuffer(int size, const float *data, GLenum usage);

GLuint makeShaderProgram(const std::string &vertex_shader, const std::string &fragment_shader);

#endif //MANDELBROT_OPENCL_UTIL_H
