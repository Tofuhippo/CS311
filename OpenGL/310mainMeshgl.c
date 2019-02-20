/*
Dawson d'Almeida and Justin T. Washington
February 15 2018
CS311 with Josh Davis

Main abstracted file that uses openGL to generate a sphere.
*/


/* On macOS, compile with...
    clang 310mainMeshgl.c -lglfw -framework OpenGL -Wno-deprecated
*/

#include <stdio.h>
#include <math.h>
#include <sys/time.h>
#include <GLFW/glfw3.h>

#include "stdlib.h"
#include "310vector.c"
#include "310mesh.c"
#include "310meshgl.c"

double getTime(void) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (double)tv.tv_sec + (double)tv.tv_usec * 0.000001;
}

void handleError(int error, const char *description) {
	fprintf(stderr, "handleError: %d\n%s\n", error, description);
}

void handleResize(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
}

GLuint buffers[2];
double angleDegree = 0.0;

meshglMesh mesh;

void initializeMesh(void) {
	meshMesh base;
	meshInitializeSphere(&base, 1, 16.0, 32.0);
	meshglInitialize(&mesh, &base);
	meshDestroy(&base);
}

/* This weird macro helps us index the GPU-side buffers in the render function
below. It is taken verbatim from the OpenGL Programming Guide, 7th Ed. */
#define BUFFER_OFFSET(bytes) ((GLubyte*) NULL + (bytes))

void render(double oldTime, double newTime) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-2.0, 2.0, -2.0, 2.0, -2.0, 2.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	GLfloat light[4] = {0.0, 1.0, 1.0, 0.0};
	glLightfv(GL_LIGHT0, GL_POSITION, light);
	angleDegree += 10.0 * (newTime - oldTime);
	glRotatef(angleDegree, 1.0, 1.0, 1.0);

	glBindBuffer(GL_ARRAY_BUFFER, mesh.buffers[0]);
	glVertexPointer(3, GL_DOUBLE, mesh.attrDim * sizeof(GLdouble), BUFFER_OFFSET(0));
	glNormalPointer(GL_DOUBLE, mesh.attrDim * sizeof(GLdouble), BUFFER_OFFSET(0));
	glColorPointer(3, GL_DOUBLE, mesh.attrDim * sizeof(GLdouble),
		BUFFER_OFFSET(3 * sizeof(GLdouble)));

	meshglRender(&mesh);
}

int main(void) {
	double oldTime;
	double newTime = getTime();
    glfwSetErrorCallback(handleError);
    if (glfwInit() == 0)
        return 1;
    GLFWwindow *window;
    window = glfwCreateWindow(768, 512, "Learning OpenGL 1.5", NULL, NULL);
    if (window == NULL) {
        glfwTerminate();
        return 2;
    }
    glfwSetWindowSizeCallback(window, handleResize);
    glfwMakeContextCurrent(window);
    fprintf(stderr, "main: OpenGL %s, GLSL %s.\n",
		glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    /* Initialize the mesh. Even though the mesh is stored in GPU memory, we
    still must call glEnableClientState, to tell OpenGL to incorporate the
    right kinds of attributes into the rendering. */
		glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		initializeMesh();
    while (glfwWindowShouldClose(window) == 0) {
      oldTime = newTime;
    	newTime = getTime();
			if (floor(newTime) - floor(oldTime) >= 1.0)
				printf("main: %f frames/sec\n", 1.0 / (newTime - oldTime));

    	render(oldTime, newTime);
    	glfwSwapBuffers(window);
    	glfwPollEvents();
    }
    /* Don't forget to deallocate the buffers that we allocated above. */
		meshglDestroy(&mesh);
		glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
