#include <iostream>

#include <GL/glut.h>

using std::cout;
using std::endl;


void display();

int main(int argc, char **argv) {
    glutInit(&argc,argv);
    cout << "hello world" << endl;

    glutInitDisplayMode(GLUT_RGB);
    glutInitWindowSize(400,400);
    int win = glutCreateWindow("mandelbrot");

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
