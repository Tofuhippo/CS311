/* Quinn Mayville */


/* On macOS, compile with...
    clang 141mainCamera.c 000pixel.o -lglfw -framework OpenGL
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GLFW/glfw3.h>

#include "000pixel.h"
#include "120vector.c"
#include "140matrix.c"
#include "040texture.c"
#include "130depth.c"
#include "130shading.c"
#include "130triangle.c"
#include "130mesh.c"
#include "140isometry.c"
#include "140camera.c"

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
#define mainVARYS 3
#define mainVARYT 4
#define mainUNIFR 0
#define mainUNIFG 1
#define mainUNIFB 2
#define mainUNIFMODELING 3
#define mainUNIFCAMERA 19
#define mainTEXR 0
#define mainTEXG 1
#define mainTEXB 2

void colorPixel(int unifDim, const double unif[], int texNum,
		const texTexture *tex[], int varyDim, const double vary[],
		double rgbd[4]) {
	double sample[tex[0]->texelDim];
	texSample(tex[0], vary[mainVARYS], vary[mainVARYT], sample);
	rgbd[0] = sample[mainTEXR] * unif[mainUNIFR];
	rgbd[1] = sample[mainTEXG] * unif[mainUNIFG];
	rgbd[2] = sample[mainTEXB] * unif[mainUNIFB];
	rgbd[3] = -vary[mainVARYZ];
}

void transformVertex(int unifDim, const double unif[], int attrDim,
		const double attr[], int varyDim, double vary[]) {
	double attrHomog[4] = {attr[0], attr[1], attr[2], 1.0};
	double worldHomog[4];
	mat441Multiply((double(*)[4])(&unif[mainUNIFMODELING]), attrHomog, worldHomog);
	mat441Multiply((double(*)[4])(&unif[mainUNIFCAMERA]), worldHomog, vary);
	vary[mainVARYS] = attr[mainATTRS];
	vary[mainVARYT] = attr[mainATTRT];
}

camCamera cam;
shaShading sha;
texTexture texture;
depthBuffer buf;
const texTexture *textures[1] = {&texture};
const texTexture **tex = textures;
meshMesh mesh;
meshMesh mesh1;
double unif[3 + 16 + 16] = {
	1.0, 1.0, 1.0,

	1.0, 0.0, 0.0, 0.0,
	0.0, 1.0, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.0, 0.0, 0.0, 1.0,

	1.0, 0.0, 0.0, 0.0,
	0.0, 1.0, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.0, 0.0, 0.0, 1.0,
};
double rotationAngle = 0.0;
double rotationAxis[3];
double translationVector[3] = {256.0, 256.0, 256.0};
double cameraAngle = 0.0;

void draw(void) {
	pixClearRGB(0.0, 0.0, 0.0);
	depthClearDepths(&buf, 1000000000);
	meshRender(&mesh1, &buf, &sha, unif, tex);
	meshRender(&mesh, &buf, &sha, unif, tex);
}

void handleKeyUp(int key, int shiftIsDown, int controlIsDown,
		int altOptionIsDown, int superCommandIsDown) {
	if (key == GLFW_KEY_ENTER) {
		if (texture.filtering == texLINEAR)
			texSetFiltering(&texture, texNEAREST);
		else
			texSetFiltering(&texture, texLINEAR);
		draw();
	}
}

void handleTimeStep(double oldTime, double newTime) {
	if (floor(newTime) - floor(oldTime) >= 1.0)
		printf("handleTimeStep: %f frames/sec\n", 1.0 / (newTime - oldTime));

	// Change unif color
	unif[mainUNIFR] = sin(newTime);
	unif[mainUNIFG] = cos(oldTime);

	// Rotate mesh
	rotationAngle += (newTime - oldTime);
	double rot[3][3], isom[4][4];
	vec3Set(1.0 / sqrt(3.0), 1.0 / sqrt(3.0), 1.0 / sqrt(3.0), rotationAxis);
	mat33AngleAxisRotation(rotationAngle, rotationAxis, rot);
	mat44Isometry(rot, translationVector, isom);
	vecCopy(16, (double *)isom, &unif[mainUNIFMODELING]);

	// Rotate camera
	double target[3] = {256, 256, 256};
	cameraAngle += 5 * (newTime - oldTime);
	camLookAt(&cam, target, 100, 3.14/2, cameraAngle);
	double invCamIsom[4][4];
	isoGetInverseHomogeneous(&(cam.isometry), invCamIsom);
	vecCopy(16, (double *)invCamIsom, &unif[mainUNIFCAMERA]);


	draw();
}

int main(void) {
	if (pixInitialize(512, 512, "Pixel Graphics") != 0)
		return 1;
	else if (texInitializeFile(&texture, "nathan_mannes.jpg") != 0)
		return 2;
	else if (meshInitializeBox(&mesh, -128.0, 128.0, -64.0, 64.0, -32.0, 32.0) != 0)
		return 3;
	else if (meshInitializeSphere(&mesh1, 64.0, 16, 32) != 0)
		return 4;
	else if (depthInitialize(&buf, 512, 512) != 0)
		return 5;
	else {
		{
			meshMesh meshB;
			printf("meshSaveFile %d\n", meshSaveFile(&mesh, "first.txt"));
			printf("meshInitializeFile %d\n", meshInitializeFile(&meshB, "first.txt"));
			printf("meshSaveFile %d\n", meshSaveFile(&meshB, "second.txt"));
		}
		texSetFiltering(&texture, texNEAREST);
		texSetLeftRight(&texture, texREPEAT);
		texSetTopBottom(&texture, texREPEAT);
		sha.unifDim = 3 + 16 + 16;
		sha.attrDim = 3 + 2 + 3;
		sha.varyDim = 3 + 2;
		sha.colorPixel = colorPixel;
		sha.transformVertex = transformVertex;
		sha.texNum = 1;

		draw();
		pixSetKeyUpHandler(handleKeyUp);
		pixSetTimeStepHandler(handleTimeStep);
		pixRun();
		meshDestroy(&mesh);
		meshDestroy(&mesh1);
		texDestroy(&texture);
		depthDestroy(&buf);
		return 0;
	}
}
