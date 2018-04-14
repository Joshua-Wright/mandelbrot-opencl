//
// Created by j0sh on 4/13/18.
//
#pragma once

#include "pre_include.h"

#include <string>

#include <GL/glew.h>
#include <GL/gl.h>
#include <CL/cl2.hpp>
#include <GLFW/glfw3.h>

using std::string;

GLuint makeTexture(int width, int height, void *data = nullptr);

GLuint makeBuffer(int size, const float *data, GLenum usage);

GLuint makeShaderProgram(const std::string &vertex_shader, const std::string &fragment_shader);

cl::Device findOpenClDevice(const cl::Platform &platform, GLFWwindow *window);

cl::Platform findOpenClPlatform();

cl::Context makeOpenCLContext(const cl::Platform &platform, cl::Device &device, GLFWwindow *window);

cl::Program makeOpenClProgram(const cl::Context &pContext, const std::string &sourceCode, cl_int &error);

inline unsigned divup(unsigned a, unsigned b) {
    return (a + b - 1) / b;
}