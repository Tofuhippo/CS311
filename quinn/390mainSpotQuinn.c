/* Quinn Mayville */

/* Renders a simple scene with a spot light. Camera and water level can be
manipulated via keyboard in the same way as previous assignments.
The positional light's x,y corrdinates can be changed with FTGH keys. */


/* On macOS, compile with...
    clang 390mainSpotQuinn.c /usr/local/gl3w/src/gl3w.o -lglfw -framework OpenGL -framework CoreFoundation -Wno-deprecated
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

/* Configure starting screen width and height */
double screenW = 768;
double screenH = 512;

/* Configure uniform and attribute names */
#define UNIFVIEWING 0
#define UNIFMODELING 1
#define UNIFDLIGHT 2
#define UNIFCLIGHT 3
#define UNIFCAMBIENT 4
#define UNIFPCAMERA 5
#define UNIFTEX0 6
#define UNIFSHININESS 7
#define UNIFPSPOT 8
#define UNIFCSPOT 9
#define UNIFDSPOT 10
#define UNIFNUM 11

#define ATTRPOSITION 0
#define ATTRTEXCOORDS 1
#define ATTRNORMAL 2
#define ATTRNUM 3

const GLchar *uniformNames[UNIFNUM] = {
	"viewing", "modeling", "dLight", "cLight", "cAmbient", "pCamera",
	"texture0", "shininess", "pSpot", "cSpot", "dSpot",
};
const GLchar **unifNames = uniformNames;
const GLchar *attributeNames[ATTRNUM] = {"position", "texCoords", "normal"};
const GLchar **attrNames = attributeNames;
shaShading sha;


double getTime(void) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (double)tv.tv_sec + (double)tv.tv_usec * 0.000001;
}

void handleError(int error, const char *description) {
	fprintf(stderr, "handleError: %d\n%s\n", error, description);
}


double camTarget[3];  // Included here since set based on landscape
/* Initializes base landscape meshes */
int initializeBaseLandscape(meshMesh *baseGrass, meshMesh *baseRock, meshMesh *baseWater) {
	meshMesh land;
	int landNum = 100;
	double landData[landNum][landNum];
	double landMin, landMean, landMax;
	time_t t;
	int i;
	/* Set seed because easier to work with fixed landscape */
	srand(1550722440);
	landFlat(landNum, landNum, (double *)landData, 0.0);
	for (i = 0; i < 32; i += 1)
		landFault(landNum, landNum, (double *)landData, 1.5 - i * 0.04);
	for (i = 0; i < 4; i += 1)
		landBlur(landNum, landNum, (double *)landData);
	landStatistics(landNum, landNum, (double *)landData, &landMin, &landMean,
		&landMax);
	double waterData[4] = {landMin, landMin, landMin, landMin};
	if (meshInitializeLandscape(&land, landNum, landNum, 1.0,
			(double *)landData) != 0)
		return 1;
	else if (meshInitializeDissectedLandscape(baseGrass, &land, M_PI / 4.0,
			1) != 0)
		return 1;
	else if (meshInitializeDissectedLandscape(baseRock, &land, M_PI / 4.0,
			0) != 0)
		return 1;
	else if (meshInitializeLandscape(baseWater, 2, 2, landNum - 1.0,
			(double *)waterData) != 0)
		return 1;
	meshDestroy(&land);

	vec3Set(landNum / 2.0, landNum / 2.0,
		landData[landNum / 2][landNum / 2], camTarget);
	return 0;
}


/* Sets the given meshgl's attributes and finishes intialization.
Assumes attributes are XYZSTNOP. */
void setMeshAttributes(meshglMesh *mesh){
	/* Get attribute locations. (same for all meshes) */
	GLint positionLoc = sha.attrLocs[ATTRPOSITION];
	GLint texCoordsLoc = sha.attrLocs[ATTRTEXCOORDS];
	GLint normalLoc = sha.attrLocs[ATTRNORMAL];
	/* Set position to XYZ */
	glEnableVertexAttribArray(positionLoc);
	glVertexAttribPointer(positionLoc, 3, GL_DOUBLE, GL_FALSE,
		mesh->attrDim * sizeof(GLdouble), BUFFER_OFFSET(0));
	glEnableVertexAttribArray(texCoordsLoc);
	/* Set texCoords to ST */
	glVertexAttribPointer(texCoordsLoc, 2, GL_DOUBLE, GL_FALSE,
		mesh->attrDim * sizeof(GLdouble), BUFFER_OFFSET(3 * sizeof(GLdouble)));
	glEnableVertexAttribArray(normalLoc);
	/* Set normal to NOP */
	glVertexAttribPointer(normalLoc, 3, GL_DOUBLE, GL_FALSE,
		mesh->attrDim * sizeof(GLdouble), BUFFER_OFFSET(5 * sizeof(GLdouble)));
	/* Finish initialization */
	meshglFinishInitialization(mesh);
}


meshglMesh grass;
meshglMesh rock;
meshglMesh water;
meshglMesh bush;
/* Initializes the meshes. Returns 0 on success, non-zero otherwise. */
int initializeMeshes(void) {
	/* Get attribute locations. (same for all meshes) */
	GLint positionLoc = sha.attrLocs[ATTRPOSITION];
	GLint texCoordsLoc = sha.attrLocs[ATTRTEXCOORDS];
	GLint normalLoc = sha.attrLocs[ATTRNORMAL];

	/* Initialize base landscape meshes */
	meshMesh baseGrass;
	meshMesh baseRock;
	meshMesh baseWater;
	if (initializeBaseLandscape(&baseGrass, &baseRock, &baseWater) != 0)
		return 1;

	/* Initialize grass mesh */
	meshglInitialize(&grass, &baseGrass);
	meshDestroy(&baseGrass);
	setMeshAttributes(&grass);

	/* Initialize rock mesh */
	meshglInitialize(&rock, &baseRock);
	meshDestroy(&baseRock);
	setMeshAttributes(&rock);

	/* Initialize water mesh */
	meshglInitialize(&water, &baseWater);
	meshDestroy(&baseWater);
	/* Set position to XYZ */
	setMeshAttributes(&water);

	/* Initialize bush mesh */
	meshMesh baseSphere;
	meshInitializeSphere(&baseSphere, 4.0, 16, 32);
	meshglInitialize(&bush, &baseSphere);
	meshDestroy(&baseSphere);
	setMeshAttributes(&bush);

	return 0;
}


/* Destroys all meshes in the scene */
void destroyMeshes(void) {
	meshglDestroy(&grass);
	meshglDestroy(&rock);
	meshglDestroy(&water);
	meshglDestroy(&bush);
}


/* Initializes shader program. Returns 0 on success, non-zero on failure. */
int initializeShaderProgram(void) {
	GLchar vertexCode[] = "\
		#version 140\n\
		uniform mat4 viewing;\
		uniform mat4 modeling;\
		in vec3 position;\
		in vec2 texCoords;\
		in vec3 normal;\
		out vec2 st;\
		out vec3 nop;\
		out vec3 pFragment;\
		void main() {\
			vec4 world = modeling * vec4(position, 1.0);\
			pFragment = vec3(world);\
			gl_Position = viewing * world;\
			st = texCoords;\
			nop = vec3(modeling * vec4(normal, 0.0));\
		}";
	GLchar fragmentCode[] = "\
		#version 140\n\
		uniform vec3 dLight;\
		uniform vec3 cLight;\
		uniform vec3 cAmbient;\
		uniform vec3 pCamera;\
		uniform sampler2D texture0;\
		uniform float shininess;\
		uniform vec3 pSpot;\
		uniform vec3 cSpot;\
		uniform vec3 dSpot;\
		in vec3 nop;\
		in vec2 st;\
		in vec3 pFragment;\
		out vec4 fragColor;\
		void main() {\
			vec3 rgbFromTex = vec3(texture(texture0, st));\
			vec3 cDiff = rgbFromTex;\
			\
			vec3 dNorm = normalize(nop);\
			vec3 dCamera = normalize(pCamera - pFragment);\
			\
			float iDiffLight = dot(dLight, dNorm);\
			if (iDiffLight < 0.0)\
				iDiffLight = 0.0;\
			vec3 diffuseLight = iDiffLight * cDiff * cLight;\
			\
			vec3 dLightSpot = pSpot - pFragment;\
			dLightSpot = normalize(dLightSpot);\
			float iDiffSpot = dot(dLightSpot, dNorm);\
			if (iDiffSpot < 0.0)\
				iDiffSpot = 0.0;\
			vec3 diffuseSpot = iDiffSpot * cDiff * cSpot;\
			\
			vec3 ambient = cDiff * cAmbient;\
			\
			float iSpecLight;\
			vec3 dReflLight;\
			if (iDiffLight == 0.0)\
				iSpecLight = 0.0;\
			else {\
				dReflLight = 2.0 * dot(dLight, dNorm) * dNorm - dLight;\
				iSpecLight = dot(dReflLight, dCamera);\
				if (iSpecLight < 0.0)\
					iSpecLight = 0.0;\
				iSpecLight = pow(iSpecLight, shininess);\
			}\
			vec3 cSpecLight = vec3(1.0, 1.0, 1.0);\
			vec3 specularLight = iSpecLight * cSpecLight * cLight;\
			\
			float iSpecSpot;\
			vec3 dReflSpot;\
			if (iDiffSpot == 0.0)\
				iSpecSpot = 0.0;\
			else {\
				dReflSpot = 2.0 * dot(dLightSpot, dNorm) * dNorm - dLightSpot;\
				iSpecSpot = dot(dReflSpot, dCamera);\
				if (iSpecSpot < 0.0)\
					iSpecSpot = 0.0;\
				iSpecSpot = pow(iSpecSpot, shininess);\
			}\
			vec3 cSpecSpot = vec3(1.0, 1.0, 1.0);\
			vec3 specularSpot = iSpecSpot * cSpecSpot * cSpot;\
			\
			float cosAngleSpot = 0.981;\
			if (dot(dLightSpot, dSpot) < cosAngleSpot) {\
				diffuseSpot = vec3(0.0, 0.0, 0.0);\
				specularSpot = vec3(0.0, 0.0, 0.0);\
			}\
			\
			vec3 color = diffuseLight + specularLight + diffuseSpot + specularSpot + ambient;\
			fragColor = vec4(color, 1.0);\
		}";

	shaInitialize(&sha, vertexCode, fragmentCode, UNIFNUM, unifNames, ATTRNUM, attrNames);
	return (sha.program == 0);
}


/* Destroys shading program */
void destroyShading(void) {
	shaDestroy(&sha);
}


texTexture texGrass;
texTexture texRock;
texTexture texWater;
texTexture texBush;
/* Initializes all textures in the scene. Returns 0 on success, non-zero on failure. */
int initializeTextures(void) {
	if (texInitializeFile(&texGrass, "grass.jpg", GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT) != 0)
		return 1;
	if (texInitializeFile(&texRock, "rockTex.jpg", GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT) != 0)
		return 1;
	if (texInitializeFile(&texWater, "waterTex.jpg", GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT) != 0)
		return 1;
	if (texInitializeFile(&texBush, "nathan_mannes.jpg", GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT) != 0)
		return 1;
	return 0;
}

/* Destroys all textures */
void destroyTextures(void) {
	texDestroy(&texGrass);
	texDestroy(&texRock);
	texDestroy(&texWater);
	texDestroy(&texBush);
}


/* Places matrix m into uniformLocation in OpenGL shaders */
void uniformMatrix44(GLdouble m[4][4], GLint uniformLocation) {
	GLfloat mTFloat[4][4];
	for (int i = 0; i < 4; i += 1)
		for (int j = 0; j < 4; j += 1)
			mTFloat[i][j] = m[j][i];
	glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, (GLfloat *)mTFloat);
}


/* Places vector v into uniformLocation in OpenGL shaders */
void uniformVector3(GLdouble v[3], GLint uniformLocation) {
	GLfloat vFloat[3];
	for (int i = 0; i < 3; i += 1)
		vFloat[i] = v[i];
	glUniform3fv(uniformLocation, 1, vFloat);
}


camCamera cam;
/* Sets the camera position */
void setCameraPosition() {
	GLdouble camPosition[3];
	vecCopy(3, (cam.isometry).translation, camPosition);
	uniformVector3(camPosition, sha.unifLocs[UNIFPCAMERA]);
}


double cameraRho;
double cameraPhi;
double cameraTheta;
double pSpot[3] = {85.0, 80.0, 5.0};  // Starting position of spot
bodyBody bodyGrass;
bodyBody bodyRock;
bodyBody bodyWater;
bodyBody bodyBush0;
bodyBody bodyBush1;
/* Initializes the lights, camera, and bodies in the scene */
void initializeScene(void) {
	/* Configure the camera once and for all. */
	cameraRho = 256.0;
	cameraPhi = M_PI / 4.0;
	cameraTheta = 0.0;
	camLookAt(&cam, camTarget, cameraRho, cameraPhi, cameraTheta);
	camSetProjectionType(&cam, camPERSPECTIVE);
	camSetFrustum(&cam, M_PI / 6.0, cameraRho, 10.0, screenW, screenH);

	/* Initialize grass body. Isometry is set at each frame in render. */
	bodyInitialize(&bodyGrass, 1, 1);
	bodySetMesh(&bodyGrass, &grass);
	bodySetTexture(&bodyGrass, 0, &texGrass);
	bodySetAuxiliary(&bodyGrass, 0, 500.0); // Shininess

	/* Initialize rock body. Isometry is set at each frame in render. */
	bodyInitialize(&bodyRock, 1, 1);
	bodySetMesh(&bodyRock, &rock);
	bodySetTexture(&bodyRock, 0, &texRock);
	bodySetAuxiliary(&bodyRock, 0, 150.0);  // Shininess

	/* Initialize water body. Isometry is set at each frame in render. */
	bodyInitialize(&bodyWater, 1, 1);
	bodySetMesh(&bodyWater, &water);
	bodySetTexture(&bodyWater, 0, &texWater);
	bodySetAuxiliary(&bodyWater, 0, 40.0);  // Shininess

	/* Initialize bush0 body. Isometry is set at each frame in render. */
	bodyInitialize(&bodyBush0, 1, 1);
	bodySetMesh(&bodyBush0, &bush);
	bodySetTexture(&bodyBush0, 0, &texBush);
	bodySetAuxiliary(&bodyBush0, 0, 20.0);  // Shininess

	/* Initialize bush1 body. Isometry is set at each frame in render. */
	bodyInitialize(&bodyBush1, 1, 1);
	bodySetMesh(&bodyBush1, &bush);
	bodySetTexture(&bodyBush1, 0, &texBush);
	bodySetAuxiliary(&bodyBush1, 0, 20.0);  // Shininess

	/* Set light direction */
	GLdouble dLight[3] = {2.0, 5.0, 40.0};
	vecUnit(3, dLight, dLight);
	uniformVector3(dLight, sha.unifLocs[UNIFDLIGHT]);

	/* Set light color */
	GLdouble cLight[3] = {0.4, 0.4, 1.0};
	uniformVector3(cLight, sha.unifLocs[UNIFCLIGHT]);

	/* Set spot position */
	uniformVector3(pSpot, sha.unifLocs[UNIFPSPOT]);

	/* Set spot color */
	GLdouble cSpot[3] = {1.0, 1.0, 0.5};
	uniformVector3(cSpot, sha.unifLocs[UNIFCSPOT]);

	/* Set spot direction */
	GLdouble dSpot[3] = {1.0, 0.0, 0.1};
	vecUnit(3, dSpot, dSpot);
	uniformVector3(dSpot, sha.unifLocs[UNIFDSPOT]);

	/* Set ambient light */
	GLdouble cAmbient[3] = {0.1, 0.1, 0.1};
	uniformVector3(cAmbient, sha.unifLocs[UNIFCAMBIENT]);

	/* Set camera position */
	setCameraPosition();
}


/* Destroys all bodies in the scene */
void destroyScene(void) {
	bodyDestroy(&bodyGrass);
	bodyDestroy(&bodyRock);
	bodyDestroy(&bodyWater);
	bodyDestroy(&bodyBush0);
	bodyDestroy(&bodyBush1);
}


/* Sends the body's isometry to the shader program and renders the body.
Assumes bodies have one auxiliary for shininess and one texture. */
void renderBody(bodyBody *body) {
	/* Load isometry into shader */
	GLdouble model[4][4];
	isoGetHomogeneous(&(body->isometry), model);
	uniformMatrix44(model, sha.unifLocs[UNIFMODELING]);

	/* Load shininess and texture, then render */
	glUniform1f(sha.unifLocs[UNIFSHININESS], body->aux[0]);
	texRender(body->tex[0], GL_TEXTURE0, 0, sha.unifLocs[UNIFTEX0]);
	meshglRender(body->mesh);
	texUnrender(body->tex[0], 0);
}


double angle = 0.0;
double waterLevel = 3.0;
/* Renders the scene */
void render(double oldTime, double newTime) {
	/* Clear buffers and use shader program */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(sha.program);

	/* Update standard isometry for bodies. Note there is no translation
	or rotation but I'm leaving this in so it can be easily modified. */
	GLdouble trans[3] = {0.0, 0.0, 0.0};
	GLdouble axis[3] = {1.0 / sqrt(3.0), 1.0 / sqrt(3.0), 1.0 / sqrt(3.0)};
	GLdouble rot[3][3];
	mat33AngleAxisRotation(angle, axis, rot);

	/* Update grass isometry */
	isoSetTranslation(&(bodyGrass.isometry), trans);
	isoSetRotation(&(bodyGrass.isometry), rot);

	/* Update rock isometry */
	isoSetTranslation(&(bodyRock.isometry), trans);
	isoSetRotation(&(bodyRock.isometry), rot);

	/* Update water isometry */
	double transWater[3] = {0.0, 0.0, waterLevel};
	isoSetTranslation(&(bodyWater.isometry), transWater);
	isoSetRotation(&(bodyWater.isometry), rot);

	/* Update bush0 isometry */
	double transBush0[3] = {50.0, 80.0, 1.0};
	isoSetTranslation(&(bodyBush0.isometry), transBush0);
	isoSetRotation(&(bodyBush0.isometry), rot);

	/* Update bush1 isometry */
	double transBush1[3] = {75.0, 20.0, -2.0};
	isoSetTranslation(&(bodyBush1.isometry), transBush1);
	isoSetRotation(&(bodyBush1.isometry), rot);

	/* Send our own viewing transformation P C^-1 to the shaders */
	GLdouble viewing[4][4];
	camGetProjectionInverseIsometry(&cam, viewing);
	uniformMatrix44(viewing, sha.unifLocs[UNIFVIEWING]);

	/* Render bodies */
	renderBody(&bodyGrass);
	renderBody(&bodyRock);
	renderBody(&bodyWater);
	renderBody(&bodyBush0);
	renderBody(&bodyBush1);
}


/* Handles resizing of window */
void handleResize(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
	camSetFrustum(&cam, M_PI / 6.0, 256, 10.0, width, height);
	screenW = width;
	screenH = height;
}


/* Handle key presses to move camera, change water level, and move
positional light. */
void handleKeyAny(GLFWwindow* window, int key, int s, int a, int m) {
	// Move camera
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
	// Change water level (isometry updated in render)
	else if (key == GLFW_KEY_J)
		waterLevel -= 0.1;
	else if (key == GLFW_KEY_U)
		waterLevel += 0.1;
	// Move positional light x,y coords on FTGH keys
	else if (key == GLFW_KEY_G)
		pSpot[0] += 1.0;
	else if (key == GLFW_KEY_T)
		pSpot[0] -= 1.0;
	else if (key == GLFW_KEY_H)
		pSpot[1] += 1.0;
	else if (key == GLFW_KEY_F)
		pSpot[1] -= 1.0;
	// Update camera position based on changes
	camSetFrustum(&cam, M_PI / 6.0, cameraRho, 10.0, screenW, screenH);
	camLookAt(&cam, camTarget, cameraRho, cameraPhi, cameraTheta);
	setCameraPosition();
	// Pass new pSpot into shader
	uniformVector3(pSpot, sha.unifLocs[UNIFPSPOT]);
}


GLFWwindow *window;
/* Performs the necessary set up for OpenGl and GLFW */
int setUpGLFW(void) {
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
	window = glfwCreateWindow(screenW, screenH, "Making a Scene", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "main: glfwCreateWindow failed.\n");
		glfwTerminate();
		return 2;
	}
	glfwSetWindowSizeCallback(window, handleResize);
	glfwMakeContextCurrent(window);
	if (gl3wInit() != 0) {
		fprintf(stderr, "main: gl3wInit failed.\n");
		glfwDestroyWindow(window);
		glfwTerminate();
		return 3;
	}
	if (gl3wIsSupported(3, 2) == 0)
		fprintf(stderr, "main: OpenGL 3.2 is not supported.\n");
	else
		fprintf(stderr, "main: OpenGL 3.2 is supported.\n");
	fprintf(stderr, "main: OpenGL %s, GLSL %s.\n",
		glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	/* User interface */
	glfwSetKeyCallback(window, handleKeyAny);

	return 0;
}


/* Destroys the glfw window and terminates glfw */
void destroyGLFW(void) {
	glfwDestroyWindow(window);
	glfwTerminate();
}


int main(void) {
	/* Timing and GLFW set up */
	double oldTime;
	double newTime = getTime();
	int setUp = setUpGLFW();
	if (setUp != 0)
		return setUp;

	/* Initialize textures, shader, meshes, and scene */
	if (initializeTextures() != 0)
		return 4;
	if (initializeShaderProgram() != 0)
		return 5;
	if (initializeMeshes() != 0)
		return 6;
	initializeScene();

	/* Run program */
    while (glfwWindowShouldClose(window) == 0) {
        oldTime = newTime;
    	newTime = getTime();
		if (floor(newTime) - floor(oldTime) >= 1.0)
			printf("main: %f frames/sec\n", 1.0 / (newTime - oldTime));
        render(oldTime, newTime);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

	/* Destroy */
    destroyShading();
	destroyScene();
	destroyMeshes();
	destroyTextures();
	destroyGLFW();
    return 0;
}
