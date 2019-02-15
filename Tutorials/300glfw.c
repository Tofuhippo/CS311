


/* On macOS, compile with...
    clang 300glfw.c -lglfw -framework OpenGL -Wno-deprecated
*/

#include <stdio.h>
#include <math.h>
#include <sys/time.h>
#include <GLFW/glfw3.h>

/* Returns the time, in seconds, since some specific moment in the distant
past. Supposedly accurate down to microseconds, but I wouldn't count on it. */
double getTime(void) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (double)tv.tv_sec + (double)tv.tv_usec * 0.000001;
}

/* This is a callback, which we register with GLFW below. It is automatically
called whenever GLFW throws an error. */
void handleError(int error, const char *description) {
	fprintf(stderr, "handleError: %d\n%s\n", error, description);
}

/* This is another callback, that is automatically called when the GLFW window
is resized. */
void handleResize(GLFWwindow *window, int width, int height) {
    /* Tell OpenGL to update the viewport transformation. By the way, all
    OpenGL function names begin with 'gl' (and not 'glfw'). */
    glViewport(0, 0, width, height);
}

/* This helper function uses OpenGL to render the window. */
void render(double oldTime, double newTime) {
	/* Clear the OpenGL framebuffer to black, before we start rendering. By the
	way, all OpenGL constants have names beginning with 'GL_'. */
	glClear(GL_COLOR_BUFFER_BIT);
	/* OpenGL's projection matrix corresponds to the matrix P in our software
	graphics engine. Here we set an orthographic projection, in terms of left,
	right, bottom, top, near, and far parameters. */
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
	/* This is the simplest way to draw a triangle in OpenGL. It is also the
	worst way. We never use it again after this tutorial. */
	glBegin(GL_TRIANGLES);
	glColor3f(1.0, 1.0, 0.0);
	glVertex2f(0.1, 0.1);
	glColor3f(0.0, 1.0, 1.0);
	glVertex2f(0.9, 0.2);
	glColor3f(1.0, 0.0, 1.0);
	glVertex2f(0.6, 0.9);
	glEnd();

	glBegin(GL_TRIANGLES);
	glColor3f(1.0, 1.0, 0.0);
	glVertex2f(0.01, 0.01);
	glColor3f(0.0, 1.0, 1.0);
	glVertex2f(0.09, 0.02);
	glColor3f(1.0, 0.0, 1.0);
	glVertex2f(0.06, 0.09);
	glEnd();
}

int main(void) {
	/* Record the current time. On the first frame of animation, it will serve
	as the 'old time'. */
	double oldTime;
	double newTime = getTime();
    /* Before calling any other GLFW functions, call glfwSetErrorCallback and
    glfwInit. By the way, all GLFW function names begin with 'glfw'. */
    glfwSetErrorCallback(handleError);
    if (glfwInit() == 0)
        return 1;
    /* Create a GLFW window and register some more callbacks. Read the GLFW
    documentation to learn about keyboard, mouse, and other callbacks. */
    GLFWwindow *window;
    window = glfwCreateWindow(768, 512, "Learning GLFW", NULL, NULL);
    if (window == NULL) {
        glfwTerminate();
        return 2;
    }
    glfwSetWindowSizeCallback(window, handleResize);
    /* A OpenGL 'context' is an instance of the OpenGL system. Each GLFW window
    comes with an OpenGL context. The following call makes the window's OpenGL
    context 'current' ---- meaning, the target for future OpenGL calls. */
    glfwMakeContextCurrent(window);
    /* As an aside, ask OpenGL for its version. */
    fprintf(stderr, "main: OpenGL %s, GLSL %s\n",
		glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));
	/* Run the user interface loop, until the user closes the window. */
    while (glfwWindowShouldClose(window) == 0) {
    	/* Update our sense of time since the last animation frame. */
    	oldTime = newTime;
    	newTime = getTime();
		if (floor(newTime) - floor(oldTime) >= 1.0)
			printf("main: %f frames/sec\n", 1.0 / (newTime - oldTime));
    	/* Even though there is no animation or user interaction, other than
    	resizing the window, we'll redraw the window on every frame, for
    	simplicity. (000pixel.o redraws only when it's needed.) */
        render(oldTime, newTime);
        /* GLFW windows are double-buffered. All of your OpenGL rendering goes
        into the hidden 'back' buffer. Then you swap the buffers, to copy the
        back buffer to the 'front' buffer, which the user sees. */
        glfwSwapBuffers(window);
        /* Process any pending user interface events, using the callbacks. */
        glfwPollEvents();
    }
	/* Always clean up GLFW like this, when you're finished. */
	glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
