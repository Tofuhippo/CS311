


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

void colorPixel(int unifDim, const double unif[], int texNum,
		const texTexture *tex[], int varyDim, const double vary[],
		double rgb[3]) {
	double sample[tex[0]->texelDim];
	texSample(tex[0], vary[mainVARYS], vary[mainVARYT], sample);
	rgb[0] = sample[mainTEXR] * unif[mainUNIFR];
	rgb[1] = sample[mainTEXG] * unif[mainUNIFG];
	rgb[2] = sample[mainTEXB] * unif[mainUNIFB];
}

void transformVertex(int unifDim, const double unif[], int attrDim,
		const double attr[], int varyDim, double vary[]) {
			double attrHomog[3] = {attr[mainATTRX], attr[mainATTRY], 1};
			double varyHomog[3];
			mat331Multiply((double(*)[3])(&unif[mainUNIFMODELING]), attrHomog, varyHomog);
			vary[mainVARYX] = varyHomog[0];
			vary[mainVARYY] = varyHomog[1];
			vary[mainVARYS] = attr[mainATTRS];
			vary[mainVARYT] = attr[mainATTRT];
}

shaShading sha;
texTexture texture;
const texTexture *textures[1] = {&texture};
const texTexture **tex = textures;
meshMesh mesh;
double unif[3 + 3 * 3] = {1.0, 1.0, 1.0,
													0.0, 0.0, 0.0,
													0.0, 0.0, 0.0,
								  				0.0, 0.0, 0.0};
double rotationAngle;
double translationVector[2] = {0.0, 0.0};

void draw(void) {
	pixClearRGB(0.0, 0.0, 0.0);
	meshRender(&mesh, &sha, unif, tex);
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
	unif[mainUNIFR] = sin(newTime);
	unif[mainUNIFG] = cos(oldTime);
	rotationAngle += (newTime - oldTime);

	double isom[3][3];
	mat33Isometry(rotationAngle, translationVector, isom);
  vecCopy(9, (double *)isom, &unif[mainUNIFMODELING]);

	draw();
}

int main(void) {
	if (pixInitialize(512, 512, "Pixel Graphics") != 0)
		return 1;
	else if (texInitializeFile(&texture, "nathan_mannes.jpg") != 0)
		return 2;
	//else if (meshInitializeRectangle(&mesh, 0.0, 512.0, 0.0, 512.0) != 0)
	else if (meshInitializeEllipse(&mesh, 256.0, 256.0, 256.0, 128.0, 40) != 0)
		return 3;
	else {
		texSetFiltering(&texture, texNEAREST);
		texSetLeftRight(&texture, texREPEAT);
		texSetTopBottom(&texture, texREPEAT);
		sha.unifDim = 3 + 3;
		sha.attrDim = 2 + 2;
		sha.varyDim = 2 + 2;
		sha.texNum = 1;
		/* New. This is how the engine knows which shaders to use. */
		sha.colorPixel = colorPixel;
		sha.transformVertex = transformVertex;
		draw();
		pixSetKeyUpHandler(handleKeyUp);
		pixSetTimeStepHandler(handleTimeStep);
		pixRun();
		meshDestroy(&mesh);
		texDestroy(&texture);
		return 0;
	}
}
