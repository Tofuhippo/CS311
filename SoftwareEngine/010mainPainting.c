/*
Dawson d'Almeida and Justin T. Washington
January 8 2018
CS311 with Josh Davis

Simple painter program with different colors, different pen sizes, an eraser
and a clear function.
Colors: RGBCMYK (Red, Green, Blue, Cyan, Magenta, Yellow, Black)
Sizes: 1,2,3,4 (1*1, 3*3, 5*5, 7*7)
Eraser: E (9*9 Black)
Clear: P
*/

/* In this tutorial our compiler invocation becomes more complicated. On macOS,
we compile with...
    clang 000linking.c 000pixel.o -lglfw -framework OpenGL
This invocation causes our compiled code to be 'linked' with three other pieces
of software, that have been compiled elsewhere: OpenGL, GLFW, and pixel.o.
OpenGL is a cross-platform, standardized library for interacting with graphics
hardware. It comes pre-installed on macOS and Linux, and is easily installed on
Windows. GLFW is a cross-platform, minimalist user interface toolkit, that we
can use to build simple OpenGL applications. Often it is not pre-installed, but
it is easy to install. 000pixel.o is a library that I have written, to provide
a simple foundation for this graphics course. You must have 000pixel.o and
000pixel.h in the same directory as 000linking.c, for the linking to work. */

/* Notice that we include 000pixel.h. The file name is enclosed in quotation
marks rather than angle brackets, to tell the C compiler to search for the file
in the working directory. Feel free to read 000pixel.h. It documents all of the
public functionality of the library 000pixel.o. */
#include <stdio.h>
#include <math.h>
#include "000pixel.h"

/* Global Variables. */
int keyPress; // which key has been pressed to retain color data
int isDown; // 1 for mouse down, 0 for mouse not down
int pixelIter; // size of pen

/* This function is a user interface 'callback'. Once we register it with the
user interface infrastructure, it is called whenever the user releases a
keyboard key. For details, see the pixSetKeyUpHandler documentation. */
void handleKeyUp(int key, int shiftIsDown, int controlIsDown,
		int altOptionIsDown, int superCommandIsDown) {
	printf("handleKeyUp: %d, shift %d, control %d, altOpt %d, supComm %d\n",
		key, shiftIsDown, controlIsDown, altOptionIsDown, superCommandIsDown);

		if (pixelIter == 5) { // resets pen size if switching from eraser
			pixelIter = 0; // default size
		}
		switch (key) { // checks for pen size change, clear, and eraser
			case 80: // clear (P)
				pixClearRGB(0.0, 0.0, 0.0);
				keyPress = 87;
				break;
			case 49:
			  pixelIter = 0; // 1*1
				break;
			case 50:
			  pixelIter = 1; // 3*3
				break;
			case 51:
			  pixelIter = 2; // 5*5
				break;
			case 52:
			  pixelIter = 3; // 7*7
				break;
			case 53:
				pixelIter = 4; // 9*9
				break;
			case 69: // eraser
				keyPress = 75;
				pixelIter = 5; // unique Eraser Size
				break;
			default:
				keyPress = key; // pass color input as default
		}
}

/* Similarly, the following callbacks handle some mouse interactions. */
void handleMouseUp(double x, double y, int button, int shiftIsDown,
		int controlIsDown, int altOptionIsDown, int superCommandIsDown) {
	printf("handleMouseUp: %d, shift %d, control %d, altOpt %d, supComm %d\n",
		button, shiftIsDown, controlIsDown, altOptionIsDown,
		superCommandIsDown);
		isDown = 0; // mouse is not down
}

/* Similarly, the following callbacks handle some mouse interactions. */
void handleMouseDown(double x, double y, int button, int shiftIsDown,
		int controlIsDown, int altOptionIsDown, int superCommandIsDown) {
	printf("handleMouseDown: %d, shift %d, control %d, altOpt %d, supComm %d\n",
		button, shiftIsDown, controlIsDown, altOptionIsDown,
		superCommandIsDown);
		isDown = 1; // mouse is down
}

/* Handles painting to screen with all pen sizes centered around cursor. */
void paintToScreen(int x, int y, int r, int g, int b){
	for (int i = -pixelIter; i <= pixelIter; i = i + 1) {
		for (int j = -pixelIter; j <= pixelIter; j = j + 1) {
			pixSetRGB(x + i, y + j, r, g, b);
		}
	}
}

void handleMouseMove(double x, double y) {
	printf("handleMouseMove: x %f, y %f\n", x, y);
	if (isDown == 1){ // draw when isDown is 1 (true)
		switch (keyPress){
			case 82: //Red
				paintToScreen(x, y, 1.0, 0.0, 0.0);
				break;
			case 71: //Green
				paintToScreen(x, y, 0.0, 1.0, 0.0);
				break;
			case 66: //Blue
				paintToScreen(x, y, 0.0, 0.0, 1.0);
				break;
			case 67: //Cyan
				paintToScreen(x, y, 0.0, 1.0, 1.0);
				break;
			case 77: //Magenta
				paintToScreen(x, y, 1.0, 0.0, 1.0);
				break;
			case 89: //Yellow
				paintToScreen(x, y, 1.0, 1.0, 0.0);
				break;
			case 75: //Black (k)
				paintToScreen(x, y, 0.0, 0.0, 0.0);
				break;
			case 87: //White
				paintToScreen(x, y, 1.0, 1.0, 1.0);
				break;
		}
	}
}

void handleMouseScroll(double xOffset, double yOffset) {
	printf("handleMouseScroll: xOffset %f, yOffset %f\n", xOffset, yOffset);
}

/* This callback is called once per animation frame. As parameters it receives
the time for the current frame and the time for the previous frame. Both times
are measured in seconds since some distant past time. */
void handleTimeStep(double oldTime, double newTime) {
	if (floor(newTime) - floor(oldTime) >= 1.0)
		printf("handleTimeStep: %f frames/s\n", 1.0 / (newTime - oldTime));
}

/* You can also set callbacks for key down, key repeat, and mouse down. See
000pixel.h for details. */

int main(void) {
	/* Make a 512 x 512 window with the title 'Pixel Graphics'. This function
	returns 0 if no error occurred. */
	if (pixInitialize(512, 512, "Pixel Graphics") != 0)
		return 1;
	else {
		/* Register the callbacks (defined above) with the user interface, so
		that they are called as needed during pixRun (invoked below). */
		pixSetKeyUpHandler(handleKeyUp);
		pixSetMouseUpHandler(handleMouseUp);
		pixSetMouseDownHandler(handleMouseDown);
		pixSetMouseMoveHandler(handleMouseMove);
		pixSetMouseScrollHandler(handleMouseScroll);
		pixSetTimeStepHandler(handleTimeStep);
		/* Clear the window to black. */
		pixClearRGB(0.0, 0.0, 0.0);

		/* Set default value to white. */
		keyPress = 87;

		/* Run the event loop. The callbacks that were registered above are
		invoked as needed. At the end, the resources supporting the window are
		deallocated. */
		pixRun();
		return 0;
	}
}
