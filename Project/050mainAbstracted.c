/*
Dawson d'Almeida and Justin T. Washington
January 17 2018
CS311 with Josh Davis

Main abstracted file that defines color pixel.
*/


/* On macOS, compile with...
    clang 050mainAbstracted.c 000pixel.o -lglfw -framework OpenGL
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GLFW/glfw3.h>

#include "000pixel.h"
#include "030vector.c"
#include "030matrix.c"
#include "040texture.c"
#include "050shading.c"

/* It is important that colorPixel correctly parse the data that we give it. To
help avoid errors in communication, we set up some compile-time constants.
Notice that the documentation for the new triRender requires mainATTRX to be 0
and mainATTRY to be 1. */
#define mainATTRX 0
#define mainATTRY 1
#define mainATTRS 2
#define mainATTRT 3
#define mainATTRR 4
#define mainATTRG 5
#define mainATTRB 6
#define mainUNIFR 0
#define mainUNIFG 1
#define mainUNIFB 2
#define mainTEXR 0
#define mainTEXG 1
#define mainTEXB 2

/* attr has already been interpolated from the vertex attributes. tex is an
array of texNum elements, each of which is of type texTexture *. rgb is the
output parameter. The other parameters should be self-explanatory. For reasons
that become clear later in the course, colorPixel is allowed to use (A) compile-
time constants such as mainATTRX, (B) functions such as sin() and texSample(),
(C) its parameters such as unifDim, and (D) any variables that it declares
locally. It is not allowed to use any other variables, such as global variables
that aren't parameters. */
void colorPixel(int unifDim, const double unif[], int texNum,
		const texTexture *tex[], int attrDim, const double attr[],
		double rgb[3]) {
			double sample[3];
			texSample(tex[0], attr[mainATTRS], attr[mainATTRT], sample);
			rgb[0] = sample[mainTEXR] * attr[mainATTRR];
			rgb[1] = sample[mainTEXG] * attr[mainATTRG];
			rgb[2] = sample[mainTEXB] * attr[mainATTRB];
}

/* We have to include 050triangle.c after defining colorPixel, because it
refers to colorPixel. (Later in the course we handle this issue better.) */
#include "050triangle.c"

/* This struct is initialized in main() below. */
shaShading sha;
/* Here we make an array of one texTexture pointer, in such a way that the
const qualifier can be enforced throughout the surrounding code. C is confusing
for stuff like this. Don't worry about mastering C at this level. It doesn't
come up much in our course. */
texTexture texture;
const texTexture *textures[1] = {&texture};
const texTexture **tex = textures;

void draw(void) {
	pixClearRGB(0.0, 0.0, 0.0);
	double a[7] = {400.0, 100.0, 1.0, 1.0, 1.0, 0.0, 0.0};
	double b[7] = {500.0, 500.0, 0.0, 1.0, 0.0, 1.0, 0.0};
	double c[7] = {30.0, 30.0, 0.0, 0.0, 0.0, 0.0, 1.0};
	double unif[3] = {1.0, 1.0, 1.0};
	triRender(&sha, unif, tex, a, b, c);
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
}

int main(void) {
	if (pixInitialize(512, 512, "Pixel Graphics") != 0)
		return 1;
	else {
		if (texInitializeFile(&texture, "josh_davis_swimmer.jpg") != 0)
			return 2;
		else {
			texSetFiltering(&texture, texNEAREST);
			texSetLeftRight(&texture, texREPEAT);
			texSetTopBottom(&texture, texREPEAT);
			sha.unifDim = 3;
			sha.attrDim = 2 + 2 + 3;
			sha.texNum = 1;
			draw();
			pixSetKeyUpHandler(handleKeyUp);
			pixSetTimeStepHandler(handleTimeStep);
			pixRun();
			texDestroy(&texture);
			return 0;
		}
	}
}
