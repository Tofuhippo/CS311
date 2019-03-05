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
#define mainUNIFTIME 3
#define mainTEXR 0
#define mainTEXG 1
#define mainTEXB 2

double time;

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
			// arrays for the returned RGB values for each texture
			double sample0[3];
			double sample1[3];
			double sample2[3];
			texSample(tex[0],
								cos(unif[mainUNIFTIME])*(attr[mainATTRS]-0.5)
								   - sin(unif[mainUNIFTIME])*(attr[mainATTRT]-0.5)
									 + attr[mainATTRS],
			 					cos(unif[mainUNIFTIME])*(attr[mainATTRS]-0.5)
									 - sin(unif[mainUNIFTIME])*(attr[mainATTRT]-0.5)
									 + attr[mainATTRT],
							  sample0);
			texSample(tex[1], attr[mainATTRS] + sin(unif[mainUNIFTIME]*.5),
				        attr[mainATTRT] + cos(unif[mainUNIFTIME]*.5), sample1);
			texSample(tex[2], attr[mainATTRS], attr[mainATTRT], sample2);

			if((int)floor(unif[mainUNIFTIME]) % 10 == 9){ // use sample2 every 9 secs
				rgb[0] = sample2[mainTEXR];
				rgb[1] = sample2[mainTEXG];
				rgb[2] = sample2[mainTEXB];
			} else { //trippy times
				// fluctuating color coefficients for trippy times
				double red = attr[mainATTRR] * (sin((unif[mainUNIFTIME])) + 5);
				double green = attr[mainATTRG] * (sin((unif[mainUNIFTIME])*0.2) + 5);
				double blue = attr[mainATTRB] * (cos((unif[mainUNIFTIME])*0.6) + 5);

				rgb[0] = sample1[mainTEXR] * sample0[mainTEXR] * red;
				rgb[1] = sample1[mainTEXG] * sample0[mainTEXG] * green;
				rgb[2] = sample1[mainTEXB] * sample0[mainTEXB] * blue;
			}
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
texTexture texture0;
texTexture texture1;
texTexture texture2;
const texTexture *textures[3] = {&texture0, &texture1, &texture2};
const texTexture **tex = textures;

// currently, our draw makes 2 triangles for each texture to render the full
// rectangle. Our corners are oriented so that the picture is upright.
void draw(void) {
	pixClearRGB(0.0, 0.0, 0.0);
	double a0[7] = {511.0, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0};
	double b0[7] = {511.0, 511.0, 1.0, 1.0, 0.0, 1.0, 0.0};
	double c0[7] = {0.0, 511.0, 0.0, 1.0, 0.0, 0.0, 1.0};
	double a1[7] = {0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0};
	double b1[7] = {511.0, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0};
	double c1[7] = {0.0, 511.0, 0.0, 1.0, 0.0, 0.0, 1.0};
	double unif[4] = {1.0, 1.0, 1.0, time};
	triRender(&sha, unif, tex, a0, b0, c0);
	triRender(&sha, unif, tex, a1, b1, c1);
}

void handleKeyUp(int key, int shiftIsDown, int controlIsDown,
		int altOptionIsDown, int superCommandIsDown) {
	if (key == GLFW_KEY_ENTER) {
		// Assume both images have same filtering
		if (texture0.filtering == texLINEAR) {
			texSetFiltering(&texture0, texNEAREST);
			texSetFiltering(&texture1, texNEAREST);
			texSetFiltering(&texture2, texNEAREST);
		}
		else {
			texSetFiltering(&texture0, texLINEAR);
			texSetFiltering(&texture1, texLINEAR);
			texSetFiltering(&texture2, texLINEAR);
		}
		draw();
	}
}

void handleTimeStep(double oldTime, double newTime) {
	if (floor(newTime) - floor(oldTime) >= 1.0)
		printf("handleTimeStep: %f frames/sec\n", 1.0 / (newTime - oldTime));
	time = newTime;
	draw();
}

int main(void) {
	if (pixInitialize(512, 512, "Pixel Graphics") != 0)
		return 1;
	else {
		if (texInitializeFile(&texture0, "josh_davis.jpg") != 0)
			return 2;
		if (texInitializeFile(&texture1, "nathan_mannes.jpg") != 0)
			return 2;
		if (texInitializeFile(&texture2, "shrek.png") != 0)
			return 2;
		else {
			texSetFiltering(&texture0, texNEAREST);
			texSetLeftRight(&texture0, texREPEAT);
			texSetTopBottom(&texture0, texREPEAT);
			texSetFiltering(&texture1, texNEAREST);
			texSetLeftRight(&texture1, texREPEAT);
			texSetTopBottom(&texture1, texREPEAT);
			texSetFiltering(&texture2, texNEAREST);
			texSetLeftRight(&texture2, texREPEAT);
			texSetTopBottom(&texture2, texREPEAT);
			sha.unifDim = 4;
			sha.attrDim = 2 + 2 + 3;
			sha.texNum = 3;
			draw();
			pixSetKeyUpHandler(handleKeyUp);
			pixSetTimeStepHandler(handleTimeStep);
			pixRun();
			texDestroy(&texture0);
			texDestroy(&texture1);
			texDestroy(&texture2);
			return 0;
		}
	}
}
