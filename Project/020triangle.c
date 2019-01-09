#include <stdio.h>
#include <math.h>
#include "000pixel.h"


/* Assumes that the points are in counterclockwise order and a is the
	 leftmost point, possibly tied with b or c. */
void triRenderALeft(const double a0, const double a1, const double b0,
		const double b1, const double c0, const double c1, const double r,
		const double g, const double b) {

			// vertical line not working

			if (c0 <= b0){ // c0 is between a0 and b0 or equal to a0 or b0
				double bottomSlope = 0; // degenerate value (a0 == b0 == c0)
				if (b0 != a0){
					bottomSlope = (b1-a1)/(b0-a0);
				}
				/* Draw left half of triangle or whole triangle if b0 == c0. */
				for (int x0 = (int)ceil(a0); x0 <= (int)floor(c0); x0++){

					double topLeftSlope = 0;
					if (c0 != a0){
						topLeftSlope = (c1-a1)/(c0-a0);
					}
					for (int x1 = (int)ceil(x0*bottomSlope - a0*bottomSlope + a1);
						x1 <= (int)floor(x0*topLeftSlope - a0*topLeftSlope + a1); x1++) {
							pixSetRGB(x0, x1, r, g, b);
					}
				}
				/* Draw right half of triangle or whole triangle if a0 == c0. */
				for (int x0 = (int)floor(c0)+1; x0 <= (int)floor(b0); x0++){

					double topRightSlope = 0;
					if (b0 != c0){
						topRightSlope = (b1-c1)/(b0-c0);
					}
					for (int x1 = (int)ceil(x0*bottomSlope - a0*bottomSlope + a1);
						x1 <= (int)floor(x0*topRightSlope - c0*topRightSlope + c1); x1++) {
							pixSetRGB(x0, x1, r, g, b);
					}
				}
			}

			else{ // c0 is right of both a0 and b0
				double topSlope = 0; // degenerate value (a0 == b0 == c0)
				if (c0 != a0){
					topSlope = (c1-a1)/(c0-a0);
				}
				/* Draw left half of triangle, bisected by b0. */
				for (int x0 = (int)ceil(a0); x0 <= (int)floor(b0); x0++){

					double bottomLeftSlope = 0;
					if (b0 != a0){
						bottomLeftSlope = (b1-a1)/(b0-a0);
					}
					for (int x1 = (int)ceil(x0*bottomLeftSlope - a0*bottomLeftSlope + a1);
						x1 <= (int)floor(x0*topSlope - a0*topSlope + a1); x1++) {
							pixSetRGB(x0, x1, r, g, b);
					}
				}
				/* Draw right half of triangle, bisected by b0. */
				for (int x0 = (int)floor(b0)+1; x0 <= (int)floor(c0); x0++){

					double bottomRightSlope = 0;
					if (c0 != b0){
						bottomRightSlope = (c1-b1)/(c0-b0);
					}
					for (int x1 = (int)ceil(x0*bottomRightSlope - b0*bottomRightSlope + b1);
						x1 <= (int)floor(x0*topSlope - a0*topSlope + a1); x1++) {
							pixSetRGB(x0, x1, r, g, b);
					}
				}
			}

}

void triRender(const double a0, const double a1, const double b0,
		const double b1, const double c0, const double c1, const double r,
		const double g, const double b) {
			/* If a is leftmost, possibly tied with b or c. */
			if (a0 <= b0 && a0 <= c0) {
				triRenderALeft(a0, a1, b0, b1, c0, c1, r, g, b);
			}
			/* If b is leftmost, possibly tied with a or c. */
			else if (b0 <= a0 && b0 <= c0) {
				triRenderALeft(b0, b1, c0, c1, a0, a1, r, g, b);
			}
			/* If c is leftmost. */
			else {
				triRenderALeft(c0, c1, a0, a1, b0, b1, r, g, b);
			}
}
