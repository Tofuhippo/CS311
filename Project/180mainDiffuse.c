/*
Dawson d'Almeida and Justin T. Washington
February 5 2018
CS311 with Josh Davis

Main abstracted file that defines the colorPixel and transformVertex functions
and demonstrates the rendering of a landscape.
*/


/* On macOS, compile with...
    clang 180mainDiffuse.c 000pixel.o 170engine.o -lglfw -framework OpenGL
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <GLFW/glfw3.h>

#include "000pixel.h"
#include "170engine.h"

#define mainSCREENSIZE 512

/*** Shaders ***/

#define mainATTRX 0
#define mainATTRY 1
#define mainATTRZ 2
#define mainATTRS 3
#define mainATTRT 4
#define mainATTRN 5
#define mainATTRO 6
#define mainATTRP 7
#define mainVARYX 0
#define mainVARYY 1
#define mainVARYZ 2
#define mainVARYW 3 // w out of transformVertex, 1/w into colorPixel
#define mainVARYWORLDZ 4
#define mainVARYS 4
#define mainVARYT 5
#define mainUNIFR 0
#define mainUNIFG 1
#define mainUNIFB 2
#define mainUNIFMODELING 3
#define mainUNIFMIN 4
#define mainUNIFMEAN 5
#define mainUNIFMAX 6
#define mainUNIFCAMERA 7
#define mainTEXR 0
#define mainTEXG 1
#define mainTEXB 2

/* Solid colors, tinted from dark (low saturation at low elevation) to light
(high saturation at high elevation). */
void colorPixel(int unifDim, const double unif[], int texNum,
	const texTexture *tex[], int varyDim, const double vary[],
	double rgbd[4]) {
		double sample[tex[0]->texelDim];
		double s = vary[mainVARYS] / vary[mainVARYW];
		double t = vary[mainVARYT] / vary[mainVARYW];
		texSample(tex[0], s, t, sample);
		rgbd[0] = sample[mainTEXR] * unif[mainUNIFR];
		rgbd[1] = sample[mainTEXG] * unif[mainUNIFG];
		rgbd[2] = sample[mainTEXB] * unif[mainUNIFB];
		rgbd[3] = vary[mainVARYZ];
}

void transformVertex(int unifDim, const double unif[], int attrDim,
	const double attr[], int varyDim, double vary[]) {
		double attrHom[4] = {attr[mainATTRX], attr[mainATTRY], attr[mainATTRZ], 1.0};
		double worldHom[4], varyHom[4];
		vecCopy(4, attrHom, worldHom);
		worldHom[2] += unif[mainUNIFMODELING];
		mat441Multiply((double(*)[4])(&unif[mainUNIFCAMERA]), worldHom, varyHom);
		vecCopy(4, varyHom, vary);
		vary[mainVARYS] = attr[mainATTRS];
		vary[mainVARYT] = attr[mainATTRT];
}

/*** Globals ***/

/* Crucial infrastructure. */
depthBuffer buf;
shaShading sha;
camCamera cam;
/* Camera control. */
double cameraTarget[3] = {0.0, 0.0, 0.0};
double cameraRho = 256.0, cameraPhi = M_PI / 4.0, cameraTheta = 0.0;
/* Textures */
texTexture grassTex;
texTexture waterTex;
texTexture rockTex;
const texTexture *texturesGrass[1] = {&grassTex};
const texTexture *texturesWater[1] = {&waterTex};
const texTexture *texturesRock[1] = {&rockTex};
const texTexture **texGrass = texturesGrass;
const texTexture **texWater = texturesWater;
const texTexture **texRock = texturesRock;
/* Meshes to be rendered. */
meshMesh grass;
double unifGrass[3 + 1 + 3 + 16 + 1] = {
	0.0, 1.0, 0.0,
	0.0,
	0.0, 0.0, 0.0,
	1.0, 0.0, 0.0, 0.0,
	0.0, 1.0, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.0, 0.0, 0.0, 1.0};
meshMesh rock;
double unifRock[3 + 1 + 3 + 16 + 1] = {
	1.0, 1.0, 1.0,
	0.0,
	0.0, 0.0, 0.0,
	1.0, 0.0, 0.0, 0.0,
	0.0, 1.0, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.0, 0.0, 0.0, 1.0};
meshMesh water;
double unifWater[3 + 1 + 3 + 16 + 1] = {
	0.0, 0.0, 1.0,
	0.0,
	0.0, 0.0, 0.0,
	1.0, 0.0, 0.0, 0.0,
	0.0, 1.0, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.0, 0.0, 0.0, 1.0};

/*** User interface ***/

void render(void) {
	double view[4][4], projInvIsom[4][4];//, viewProjInvIsom[4][4];
	camGetProjectionInverseIsometry(&cam, projInvIsom);
	mat44Viewport(mainSCREENSIZE, mainSCREENSIZE, view);
	//mat444Multiply(view, projInvIsom, viewProjInvIsom);
	pixClearRGB(0.0, 0.0, 0.0);
	depthClearDepths(&buf, 1000000000.0);
	// Copy cam projection inverse isometry to unifs, pass viewport to meshRender
	vecCopy(16, (double *)projInvIsom, &unifGrass[mainUNIFCAMERA]);
	meshRender(&grass, &buf, view, &sha, unifGrass, texGrass);
	vecCopy(16, (double *)projInvIsom, &unifRock[mainUNIFCAMERA]);
	meshRender(&rock, &buf, view, &sha, unifRock, texRock);
	vecCopy(16, (double *)projInvIsom, &unifWater[mainUNIFCAMERA]);
	meshRender(&water, &buf, view, &sha, unifWater, texWater);
}

void handleKeyAny(int key, int shiftIsDown, int controlIsDown,
		int altOptionIsDown, int superCommandIsDown) {
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
		cameraTarget[0] -= 0.5;
	else if (key == GLFW_KEY_SEMICOLON)
		cameraTarget[0] += 0.5;
	else if (key == GLFW_KEY_L)
		cameraTarget[1] -= 0.5;
	else if (key == GLFW_KEY_O)
		cameraTarget[1] += 0.5;
	else if (key == GLFW_KEY_I)
		cameraTarget[2] -= 0.5;
	else if (key == GLFW_KEY_P)
		cameraTarget[2] += 0.5;
	else if (key == GLFW_KEY_J)
		unifWater[mainUNIFMODELING] -= 0.1;
	else if (key == GLFW_KEY_U)
		unifWater[mainUNIFMODELING] += 0.1;
	else if (key == GLFW_KEY_1)
		camSetProjectionType(&cam, 1);
	else if (key == GLFW_KEY_2)
		camSetProjectionType(&cam, 0);
	camSetFrustum(&cam, M_PI / 6.0, cameraRho, 10.0, mainSCREENSIZE,
		mainSCREENSIZE);
	camLookAt(&cam, cameraTarget, cameraRho, cameraPhi, cameraTheta);
}

void handleTimeStep(double oldTime, double newTime) {
	if (floor(newTime) - floor(oldTime) >= 1.0)
		printf("handleTimeStep: %f frames/sec\n", 1.0 / (newTime - oldTime));
	render();
}

int main(void) {
	/* Design landscape and water. */
	int landNum = 100;
	double landData[landNum][landNum];
	double landMin, landMean, landMax;
	time_t t;
	int i;
	srand((unsigned)time(&t));
	landFlat(landNum, landNum, (double *)landData, 0.0);
	for (i = 0; i < 32; i += 1)
		landFault(landNum, landNum, (double *)landData, 1.5 - i * 0.04);
	for (i = 0; i < 4; i += 1)
		landBlur(landNum, landNum, (double *)landData);
	landStatistics(landNum, landNum, (double *)landData, &landMin, &landMean,
		&landMax);
	double waterData[4] = {landMin, landMin, landMin, landMin};
	unifGrass[mainUNIFMIN] = landMin;
	unifGrass[mainUNIFMEAN] = landMean;
	unifGrass[mainUNIFMAX] = landMax;
	unifRock[mainUNIFMIN] = landMin;
	unifRock[mainUNIFMEAN] = landMean;
	unifRock[mainUNIFMAX] = landMax;
	unifWater[mainUNIFMIN] = landMin;
	unifWater[mainUNIFMEAN] = landMean;
	unifWater[mainUNIFMAX] = landMax;
	meshMesh land;
	/* Begin configuring scene. */
	if (pixInitialize(mainSCREENSIZE, mainSCREENSIZE, "Pixel Graphics") != 0)
		return 1;
	if (depthInitialize(&buf, mainSCREENSIZE, mainSCREENSIZE) != 0)
		return 2;
	if (meshInitializeLandscape(&land, landNum, landNum, 1.0,
			(double *)landData) != 0)
		return 3;
	if (meshInitializeDissectedLandscape(&grass, &land, M_PI / 4.0,
			1) != 0)
		return 4;
	if (meshInitializeDissectedLandscape(&rock, &land, M_PI / 4.0,
			0) != 0)
		return 5;
	if (meshInitializeLandscape(&water, 2, 2, landNum - 1.0,
			(double *)waterData) != 0)
		return 6;
	if (texInitializeFile(&grassTex, "grassTex.jpg") != 0)
		return 7;
	if (texInitializeFile(&waterTex, "waterTex.jpg") != 0)
	  return 8;
	if (texInitializeFile(&rockTex, "rockTex.jpeg") != 0)
		return 9;
	else {
		meshDestroy(&land);
		/* Continue configuring scene. */
		sha.unifDim = 3 + 1 + 3 + 16;
		sha.attrDim = 3 + 2 + 3;
		sha.varyDim = 4 + 1 + 2;
		sha.colorPixel = colorPixel;
		sha.transformVertex = transformVertex;
		sha.texNum = 1;
		texSetFiltering(&grassTex, texNEAREST);
		texSetLeftRight(&grassTex, texREPEAT);
		texSetTopBottom(&grassTex, texREPEAT);
		texSetFiltering(&waterTex, texNEAREST);
		texSetLeftRight(&waterTex, texREPEAT);
		texSetTopBottom(&waterTex, texREPEAT);
		texSetFiltering(&rockTex, texNEAREST);
		texSetLeftRight(&rockTex, texREPEAT);
		texSetTopBottom(&rockTex, texREPEAT);
		camSetProjectionType(&cam, camORTHOGRAPHIC);
		camSetFrustum(&cam, M_PI / 6.0, cameraRho, 10.0, mainSCREENSIZE,
			mainSCREENSIZE);
		vec3Set(landNum / 2.0, landNum / 2.0,
			landData[landNum / 2][landNum / 2], cameraTarget);
		camLookAt(&cam, cameraTarget, cameraRho, cameraPhi, cameraTheta);
		/* User interface. */
		pixSetKeyDownHandler(handleKeyAny);
		pixSetKeyRepeatHandler(handleKeyAny);
		pixSetKeyUpHandler(handleKeyAny);
		pixSetTimeStepHandler(handleTimeStep);
		pixRun();
		/* Clean up. */
		meshDestroy(&grass);
		meshDestroy(&rock);
		meshDestroy(&water);
		depthDestroy(&buf);
		texDestroy(&grassTex);
		texDestroy(&waterTex);
		texDestroy(&rockTex);
		return 0;
	}
}
