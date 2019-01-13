/*
Dawson d'Almeida and Justin T. Washington
January 10 2018
CS311 with Josh Davis

Main file for using 020traingle.c to render triangles to screen.
*/

#include <stdio.h>
#include <math.h>
#include "030vector.c"
#include "030matrix.c"
#include "030triangle.c"


int main(void) {
	/* Make a 512 x 512 window with the title 'Pixel Graphics'. This function
	returns 0 if no error occurred. */
	if (pixInitialize(512, 512, "Pixel Graphics") != 0)
		return 1;
	else {
		/* Register the callbacks (defined above) with the user interface, so
		that they are called as needed during pixRun (invoked below). */
		/* Clear the window to black. */
		pixClearRGB(0.0, 0.0, 0.0);
		/*
		a0 <= b0 <= c0
		double a[2] = {50, 150};
		double b[2] = {300, 20};
		double c[2] = {200, 450};
		double rgb[3] = {1.0, 1.0, 1.0};
		double alpha[3] = {1.0, 0.0, 0.0};
		double beta[3] = {0.0, 1.0, 0.0};
		double gamma[3] = {0.0, 0.0, 1.0};
		*/

		/*
		a0 < b0 < c0
		double a[2] = {50, 50};
		double b[2] = {300, 20};
		double c[2] = {400, 450};
		double rgb[3] = {0.2, 0.2, 0.2};
		double alpha[3] = {1.0, 0.0, 0.0};
		double beta[3] = {0.0, 1.0, 0.0};
		double gamma[3] = {0.0, 0.0, 1.0};
		*/


    triRender(a, b, c, rgb, alpha, beta, gamma); //normal a0<c0<b0

		/* Run the event loop. The callbacks that were registered above are
		invoked as needed. At the end, the resources supporting the window are
		deallocated. */
		pixRun();

		return 0;
	}
}
