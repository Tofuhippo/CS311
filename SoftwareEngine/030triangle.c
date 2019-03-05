/*
Dawson d'Almeida and Justin T. Washington
January 10 2018
CS311 with Josh Davis

Renders triangle to screen.
*/

#include <stdio.h>
#include <math.h>
#include "000pixel.h"


/* Helper for triRender, which assumes that the points are in counterclockwise
	 order and a is the leftmost point, possibly tied with b or c, then renders
	 the traingle to the screen. */
void triRenderALeft(const double a[], const double b[], const double c[],
	const double rgb[], const double alpha[3], const double beta[3],
		const double gamma[3]) {

			/* Calculate inverse m for interpolating alpha, beta, and gamma. */
			double bMinusA[2];
			vecSubtract(2, b, a, bMinusA);
			double cMinusA[2];
			vecSubtract(2, c, a, cMinusA);
			double m[2][2];
			double mInv[2][2];
			mat22Columns(bMinusA, cMinusA, m);
			// If det(m) == 0, then we have a degenerate triangle.
			if (mat22Invert(m, mInv) == 0.0){
				printf("Degenerate Triangle");
			}

			if (c[0] <= b[0]){ // c[0] is between a[0] and b[0] or equal to a[0] or b[0]
				double bottomSlope = 0; // degenerate value (a[0] == b[0] == c[0])
				if (b[0] != a[0]){
					bottomSlope = (b[1]-a[1])/(b[0]-a[0]);
				}
				/* Draw left half of triangle or whole triangle if b[0] == c[0]. */
				for (int x0 = (int)ceil(a[0]); x0 <= (int)floor(c[0]); x0++){
					double topLeftSlope = 0;
					if (c[0] != a[0]){
						topLeftSlope = (c[1]-a[1])/(c[0]-a[0]);
					}
					for (int x1 = (int)ceil(x0*bottomSlope - a[0]*bottomSlope + a[1]);
						x1 <= (int)floor(x0*topLeftSlope - c[0]*topLeftSlope + c[1]); x1++) {
							double pAndQ[2];
							double x[2] = {x0, x1};
							double xMinusA[2];
							vecSubtract(2, x, a, xMinusA);
							mat221Multiply(mInv, xMinusA, pAndQ);
							pixSetRGB(x0, x1,
								 				rgb[0] * (alpha[0] + pAndQ[0] * (beta[0] - alpha[0]) +
												                     pAndQ[1] * (gamma[0] - alpha[0])),
												rgb[1] * (alpha[1] + pAndQ[0] * (beta[1] - alpha[1]) +
																	           pAndQ[1] * (gamma[1] - alpha[1])),
											  rgb[2] * (alpha[2] + pAndQ[0] * (beta[2] - alpha[2]) +
												                     pAndQ[1] * (gamma[2] - alpha[2])));
					}
				}
				/* Draw right half of triangle or whole triangle if a[0] == c[0]. */
				for (int x0 = (int)floor(c[0])+1; x0 <= (int)floor(b[0]); x0++){
					double topRightSlope = 0;
					if (b[0] != c[0]){
						topRightSlope = (b[1]-c[1])/(b[0]-c[0]);
					}
					for (int x1 = (int)ceil(x0*bottomSlope - a[0]*bottomSlope + a[1]);
						x1 <= (int)floor(x0*topRightSlope - c[0]*topRightSlope + c[1]); x1++) {
							double pAndQ[2];
							double x[2] = {x0, x1};
							double xMinusA[2];
							vecSubtract(2, x, a, xMinusA);
							mat221Multiply(mInv, xMinusA, pAndQ);
							pixSetRGB(x0, x1,
								 				rgb[0] * (alpha[0] + pAndQ[0] * (beta[0] - alpha[0]) +
												                     pAndQ[1] * (gamma[0] - alpha[0])),
												rgb[1] * (alpha[1] + pAndQ[0] * (beta[1] - alpha[1]) +
																	           pAndQ[1] * (gamma[1] - alpha[1])),
											  rgb[2] * (alpha[2] + pAndQ[0] * (beta[2] - alpha[2]) +
												                     pAndQ[1] * (gamma[2] - alpha[2])));
					}
				}
			}
			else{ // c[0] is right of both a[0] and b[0]
				double topSlope = 0; // degenerate value (a[0] == b[0] == c[0])
				if (c[0] != a[0]){
					topSlope = (c[1]-a[1])/(c[0]-a[0]);
				}
				/* Draw left half of triangle, bisected by b[0]. */
				for (int x0 = (int)ceil(a[0]); x0 <= (int)floor(b[0]); x0++){
					double bottomLeftSlope = 0;
					if (b[0] != a[0]){
						bottomLeftSlope = (b[1]-a[1])/(b[0]-a[0]);
					}
					for (int x1 = (int)ceil(x0*bottomLeftSlope - a[0]*bottomLeftSlope + a[1]);
						x1 <= (int)floor(x0*topSlope - c[0]*topSlope + c[1]); x1++) {
							double pAndQ[2];
							double x[2] = {x0, x1};
							double xMinusA[2];
							vecSubtract(2, x, a, xMinusA);
							mat221Multiply(mInv, xMinusA, pAndQ);
							pixSetRGB(x0, x1,
								 				rgb[0] * (alpha[0] + pAndQ[0] * (beta[0] - alpha[0]) +
												                     pAndQ[1] * (gamma[0] - alpha[0])),
												rgb[1] * (alpha[1] + pAndQ[0] * (beta[1] - alpha[1]) +
																	           pAndQ[1] * (gamma[1] - alpha[1])),
											  rgb[2] * (alpha[2] + pAndQ[0] * (beta[2] - alpha[2]) +
												                     pAndQ[1] * (gamma[2] - alpha[2])));
					}
				}
				/* Draw right half of triangle, bisected by b[0]. */
				for (int x0 = (int)floor(b[0])+1; x0 <= (int)floor(c[0]); x0++){
					double bottomRightSlope = 0;
					if (c[0] != b[0]){
						bottomRightSlope = (c[1]-b[1])/(c[0]-b[0]);
					}
					for (int x1 = (int)ceil(x0*bottomRightSlope - b[0]*bottomRightSlope + b[1]);
						x1 <= (int)floor(x0*topSlope - a[0]*topSlope + a[1]); x1++) {
							double pAndQ[2];
							double x[2] = {x0, x1};
							double xMinusA[2];
							vecSubtract(2, x, a, xMinusA);
							mat221Multiply(mInv, xMinusA, pAndQ);
							pixSetRGB(x0, x1,
								 				rgb[0] * (alpha[0] + pAndQ[0] * (beta[0] - alpha[0]) +
												                     pAndQ[1] * (gamma[0] - alpha[0])),
												rgb[1] * (alpha[1] + pAndQ[0] * (beta[1] - alpha[1]) +
																	           pAndQ[1] * (gamma[1] - alpha[1])),
											  rgb[2] * (alpha[2] + pAndQ[0] * (beta[2] - alpha[2]) +
												                     pAndQ[1] * (gamma[2] - alpha[2])));
					}
				}
			}
}

/* Takes in 3 pairs of coordinates that are oriented in a counterclockwise
   fashion and renders them to the screen with the colors defined by the r, g,
	 and b parameters. */
void triRender(const double a[], const double b[], const double c[],
	const double rgb[], const double alpha[3], const double beta[3],
		const double gamma[3]) {
			/* If a is leftmost, possibly tied with b or c. */
			if (a[0] <= b[0] && a[0] <= c[0]) {
				triRenderALeft(a, b, c, rgb, alpha, beta, gamma);
			}
			/* If b is leftmost, possibly tied with c. */
			else if (b[0] <= a[0] && b[0] <= c[0]) {
				triRenderALeft(b, c, a, rgb, beta, gamma, alpha);
			}
			/* If c is leftmost. */
			else {
				triRenderALeft(c, a, b, rgb, gamma, alpha, beta);
			}
}
