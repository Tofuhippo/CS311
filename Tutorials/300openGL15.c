


/* On macOS, compile with...
    clang 300openGL15.c -lglfw -framework OpenGL -Wno-deprecated
*/

#include <stdio.h>
#include <math.h>
#include <sys/time.h>
#include <GLFW/glfw3.h>

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

/* We're going to use the same mesh as in the preceding tutorial, but we're
going to load it into GPU memory, instead of keeping it in CPU memory. Once
it's loaded, we will refer to it using the two unsigned integers in the buffers
variable. */
#define triNum 8
#define vertNum 6
#define attrDim 6
GLuint buffers[2];
double angleDegree = 0.0;

void initializeMesh(void) {
	/* Here are the mesh data, which no longer need to be global. I've combined
	the positions and colors into a single array, with attrDim == 6 attributes
	per vertex. */
	GLdouble attributes[vertNum * attrDim] = {
		1.0, 0.0, 0.0, 1.0, 0.0, 0.0,
		-1.0, 0.0, 0.0, 1.0, 1.0, 0.0,
		0.0, 1.0, 0.0, 0.0, 1.0, 0.0,
		0.0, -1.0, 0.0, 0.0, 1.0, 1.0,
		0.0, 0.0, 1.0, 0.0, 0.0, 1.0,
		0.0, 0.0, -1.0, 1.0, 0.0, 1.0};
	GLuint triangles[triNum * 3] = {
		0, 2, 4,
		2, 1, 4,
		1, 3, 4,
		3, 0, 4,
		2, 0, 5,
		1, 2, 5,
		3, 1, 5,
		0, 3, 5};
    /* Ask OpenGL for two buffers in GPU memory. Copy the attributes and
    triangles into them. GL_STATIC_DRAW is a performance hint. It means that we
    expect to render using these buffers many times, without editing them.
    Performance hints help OpenGL decide how to organize the data in memory. */
	glGenBuffers(2, buffers);
	glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, vertNum * attrDim * sizeof(GLdouble),
		(GLvoid *)attributes, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, triNum * 3 * sizeof(GLuint),
		(GLvoid *)triangles, GL_STATIC_DRAW);
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
	/* As in the preceding tutorial, rendering the mesh requires calls to
	glVertexPointer, glNormalPointer, and glColorPointer. What's different is
	that we don't pass buffers in CPU memory to those functions anymore.
	Instead we 'bind' a GPU-side buffer and pass offsets into that. */
	glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
	glVertexPointer(3, GL_DOUBLE, attrDim * sizeof(GLdouble), BUFFER_OFFSET(0));
	glNormalPointer(GL_DOUBLE, attrDim * sizeof(GLdouble), BUFFER_OFFSET(0));
	/* While the vertex positions and normals start at index 0 of the buffer,
	the color data start at index 3. */
	glColorPointer(3, GL_DOUBLE, attrDim * sizeof(GLdouble),
		BUFFER_OFFSET(3 * sizeof(GLdouble)));
	/* Wondering what 'attrDim * sizeof(GLdouble)' was doing in those
	functions? It is the 'stride' between the start of one vertex and the start
	of another. It is needed here, because we have packed multiple kinds of
	attributes into a single attribute array. */
	/* Passing the triangle indices is similar to passing the vertex
	attributes: bind a GPU-side buffer and pass an index into it. */
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);
  glDrawElements(GL_TRIANGLES, triNum * 3, GL_UNSIGNED_INT, BUFFER_OFFSET(0));
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
    glDeleteBuffers(2, buffers);
		glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
