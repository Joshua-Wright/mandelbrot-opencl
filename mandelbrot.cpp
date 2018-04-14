#include <iostream>

#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/gl.h>
#include <CL/cl2.hpp>

#include "util.h"
#include "shaders.h"

using std::cout;
using std::endl;
using cl::Context;

void display();

void keyboard(unsigned char key, int x, int y);


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

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 27: //escape
            exit(0);
            break;
        default:
            break;
    }
}


int main(int argc, char **argv) {
    glutInit(&argc, argv);
    cout << "hello world" << endl;

    glutInitDisplayMode(GLUT_RGB);
    glutInitWindowSize(width, height);
    int win = glutCreateWindow("mandelbrot");

    GLenum err = glewInit();

    if (err != GLEW_OK) {
        cout << glewGetErrorString(err);
        exit(1);
    } else if (!GLEW_VERSION_2_1) {
        exit(1);
    }

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

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);

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

    glutMainLoop();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glutSwapBuffers();
}
