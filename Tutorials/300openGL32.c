


/* On macOS, compile with...
    clang 300openGL32.c /usr/local/gl3w/src/gl3w.o -lglfw -framework OpenGL -framework CoreFoundation -Wno-deprecated
...and you might have to change the location of gl3w.o based on your
installation.

Until now we've been using OpenGL 2.x, which is backward-compatible with OpenGL
1.x. However, later versions of OpenGL are not backward-compatible.
Fortunately, these later OpenGLs provide facilities for managing the OpenGL
version used by your program. In practice, these facilities are always used in
conjunction with an OpenGL extension manager such as GL3W.

In this tutorial we tweak 300openGL20b.c to make it use OpenGL 3.2. There are
four big changes. First, we ask GLFW for an OpenGL 3.2 context. Second, we link
to and initialize GL3W. Third, we adjust our GLSL code to conform to GLSL 1.4.
Fourth, we wrap our mesh buffers in a 'vertex array object'. */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
/* Include the GL3W header before any other OpenGL-related headers. */
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

#include "300shading.c"
#include "120vector.c"
#include "140matrix.c"
#include "140isometry.c"
#include "150camera.c"

#define BUFFER_OFFSET(bytes) ((GLubyte*) NULL + (bytes))

GLuint program;
GLint positionLoc, colorLoc;
GLint viewingLoc, modelingLoc;

#define TRINUM 8
#define VERTNUM 6
#define ATTRDIM 6
GLdouble angle = 0.0;
/* In addition to the two vertex buffer objects (VBOs) storing our mesh, we
also make a vertex array object (VAO). Confusingly, a VAO does NOT store an
array of vertices. That's a VBO's job. Rather, a VAO stores configuration
information about how a mesh is to be rendered. Using a VAO will make our
render function much simpler. Whether or not you appreciate that convenience,
it's required in OpenGL 3.2, so get used to it. */
GLuint octaBuffers[2];
GLuint octaVAO;
isoIsometry modeling;
camCamera cam;

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
    camSetFrustum(&cam, M_PI / 6.0, 5.0, 10.0, width, height);
}

void initializeMesh(void) {
	/* The first part of this function is the same as in 500openGL20b.c. */
	GLdouble attributes[VERTNUM * ATTRDIM] = {
		1.0, 0.0, 0.0, 1.0, 0.0, 0.0,
		-1.0, 0.0, 0.0, 1.0, 1.0, 0.0,
		0.0, 1.0, 0.0, 0.0, 1.0, 0.0,
		0.0, -1.0, 0.0, 0.0, 1.0, 1.0,
		0.0, 0.0, 1.0, 0.0, 0.0, 1.0,
		0.0, 0.0, -1.0, 1.0, 0.0, 1.0};
	GLuint triangles[TRINUM * 3] = {
		0, 2, 4,
		2, 1, 4,
		1, 3, 4,
		3, 0, 4,
		2, 0, 5,
		1, 2, 5,
		3, 1, 5,
		0, 3, 5};
	glGenBuffers(2, octaBuffers);
	glBindBuffer(GL_ARRAY_BUFFER, octaBuffers[0]);
	glBufferData(GL_ARRAY_BUFFER, VERTNUM * ATTRDIM * sizeof(GLdouble),
		(GLvoid *)attributes, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, octaBuffers[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, TRINUM * 3 * sizeof(GLuint),
		(GLvoid *)triangles, GL_STATIC_DRAW);
	/* The new stuff starts here. Make one vertex array object (VAO). Bind it,
	so that we can edit it. */
	glGenVertexArrays(1, &octaVAO);
	glBindVertexArray(octaVAO);
	/* Tell the VAO about the attribute arrays and how they should hook into
	the vertex shader. These OpenGL calls used to happen at rendering time. Now
	they happen at initialization time, and the VAO remembers them. Magic. */
	glBindBuffer(GL_ARRAY_BUFFER, octaBuffers[0]);
	glEnableVertexAttribArray(positionLoc);
	glVertexAttribPointer(positionLoc, 3, GL_DOUBLE, GL_FALSE,
		ATTRDIM * sizeof(GLdouble), BUFFER_OFFSET(0));
	glEnableVertexAttribArray(colorLoc);
	glVertexAttribPointer(colorLoc, 3, GL_DOUBLE, GL_FALSE,
		ATTRDIM * sizeof(GLdouble), BUFFER_OFFSET(3 * sizeof(GLdouble)));
	/* Also tell the VAO about the array of triangle indices (but don't
	actually draw the triangles now). */
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, octaBuffers[1]);
	/* When you're done issuing commands to a VAO, always unbind it by binding
	the trivial VAO. (Failure to do this recently cost me several hours of
	debugging.) */
	glBindVertexArray(0);
}

int initializeShaderProgram(void) {
	/* The vertex shader must begin with a GLSL version number declaration ---
	in this case, 1.40. The attribute qualifier is replaced with the more
	generic in. The varying qualifier is replaced with the more generic out. */
	GLchar vertexCode[] = "\
		#version 140\n\
		uniform mat4 viewing;\
		uniform mat4 modeling;\
		in vec3 position;\
		in vec3 color;\
		out vec4 rgba;\
		void main() {\
			gl_Position = viewing * modeling * vec4(position, 1.0);\
			rgba = vec4(color, 1.0);\
		}";
	/* The fragment shader must begin with a GLSL version number declaration.
	The varying qualifier is replaced with the more generic in. There is no
	longer a special output variable gl_FragColor. Instead, we declare our
	own output variable fragColor. As long as there is just one output, OpenGL
	hooks it to the default framebuffer. So it doesn't need any more setup in
	this tutorial. */
	GLchar fragmentCode[] = "\
		#version 140\n\
		in vec4 rgba;\
		out vec4 fragColor;\
		void main() {\
			fragColor = rgba;\
		}";
	/* By the way, there's no texture-mapping in this example, but for future
	reference the GLSL 1.1 function texture2D has been renamed 'texture',
	because texture is now a whole class of overloaded functions. */
	program = shaMakeProgram(vertexCode, fragmentCode);
	if (program != 0) {
		glUseProgram(program);
		positionLoc = glGetAttribLocation(program, "position");
		colorLoc = glGetAttribLocation(program, "color");
		viewingLoc = glGetUniformLocation(program, "viewing");
		modelingLoc = glGetUniformLocation(program, "modeling");
	}
	return (program == 0);
}

void uniformMatrix44(double m[4][4], GLint uniformLocation) {
	GLfloat mTFloat[4][4];
	for (int i = 0; i < 4; i += 1)
		for (int j = 0; j < 4; j += 1)
			mTFloat[i][j] = m[j][i];
	glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, (GLfloat *)mTFloat);
}

void uniformVector3(double v[3], GLint uniformLocation) {
	GLfloat vFloat[3];
	for (int i = 0; i < 3; i += 1)
		vFloat[i] = v[i];
	glUniform3fv(uniformLocation, 1, vFloat);
}

void render(double oldTime, double newTime) {
	/* The first part of this function is the same as in 300openGL20b.c. */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(program);
	double trans[3] = {0.0, 0.0, 0.0};
	isoSetTranslation(&modeling, trans);
	angle += 2.1 * (newTime - oldTime);
	double axis[3] = {1.0 / sqrt(3.0), 1.0 / sqrt(3.0), 1.0 / sqrt(3.0)};
	double rot[3][3];
	mat33AngleAxisRotation(angle, axis, rot);
	isoSetRotation(&modeling, rot);
	double model[4][4];
	isoGetHomogeneous(&modeling, model);
	uniformMatrix44(model, modelingLoc);
	double viewing[4][4];
	camGetProjectionInverseIsometry(&cam, viewing);
	uniformMatrix44(viewing, viewingLoc);
	/* Compare this mesh-rendering code to that in 300openGL20b.c. It's so
	simple. Intuitively, the configuration is already stored in the vertex
	array object, so we need only bind it, render it, and unbind it. */
	glBindVertexArray(octaVAO);
	glDrawElements(GL_TRIANGLES, TRINUM * 3, GL_UNSIGNED_INT, BUFFER_OFFSET(0));
	glBindVertexArray(0);
}

int main(void) {
	double oldTime;
	double newTime = getTime();
    glfwSetErrorCallback(handleError);
    if (glfwInit() == 0) {
    	fprintf(stderr, "main: glfwInit failed.\n");
        return 1;
    }
    /* Ask GLFW to supply an OpenGL 3.2 context. */
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    GLFWwindow *window;
    window = glfwCreateWindow(768, 512, "Learning OpenGL 3.2", NULL, NULL);
    if (window == NULL) {
    	fprintf(stderr, "main: glfwCreateWindow failed.\n");
        glfwTerminate();
        return 2;
    }
    glfwSetWindowSizeCallback(window, handleResize);
    glfwMakeContextCurrent(window);
    /* You might think that getting an OpenGL 3.2 context would make OpenGL 3.2
    available to us. But you'd be wrong. The following call 'loads' a bunch of
    OpenGL 3.2 functions, so that we can use them. This is why we use GL3W. */
    if (gl3wInit() != 0) {
    	fprintf(stderr, "main: gl3wInit failed.\n");
    	glfwDestroyWindow(window);
    	glfwTerminate();
    	return 3;
    }
    /* We rarely invoke any GL3W functions other than gl3wInit. But just for an
    educational example, let's ask GL3W about OpenGL support. */
	if (gl3wIsSupported(3, 2) == 0)
		fprintf(stderr, "main: OpenGL 3.2 is not supported.\n");
	else
		fprintf(stderr, "main: OpenGL 3.2 is supported.\n");
    fprintf(stderr, "main: OpenGL %s, GLSL %s.\n",
		glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    double target[3] = {0.0, 0.0, 0.0};
	camLookAt(&cam, target, 5.0, M_PI / 3.0, -M_PI / 4.0);
	camSetProjectionType(&cam, camPERSPECTIVE);
	camSetFrustum(&cam, M_PI / 6.0, 5.0, 10.0, 768, 512);
    /* Initialize the shader program before the mesh, so that the shader
    locations are already set up by the time the vertex array object is
    initialized. */
    if (initializeShaderProgram() != 0)
    	return 4;
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
    glDeleteProgram(program);
    /* Delete not just the buffers, but also the vertex array object. */
	glDeleteBuffers(2, octaBuffers);
	glDeleteVertexArrays(1, &octaVAO);
	glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
