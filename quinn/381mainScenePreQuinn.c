/*
Dawson d'Almeida and Justin T. Washington
February 21 2018
CS311 with Josh Davis

Main abstracted file that uses openGL to generate a sphere.
*/

/* On macOS, compile with...
    clang 381mainScenePreQuinn.c /usr/local/gl3w/src/gl3w.o -lglfw -framework OpenGL -framework CoreFoundation -Wno-deprecated
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
#include "370body.c"
#include "140landscape.c"

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
#define AUXDIM 0
#define TEXNUM 1

const GLchar *uniformNames[UNIFNUM] = {"viewing", "modeling", "dLight",
                                       "cLight", "ambientLight", "pCamera",
                                       "texture0"};
const GLchar **unifNames = uniformNames;
const GLchar *attributeNames[ATTRNUM] = {"position", "st", "normal"};
const GLchar **attrNames = attributeNames;


/* The angle variable is no longer in degrees. That's a relief. */
GLdouble angle = 0.0;
/* Things that there are unique to each scene */
camCamera cam;
shaShading sha;

/* Things that can be unique for each body */
/* Added multiple bodies in 380 */
// bodyBody pillNathanBody;
// bodyBody cubeNathanBody;
bodyBody grassBody;
bodyBody rockBody;
bodyBody waterBody;

isoIsometry modeling;

// meshglMesh pillMesh;
// meshglMesh cubeMesh;
meshglMesh grassMesh;
meshglMesh rockMesh;
meshglMesh waterMesh;
// meshglMesh leavesMesh;
// meshglMesh trunkMesh;

meshMesh landBase;

texTexture grassTex;
texTexture rockTex;
texTexture waterTex;
texTexture nathanTex;

// For land mesh intialization
#define LANDNUM 100
double landData[LANDNUM][LANDNUM];
double landMin, landMean, landMax;
double waterData[4];


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

void meshInitializeMiddleStep(bodyBody body){
	/* Updated in 350 */
	/* Tell the VAO about the attribute arrays and how they should hook into
	the vertex shader. These OpenGL calls used to happen at rendering time. Now
	they happen at initialization time, and the VAO remembers them. Magic. */
	glEnableVertexAttribArray(sha.attrLocs[ATTRPOSITION]);
	glVertexAttribPointer(sha.attrLocs[ATTRPOSITION], 3, GL_DOUBLE, GL_FALSE,
		body.mesh->attrDim * sizeof(GLdouble), BUFFER_OFFSET(0));
  glEnableVertexAttribArray(sha.attrLocs[ATTRST]);
	glVertexAttribPointer(sha.attrLocs[ATTRST], 2, GL_DOUBLE, GL_FALSE,
		body.mesh->attrDim * sizeof(GLdouble), BUFFER_OFFSET(3 * sizeof(GLdouble)));
  glEnableVertexAttribArray(sha.attrLocs[ATTRNORMAL]);
  glVertexAttribPointer(sha.attrLocs[ATTRNORMAL], 3, GL_DOUBLE, GL_FALSE,
    body.mesh->attrDim * sizeof(GLdouble), BUFFER_OFFSET(5 * sizeof(GLdouble)));
}

void initializeMesh(void) {
	// meshMesh basePill;
	// meshInitializeCapsule(&basePill, 0.5, 2.0, 16.0, 32.0);
	// meshglInitialize(pillNathanBody.mesh, &basePill);
	// meshInitializeMiddleStep(pillNathanBody);
	// meshglFinishInitialization(pillNathanBody.mesh);
	// meshDestroy(&basePill);

  // meshMesh baseBox;
  // meshInitializeBox(&baseBox, -0.5, 0.5, -0.5, 0.5, -0.5, 0.5);
  // meshglInitialize(cubeNathanBody.mesh, &baseBox);
	// meshInitializeMiddleStep(cubeNathanBody);
	// meshglFinishInitialization(cubeNathanBody.mesh);
	// meshDestroy(&baseBox);

  meshglMesh landMesh;
  if (meshInitializeLandscape(&landBase, LANDNUM, LANDNUM, 1.0,
			(double *)landData) != 0){
    printf("meshInitializeLandscape failed\n");
    exit(0);
  }

  meshMesh grassBase;
  if (meshInitializeDissectedLandscape(&grassBase, &landBase, M_PI / 4.0,
			1) != 0){
		printf("meshInitializeDissectedLandscape grass failed\n");
    exit(0);
  }
  meshglInitialize(grassBody.mesh, &grassBase);
	meshInitializeMiddleStep(grassBody);
	meshglFinishInitialization(grassBody.mesh);
	meshDestroy(&grassBase);

  meshMesh rockBase;
  if (meshInitializeDissectedLandscape(&rockBase, &landBase, M_PI / 4.0,
			1) != 0){
		printf("meshInitializeDissectedLandscape rock failed\n");
    exit(0);
  }
  meshglInitialize(rockBody.mesh, &rockBase);
	meshInitializeMiddleStep(rockBody);
	meshglFinishInitialization(rockBody.mesh);
	meshDestroy(&rockBase);

  meshMesh waterBase;
  if (meshInitializeLandscape(&waterBase, 2, 2, LANDNUM - 1.0,
			(double *)waterData) != 0){
		printf("meshInitializeLandscape water failed\n");
    exit(0);
  }
  meshglInitialize(waterBody.mesh, &waterBase);
	meshInitializeMiddleStep(waterBody);
	meshglFinishInitialization(waterBody.mesh);
	meshDestroy(&waterBase);

  meshDestroy(&landBase);
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

  /** TODO: ABSTRACT THIS MODELING CONFIGURATION OUT SO WE CAN EASILY DO MULTIPLE ISOMS **/
	/* Send our own modeling transformation M to the shaders. */
  /* Uniformly done for all body/mesh/texture/etc */
	GLdouble trans[3] = {0.0, 0.0, 0.0};
	isoSetTranslation(&modeling, trans);
	angle += 0;//1 * (newTime - oldTime);
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

  /* Individually done for each body/mesh/texture/etc */
  /* Replaced bind, render, unbind in 340 cause of ABSTRACTION. */

  // texRender(pillNathanBody.tex[0], GL_TEXTURE0, 0, sha.unifLocs[UNIFTEXTURE]);
  // meshglRender(pillNathanBody.mesh);
  // texUnrender(pillNathanBody.tex[0], GL_TEXTURE0);

  // texRender(cubeNathanBody.tex[0], GL_TEXTURE0, 0, sha.unifLocs[UNIFTEXTURE]);
  // meshglRender(cubeNathanBody.mesh);
  // texUnrender(cubeNathanBody.tex[0], GL_TEXTURE0);

  texRender(grassBody.tex[0], GL_TEXTURE0, 0, sha.unifLocs[UNIFTEXTURE]);
  meshglRender(grassBody.mesh);
  texUnrender(grassBody.tex[0], GL_TEXTURE0);

  texRender(rockBody.tex[0], GL_TEXTURE0, 0, sha.unifLocs[UNIFTEXTURE]);
  meshglRender(rockBody.mesh);
  texUnrender(rockBody.tex[0], GL_TEXTURE0);

  texRender(waterBody.tex[0], GL_TEXTURE0, 0, sha.unifLocs[UNIFTEXTURE]);
  meshglRender(waterBody.mesh);
  texUnrender(waterBody.tex[0], GL_TEXTURE0);
}


/**************************************/
/*     Helper functions for main      */
/**************************************/

/* Initialize GLFWwindow *window and request openGL version. */
GLFWwindow *window;
int requestOpenGLVersion(int major, int minor){
  /* Ask GLFW to supply an OpenGL 3.2 context. */
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
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
	if (gl3wIsSupported(major, minor) == 0)
		fprintf(stderr, "main: OpenGL %i.%i is not supported.\n", major, minor);
	else
		fprintf(stderr, "main: OpenGL %i.%i is supported.\n", major, minor);
	fprintf(stderr, "main: OpenGL %s, GLSL %s.\n",
					glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
  return 0;
}

/* Configure the Camera: Define target, look @ it, set projType and Frustum */
void configureCamera(void){
  GLdouble target[3] = {0.0, 0.0, 0.0};
  double cameraRho = 256.0, cameraPhi = M_PI / 4.0, cameraTheta = 0.0;
  //camLookAt(&cam, target, 5.0, M_PI / 3.0, -M_PI / 4.0);
  camSetProjectionType(&cam, camPERSPECTIVE);

  double mainSCREENSIZE = 512;
  camSetFrustum(&cam, M_PI / 6.0, cameraRho, 10.0, mainSCREENSIZE,
    mainSCREENSIZE);
  camLookAt(&cam, target, cameraRho, cameraPhi, cameraTheta);
  //camSetFrustum(&cam, M_PI / 6.0, 5.0, 10.0, 768, 512);
}

void initializeLandscapeHelper() {
  /* Design landscape and water. */
	time_t t;
	int i;
	srand((unsigned)time(&t));
	landFlat(LANDNUM, LANDNUM, (double *)landData, 0.0);
	for (i = 0; i < 32; i += 1)
		landFault(LANDNUM, LANDNUM, (double *)landData, 1.5 - i * 0.04);
	for (i = 0; i < 4; i += 1)
		landBlur(LANDNUM, LANDNUM, (double *)landData);
	landStatistics(LANDNUM, LANDNUM, (double *)landData, &landMin, &landMean,
		&landMax);
	waterData[0] = waterData[1] = waterData[2] = waterData[3] = landMin;
  // From non-OpenGL main file:
	// unifGrass[mainUNIFMIN] = landMin;
	// unifGrass[mainUNIFMEAN] = landMean;
	// unifGrass[mainUNIFMAX] = landMax;
	// unifRock[mainUNIFMIN] = landMin;
	// unifRock[mainUNIFMEAN] = landMean;
	// unifRock[mainUNIFMAX] = landMax;
	// unifWater[mainUNIFMIN] = landMin;
	// unifWater[mainUNIFMEAN] = landMean;
	// unifWater[mainUNIFMAX] = landMax;
}

/* Abstracting the intialization of body, mesh(gl) and attaching of mesh to body
   and tex to mesh */
void configureBody(bodyBody body, meshglMesh mesh, texTexture tex) {
  if(bodyInitialize(&body, AUXDIM, TEXNUM)){
    printf("A body failed to initialize in configureBody");
    exit(0);
  }
  /* needs to be abstracted to handle multiple texutures still */
  /* Attach at texture to the body */
  bodySetTexture(&body, 0, &tex);
  /* Attach at meshgl to the body */
  bodySetMesh(&body, &mesh);
  /* Initialize the meshgl attached to the body from a base mesh */
}

int main(void) {
	double oldTime;
	double newTime = getTime();
	glfwSetErrorCallback(handleError);
	if (glfwInit() == 0) {
		fprintf(stderr, "main: glfwInit failed.\n");
		return 1;
	}
  /* request a particular version of openGL and utilize the global window */
	if (requestOpenGLVersion(3, 2) != 0)
    return 23;

  /* for testing, changes window's background */
  //glClearColor(1.0, 1.0, 0.0, 1.0);

  /* Basic camera configuring, no parameters right now (see helper to change)*/
  configureCamera();

  /* initialize shader program from XXXshading.c */
  if (initializeShaderProgram() != 0)
		return 4;

  initializeLandscapeHelper();

  /* 370: Intialize body to hold isometry, mesh, tex, and aux. */
  // if (bodyInitialize(&pillNathanBody, AUXDIM, TEXNUM))
  //   return 5;
  // if (bodyInitialize(&cubeNathanBody, AUXDIM, TEXNUM))
  //   return 52;
  if (bodyInitialize(&grassBody, AUXDIM, TEXNUM))
    return 51;
  if (bodyInitialize(&rockBody, AUXDIM, TEXNUM))
    return 52;
  if (bodyInitialize(&waterBody, AUXDIM, TEXNUM))
    return 53;

  // Initialize texture
  // bodySetTexture(&pillNathanBody, 0, &nathanTex);
  // bodySetTexture(&cubeNathanBody, 0, &nathanTex);
  bodySetTexture(&grassBody, 0, &grassTex);
  bodySetTexture(&rockBody, 0, &rockTex);
  bodySetTexture(&waterBody, 0, &waterTex);

  // if (texInitializeFile(&nathanTex, "nathan_mannes.jpg", GL_LINEAR, GL_LINEAR,
  //                       GL_REPEAT, GL_REPEAT))
  //   return 6;
  if (texInitializeFile(&grassTex, "grassTex.jpg", GL_LINEAR, GL_LINEAR,
                        GL_REPEAT, GL_REPEAT))
    return 6;
  if (texInitializeFile(&rockTex, "rock2Tex.jpg", GL_LINEAR, GL_LINEAR,
                        GL_REPEAT, GL_REPEAT))
    return 6;
  if (texInitializeFile(&waterTex, "waterTex.jpg", GL_LINEAR, GL_LINEAR,
                        GL_REPEAT, GL_REPEAT))
    return 6;

	// Initialize mesh after shader so that we can use the attrLocs
  // TODO: Abstract this better
  // bodySetMesh(&pillNathanBody, &pillMesh);
  // bodySetMesh(&cubeNathanBody, &cubeMesh);
  bodySetMesh(&grassBody, &grassMesh);
  bodySetMesh(&rockBody, &rockMesh);
  bodySetMesh(&waterBody, &waterMesh);

  initializeMesh(); //<-- this piece in particular

  while (glfwWindowShouldClose(window) == 0) {
    oldTime = newTime;
  	newTime = getTime();
		if (floor(newTime) - floor(oldTime) >= 1.0)
			printf("main: %f frames/sec\n", 1.0 / (newTime - oldTime));
  	render(oldTime, newTime);
  	glfwSwapBuffers(window);
  	glfwPollEvents();
  }

  //probs could abstract this better too
  // texDestroy(pillNathanBody.tex[0]);
  // texDestroy(cubeNathanBody.tex[0]);
  texDestroy(grassBody.tex[0]);
  texDestroy(rockBody.tex[0]);
  texDestroy(waterBody.tex[0]);
  shaDestroy(&sha);
	// meshglDestroy(pillNathanBody.mesh);
  // meshglDestroy(cubeNathanBody.mesh);
  meshglDestroy(grassBody.mesh);
  meshglDestroy(rockBody.mesh);
  meshglDestroy(waterBody.mesh);
  // bodyDestroy(&pillNathanBody);
  // bodyDestroy(&cubeNathanBody);
  bodyDestroy(&grassBody);
  bodyDestroy(&rockBody);
  bodyDestroy(&waterBody);
	glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
