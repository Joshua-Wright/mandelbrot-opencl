#include "pre_include.h"

#include <iostream>
#include <thread>
#include <chrono>

#include <GL/glew.h>
#include <GL/gl.h>
#include <CL/cl2.hpp>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <fstream>

#include "util.h"
#include "shaders.h"

using std::cout;
using std::endl;

void drawTriangles();

bool processTimeStep();

void glfw_error_callback(int error, const char *desc);

//int width = 1024;
//int height = 1024;
//double aspect_ratio = 1.0;
int width = 1920;
int height = 1080;
double aspect_ratio = double(height) / double(width);

// OpenCL state
cl::Context context;
cl::CommandQueue queue;
cl::Device device;
cl::Kernel mandelbrot_kernel;
cl::ImageGL opencl_texture;

// OpenGL state
GLuint vao;
GLuint vbo;
GLuint tbo;
GLuint ibo;
GLuint texture;
int matrix_loc;
GLuint shader_program;
GLFWwindow *window;

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

// sate for UI state machine
enum state_t {
    IDLE,
    INTERACT,
    RENDER,
} state = RENDER;

int frames_after_interact = 0;
const int interact_frame_wait = 10;
int lastRow = 0;
const size_t chunkSize = 16;
double scale = 2.0f;
double translate_x = 0.0f;
double translate_y = 0.0f;
bool screen_is_dirty = false;
double cursor_x = -1;
double cursor_y = -1;
bool mouse_is_clicked = false;

void setRenderInteract() {
    frames_after_interact = 0;
    lastRow = 0;
    state = INTERACT;
    screen_is_dirty = true;
}

void beginRender() {
    matrix[0] = 1.0f;
    matrix[5] = -1.0f;
    state = RENDER;
    frames_after_interact = 0;
    glBindTexture(GL_TEXTURE_2D, texture);
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, width, height);
    lastRow = 0;
}

void glfw_key_callback(GLFWwindow *wind, int key, int scancode, int action, int mods) {
//    cout << "glfw_key_callback " << key << " " << scancode << " " << action << " " << mods << endl;
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(wind, GL_TRUE);
                break;
            case GLFW_KEY_D:
                cout << "scale=" << scale << endl;
                cout << "translate_x=" << translate_x << endl;
                cout << "translate_y=" << translate_y << endl;
                break;
            case GLFW_KEY_R: {
                cout << "reloading kernel" << endl;
                cl_int opencl_error;
                std::ifstream sourceFile("mandelbrot.cl");
                if (!sourceFile.is_open()) {
                    cout << "failed to open source file mandelbrot.cl" << endl;
                    return;
                }
                std::string sourceCode(
                        std::istreambuf_iterator<char>(sourceFile),
                        (std::istreambuf_iterator<char>()));
                auto kernel_source = inferno_colormap + sourceCode + fractal_kernel;
                auto opencl_program = makeOpenClProgram(context, kernel_source, opencl_error);
                if (opencl_error != CL_SUCCESS) {
                    cout << "failed to make kernel" << endl;
                    return;
                }
                opencl_error = opencl_program.build({device});
                if (opencl_error != CL_SUCCESS) {
                    cout << "failed to build kernel" << endl;
                    std::string log = opencl_program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
                    cout << log << endl;
                    return;
                }
                auto new_mandelbrot_kernel = cl::Kernel(opencl_program, "fractal", &opencl_error);
                if (opencl_error != CL_SUCCESS) {
                    cout << "failed to set kernel" << endl;
                    return;
                }
                mandelbrot_kernel = new_mandelbrot_kernel;
                cout << "kernel reloaded successfully" << endl;
                beginRender();
                break;
            }
            default:
                break;
        }
    }
}

void glfw_window_refresh_callback(GLFWwindow *window) {
    drawTriangles();
    glfwSwapBuffers(window);
}

void glfw_scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    if (yoffset < 0.0) {
        scale *= 1.1;
        matrix[0] *= 0.9;
        matrix[5] *= 0.9;
        setRenderInteract();
    } else if (yoffset > 0.0) {
        scale *= 0.9;
        matrix[0] *= 1.1;
        matrix[5] *= 1.1;
        setRenderInteract();
    }
}

static void glfw_cursor_position_callback(GLFWwindow *window, double xpos, double ypos) {
//    cout << "glfw_cursor_position_callback " << xpos << " " << ypos << endl;
    if (mouse_is_clicked) {
        matrix[12] = (xpos - cursor_x) / width * 2;
        matrix[13] = -(ypos - cursor_y) / height * 2;
        setRenderInteract();
    }
}

void glfw_mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
//    cout << "glfw_mouse_button_callback " << button << " " << action << " " << mods << endl;
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        mouse_is_clicked = true;
        glfwGetCursorPos(window, &cursor_x, &cursor_y);
        setRenderInteract();
    } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        mouse_is_clicked = false;
        cursor_x = -1;
        cursor_y = -1;
        translate_x -= matrix[12] * scale;
        translate_y -= matrix[13] * scale * aspect_ratio;
        matrix[12] = 0;
        matrix[13] = 0;

        // copy framebuffer to the texture so dragging lines up nicely
        glBindTexture(GL_TEXTURE_2D, texture);
        glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, width, height);

        setRenderInteract();
    }
}

void mainLoopStateTransition() {
    using namespace std::chrono_literals;
    switch (state) {
        case IDLE:
            std::this_thread::sleep_for(15ms);
            break;
        case INTERACT:
            if (frames_after_interact > interact_frame_wait && !mouse_is_clicked) {
                beginRender();
            } else {
                frames_after_interact++;
            }
            break;
        case RENDER:
            bool more_render_needed = processTimeStep();
            if (!more_render_needed) { state = IDLE; }
            break;
    }
    if (screen_is_dirty) {
        drawTriangles();
        glfwSwapBuffers(window);
        screen_is_dirty = false;
    }
}

void mainLoop() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        mainLoopStateTransition();
    }
}

int main(int argc, char **argv) {
    if (!glfwInit()) {
        return 255;
    }

    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwSetErrorCallback(glfw_error_callback);
    window = glfwCreateWindow(width, height, "Mandelbrot Set", nullptr, nullptr);
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
    device = findOpenClDevice(platform, window);
    context = makeOpenCLContext(platform, device, window);
    cl_int opencl_error;
    queue = cl::CommandQueue(context, device, 0, &opencl_error);
    if (opencl_error != CL_SUCCESS) { return 201; }
    auto kernel_source = inferno_colormap + mandelbrot_cl + fractal_kernel;
    auto opencl_program = makeOpenClProgram(context, kernel_source, opencl_error);
    if (opencl_error != CL_SUCCESS) { return 202; }
    opencl_error = opencl_program.build({device});
    if (opencl_error != CL_SUCCESS) { return 203; }
    mandelbrot_kernel = cl::Kernel(opencl_program, "fractal", &opencl_error);
    if (opencl_error != CL_SUCCESS) { return 204; }

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
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(1);
    // attach ibo
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBindVertexArray(0);

    opencl_texture = cl::ImageGL(context, CL_MEM_READ_WRITE, GL_TEXTURE_2D, 0, texture, &opencl_error);
    if (opencl_error != CL_SUCCESS) {
        std::cout << "Failed to create OpenGL texture refrence: " << opencl_error << std::endl;
        return 250;
    }

    glfwSetKeyCallback(window, glfw_key_callback);
    glfwSetScrollCallback(window, glfw_scroll_callback);
    glfwSetCursorPosCallback(window, glfw_cursor_position_callback);
    glfwSetMouseButtonCallback(window, glfw_mouse_button_callback);
    glfwSetWindowRefreshCallback(window, glfw_window_refresh_callback);

    glUseProgram(shader_program);
    matrix_loc = glGetUniformLocation(shader_program, "matrix");
    auto tex_loc = glGetUniformLocation(shader_program, "tex");
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(tex_loc, 0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glGenerateMipmap(GL_TEXTURE_2D);
    glUniformMatrix4fv(matrix_loc, 1, GL_FALSE, matrix);
    glBindVertexArray(vao);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    drawTriangles();
    glfwSwapBuffers(window);

    mainLoop();

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(0);
}

void drawTriangles() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // set project matrix
    glUniformMatrix4fv(matrix_loc, 1, GL_FALSE, matrix);
    // now render stuff
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

bool processTimeStep() {
//    cout << "render " << lastRow << endl;
    cl::Event ev;
    size_t dims[] = {width, height};
    glFinish();

    std::vector<cl::Memory> objs;
    objs.clear();
    objs.push_back(opencl_texture);
    // flush opengl commands and wait for object acquisition
    cl_int res = queue.enqueueAcquireGLObjects(&objs, nullptr, &ev);
    ev.wait();
    if (res != CL_SUCCESS) {
        std::cout << "Failed acquiring GL object: " << res << std::endl;
        exit(248);
    }
    cl::NDRange local(16, 16);
    if (lastRow >= local[1] * divup(dims[1], local[1])) {
        lastRow = 0;
        cout << "idle render" << endl;
        screen_is_dirty = false;
        return false;
    }
    cl::NDRange offset(0, lastRow);
    cl::NDRange global(local[0] * divup(dims[0], local[0]), chunkSize);
    // set kernel arguments
    mandelbrot_kernel.setArg(0, opencl_texture);
    mandelbrot_kernel.setArg(1, (int) dims[0]);
    mandelbrot_kernel.setArg(2, (int) dims[1]);
    mandelbrot_kernel.setArg(3, scale);
    mandelbrot_kernel.setArg(4, scale * aspect_ratio);
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
    if (lastRow >= local[1] * divup(dims[1], local[1])) {
        lastRow = 0;
        cout << "finished render" << endl;
        screen_is_dirty = true;
        return false;
    } else {
        screen_is_dirty = true;
        return true;
    }
}

void glfw_error_callback(int error, const char *desc) {
    cout << error << ": " << desc << endl;
    exit(1);
}
