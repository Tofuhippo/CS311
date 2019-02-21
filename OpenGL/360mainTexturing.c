/*
Dawson d'Almeida and Justin T. Washington
February 20 2018
CS311 with Josh Davis

Main abstracted file that uses openGL to generate a sphere.
*/


/* On macOS, compile with...
    clang 360mainTexturing.c /usr/local/gl3w/src/gl3w.o -lglfw -framework OpenGL -framework CoreFoundation -Wno-deprecated
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

#include "320shading.c"
#include "310vector.c"
#include "320matrix.c"
#include "320isometry.c"
#include "320camera.c"
#include "310mesh.c"
#include "350meshgl.c"
#include "360texture.c"

#define BUFFER_OFFSET(bytes) ((GLubyte*) NULL + (bytes))

#define UNIFVIEWING 0
#define UNIFMODELING 1
#define UNIFDLIGHT 2
#define UNIFCLIGHT 3
#define UNIFAMBIENTLIGHT 4
#define UNIFPCAMERA 5
#define UNIFTEXTURE 6
#define ATTRPOSITION 0
#define ATTRST 1
#define ATTRNORMAL 2

// Number of categories that the attr/unif are representing
// (i.e. position, color, etc..)
#define UNIFNUM 7
#define ATTRNUM 3

const GLchar *uniformNames[UNIFNUM] = {"viewing", "modeling", "dLight",
                                       "cLight", "ambientLight", "pCamera",
                                       "texture0"};
const GLchar **unifNames = uniformNames;
const GLchar *attributeNames[ATTRNUM] = {"position", "st", "normal"};
const GLchar **attrNames = attributeNames;

/* The angle variable is no longer in degrees. That's a relief. */
GLdouble angle = 0.0;

/* These are from 330 (i think)*/
isoIsometry modeling;
camCamera cam;
shaShading sha;

double angleDegree = 0.0;

meshglMesh mesh;
texTexture tex;


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

void meshInitializeMiddleStep(void){
	/* Updated in 350 */
	/* Tell the VAO about the attribute arrays and how they should hook into
	the vertex shader. These OpenGL calls used to happen at rendering time. Now
	they happen at initialization time, and the VAO remembers them. Magic. */
	glEnableVertexAttribArray(sha.attrLocs[ATTRPOSITION]);
	glVertexAttribPointer(sha.attrLocs[ATTRPOSITION], 3, GL_DOUBLE, GL_FALSE,
		mesh.attrDim * sizeof(GLdouble), BUFFER_OFFSET(0));
  glEnableVertexAttribArray(sha.attrLocs[ATTRST]);
	glVertexAttribPointer(sha.attrLocs[ATTRST], 2, GL_DOUBLE, GL_FALSE,
		mesh.attrDim * sizeof(GLdouble), BUFFER_OFFSET(3 * sizeof(GLdouble)));
  glEnableVertexAttribArray(sha.attrLocs[ATTRNORMAL]);
  glVertexAttribPointer(sha.attrLocs[ATTRNORMAL], 3, GL_DOUBLE, GL_FALSE,
    mesh.attrDim * sizeof(GLdouble), BUFFER_OFFSET(5 * sizeof(GLdouble)));
}

void initializeMesh(void) {
	meshMesh base;
	meshInitializeCapsule(&base, 0.5, 2.0, 16.0, 32.0);
	meshglInitialize(&mesh, &base);
	meshInitializeMiddleStep();
	meshglFinishInitialization(&mesh);
	meshDestroy(&base);
}

/* Returns 0 on success, non-zero on failure. */
int initializeShaderProgram(void) {
	/* The two matrices will be sent to the shaders as uniforms. */
	// Using position as nop because we are using a unit sphere
	GLchar vertexCode[] = "\
    #version 140\n\
		uniform mat4 viewing;\
		uniform mat4 modeling;\
		in vec3 position;\
		in vec2 st;\
    in vec3 normal;\
		out vec2 texCoords;\
    out vec3 nop;\
    out vec4 world;\
		void main() {\
      world = modeling * vec4(position, 1.0);\
			gl_Position = viewing * world;\
			nop = vec3(modeling * vec4(normal, 0.0));\
      texCoords = st;\
		}";
	GLchar fragmentCode[] = "\
    #version 140\n\
		uniform vec3 dLight;\
		uniform vec3 cLight;\
		uniform vec3 ambientLight;\
		uniform vec3 pCamera;\
    uniform sampler2D texture0;\
		in vec2 texCoords;\
		in vec3 nop;\
    in vec4 world;\
    out vec4 fragColor;\
		void main() {\
      vec3 rgbFromTex = vec3(texture(texture0, texCoords));\
			vec3 dNormal = normalize(nop);\
			vec3 dLightNorm = normalize(dLight);\
      vec3 pFragment = vec3(world);\
			vec3 dCameraNorm = normalize(pCamera - pFragment);\
			float iDiff = dot(dLightNorm, dNormal);\
			if (iDiff < 0.0) {\
				iDiff = 0.0;\
			}\
			vec3 diffuse = iDiff * cLight * rgbFromTex;\
			vec3 ambient = rgbFromTex * ambientLight;\
			vec3 dRefl = normalize((2.0 * dot(dLightNorm, dNormal)) * dNormal - dLightNorm);\
			float shininess = 10.0;\
			float iSpec = dot(dRefl, dCameraNorm);\
			if (iDiff <= 0.0 || iSpec < 0.0) {\
				iSpec = 0.0;\
			}\
      iSpec = pow(iSpec, shininess);\
			vec3 cSurface = vec3(1.0, 1.0, 1.0);\
			vec3 specular = iSpec * cSurface * cLight;\
			fragColor = vec4(diffuse + ambient + specular, 1.0);\
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

/* This weird macro helps us index the GPU-side buffers in the render function
below. It is taken verbatim from the OpenGL Programming Guide, 7th Ed. */
#define BUFFER_OFFSET(bytes) ((GLubyte*) NULL + (bytes))

void render(double oldTime, double newTime) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(sha.program);

	/* Set dLight, cLight, ambientLight in uniforms. */
	GLdouble dLight[3] = {10.0, 10.0, 10.0};
	GLdouble cLight[3] = {1.0, 1.0, 1.0};
	GLdouble ambientLight[3] = {0.1, 0.1, 0.1};
	GLdouble pCamera[3] = {15.0, 0.0, 10.0};
	uniformVector3(dLight, sha.unifLocs[UNIFDLIGHT]);
	uniformVector3(cLight, sha.unifLocs[UNIFCLIGHT]);
	uniformVector3(ambientLight, sha.unifLocs[UNIFAMBIENTLIGHT]);
	uniformVector3(pCamera, sha.unifLocs[UNIFPCAMERA]);

	/* Send our own modeling transformation M to the shaders. */
	GLdouble trans[3] = {0.0, 0.0, 0.0};
	isoSetTranslation(&modeling, trans);
	angle += 1 * (newTime - oldTime);
	GLdouble axis[3] = {1.0 / sqrt(3.0), 1.0 / sqrt(3.0), 1.0 / sqrt(3.0)};
	GLdouble rot[3][3];
	mat33AngleAxisRotation(angle, axis, rot);
	isoSetRotation(&modeling, rot);
	GLdouble model[4][4];
	isoGetHomogeneous(&modeling, model);
	uniformMatrix44(model, sha.unifLocs[UNIFMODELING]);
	/* Send our own viewing transformation P C^-1 to the shaders. */
	GLdouble viewing[4][4];
	camGetProjectionInverseIsometry(&cam, viewing);
	uniformMatrix44(viewing, sha.unifLocs[UNIFVIEWING]);
  /* Replaced bind, render, unbind in 340 cause of ABSTRACTION. */
  texRender(&tex, GL_TEXTURE0, 0, sha.unifLocs[UNIFTEXTURE]);
  meshglRender(&mesh);
  texUnrender(&tex, GL_TEXTURE0);
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
	/* Configure the camera once and for all. */
  //glClearColor(1.0, 1.0, 1.0, 1.0);
	GLdouble target[3] = {0.0, 0.0, 0.0};
	camLookAt(&cam, target, 5.0, M_PI / 3.0, -M_PI / 4.0);
	camSetProjectionType(&cam, camPERSPECTIVE);
	camSetFrustum(&cam, M_PI / 6.0, 5.0, 10.0, 768, 512);
	if (initializeShaderProgram() != 0)
		return 4;
  if (texInitializeFile(&tex, "nathan_mannes.jpg", GL_LINEAR, GL_LINEAR,
                        GL_REPEAT, GL_REPEAT))
    return 5;
	// Initialize mesh after shader so that we can use the attrLocs
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
  texDestroy(&tex);
  shaDestroy(&sha);
	meshglDestroy(&mesh);
	glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
