#include "util.h"
#include <iostream>
#include <thread>
#include <chrono>

#include <GL/glew.h>
#include <GL/gl.h>
#include <CL/cl2.hpp>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "shaders.h"

using std::cout;
using std::endl;
using cl::Context;

void drawTriangles();

GLuint vao;
GLuint vbo;
GLuint tbo;
GLuint ibo;
GLuint texture;
GLuint program;

int width = 1024;
int height = 1024;

Context context;

float matrix[16] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, -1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
};

const float vertices[12] = {
        -1.0f, -1.0f, 0.0,
        1.0f, -1.0f, 0.0,
        1.0f, 1.0f, 0.0,
        -1.0f, 1.0f, 0.0,
};

const float texcords[8] = {
        0.0, 1.0,
        1.0, 1.0,
        1.0, 0.0,
        0.0, 0.0,
};

const uint indices[6] = {0, 1, 2, 0, 2, 3};


void glfwErrorCallback(int error, const char *desc) {
    cout << error << ": " << desc << endl;
    exit(1);
}

void glfwKeyCallback(GLFWwindow *wind, int key, int scancode, int action, int mods) {
    cout << "glfw_key_callback " << key << " " << scancode << " " << action << " " << mods << endl;
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(wind, GL_TRUE);
                break;
            default:
                break;
        }
    }
}

void glfwWindowRefreshCallback(GLFWwindow *window) {
    drawTriangles();
    glfwSwapBuffers(window);
}


int main(int argc, char **argv) {
    if (!glfwInit()) {
        return 255;
    }

    glfwSetErrorCallback(glfwErrorCallback);
    GLFWwindow *window = glfwCreateWindow(width, height, "Mandelbrot Set", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        exit(1);
    }
    glfwMakeContextCurrent(window);

    GLenum err = glewInit();
    if (err != GLEW_OK) {
        cout << glewGetErrorString(err);
        exit(1);
    } else if (!GLEW_VERSION_2_1) {
        exit(1);
    }

    Device device = findOpenClDevice();

    program = makeShaderProgram(vertex_shader, fragment_shader);
    texture = makeTexture(width, height);
    GLuint vbo = makeBuffer(12, vertices, GL_STATIC_DRAW);
    GLuint tbo = makeBuffer(8, texcords, GL_STATIC_DRAW);
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * 6, indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // bind vao
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    // attach vbo
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);
    // attach tbo
    glBindBuffer(GL_ARRAY_BUFFER, tbo);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(1);
    // attach ibo
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBindVertexArray(0);

    glfwSetKeyCallback(window, glfwKeyCallback);
//    glfwSetScrollCallback(window, glfw_scroll_callback);
//    glfwSetCursorPosCallback(window, glfw_cursor_position_callback);
//    glfwSetMouseButtonCallback(window, glfw_mouse_button_callback);
//    glfwSetFramebufferSizeCallback(window, glfw_framebuffer_size_callback);
    glfwSetWindowRefreshCallback(window, glfwWindowRefreshCallback);

    glUseProgram(program);
    auto mat_loc = glGetUniformLocation(program, "matrix");
    auto tex_loc = glGetUniformLocation(program, "tex");
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(tex_loc, 0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glGenerateMipmap(GL_TEXTURE_2D);
    glUniformMatrix4fv(mat_loc, 1, GL_FALSE, matrix);
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    drawTriangles();
    glfwSwapBuffers(window);

    while (!glfwWindowShouldClose(window)) {
        using namespace std::chrono_literals;
        glfwPollEvents();
        std::this_thread::sleep_for(15ms);
    }
}

void drawTriangles() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}
