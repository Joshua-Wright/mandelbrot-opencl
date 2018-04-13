//
// Created by j0sh on 4/13/18.
//


#include <iostream>
#include "util.h"

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
