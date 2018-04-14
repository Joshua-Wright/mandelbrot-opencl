#include "pre_include.h"

#include <iostream>
#include <thread>
#include <chrono>

#include <GL/glew.h>
#include <GL/gl.h>
#include <CL/cl2.hpp>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "util.h"
#include "shaders.h"

using std::cout;
using std::endl;

void drawTriangles();

void processTimeStep();

GLuint vao;
GLuint vbo;
GLuint tbo;
GLuint ibo;
GLuint texture;
GLuint shader_program;

int width = 1024;
int height = 1024;

cl::Context context;
cl::CommandQueue queue;
cl::Kernel mandelbrot_kernel;
cl::ImageGL opencl_texture;

int lastRow = 0;
bool doRender = true;
const size_t chunkSize = 16;
float scale = 2.0f;
float translate_x = 0.0f;
float translate_y = 0.0f;

bool screen_is_dirty = false;


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

    auto platform = findOpenClPlatform();
    auto device = findOpenClDevice(platform, window);
    context = makeOpenCLContext(platform, device, window);
    cl_int opencl_error;
    auto opencl_program = makeOpenClProgram(context, mandelbrot_cl, opencl_error);
    queue = cl::CommandQueue(context, device);
    opencl_program.build({device});
    mandelbrot_kernel = cl::Kernel(opencl_program, "fractal");


    shader_program = makeShaderProgram(vertex_shader, fragment_shader);
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

    opencl_texture = cl::ImageGL(context, CL_MEM_READ_WRITE, GL_TEXTURE_2D, 0, texture, &opencl_error);
    if (opencl_error != CL_SUCCESS) {
        std::cout << "Failed to create OpenGL texture refrence: " << opencl_error << std::endl;
        return 250;
    }

    glfwSetKeyCallback(window, glfwKeyCallback);
//    glfwSetScrollCallback(window, glfw_scroll_callback);
//    glfwSetCursorPosCallback(window, glfw_cursor_position_callback);
//    glfwSetMouseButtonCallback(window, glfw_mouse_button_callback);
//    glfwSetFramebufferSizeCallback(window, glfw_framebuffer_size_callback);
    glfwSetWindowRefreshCallback(window, glfwWindowRefreshCallback);

    glUseProgram(shader_program);
    auto mat_loc = glGetUniformLocation(shader_program, "matrix");
    auto tex_loc = glGetUniformLocation(shader_program, "tex");
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
        processTimeStep();
        if (screen_is_dirty) {
            drawTriangles();
            glfwSwapBuffers(window);
            screen_is_dirty = false;
        }
        glfwPollEvents();
        std::this_thread::sleep_for(15ms);
    }
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

void drawTriangles() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void processTimeStep() {
    cout << "render " << lastRow << endl;
    if (!doRender) {
        cout << "idle render" << endl;
        return;
    }
    cl::Event ev;
    size_t dims[] = {width, height};
    try {
        glFinish();

        std::vector<cl::Memory> objs;
        objs.clear();
        objs.push_back(opencl_texture);
        // flush opengl commands and wait for object acquisition
        cl_int res = queue.enqueueAcquireGLObjects(&objs, NULL, &ev);
        ev.wait();
        if (res != CL_SUCCESS) {
            std::cout << "Failed acquiring GL object: " << res << std::endl;
            exit(248);
        }
        cl::NDRange local(16, 16);
        if (lastRow >= local[1] * divup(dims[1], local[1])) {
            lastRow = 0;
            doRender = false;
            cout << "finished render" << endl;
            return;
        }
        cl::NDRange offset(0, lastRow);
        cl::NDRange global(local[0] * divup(dims[0], local[0]), chunkSize);
        // set kernel arguments
        mandelbrot_kernel.setArg(0, opencl_texture);
        mandelbrot_kernel.setArg(1, (int) dims[0]);
        mandelbrot_kernel.setArg(2, (int) dims[1]);
        mandelbrot_kernel.setArg(3, scale);
        mandelbrot_kernel.setArg(4, scale);
        mandelbrot_kernel.setArg(5, translate_x);
        mandelbrot_kernel.setArg(6, translate_y);
        queue.enqueueNDRangeKernel(mandelbrot_kernel, offset, global, local);
        // release opengl object
        res = queue.enqueueReleaseGLObjects(&objs);
        ev.wait();
        if (res != CL_SUCCESS) {
            std::cout << "Failed releasing GL object: " << res << std::endl;
            exit(247);
        }
        queue.finish();
        lastRow += chunkSize;
        screen_is_dirty = true;
    } catch (cl::Error err) {
        std::cout << err.what() << "(" << err.err() << ")" << std::endl;
    }
}
