/*
Dawson d'Almeida and Justin T. Washington
February 18 2018
CS311 with Josh Davis

Main abstracted file that uses openGL to generate a sphere with lighting.
*/

/* On macOS, compile with...
    clang 330mainLighting.c -lglfw -framework OpenGL -Wno-deprecated
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <GLFW/glfw3.h>

#include "320shading.c"
#include "310vector.c"
#include "320matrix.c"
#include "320isometry.c"
#include "320camera.c"

#define BUFFER_OFFSET(bytes) ((GLubyte*) NULL + (bytes))

#define UNIFVIEWING 0
#define UNIFMODELING 1
#define UNIFDLIGHT 2
#define UNIFCLIGHT 3
#define UNIFAMBIENTLIGHT 4
#define UNIFDCAMERA 5
#define ATTRPOSITION 0
#define ATTRCOLOR 1

// Number of categories that the attr/unif are representing
// (i.e. position, color, etc..)
#define UNIFNUM 6
#define ATTRNUM 2

const GLchar *uniformNames[UNIFNUM] = {"viewing", "modeling", "dLight",
                                       "cLight", "ambientLight", "dCamera"};
const GLchar **unifNames = uniformNames;
const GLchar *attributeNames[ATTRNUM] = {"position", "color"};
const GLchar **attrNames = attributeNames;

#define TRINUM 8
#define VERTNUM 6
// Number of individual indices in attr/unif
#define ATTRDIM 6 // xyz rgb
#define UNIFDIM 32 // viewing, modeling

/* The angle variable is no longer in degrees. That's a relief. */
GLdouble angle = 0.0;
GLuint buffers[2];
/* These are new. */
isoIsometry modeling;
camCamera cam;
shaShading sha;

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
    /* The documentation for camSetFrustum says that we must re-call it here. */
	camSetFrustum(&cam, M_PI / 6.0, 5.0, 10.0, width, height);
}

void initializeMesh(void) {
	// These are the attributes -> if we want to add normal, then we have to add
	// values here
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
	glGenBuffers(2, buffers);
	glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, VERTNUM * ATTRDIM * sizeof(GLdouble),
		(GLvoid *)attributes, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, TRINUM * 3 * sizeof(GLuint),
		(GLvoid *)triangles, GL_STATIC_DRAW);
}

/* Returns 0 on success, non-zero on failure. */
int initializeShaderProgram(void) {
	/* The two matrices will be sent to the shaders as uniforms. */
	// Using position as nop because we are using a unit sphere
	GLchar vertexCode[] = "\
		uniform mat4 viewing;\
		uniform mat4 modeling;\
		attribute vec3 position;\
		attribute vec3 color;\
		varying vec3 rgb;\
		varying vec3 nop;\
		void main() {\
			gl_Position = viewing * modeling * vec4(position, 1.0);\
			rgb = color;\
			nop = vec3(modeling * vec4(position, 0.0));\
		}";
	GLchar fragmentCode[] = "\
		uniform vec3 dLight;\
		uniform vec3 cLight;\
		uniform vec3 ambientLight;\
		uniform vec3 dCamera;\
		varying vec3 rgb;\
		varying vec3 nop;\
		void main() {\
			vec3 dNormal = normalize(nop);\
			vec3 dLightNorm = normalize(dLight);\
			vec3 dCameraNorm = normalize(dCamera);\
			float iDiff = dot(dLightNorm, dNormal);\
			if (iDiff < 0.0) {\
				iDiff = 0.0;\
			}\
			vec3 diffuse = iDiff * cLight * rgb;\
			vec3 ambient = ambientLight * rgb;\
			vec3 dRefl = normalize((2.0 * dot(dLightNorm, dNormal)) * dNormal - dLightNorm);\
			float shininess = 32.0;\
			float iSpec = pow(dot(dRefl, dCameraNorm), shininess);\
			if (iDiff <= 0.0 || iSpec < 0.0) {\
				iSpec = 0.0;\
			}\
			vec3 cSurface = vec3(1.0, 1.0, 1.0);\
			vec3 specular = iSpec * cSurface * cLight;\
			gl_FragColor = vec4(diffuse + ambient + specular, 1.0);\
		}";
	shaInitialize(&sha, vertexCode, fragmentCode, UNIFNUM, unifNames, ATTRNUM,
		            attrNames);
	return (sha.program == 0);
}

/* We want to pass 4x4 matrices into uniforms in OpenGL shaders, but there are
two obstacles. First, our matrix library uses double matrices, but OpenGL
shaders expect GLfloat matrices. Second, C matrices are implicitly stored one-
row-after-another, while OpenGL shaders expect matrices to be stored one-column-
after-another. This function plows through both of those obstacles. */
void uniformMatrix44(GLdouble m[4][4], GLint uniformLocation) {
	GLfloat mTFloat[4][4];
	for (int i = 0; i < 4; i += 1)
		for (int j = 0; j < 4; j += 1)
			mTFloat[i][j] = m[j][i];
	glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, (GLfloat *)mTFloat);
}

/* Here is a similar function for vectors. */
void uniformVector3(GLdouble v[3], GLint uniformLocation) {
	GLfloat vFloat[3];
	for (int i = 0; i < 3; i += 1)
		vFloat[i] = v[i];
	glUniform3fv(uniformLocation, 1, vFloat);
}

void render(double oldTime, double newTime) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(sha.program);
	GLint modelingLoc = sha.unifLocs[UNIFMODELING];
	GLint viewingLoc = sha.unifLocs[UNIFVIEWING];
	GLint dLightLoc = sha.unifLocs[UNIFDLIGHT];
	GLint cLightLoc = sha.unifLocs[UNIFCLIGHT];
	GLint ambientLightLoc = sha.unifLocs[UNIFAMBIENTLIGHT];
	GLint dCameraLoc = sha.unifLocs[UNIFDCAMERA];
	GLint positionLoc = sha.attrLocs[ATTRPOSITION];
	GLint colorLoc = sha.attrLocs[ATTRCOLOR];

	/* Set dLight, cLight, ambientLight in uniforms. */
	GLdouble dLight[3] = {10.0, 10.0, 10.0};
	GLdouble cLight[3] = {1.0, 1.0, 1.0};
	GLdouble ambientLight[3] = {0.1, 0.1, 0.1};
	GLdouble dCamera[3] = {15.0, 0.0, 10.0};
	uniformVector3(dLight, dLightLoc);
	uniformVector3(cLight, cLightLoc);
	uniformVector3(ambientLight, ambientLightLoc);
	uniformVector3(dCamera, dCameraLoc);

	/* Send our own modeling transformation M to the shaders. */
	GLdouble trans[3] = {0.0, 0.0, 0.0};
	isoSetTranslation(&modeling, trans);
	angle += 0.1 * (newTime - oldTime);
	GLdouble axis[3] = {1.0 / sqrt(3.0), 1.0 / sqrt(3.0), 1.0 / sqrt(3.0)};
	GLdouble rot[3][3];
	mat33AngleAxisRotation(angle, axis, rot);
	isoSetRotation(&modeling, rot);
	GLdouble model[4][4];
	isoGetHomogeneous(&modeling, model);
	uniformMatrix44(model, modelingLoc);
	/* Send our own viewing transformation P C^-1 to the shaders. */
	GLdouble viewing[4][4];
	camGetProjectionInverseIsometry(&cam, viewing);
	uniformMatrix44(viewing, viewingLoc);
	/* The rest of the function is just as in the preceding tutorial. */
	glEnableVertexAttribArray(positionLoc);
	glEnableVertexAttribArray(colorLoc);
	glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
	glVertexAttribPointer(positionLoc, 3, GL_DOUBLE, GL_FALSE,
		ATTRDIM * sizeof(GLdouble), BUFFER_OFFSET(0));
	glVertexAttribPointer(colorLoc, 3, GL_DOUBLE, GL_FALSE,
		ATTRDIM * sizeof(GLdouble), BUFFER_OFFSET(3 * sizeof(GLdouble)));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);
	glDrawElements(GL_TRIANGLES, TRINUM * 3, GL_UNSIGNED_INT, BUFFER_OFFSET(0));
	glDisableVertexAttribArray(positionLoc);
	glDisableVertexAttribArray(colorLoc);

}

int main(void) {
	double oldTime;
	double newTime = getTime();
    glfwSetErrorCallback(handleError);
    if (glfwInit() == 0)
        return 1;
    GLFWwindow *window;
    window = glfwCreateWindow(768, 512, "Learning OpenGL 2.0", NULL, NULL);
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
    /* Configure the camera once and for all. */
	GLdouble target[3] = {0.0, 0.0, 0.0};
	camLookAt(&cam, target, 5.0, M_PI / 3.0, -M_PI / 4.0);
	camSetProjectionType(&cam, camPERSPECTIVE);
	camSetFrustum(&cam, M_PI / 6.0, 5.0, 10.0, 768, 512);
	/* The rest of the program is identical to the preceding tutorial. */
    initializeMesh();
    if (initializeShaderProgram() != 0)
    	return 3;
    while (glfwWindowShouldClose(window) == 0) {
        oldTime = newTime;
    	newTime = getTime();
		if (floor(newTime) - floor(oldTime) >= 1.0)
			printf("main: %f frames/sec\n", 1.0 / (newTime - oldTime));
        render(oldTime, newTime);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
  shaDestroy(&sha);
	glDeleteBuffers(2, buffers);
	glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
