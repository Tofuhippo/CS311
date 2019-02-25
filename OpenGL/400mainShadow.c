/*
Dawson d'Almeida and Justin T. Washington
February 25 2018
CS311 with Josh Davis

Main abstracted file that uses openGL to generate a sphere.
Structure/abstraction modelled off of Quinn Mayville's 380mainScene.
*/

/* On macOS, compile with...
    clang 400mainShadow.c /usr/local/gl3w/src/gl3w.o -lglfw -framework OpenGL -framework CoreFoundation -Wno-deprecated
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
#include "400shadow.c"

#define BUFFER_OFFSET(bytes) ((GLubyte*) NULL + (bytes))

/* Configure starting screen width and height */
double screenWidth = 768;
double screenHeight = 512;

#define UNIFVIEWING 0
#define UNIFMODELING 1
#define UNIFDLIGHT 2
#define UNIFCLIGHT 3
#define UNIFAMBIENTLIGHT 4
#define UNIFPCAMERA 5
#define UNIFTEXTURE 6
#define UNIFSHININESS 7
#define UNIFPLIGHTSPOT 8
#define UNIFCLIGHTSPOT 9
#define UNIFDSPOT 10
#define ATTRPOSITION 0
#define ATTRST 1
#define ATTRNORMAL 2

// Number of categories that the attr/unif are representing
// (i.e. position, color, etc..)
#define UNIFNUM 11
#define ATTRNUM 3
#define AUXDIM 1
#define TEXNUM 1

const GLchar *uniformNames[UNIFNUM] = {"viewing", "modeling", "dLight",
                                       "cLight", "ambientLight", "pCamera",
                                       "texture0", "shininess", "pLightSpot",
                                       "cLightSpot", "dSpot"}; // This breaks it - Why
const GLchar **unifNames = uniformNames;
const GLchar *attributeNames[ATTRNUM] = {"position", "st", "normal"};
const GLchar **attrNames = attributeNames;

/* Create shading struct. */
shaShading sha;


/***********************************/
/*     Start: Josh's Functions     */
/***********************************/

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

/*********************************/
/*     End: Josh's Functions     */
/*********************************/


/**********************************************************************/
/*     Start: Initialization, rendering, and destroying Functions     */
/**********************************************************************/

double camTarget[3];
/* Initialize landscape base meshes. Returns 0 if successful,
non-zero otherwise. */
int initializeBaseLandscape(meshMesh *grassBase, meshMesh *rockBase,
  meshMesh *waterBase) {
    meshMesh landBase;
    int landNum = 100;
    double landData[landNum][landNum];
    double landMin, landMean, landMax;
    time_t t;
  	int i;
    /* Set seed because easier to work with fixed landscape (from Quinn) */
  	srand(1550722440);
  	landFlat(landNum, landNum, (double *)landData, 0.0);
  	for (i = 0; i < 32; i += 1)
  		landFault(landNum, landNum, (double *)landData, 1.5 - i * 0.04);
  	for (i = 0; i < 4; i += 1)
  		landBlur(landNum, landNum, (double *)landData);
  	landStatistics(landNum, landNum, (double *)landData, &landMin, &landMean,
  		&landMax);
  	double waterData[4] = {landMin, landMin, landMin, landMin};
    if (meshInitializeLandscape(&landBase, landNum, landNum, 1.0,
  			(double *)landData) != 0){
      printf("meshInitializeLandscape failed\n");
  		return 1;
    }
  	else if (meshInitializeDissectedLandscape(grassBase, &landBase, M_PI / 4.0,
  			1) != 0){
      printf("meshInitializeDissectedLandscape grass failed\n");
  		return 1;
    }
  	else if (meshInitializeDissectedLandscape(rockBase, &landBase, M_PI / 4.0,
  			0) != 0){
      printf("meshInitializeDissectedLandscape rock failed\n");
  		return 1;
    }
  	else if (meshInitializeLandscape(waterBase, 2, 2, landNum - 1.0,
  			(double *)waterData) != 0){
      printf("meshInitializeLandscape water failed\n");
  		return 1;
    }
  	meshDestroy(&landBase);

  	vec3Set(landNum / 2.0, landNum / 2.0,
  		landData[landNum / 2][landNum / 2], camTarget);
  	return 0;
  }


/* Binds the attributes of mesh in GPU. */
void meshInitializeMiddleStep(meshglMesh *mesh) {
	/* Updated in 350 */
	/* Tell the VAO about the attribute arrays and how they should hook into
	the vertex shader. These OpenGL calls used to happen at rendering time. Now
	they happen at initialization time, and the VAO remembers them. Magic. */
	glEnableVertexAttribArray(sha.attrLocs[ATTRPOSITION]);
	glVertexAttribPointer(sha.attrLocs[ATTRPOSITION], 3, GL_DOUBLE, GL_FALSE,
		mesh->attrDim * sizeof(GLdouble), BUFFER_OFFSET(0));
  glEnableVertexAttribArray(sha.attrLocs[ATTRST]);
	glVertexAttribPointer(sha.attrLocs[ATTRST], 2, GL_DOUBLE, GL_FALSE,
		mesh->attrDim * sizeof(GLdouble), BUFFER_OFFSET(3 * sizeof(GLdouble)));
  glEnableVertexAttribArray(sha.attrLocs[ATTRNORMAL]);
  glVertexAttribPointer(sha.attrLocs[ATTRNORMAL], 3, GL_DOUBLE, GL_FALSE,
    mesh->attrDim * sizeof(GLdouble), BUFFER_OFFSET(5 * sizeof(GLdouble)));

  meshglFinishInitialization(mesh);
}


/* Create meshglMesh structs. */
meshglMesh grassMesh;
meshglMesh rockMesh;
meshglMesh waterMesh;
meshglMesh epcotPillMesh;
/* Initializes the meshes. Returns 0 if successful. */
int initializeMeshes(void) {
  /* Initialize base mesh's for meshglMesh set up. */
  meshMesh grassBase;
  meshMesh rockBase;
  meshMesh waterBase;
  meshMesh epcotPillBase;
  if (initializeBaseLandscape(&grassBase, &rockBase, &waterBase) != 0)
		return 1;

  /* Initialize grass meshgl. */
  meshglInitialize(&grassMesh, &grassBase);
	meshInitializeMiddleStep(&grassMesh); // Calls final step
	meshDestroy(&grassBase);

  /* Initialize rock meshgl. */
  meshglInitialize(&rockMesh, &rockBase);
	meshInitializeMiddleStep(&rockMesh); // Calls final step
	meshDestroy(&rockBase);

  /* Initialize water meshgl. */
  meshglInitialize(&waterMesh, &waterBase);
	meshInitializeMiddleStep(&waterMesh); // Calls final step
	meshDestroy(&waterBase);

  meshInitializeCapsule(&epcotPillBase, 10.0, 20.0, 16.0, 32.0);
  meshglInitialize(&epcotPillMesh, &epcotPillBase);
  meshInitializeMiddleStep(&epcotPillMesh); // Calls final step
	meshDestroy(&epcotPillBase);

  return 0;
}


/* Destroys meshes. */
void destroyMeshes(void) {
  meshglDestroy(&grassMesh);
	meshglDestroy(&rockMesh);
	meshglDestroy(&waterMesh);
  meshglDestroy(&epcotPillMesh);
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
    uniform float shininess;\
    uniform vec3 pLightSpot;\
    uniform vec3 cLightSpot;\
    uniform vec3 dSpot;\
		in vec2 texCoords;\
		in vec3 nop;\
    in vec4 world;\
    out vec4 fragColor;\
		void main() {\
      vec3 rgbFromTex = vec3(texture(texture0, texCoords));\
      vec3 cDiff = rgbFromTex;\
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
			float iSpec = dot(dRefl, dCameraNorm);\
			if (iDiff <= 0.0 || iSpec < 0.0) {\
				iSpec = 0.0;\
			}\
      iSpec = pow(iSpec, shininess);\
			vec3 cSurface = vec3(1.0, 1.0, 1.0);\
			vec3 specular = iSpec * cSurface * cLight;\
      \
      float angleSpot = 0.1;\
      float cosSpotAngle = .98;\
      vec3 dLightSpot = normalize(pLightSpot - pFragment);\
      float iDiffSpot = dot(dLightSpot, dNormal);\
      if (iDiffSpot < 0.0) {\
				iDiffSpot = 0.0;\
			}\
      vec3 diffuseSpot = iDiffSpot * cLightSpot * rgbFromTex;\
      vec3 dReflSpot = normalize((2.0 * dot(dLightSpot, dNormal)) * dNormal - dLightSpot);\
			float iSpecSpot = dot(dReflSpot, dSpot);\
			if (iDiffSpot <= 0.0 || iSpecSpot < 0.0) {\
				iSpecSpot = 0.0;\
			}\
      iSpecSpot = pow(iSpecSpot, shininess);\
      vec3 specularSpot = iSpecSpot * cSurface * cLightSpot;\
      if (dot(dSpot, dLightSpot) < cosSpotAngle) {\
        diffuseSpot = vec3(0.0, 0.0, 0.0);\
        specularSpot = vec3(0.0, 0.0, 0.0);\
      }\
      fragColor = vec4(diffuse + ambient + specular + diffuseSpot + specularSpot, 1.0);\
		}";
	shaInitialize(&sha, vertexCode, fragmentCode, UNIFNUM, unifNames, ATTRNUM,
		            attrNames);
	return (sha.program == 0);
}


/* Destroys shading program(s). */
void destroyShading(void) {
  shaDestroy(&sha);
}


texTexture grassTex;
texTexture rockTex;
texTexture waterTex;
texTexture epcotTex;
/* Initializes textures. Return 0 if successful, non-zero otherwise. */
int initializeTextures(void) {
  if (texInitializeFile(&grassTex, "grassTex.jpg", GL_LINEAR, GL_LINEAR,
                        GL_REPEAT, GL_REPEAT))
    return 1;
  if (texInitializeFile(&rockTex, "rockTex.jpg", GL_LINEAR, GL_LINEAR,
                        GL_REPEAT, GL_REPEAT))
    return 1;
  if (texInitializeFile(&waterTex, "waterTex.jpg", GL_LINEAR, GL_LINEAR,
                        GL_REPEAT, GL_REPEAT))
    return 1;
  if (texInitializeFile(&epcotTex, "epcotTex.jpg", GL_LINEAR, GL_LINEAR,
                        GL_REPEAT, GL_REPEAT))
    return 1;
  return 0;
}


/* Destroys textures. */
void destroyTextures(void) {
  texDestroy(&grassTex);
  texDestroy(&rockTex);
  texDestroy(&waterTex);
  texDestroy(&epcotTex);
}


camCamera cam;
/* Sets the camera position. */
void setCameraPosition() {
	GLdouble camPosition[3];
	vecCopy(3, (cam.isometry).translation, camPosition);
	uniformVector3(camPosition, sha.unifLocs[UNIFPCAMERA]);
}


camCamera spotCam; // Should have same position and direction as spotlight
/* Sets the spotlight camera position. */
void setSpotCameraPosition(GLdouble[3] pSpot, GLdouble[3] dSpot) {
	GLdouble camSpotPosition[3];
	vecCopy(3, (spotCam.isometry).translation, pSpot);
  // Maybe do this?
  // camLookAt(&spotCam, target[3], GLdouble rho, GLdouble phi,
  // 		GLdouble theta)
	uniformVector3(camPosition, sha.unifLocs[UNIFPCAMERA]);
}


double cameraRho;
double cameraPhi;
double cameraTheta;
bodyBody grassBody;
bodyBody rockBody;
bodyBody waterBody;
bodyBody epcotPillBody;
/* Initializes the lights, camera, and bodies in the scene. Returns 0 if
successful, non-zero otherwise. */
int initializeScene(void) {
  /* Configure the camera. */
  cameraRho = 256.0;
  cameraPhi = M_PI / 4.0;
  cameraTheta = 0.0;
  camLookAt(&cam, camTarget, cameraRho, cameraPhi, cameraTheta);
	camSetProjectionType(&cam, camPERSPECTIVE);
	camSetFrustum(&cam, M_PI / 6.0, cameraRho, 10.0, screenWidth, screenHeight);

  /* Initialize grass body and texture. */
  if (bodyInitialize(&grassBody, AUXDIM, TEXNUM))
    return 1;
  bodySetTexture(&grassBody, 0, &grassTex);
  bodySetMesh(&grassBody, &grassMesh);
  /* Set grass's shininess auxiliary. */
  bodySetAuxiliary(&grassBody, 0, 5000000.0);

  /* Initialize rock body and texture. */
  if (bodyInitialize(&rockBody, AUXDIM, TEXNUM))
    return 1;
  bodySetTexture(&rockBody, 0, &rockTex);
  bodySetMesh(&rockBody, &rockMesh);
  /* Set rock's shininess auxiliary. */
  bodySetAuxiliary(&rockBody, 0, 150.0);

  /* Initialize water body and texture. */
  if (bodyInitialize(&waterBody, AUXDIM, TEXNUM))
    return 1;
  bodySetTexture(&waterBody, 0, &waterTex);
  bodySetMesh(&waterBody, &waterMesh);
  /* Set water's shininess auxiliary. */
  bodySetAuxiliary(&waterBody, 0, 40.0);

  /* Initialize epcot pill body and texture. */
  if (bodyInitialize(&epcotPillBody, AUXDIM, TEXNUM))
    return 1;
  bodySetTexture(&epcotPillBody, 0, &epcotTex);
  bodySetMesh(&epcotPillBody, &epcotPillMesh);
  /* Set grass's shininess auxiliary. */
  bodySetAuxiliary(&epcotPillBody, 0, 50.0);

  /* Set dLight, cLight, and ambientLight. */
  GLdouble dLight[3] = {10.0, 10.0, 10.0};
	GLdouble cLight[3] = {0.2, 0.2, 0.2};
	GLdouble ambientLight[3] = {0.1, 0.1, 0.1};
  uniformVector3(dLight, sha.unifLocs[UNIFDLIGHT]);
	uniformVector3(cLight, sha.unifLocs[UNIFCLIGHT]);
	uniformVector3(ambientLight, sha.unifLocs[UNIFAMBIENTLIGHT]);

  /* Set pLightSpot, cLightSpot, and dSpot. */
  GLdouble pLightSpot[3] = {50.0, 0.0, 5.0};
  GLdouble cLightSpot[3] = {1.0, 1.0, 1.0};
  GLdouble dSpot[3] = {0.0, -1.0, 0.5};
  uniformVector3(pLightSpot, sha.unifLocs[UNIFPLIGHTSPOT]);
	uniformVector3(cLightSpot, sha.unifLocs[UNIFCLIGHTSPOT]);
  uniformVector3(dSpot, sha.unifLocs[UNIFDSPOT]);

  /* Set camera position. */
  setCameraPosition();
  return 0;
}


/* Destroys all bodies in scene. */
void destroyScene(void) {
	bodyDestroy(&grassBody);
	bodyDestroy(&rockBody);
	bodyDestroy(&waterBody);
  bodyDestroy(&epcotPillBody);
}


shadowMap map;
/* Initializes shadowMap. */
int initializeShadowMap(void) {
  if (shadowInitialize(&map, screenWidth, screenHeight) != 0)
    return 1;
  return 0;
}


/* Destroys shadowMap. */
void destroyShadowMap(void) {
  shadowDestroy(&map);
}


/* Sets individual body's isometry and send it to the shader program. Then
renders the body. */
void renderBody(bodyBody *body, GLdouble trans[3], GLdouble rot[3][3]) {
  /* Update body's isometry. */
  isoSetTranslation(&(body->isometry), trans);
	isoSetRotation(&(body->isometry), rot);

  /* Load isometry into shader. */
  GLdouble model[4][4];
	isoGetHomogeneous(&(body->isometry), model);
  /* This seems weird. Why do we keep changing the UNIF for every different
  body? */
	uniformMatrix44(model, sha.unifLocs[UNIFMODELING]);

  /* Replaced bind, render, unbind in 340 cause of ABSTRACTION. */
  /* Load shininess and texture, then render */
	glUniform1f(sha.unifLocs[UNIFSHININESS], body->aux[0]); // Gl function
	texRender((body->tex[0]), GL_TEXTURE0, 0, sha.unifLocs[UNIFTEXTURE]);
	meshglRender(body->mesh);
	texUnrender((body->tex[0]), 0);
}


double angle = 0.0;
double waterLevel = 3.0;
/* Renders the scene. */
void render(double oldTime, double newTime) {
  /* Clear buffer and shader program. */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(sha.program);

	/* Update isometry for bodies. */
	GLdouble trans[3] = {0.0, 0.0, 0.0};
	GLdouble axis[3] = {1.0 / sqrt(3.0), 1.0 / sqrt(3.0), 1.0 / sqrt(3.0)};
	GLdouble rot[3][3];
	mat33AngleAxisRotation(angle, axis, rot);

  /* Individually done for each body/mesh/texture/etc */
  /* Render grass body. */
  renderBody(&grassBody, trans, rot);

  /* Render rock body. */
  renderBody(&rockBody, trans, rot);

  /* Render water body. */
  waterLevel += sin(newTime)/500;
  double transWater[3] = {0.0, 0.0, waterLevel};
  renderBody(&waterBody, transWater, rot);

  /* Render epcot's spicy body. */
  double transepcot[3] = {50.0, 50.0, 0.0};
  renderBody(&epcotPillBody, transepcot, rot);

  /* Send our own viewing transformation P C^-1 to the shaders */
	GLdouble viewing[4][4];
	camGetProjectionInverseIsometry(&cam, viewing);
	uniformMatrix44(viewing, sha.unifLocs[UNIFVIEWING]);
}


/********************************************************************/
/*     End: Initialization, rendering, and destroying Functions     */
/********************************************************************/


/*************************************************/
/*     Start: Main and main helper functions     */
/*************************************************/

/* Handle key presses to move camera and change water level. */
void handleKeyAny(GLFWwindow* window, int key, int s, int a, int m) {
	if (key == GLFW_KEY_A)
		cameraTheta -= M_PI / 100;
	else if (key == GLFW_KEY_D)
		cameraTheta += M_PI / 100;
	else if (key == GLFW_KEY_W)
		cameraPhi -= M_PI / 100;
	else if (key == GLFW_KEY_S)
		cameraPhi += M_PI / 100;
	else if (key == GLFW_KEY_Q)
		cameraRho *= 0.9;
	else if (key == GLFW_KEY_E)
		cameraRho *= 1.1;
	else if (key == GLFW_KEY_K)
		camTarget[0] -= 0.5;
	else if (key == GLFW_KEY_SEMICOLON)
		camTarget[0] += 0.5;
	else if (key == GLFW_KEY_L)
		camTarget[1] -= 0.5;
	else if (key == GLFW_KEY_O)
		camTarget[1] += 0.5;
	else if (key == GLFW_KEY_I)
		camTarget[2] -= 0.5;
	else if (key == GLFW_KEY_P)
		camTarget[2] += 0.5;
	else if (key == GLFW_KEY_J)
		waterLevel -= 0.1;
	else if (key == GLFW_KEY_U)
		waterLevel += 0.1;
	camSetFrustum(&cam, M_PI / 6.0, cameraRho, 10.0, screenWidth, screenHeight);
	camLookAt(&cam, camTarget, cameraRho, cameraPhi, cameraTheta);
	setCameraPosition();
}


GLFWwindow *window;
/* Initialize GLFWwindow *window and requests openGL version. */
int requestOpenGLVersion(int major, int minor){
  glfwSetErrorCallback(handleError);
	if (glfwInit() == 0) {
		fprintf(stderr, "main: glfwInit failed.\n");
		return 1;
	}
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


int main(void) {
	double oldTime;
	double newTime = getTime();

  /* Request a particular version of openGL and utilize the global window */
  int openGLRequest = requestOpenGLVersion(3, 2);
	if (openGLRequest != 0)
    return openGLRequest;

  /* User interface. */
  glfwSetKeyCallback(window, handleKeyAny);

  /* for testing, changes window's background */
  //glClearColor(1.0, 1.0, 0.0, 1.0);

  /* Initialize textures, shader, meshes, and scene */
	if (initializeTextures() != 0)
		return 4;
	if (initializeShaderProgram() != 0)
		return 5;
	if (initializeMeshes() != 0)
		return 6;
  if (initializeShadowMap() != 0)
		return 7;
	initializeScene(); // Where body initialization happens

  /* Run the program. */
  while (glfwWindowShouldClose(window) == 0) {
    oldTime = newTime;
  	newTime = getTime();
		if (floor(newTime) - floor(oldTime) >= 1.0)
			printf("main: %f frames/sec\n", 1.0 / (newTime - oldTime));
  	render(oldTime, newTime);
  	glfwSwapBuffers(window);
  	glfwPollEvents();
  }

  /* Destroy and terminate. */
  destroyTextures();
  destroyShading();
  destroyMeshes();
  destroyScene();
  destroyShadowMap();
	glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
