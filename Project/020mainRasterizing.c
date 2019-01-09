#include <stdio.h>
#include <math.h>
#include "020triangle.c"

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

    /* Working Test Cases. */
    // First case: a0 <= c0 <= b0:
    //triRender(50, 150, 300, 20, 200, 450, 1.0, 0.0, 1.0); //normal a0<c0<b0
    //triRender(200, 450, 50, 150, 300, 20, 1.0, 0.0, 1.0); //normal c0<b0<a0
    //triRender(300, 20, 200, 450, 50, 150, 1.0, 0.0, 1.0); //normal b0<a0<c0
    //triRender(50, 150, 300, 20, 50, 450, 1.0, 0.0, 1.0); //leftvert a0==c0
    //triRender(50, 150, 300, 20, 300, 450, 1.0, 0.0, 1.0); //rightvert c0==b0
    //triRender(50, 150, 300, 150, 200, 450, 1.0, 0.0, 1.0); //botHorizontal c0==b0
		//triRender(50, 150, 300, 150, 200, 150, 1.0, 0.0, 1.0); //horizontal a1==b1==c1

		// Second case: a0 < b0 < c0:


    /* Non-Working Test Cases. */
    // First case: a0 <= c0 <= b0:
    //triRender(150, 150, 150, 20, 150, 450, 1.0, 0.0, 1.0); //degenerate a0==b0==c0

		triRender(150, 150, 200, 20, 300, 450, 1.0, 0.0, 1.0); //normal a0<b0<c0


		/* Run the event loop. The callbacks that were registered above are
		invoked as needed. At the end, the resources supporting the window are
		deallocated. */
		pixRun();

		return 0;
	}
}
