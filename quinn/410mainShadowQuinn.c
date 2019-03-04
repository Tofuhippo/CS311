/* Quinn Mayville */

/* Renders a simple scene with a spot light. Includes more prep for shadow mapping.
Sets up shadow map behind the scenes but does not use it in final rendering pass.
Camera and water level can be manipulated via keyboard in the same way as previous assignments.
The positional light's x,y corrdinates can be changed with FTGH keys. */


/* On macOS, compile with...
    clang 410mainShadowQuinn.c /usr/local/gl3w/src/gl3w.o -lglfw -framework OpenGL -framework CoreFoundation -Wno-deprecated
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
#include "410meshglQuinn.c"
#include "360texture.c"
#include "370body.c"
#include "140landscape.c"
#include "400shadow.c"

#define BUFFER_OFFSET(bytes) ((GLubyte*) NULL + (bytes))

double getTime(void) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (double)tv.tv_sec + (double)tv.tv_usec * 0.000001;
}

void handleError(int error, const char *description) {
	fprintf(stderr, "handleError: %d\n%s\n", error, description);
}


/* Configure starting screen width and height */
double screenW = 768;
double screenH = 512;

/* Configure uniform and attribute names for base shader */
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
#define UNIFCOSSPOT 11
#define UNIFVIEWINGS 12
#define UNIFNUM 13

#define ATTRPOSITION 0
#define ATTRTEXCOORDS 1
#define ATTRNORMAL 2
#define ATTRNUM 3

const GLchar *uniformNames[UNIFNUM] = {
	"viewing", "modeling", "dLight", "cLight", "cAmbient", "pCamera",
	"texture0", "shininess", "pSpot", "cSpot", "dSpot", "cosHalfAngleSpot",
	"viewingS",
};
const GLchar **unifNames = uniformNames;
const GLchar *attributeNames[ATTRNUM] = {"position", "texCoords", "normal"};
const GLchar **attrNames = attributeNames;

/* Configure uniform and attribute names for shadow mapping shader */
#define UNIFSHADOWVIEWING 0
#define UNIFSHADOWMODELING 1
#define UNIFSHADOWRGB 2
#define UNIFSHADOWNUM 3

#define ATTRSHADOWPOSITION 0
#define ATTRSHADOWNUM 1

const GLchar *uniformShadowNames[UNIFSHADOWNUM] = {
	"viewing", "modeling", "rgb",
};
const GLchar **unifShadowNames = uniformShadowNames;
const GLchar *attributeShadowNames[ATTRSHADOWNUM] = {"position"};
const GLchar **attrShadowNames = attributeShadowNames;


/* Initializes shader programs. Returns 0 on success, non-zero on failure. */
shaShading sha;
shaShading shaShadow;
int initializeShaderPrograms(void) {
	/* Regular shader program */
	GLchar vertexCode[] = "\
		#version 140\n\
		uniform mat4 viewing;\
		uniform mat4 modeling;\
		uniform mat4 viewingS;\
		in vec3 position;\
		in vec2 texCoords;\
		in vec3 normal;\
		out vec2 st;\
		out vec3 nop;\
		out vec3 pFragment;\
		out vec4 pFragmentS;\
		void main() {\
			mat4 scaleBias = mat4(\
				0.5, 0.0, 0.0, 0.0,\
				0.0, 0.5, 0.0, 0.0,\
				0.0, 0.0, 0.5, 0.0,\
				0.5, 0.5, 0.5, 1.0);\
			vec4 world = modeling * vec4(position, 1.0);\
			pFragment = vec3(world);\
			gl_Position = viewing * world;\
			pFragmentS = scaleBias * viewingS * world;\
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
		uniform float cosHalfAngleSpot;\
		in vec3 nop;\
		in vec2 st;\
		in vec3 pFragment;\
		in vec4 pFragmentS;\
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
			if (dot(dLightSpot, dSpot) < cosHalfAngleSpot) {\
				diffuseSpot = vec3(0.0, 0.0, 0.0);\
				specularSpot = vec3(0.0, 0.0, 0.0);\
			}\
			\
			vec3 color = diffuseLight + specularLight + diffuseSpot + specularSpot + ambient;\
			fragColor = vec4(color, 1.0);\
		}";

	shaInitialize(&sha, vertexCode, fragmentCode, UNIFNUM, unifNames, ATTRNUM, attrNames);

	/* Shadow mapping shader program */
	GLchar vertexCodeShadow[] = "\
		#version 140\n\
		uniform mat4 viewing;\
		uniform mat4 modeling;\
		in vec3 position;\
		void main() {\
			vec4 world = modeling * vec4(position, 1.0);\
			gl_Position = viewing * world;\
		}";
	GLchar fragmentCodeShadow[] = "\
		#version 140\n\
		uniform vec3 rgb;\
		out vec4 fragColor;\
		void main() {\
			fragColor = vec4(rgb, 1.0);\
		}";

	shaInitialize(&shaShadow, vertexCodeShadow, fragmentCodeShadow,
		UNIFSHADOWNUM, unifShadowNames, ATTRSHADOWNUM, attrShadowNames
	);

	/* Switch to use regular program for set-up */
	glUseProgram(sha.program);

	return (sha.program == 0 || shaShadow.program == 0);
}


/* Destroys shading program */
void destroyShadingPrograms(void) {
	shaDestroy(&sha);
	shaDestroy(&shaShadow);
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
	/* Get attribute locations for main shader. (same for all meshes) */
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
	/* Initialize for shadow mapping shader */
	meshglContinueInitialization(mesh);

	/* Get attribute locations for shadow mapping shader. (same for all meshes) */
	positionLoc = shaShadow.attrLocs[ATTRSHADOWPOSITION];
	/* Set position to XYZ */
	glEnableVertexAttribArray(positionLoc);
	glVertexAttribPointer(positionLoc, 3, GL_DOUBLE, GL_FALSE,
		mesh->attrDim * sizeof(GLdouble), BUFFER_OFFSET(0));
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


GLuint shadowW = 512;
GLuint shadowH = 512;
shadowMap map;
/* Initializes shadow map. Returns 0 on success, non-zero on failure. */
int initializeShadowMap(void) {
	return shadowInitialize(&map, shadowW, shadowH);
}


/* Destroys shadow map */
void destroyShadowMap(void) {
	shadowDestroy(&map);
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
	if (texInitializeFile(&texBush, "epcotTex.jpg", GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT) != 0)
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
camCamera camSpot;
double cameraRho = 256.0;
double cameraPhi = M_PI / 4.0;
double cameraTheta = 0.0;
double pSpot[3] = {85.0, 80.0, 5.0};  // Starting position of spot
double camSpotTheta =  M_PI / 8.0;
double cosSpot;
/* Sets the positions of the cameras and moves the spot light with the
corresponding camera. */
void updateCamerasPositions(void) {
	/* Set main camera position */
	camSetFrustum(&cam, M_PI / 6.0, cameraRho, 10.0, screenW, screenH);
	camLookAt(&cam, camTarget, cameraRho, cameraPhi, cameraTheta);

	/* Set spot light camera position */
	camSetFrustum(&camSpot, camSpotTheta, 256.0, 10.0, shadowW, shadowH);
	camLookAt(&camSpot, pSpot, 0.0, 1.47, 0.0);

	GLdouble camPosition[3];
	/* Pass main camera position to uniforms */
	vecCopy(3, (cam.isometry).translation, camPosition);
	uniformVector3(camPosition, sha.unifLocs[UNIFPCAMERA]);

	/* Set spot light position in uniforms based on camera position */
	vecCopy(3, (camSpot.isometry).translation, camPosition);
	uniformVector3(camPosition, sha.unifLocs[UNIFPSPOT]);

	/* Set spot angle in uniforms based on camera frustum angle */
	glUniform1f(sha.unifLocs[UNIFCOSSPOT], cosSpot);

	/* Set spot direction in uniform based on camera direction */
	double camEyeCoords[3] = {0.0, 0.0, 1.0};
	double camDirection[3];
	isoRotateVector(&(camSpot.isometry), camEyeCoords, camDirection);
	vecUnit(3, camDirection, camDirection);
	uniformVector3(camDirection, sha.unifLocs[UNIFDSPOT]);
}


bodyBody bodyGrass;
bodyBody bodyRock;
bodyBody bodyWater;
bodyBody bodyBush0;
bodyBody bodyBush1;
/* Initializes the lights, camera, and bodies in the scene */
void initializeScene(void) {
	/* Initialize the cameras */
	camSetProjectionType(&cam, camPERSPECTIVE);
	camSetProjectionType(&camSpot, camPERSPECTIVE);

	/* Compute angle for spotlight (fixed) */
	cosSpot = cos(camSpotTheta/2.0);

	/* Set cameras' (and spot light )positions */
	updateCamerasPositions();

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

	/* Set spot color */
	GLdouble cSpot[3] = {1.0, 1.0, 0.5};
	uniformVector3(cSpot, sha.unifLocs[UNIFCSPOT]);

	/* Set ambient light */
	GLdouble cAmbient[3] = {0.1, 0.1, 0.1};
	uniformVector3(cAmbient, sha.unifLocs[UNIFCAMBIENT]);

}


/* Destroys all bodies in the scene */
void destroyScene(void) {
	bodyDestroy(&bodyGrass);
	bodyDestroy(&bodyRock);
	bodyDestroy(&bodyWater);
	bodyDestroy(&bodyBush0);
	bodyDestroy(&bodyBush1);
}


/* Sends the body's isometry to the regular shader program and renders the body.
Assumes bodies have one auxiliary for shininess and one texture. */
void renderBodyRegular(bodyBody *body) {
	/* Compute modeling isometry */
	GLdouble model[4][4];
	isoGetHomogeneous(&(body->isometry), model);
	uniformMatrix44(model, sha.unifLocs[UNIFMODELING]);

	/* Load modeling, shininess and texture */
	uniformMatrix44(model, sha.unifLocs[UNIFMODELING]);
	glUniform1f(sha.unifLocs[UNIFSHININESS], body->aux[0]);
	texRender(body->tex[0], GL_TEXTURE0, 0, sha.unifLocs[UNIFTEX0]);

	/* Render */
	meshglRender(body->mesh, 0);
	texUnrender(body->tex[0], 0);
}


/* Sends the body's isometry to the shadow mapping shader program and renders the body. */
void renderBodyShadow(bodyBody *body) {
	/* Compute modeling isometry */
	GLdouble model[4][4];
	isoGetHomogeneous(&(body->isometry), model);
	uniformMatrix44(model, sha.unifLocs[UNIFMODELING]);
	uniformMatrix44(model, shaShadow.unifLocs[UNIFMODELING]);

	/* Render */
	meshglRender(body->mesh, 1);
}


double angle = 0.0;
double waterLevel = 3.0;
/* Renders the scene regularly. */
void renderRegularly(double oldTime, double newTime) {
	/* Clear buffers and use shader program */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(sha.program);

	/* Load spot light's  projection inverse isometry into shader */
	double spotPCInv[4][4];
	camGetProjectionInverseIsometry(&camSpot, spotPCInv);
	uniformMatrix44(spotPCInv, sha.unifLocs[UNIFVIEWINGS]);

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
	renderBodyRegular(&bodyGrass);
	renderBodyRegular(&bodyRock);
	renderBodyRegular(&bodyWater);
	renderBodyRegular(&bodyBush0);
	renderBodyRegular(&bodyBush1);
}


/* Set bodies uniform colors */
double colorGrass[3] = {0.0, 1.0, 0.0};
double colorRock[3] = {1.0, 0.0, 0.0};
double colorWater[3] = {0.0, 0.0, 1.0};
double colorBush0[3] = {0.0, 1.0, 1.0};
double colorBush1[3] = {1.0, 1.0, 0.0};

/* Renders the scene from the spot light camera for shadow mapping */
void renderShadowly(double oldTime, double newTime) {
	/* Clear buffers and use shader program */
	glClear(GL_DEPTH_BUFFER_BIT);
	glUseProgram(shaShadow.program);

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
	camGetProjectionInverseIsometry(&camSpot, viewing);
	uniformMatrix44(viewing, shaShadow.unifLocs[UNIFVIEWING]);

	/* Render bodies */
	uniformVector3(colorGrass, shaShadow.unifLocs[UNIFSHADOWRGB]);
	renderBodyShadow(&bodyGrass);

	uniformVector3(colorRock, shaShadow.unifLocs[UNIFSHADOWRGB]);
	renderBodyShadow(&bodyRock);

	uniformVector3(colorWater, shaShadow.unifLocs[UNIFSHADOWRGB]);
	renderBodyShadow(&bodyWater);

	uniformVector3(colorBush0, shaShadow.unifLocs[UNIFSHADOWRGB]);
	renderBodyShadow(&bodyBush0);

	uniformVector3(colorBush1, shaShadow.unifLocs[UNIFSHADOWRGB]);
	renderBodyShadow(&bodyBush1);
}


/* Renders the scene */
void render(double oldTime, double newTime) {
	shadowRenderFirst(&map);
	renderShadowly(oldTime, newTime);
	shadowUnrenderFirst(&map);
	renderRegularly(oldTime, newTime);
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

	/* Update cameras' positions based on changes */
	updateCamerasPositions();
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
	window = glfwCreateWindow(screenW, screenH, "Making a Scene with Shadows", NULL, NULL);
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
	if (initializeShaderPrograms() != 0)
		return 5;
	if (initializeMeshes() != 0)
		return 6;
	if (initializeShadowMap() != 0)
		return 7;
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
    destroyShadingPrograms();
	destroyShadowMap();
	destroyScene();
	destroyMeshes();
	destroyTextures();
	destroyGLFW();
    return 0;
}
