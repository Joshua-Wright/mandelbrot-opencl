//
// Created by j0sh on 4/13/18.
//
#pragma once

#include "pre_include.h"

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
