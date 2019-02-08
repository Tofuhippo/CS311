/*
Dawson d'Almeida and Justin T. Washington
February 5 2018
CS311 with Josh Davis

Main abstracted file that defines the colorPixel and transformVertex functions
and demonstrates the rendering of a two meshes using camera isometry.
*/

/*
Warm-up for lighting:
1. Box mesh vertices: 24
	 We need four vertices for each side (6 * 4 = 24)
	 ^^ fix this, top and bottom each need 6 for proper s and t coordinates = 12
	 	tryna figure out how other 12 fit with the orientations of the sides still
2. XYZ and NOP are the same. These are the same because it is a sphere centered
   at the origin with radius 1, so the points are the same as the outward facing
	 normals.
3. If the radius were to be 7, then XYZ = 7 * NOP (individually)
*/


/* On macOS, compile with...
    clang 170mainEngine.c 000pixel.o 170engine.o -lglfw -framework OpenGL
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
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
#define mainVARYS 4
#define mainVARYT 5
#define mainUNIFR 0
#define mainUNIFG 1
#define mainUNIFB 2
#define mainUNIFCAMERA 3
#define mainTEXR 0
#define mainTEXG 1
#define mainTEXB 2

void colorPixel(int unifDim, const double unif[], int texNum,
		const texTexture *tex[], int varyDim, const double vary[],
		double rgbd[4]) {
	// Only apply perspective corrected interpolation on s, t
	// This fixed the black dots and artifacts
	double s = vary[mainVARYS] / vary[mainVARYW];
	double t = vary[mainVARYT] / vary[mainVARYW];
	double sample[tex[0]->texelDim];
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
	//mat441Multiply((double(*)[4])(&unif[mainUNIFMODELING]), attrHom, worldHom);
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
double cameraRho = 10.0, cameraPhi = M_PI / 4.0, cameraTheta = 0.0;
/* Meshes to be rendered. */
texTexture texture;
const texTexture *textures[1] = {&texture};
const texTexture **tex = textures;
meshMesh box;
double unifBox[3 + 16] = {
	1.0, 1.0, 1.0,
	1.0, 0.0, 0.0, 0.0,
	0.0, 1.0, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.0, 0.0, 0.0, 1.0};
meshMesh sphere;
double unifSphere[3 + 16] = {
	1.0, 1.0, 1.0,
	1.0, 0.0, 0.0, 0.0,
	0.0, 1.0, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.0, 0.0, 0.0, 1.0};

/*** User interface ***/

void render(void) {
	double view[4][4], projInvIsom[4][4];
	camGetProjectionInverseIsometry(&cam, projInvIsom);
	mat44Viewport(mainSCREENSIZE, mainSCREENSIZE, view);
	pixClearRGB(0.0, 0.0, 0.0);
	depthClearDepths(&buf, 1000000000.0);
	vecCopy(16, (double *)projInvIsom, &unifBox[mainUNIFCAMERA]);
	meshRender(&box, &buf, view, &sha, unifBox, tex);
	vecCopy(16, (double *)projInvIsom, &unifSphere[mainUNIFCAMERA]);
	meshRender(&sphere, &buf, view, &sha, unifSphere, tex);
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
	/* Begin configuring scene. */
	if (pixInitialize(mainSCREENSIZE, mainSCREENSIZE, "Pixel Graphics") != 0)
		return 1;
	else if (depthInitialize(&buf, mainSCREENSIZE, mainSCREENSIZE) != 0)
		return 2;
	else if (texInitializeFile(&texture, "nathan_mannes.jpg") != 0)
		return 3;
	else if (meshInitializeBox(&box, -2.0, 2.0, -2.0, 2.0, -2.0, -1.0) != 0)
		return 4;
	else if (meshInitializeSphere(&sphere, 1.0, 16, 32) != 0)
		return 5;
	else {
		/* Continue configuring scene. */
		sha.unifDim = 3 + 16;
		sha.attrDim = 3 + 2 + 3;
		sha.varyDim = 3 + 1 + 2;
		sha.colorPixel = colorPixel;
		sha.transformVertex = transformVertex;
		sha.texNum = 1;
		texSetFiltering(&texture, texNEAREST);
		texSetLeftRight(&texture, texREPEAT);
		texSetTopBottom(&texture, texREPEAT);
		camSetProjectionType(&cam, camORTHOGRAPHIC);
		camSetFrustum(&cam, M_PI / 6.0, cameraRho, 10.0, mainSCREENSIZE,
			mainSCREENSIZE);
		camLookAt(&cam, cameraTarget, cameraRho, cameraPhi, cameraTheta);
		/* User interface. */
		pixSetKeyDownHandler(handleKeyAny);
		pixSetKeyRepeatHandler(handleKeyAny);
		pixSetKeyUpHandler(handleKeyAny);
		pixSetTimeStepHandler(handleTimeStep);
		pixRun();
		/* Clean up. */
		meshSaveFile(&box, "box.txt");
		meshSaveFile(&sphere, "sphere.txt");
		meshDestroy(&box);
		meshDestroy(&sphere);
		texDestroy(&texture);
		depthDestroy(&buf);
		return 0;
	}
}
