/*
Dawson d'Almeida and Justin T. Washington
January 23 2018
CS311 with Josh Davis

Main abstracted file that uses two different shaders, textures and meshes to
demonstrate the abstraction of shaders, meshes and textures.
*/


/* On macOS, compile with...
    clang 090mainAbstracted.c 000pixel.o -lglfw -framework OpenGL
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GLFW/glfw3.h>

#include "000pixel.h"
#include "030vector.c"
#include "100matrix.c"
#include "040texture.c"
#include "090shading.c"
/* New. We no longer need to include these files after colorPixel and
transformVertex. So instead we include them up here. It's good C style to have
all #includes in one section near the top of the file. */
#include "090triangle.c"
#include "090mesh.c"

#define mainATTRX 0
#define mainATTRY 1
#define mainATTRS 2
#define mainATTRT 3
#define mainVARYX 0
#define mainVARYY 1
#define mainVARYS 2
#define mainVARYT 3
#define mainUNIFR 0
#define mainUNIFG 1
#define mainUNIFB 2
#define mainUNIFMODELING 3
#define mainTEXR 0
#define mainTEXG 1
#define mainTEXB 2

void colorPixel0(int unifDim, const double unif[], int texNum,
		const texTexture *tex[], int varyDim, const double vary[],
		double rgb[3]) {
	double sample[tex[0]->texelDim];
	texSample(tex[0], vary[mainVARYS], vary[mainVARYT], sample);
	rgb[0] = sample[mainTEXR] * unif[mainUNIFR];
	rgb[1] = sample[mainTEXG] * unif[mainUNIFG];
	rgb[2] = sample[mainTEXB] * unif[mainUNIFB];
}

void colorPixel1(int unifDim, const double unif[], int texNum,
		const texTexture *tex[], int varyDim, const double vary[],
		double rgb[3]) {
	double sample[tex[0]->texelDim];
	texSample(tex[0], vary[mainVARYS], vary[mainVARYT], sample);
	rgb[0] = sample[mainTEXR] * unif[mainUNIFR];
	rgb[1] = sample[mainTEXG] * unif[mainUNIFG];
	rgb[2] = sample[mainTEXB] * unif[mainUNIFB];
}

void transformVertex0(int unifDim, const double unif[], int attrDim,
		const double attr[], int varyDim, double vary[]) {
			double attrHomog[3] = {attr[mainATTRX], attr[mainATTRY], 1};
			double varyHomog[3];
			mat331Multiply((double(*)[3])(&unif[mainUNIFMODELING]), attrHomog, varyHomog);
			vary[mainVARYX] = varyHomog[0];
			vary[mainVARYY] = varyHomog[1];
			vary[mainVARYS] = attr[mainATTRS];
			vary[mainVARYT] = attr[mainATTRT];
}

void transformVertex1(int unifDim, const double unif[], int attrDim,
		const double attr[], int varyDim, double vary[]) {
			double attrHomog[3] = {attr[mainATTRX], attr[mainATTRY], 1};
			double varyHomog[3];
			mat331Multiply((double(*)[3])(&unif[mainUNIFMODELING]), attrHomog, varyHomog);
			vary[mainVARYX] = varyHomog[0];
			vary[mainVARYY] = varyHomog[1];
			vary[mainVARYS] = attr[mainATTRS];
			vary[mainVARYT] = attr[mainATTRT];
}

shaShading sha0, sha1;
texTexture texture0, texture1;
const texTexture *textures0[1] = {&texture0};
const texTexture *textures1[1] = {&texture1};
const texTexture **tex0 = textures0;
const texTexture **tex1 = textures1;
meshMesh mesh0, mesh1;
double unif0[3 + 3 * 3] = {1.0, 1.0, 1.0,
													 0.0, 0.0, 0.0,
													 0.0, 0.0, 0.0,
								  				 0.0, 0.0, 0.0};
double unif1[3 + 3 * 3] = {1.0, 1.0, 1.0,
													 0.0, 0.0, 0.0,
													 0.0, 0.0, 0.0,
								  				 0.0, 0.0, 0.0};
double rotationAngle0, rotationAngle1;
double translationVector0[2] = {0.0, 0.0};
double translationVector1[2] = {200.0, 200.0};

void draw(void) {
	pixClearRGB(0.0, 0.0, 0.0);
	meshRender(&mesh0, &sha0, unif0, tex0);
	meshRender(&mesh1, &sha1, unif1, tex1);
}

void handleKeyUp(int key, int shiftIsDown, int controlIsDown,
		int altOptionIsDown, int superCommandIsDown) {
	if (key == GLFW_KEY_ENTER) {
		if (texture0.filtering == texLINEAR) {
			texSetFiltering(&texture0, texNEAREST);
			texSetFiltering(&texture1, texNEAREST);
		}
		else {
			texSetFiltering(&texture0, texLINEAR);
			texSetFiltering(&texture1, texLINEAR);
		}
		draw();
	}
}

void handleTimeStep(double oldTime, double newTime) {
	if (floor(newTime) - floor(oldTime) >= 1.0)
		printf("handleTimeStep: %f frames/sec\n", 1.0 / (newTime - oldTime));
	//unif[mainUNIFR] = sin(newTime);
	//unif[mainUNIFG] = cos(oldTime);
	//rotationAngle0 += (newTime - oldTime);
	rotationAngle1 -= (newTime - oldTime);
	translationVector0[0] += sin(newTime);// - oldTime);
	translationVector0[1] += cos(newTime);// - oldTime);
	//translationVector1[0] -= sin(newTime);// - oldTime);
	//translationVector1[1] -= cos(newTime);// - oldTime);

	double isom0[3][3], isom1[3][3];
	mat33Isometry(rotationAngle0, translationVector0, isom0);
	mat33Isometry(rotationAngle1, translationVector1, isom1);
  vecCopy(9, (double *)isom0, &unif0[mainUNIFMODELING]);
	vecCopy(9, (double *)isom1, &unif1[mainUNIFMODELING]);

	draw();
}

int main(void) {
	if (pixInitialize(512, 512, "Pixel Graphics") != 0)
		return 1;
	else if (texInitializeFile(&texture0, "nathan_mannes.jpg") != 0)
		return 2;
	else if (texInitializeFile(&texture1, "shrek.png") != 0)
		return 2;
	//else if (meshInitializeRectangle(&mesh, 0.0, 512.0, 0.0, 512.0) != 0)
	else if (meshInitializeEllipse(&mesh0, 256.0, 256.0, 256.0, 128.0, 40) != 0)
		return 3;
	else if (meshInitializeEllipse(&mesh1, 256.0, 256.0, 256.0, 128.0, 40) != 0)
		return 3;
	else {
		texSetFiltering(&texture0, texNEAREST);
		texSetLeftRight(&texture0, texREPEAT);
		texSetTopBottom(&texture0, texREPEAT);
		texSetFiltering(&texture1, texNEAREST);
		texSetLeftRight(&texture1, texREPEAT);
		texSetTopBottom(&texture1, texREPEAT);
		sha0.unifDim = 3 + 3;
		sha0.attrDim = 2 + 2;
		sha0.varyDim = 2 + 2;
		sha0.texNum = 1;
		sha0.colorPixel = colorPixel0;
		sha0.transformVertex = transformVertex0;

		sha1.unifDim = 3 + 3;
		sha1.attrDim = 2 + 2;
		sha1.varyDim = 2 + 2;
		sha1.texNum = 1;
		sha1.colorPixel = colorPixel1;
		sha1.transformVertex = transformVertex1;

		draw();
		pixSetKeyUpHandler(handleKeyUp);
		pixSetTimeStepHandler(handleTimeStep);
		pixRun();
		meshDestroy(&mesh0);
		texDestroy(&texture0);
		meshDestroy(&mesh1);
		texDestroy(&texture1);
		return 0;
	}
}
