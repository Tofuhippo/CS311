


/* On macOS, compile with...
    clang 300openGL14.c -lglfw -framework OpenGL -Wno-deprecated
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

void handleError(int error, const char *description) {
	fprintf(stderr, "handleError: %d\n%s\n", error, description);
}

void handleResize(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
}

/* Here we start building a mesh. It will be a unit sphere, crudely
approximated using 8 triangles. At each vertex it will have an XYZ position and
an RGB color. Because it's a unit sphere, the normals NOP are identical to the
positions XYZ, so we save memory and typing by not storing the normals. */
#define triNum 8
#define vertNum 6
/* For compatibility across hardware, operating systems, and compilers,
OpenGL defines its own versions of double, int, unsigned int, etc. Use these
OpenGL types whenever passing arrays or pointers to OpenGL (unless OpenGL says
otherwise). Do not assume that these types equal your usual C types. */
GLdouble positions[vertNum * 3] = {
	1.0, 0.0, 0.0,
	-1.0, 0.0, 0.0,
	0.0, 1.0, 0.0,
	0.0, -1.0, 0.0,
	0.0, 0.0, 1.0,
	0.0, 0.0, -1.0};
GLdouble colors[vertNum * 3] = {
	1.0, 0.0, 0.0,
	1.0, 1.0, 0.0,
	0.0, 1.0, 0.0,
	0.0, 1.0, 1.0,
	0.0, 0.0, 1.0,
	1.0, 0.0, 1.0};
GLuint triangles[triNum * 3] = {
	0, 2, 4,
	2, 1, 4,
	1, 3, 4,
	3, 0, 4,
	2, 0, 5,
	1, 2, 5,
	3, 1, 5,
	0, 3, 5};
/* We'll use this angle to animate a rotation of the mesh. Unlike every other
software I've ever seen, OpenGL 1.x operates in degrees rather than radians.
Forgetting whether an angle is in degrees or radians is a common cause of hard-
to-find bugs. Therefore, whenever I program with a variable in degrees, I name
that variable with 'Degree'. Like a skull on a bottle of poison. */
double angleDegree = 0.0;

void render(double oldTime, double newTime) {
	/* Clear not just the color buffer, but also the depth buffer. */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-2.0, 2.0, -2.0, 2.0, -2.0, 2.0); // Why these values?
	/* OpenGL's modelview matrix corresponds to C^-1 M in the notation of our
	software graphics engine. That is, it's all of the isometries before
	projection. */
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	/* OpenGL 1.x lighting calculations are done in eye coordinates, not world
	coordinates. When you set a light's position, it is automatically
	transformed by the current modelview matrix. So it is crucial to set light
	positions after setting the modelview matrix. */
	GLfloat light[4] = {0.0, 1.0, 1.0, 0.0};
	glLightfv(GL_LIGHT0, GL_POSITION, light);
	/* Animate by rotating 10 degrees per second about the unit vector that
	points from the origin toward <1, 1, 1>. glRotatef accomplishes this by
	multiplying a rotation onto the modelview matrix. */
	angleDegree += 10.0 * (newTime - oldTime);
	glRotatef(angleDegree, 1.0, 1.0, 1.0);
	/* Send the arrays of attribute data to OpenGL. Notice that we're sending
	the same array for positions and normals, because our mesh is special in
	that way. */
	glVertexPointer(3, GL_DOUBLE, 0, positions);
	glNormalPointer(GL_DOUBLE, 0, positions);
	glColorPointer(3, GL_DOUBLE, 0, colors);
	/* Draw the triangles, each one a triple of attribute array indices. */
    glDrawElements(GL_TRIANGLES, triNum * 3, GL_UNSIGNED_INT, triangles);
}

int main(void) {
	/* Record the current time. On the first frame of animation, it will serve
	as the 'old time'. */
	double oldTime;
	double newTime = getTime();
    glfwSetErrorCallback(handleError);
    if (glfwInit() == 0)
        return 1;
    GLFWwindow *window;
    window = glfwCreateWindow(768, 512, "Learning OpenGL 1.4", NULL, NULL);
    if (window == NULL) {
        glfwTerminate();
        return 2;
    }
    glfwSetWindowSizeCallback(window, handleResize);
    glfwMakeContextCurrent(window);
    fprintf(stderr, "main: OpenGL %s, GLSL %s.\n",
		glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));
    /* Enable some OpenGL features. Several lights are available, but we'll use
    only the first light (light 0). It defaults to diffuse and ambient lighting
    (not specular or emissive). */
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    /* In diffuse and ambient lighting calculations, the color of the surface
    will be taken from the interpolated color attributes. */
    glEnable(GL_COLOR_MATERIAL);
    /* Enable certain ways of passing attribute information into OpenGL. Just
    to give you an idea of your options in OpenGL 1.4, I explicitly disable the
    other ways of passing attributes. */
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_FOG_COORD_ARRAY);
    glDisableClientState(GL_SECONDARY_COLOR_ARRAY);
    glDisableClientState(GL_EDGE_FLAG_ARRAY);
    glDisableClientState(GL_INDEX_ARRAY);
    while (glfwWindowShouldClose(window) == 0) {
    	oldTime = newTime;
    	newTime = getTime();
		if (floor(newTime) - floor(oldTime) >= 1.0)
			printf("main: %f frames/sec\n", 1.0 / (newTime - oldTime));
        render(oldTime, newTime);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
	glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
