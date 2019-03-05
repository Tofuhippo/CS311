/*
Dawson d'Almeida and Justin T. Washington
January 21 2018
CS311 with Josh Davis

Main abstracted file that defines color pixel and transformation of vertices.
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
#include "080shading.c"


/* It is important that colorPixel correctly parse the data that we give it. To
help avoid errors in communication, we set up some compile-time constants.
Notice that the documentation for the new triRender requires mainVARYX to be 0
and mainVARYY to be 1. */
#define mainATTRX 0
#define mainATTRY 1
#define mainATTRS 2
#define mainATTRT 3
#define mainATTRR 4
#define mainATTRG 5
#define mainATTRB 6
#define mainVARYX 0
#define mainVARYY 1
#define mainVARYS 2
#define mainVARYT 3
#define mainVARYR 4
#define mainVARYG 5
#define mainVARYB 6
#define mainUNIFR 0
#define mainUNIFG 1
#define mainUNIFB 2
#define mainUNIFTIME 3
#define mainUNIFROT 4
#define mainUNIFXTRANS 5
#define mainUNIFYTRANS 6
#define mainTEXR 0
#define mainTEXG 1
#define mainTEXB 2

double time;

/* vary (previously attr) has already been interpolated from the vertex attributes. tex is an
array of texNum elements, each of which is of type texTexture *. rgb is the
output parameter. The other parameters should be self-explanatory. For reasons
that become clear later in the course, colorPixel is allowed to use (A) compile-
time constants such as mainVARYX, (B) functions such as sin() and texSample(),
(C) its parameters such as unifDim, and (D) any variables that it declares
locally. It is not allowed to use any other variables, such as global variables
that aren't parameters. */
void colorPixel(int unifDim, const double unif[], int texNum,
		const texTexture *tex[], int varyDim, const double vary[],
		double rgb[3]) {
			// arrays for the returned RGB values for each texture
			double sample[3];
			texSample(tex[0], vary[mainVARYS], vary[mainVARYT], sample);
			rgb[0] = sample[mainTEXR];
			rgb[1] = sample[mainTEXG];
			rgb[2] = sample[mainTEXB];
}

/* Outputs vary, based on the other parameters, which are unaltered. Like
colorPixel, this function should not access any variables other than its
parameters and any local variables that it declares. */
void transformVertex(int unifDim, const double unif[], int attrDim,
		const double attr[], int varyDim, double vary[]) {
			double xy[2] = {attr[mainATTRX], attr[mainATTRY]};
			double xyChanged[2] = {attr[mainATTRX], attr[mainATTRY]};

			// If unif[mainUNIFROT] is not 0, apply rotation.
			if (unif[mainUNIFROT]){
				double theta = unif[mainUNIFTIME];
				double rotationMatrix[2][2] = {{cos(theta), -sin(theta)},
			                                 {sin(theta), cos(theta)}};

				mat221Multiply(rotationMatrix, xy, xyChanged);
			}
			// Apply static transposition on x and y.
			vary[mainVARYX] = xyChanged[0] + unif[mainUNIFXTRANS];
			vary[mainVARYY] = xyChanged[1] + unif[mainUNIFYTRANS];
    	vary[mainVARYS] = attr[mainATTRS];
    	vary[mainVARYT] = attr[mainATTRT];
}

/* We have to include 050triangle.c after defining colorPixel, because it
refers to colorPixel. (Later in the course we handle this issue better.) */
#include "080triangle.c"
#include "080mesh.c"

/* These structs is initialized in main() below. */
shaShading sha;
meshMesh mesh;
/* Here we make an array of one texTexture pointer, in such a way that the
const qualifier can be enforced throughout the surrounding code. C is confusing
for stuff like this. Don't worry about mastering C at this level. It doesn't
come up much in our course. */
texTexture texture;
const texTexture *textures[1] = {&texture};
const texTexture **tex = textures;


// currently, our draw makes 2 triangles for each texture to render the full
// rectangle. Our corners are oriented so that the picture is upright.
void draw(void) {
	pixClearRGB(0.0, 0.0, 0.0);

  meshInitializeEllipse(&mesh, 256.0, 256.0, 200.0, 200.0, 20);
	//meshInitializeRectangle(&mesh, 20.0, 500.0, 20.0, 500.0);
	double unif[7] = {1.0, 1.0, 1.0, time, 1, 0, 0};

  meshRender(&mesh, &sha, unif, tex);

}

void handleKeyUp(int key, int shiftIsDown, int controlIsDown,
		int altOptionIsDown, int superCommandIsDown) {
	if (key == GLFW_KEY_ENTER) {
		// Assume both images have same filtering
		if (texture.filtering == texLINEAR) {
			texSetFiltering(&texture, texNEAREST);
		}
		else {
			texSetFiltering(&texture, texLINEAR);
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
		if (texInitializeFile(&texture, "nathan_mannes.jpg") != 0)
			return 2;
		else {
			texSetFiltering(&texture, texNEAREST);
			texSetLeftRight(&texture, texREPEAT);
			texSetTopBottom(&texture, texREPEAT);

			sha.unifDim = 7;
			sha.attrDim = 2 + 2;
			sha.varyDim = 2 + 2;
			sha.texNum = 1;

			draw();
			pixSetKeyUpHandler(handleKeyUp);
			pixSetTimeStepHandler(handleTimeStep);
			pixRun();
			texDestroy(&texture);
      meshDestroy(&mesh);
			return 0;
		}
	}
}
