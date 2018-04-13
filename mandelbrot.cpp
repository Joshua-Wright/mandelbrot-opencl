#include <iostream>

#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/gl.h>

#include "util.h"

using std::cout;
using std::endl;

std::string fragment_shader = R"'''(
#version 330

uniform sampler2D tex;
in vec2 texcoord;

out vec4 fragColor;

void main()
{
    fragColor = texture2D(tex,texcoord);
}
)'''";

std::string vertex_shader = R"'''(
#version 330

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 tex;

uniform mat4 matrix;

out vec2 texcoord;

void main() {
    texcoord = tex;
    gl_Position = matrix * vec4(pos,1.0);
}
)'''";


void display();

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    cout << "hello world" << endl;

    glutInitDisplayMode(GLUT_RGB);
    glutInitWindowSize(400, 400);
    int win = glutCreateWindow("mandelbrot");

    GLenum err = glewInit();

    if (err != GLEW_OK) {
        cout << glewGetErrorString(err);
        exit(1);
    } else if (!GLEW_VERSION_2_1) {
        exit(1);
    }

    auto program = makeShaderProgram(vertex_shader, fragment_shader);

    glutDisplayFunc(display);
    glutMainLoop();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(1, 0, 0);
    glBegin(GL_TRIANGLES);
    {
        glVertex2f(1, 0);
        glVertex2f(0, 0);
        glVertex2f(0, 1);
    }
    glEnd();
    glutSwapBuffers();
}
