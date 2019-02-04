/*
Dawson d'Almeida and Justin T. Washington
January 27 2018
CS311 with Josh Davis

Renders triangle to screen.
*/

#include <stdio.h>
#include <math.h>
#include "000pixel.h"


/* Helper for triRender, which assumes that the points are in counterclockwise
	 order and a is the leftmost point, possibly tied with b or c, then renders
	 the traingle to the screen. */
void triRenderALeft(const shaShading *sha, depthBuffer *buf, const double unif[],
	 const texTexture *tex[], const double a[], const double b[],
	 const double c[]) {

			/* Calculate inverse m for interpolating alpha, beta, and gamma. */
			double bMinusA[2], cMinusA[2];
			double aPos[2] = {a[0], a[1]};
			double bPos[2] = {b[0], b[1]};
			double cPos[2] = {c[0], c[1]};
			vecSubtract(2, bPos, aPos, bMinusA);
			vecSubtract(2, cPos, aPos, cMinusA);
			double m[2][2];
			double mInv[2][2];
			mat22Columns(bMinusA, cMinusA, m);
			//mat22Print(m);
			// If det(m) == 0, then we have a degenerate triangle.
			if (mat22Invert(m, mInv) <= 0.0){
				return;
			}

			// rgbd[4] is the array passed into colorPixel for calculating the
			// color that we use in pixSetRGB.
			double rgbd[4];
			// vary holds x0, x1, and other interpolated values after vertex
			// transformation (ex. s and t; r, g, and b; etc...)
			double vary[sha->varyDim];
			double pAndQ[2], xMinusA[2];
			// Values used for Optimization
			int botX0, botX1, topX0, topX1;

			if (c[0] <= b[0]){ // c[0] is between a[0] and b[0] or equal to a[0] or b[0]
				double bottomSlope = 0; // degenerate value (a[0] == b[0] == c[0])
				if (b[0] != a[0]){
					bottomSlope = (b[1]-a[1])/(b[0]-a[0]);
				}
				// Optimization of for loop: loop over values only in screen space
				botX0 = (int)ceil(a[0]);
				topX0 = (int)floor(c[0]);
				if (botX0 < 0){
					botX0 = 0;
				}
				if (topX0 > 512){
					topX0 = 512;
				}
				/* Draw left half of triangle or whole triangle if b[0] == c[0]. */
				for (int x0 = botX0; x0 <= topX0; x0++){
					double topLeftSlope = 0;
					if (c[0] != a[0]){
						topLeftSlope = (c[1]-a[1])/(c[0]-a[0]);
					}
					// Optimization of for loop: loop over values only in screen space
					botX1 = (int)ceil(x0*bottomSlope - a[0]*bottomSlope + a[1]);
					topX1 = (int)floor(x0*topLeftSlope - c[0]*topLeftSlope + c[1]);
					if (botX1 < 0){
						botX1 = 0;
					}
					if (topX1 > 512){
						topX1 = 512;
					}
					for (int x1 = botX1; x1 <= topX1; x1++) {
							double x[2] = {x0, x1};
							vecSubtract(2, x, aPos, xMinusA);
							mat221Multiply(mInv, xMinusA, pAndQ);
							vary[0] = x0;
							vary[1] = x1;
							for (int i = 2; i < sha->varyDim; i++){
								vary[i] = a[i] + pAndQ[0] * (b[i] - a[i]) +
																	 pAndQ[1] * (c[i] - a[i]);
							}
							// right now, we perform calculations for pixel color while lookin
							// for depth, which is inefficient but oh well
							sha->colorPixel(sha->unifDim, unif, sha->texNum,
								              tex, sha->varyDim, vary, rgbd);
							// now depth is in rgbd[3], so check depth of current pixel (x0, x1)
							double curPixDepth = depthGetDepth(buf, x0, x1);
							if (0<=x0 && x0<512 && 0<=x1 && x1<512 && curPixDepth > rgbd[3]){
								// draw pixel and update depth if pixel is closer
								pixSetRGB(x0, x1, rgbd[0], rgbd[1], rgbd[2]);
								depthSetDepth(buf, x0, x1, rgbd[3]);
							}
							// otherwise do nothing
					}
				}
				// Optimization of for loop: loop over values only in screen space
				botX0 = (int)ceil(c[0]);
				topX0 = (int)floor(b[0]);
				if (botX0 < 0){
					botX0 = 0;
				}
				if (topX0 > 512){
					topX0 = 512;
				}
				// Optimization of for loop: loop over values only in screen space
				/* Draw right half of triangle or whole triangle if a[0] == c[0]. */
				for (int x0 = botX0; x0 <= topX0; x0++){
					double topRightSlope = 0;
					if (b[0] != c[0]){
						topRightSlope = (b[1]-c[1])/(b[0]-c[0]);
					}
					// Optimization of for loop: loop over values only in screen space
					botX1 = (int)ceil(x0*bottomSlope - a[0]*bottomSlope + a[1]);
					topX1 = (int)floor(x0*topRightSlope - c[0]*topRightSlope + c[1]);
					if (botX1 < 0){
						botX1 = 0;
					}
					if (topX1 > 512){
						topX1 = 512;
					}
					for (int x1 = botX1; x1 <= topX1; x1++) {
							double x[2] = {x0, x1};
							vecSubtract(2, x, aPos, xMinusA);
							mat221Multiply(mInv, xMinusA, pAndQ);
							vary[0] = x0;
							vary[1] = x1;
							for (int i = 2; i < sha->varyDim; i++){
								vary[i] = a[i] + pAndQ[0] * (b[i] - a[i]) +
																	 pAndQ[1] * (c[i] - a[i]);
							}
							// right now, we perform calculations for pixel color while lookin
							// for depth, which is inefficient but oh well
							sha->colorPixel(sha->unifDim, unif, sha->texNum,
								              tex, sha->varyDim, vary, rgbd);
							// now depth is in rgbd[3], so check depth of current pixel (x0, x1)
							double curPixDepth = depthGetDepth(buf, x0, x1);
							if (0<=x0 && x0<512 && 0<=x1 && x1<512 && curPixDepth > rgbd[3]){
								// draw pixel and update depth if pixel is closer
								pixSetRGB(x0, x1, rgbd[0], rgbd[1], rgbd[2]);
								depthSetDepth(buf, x0, x1, rgbd[3]);
							}
							// otherwise do nothing
					}
				}
			}
			else{ // c[0] is right of both a[0] and b[0]
				double topSlope = 0; // degenerate value (a[0] == b[0] == c[0])
				if (c[0] != a[0]){
					topSlope = (c[1]-a[1])/(c[0]-a[0]);
				}
				// Optimization of for loop: loop over values only in screen space
				botX0 = (int)ceil(a[0]);
				topX0 = (int)floor(b[0]);
				if (botX0 < 0){
					botX0 = 0;
				}
				if (topX0 > 512){
					topX0 = 512;
				}
				/* Draw left half of triangle, bisected by b[0]. */
				for (int x0 = botX0; x0 <= topX0; x0++){
					double bottomLeftSlope = 0;
					if (b[0] != a[0]){
						bottomLeftSlope = (b[1]-a[1])/(b[0]-a[0]);
					}
					// Optimization of for loop: loop over values only in screen space
					botX1 = (int)ceil(x0*bottomLeftSlope - a[0]*bottomLeftSlope + a[1]);
					topX1 = (int)floor(x0*topSlope - c[0]*topSlope + c[1]);
					if (botX1 < 0){
						botX1 = 0;
					}
					if (topX1 > 512){
						topX1 = 512;
					}
					for (int x1 = botX1; x1 <= topX1; x1++){
							double x[2] = {x0, x1};
							vecSubtract(2, x, aPos, xMinusA);
							mat221Multiply(mInv, xMinusA, pAndQ);
							vary[0] = x0;
							vary[1] = x1;
							for (int i = 2; i < sha->varyDim; i++){
								vary[i] = a[i] + pAndQ[0] * (b[i] - a[i]) +
																	 pAndQ[1] * (c[i] - a[i]);
							}
							// right now, we perform calculations for pixel color while lookin
							// for depth, which is inefficient but oh well
							sha->colorPixel(sha->unifDim, unif, sha->texNum,
								              tex, sha->varyDim, vary, rgbd);
							// now depth is in rgbd[3], so check depth of current pixel (x0, x1)
							double curPixDepth = depthGetDepth(buf, x0, x1);
							if (0<=x0 && x0<512 && 0<=x1 && x1<512 && curPixDepth > rgbd[3]){
								// draw pixel and update depth if pixel is closer
								pixSetRGB(x0, x1, rgbd[0], rgbd[1], rgbd[2]);
								depthSetDepth(buf, x0, x1, rgbd[3]);
							}
							// otherwise do nothing
					}
				}
				// Optimization of for loop: loop over values only in screen space
				botX0 = (int)floor(b[0])+1;
				topX0 = (int)floor(c[0]);
				if (botX0 < 0){
					botX0 = 0;
				}
				if (topX0 > 512){
					topX0 = 512;
				}
				/* Draw right half of triangle, bisected by b[0]. */
				for (int x0 = botX0; x0 <= topX0; x0++){
					double bottomRightSlope = 0;
					if (c[0] != b[0]){
						bottomRightSlope = (c[1]-b[1])/(c[0]-b[0]);
					}
					// Optimization of for loop: loop over values only in screen space
					botX1 = (int)ceil(x0*bottomRightSlope - b[0]*bottomRightSlope + b[1]);
					topX1 = (int)floor(x0*topSlope - a[0]*topSlope + a[1]);
					if (botX1 < 0){
						botX1 = 0;
					}
					if (topX1 > 512){
						topX1 = 512;
					}
					for (int x1 = botX1; x1 <= topX1; x1++){
							double x[2] = {x0, x1};
							vecSubtract(2, x, aPos, xMinusA);
							mat221Multiply(mInv, xMinusA, pAndQ);
							vary[0] = x0;
							vary[1] = x1;
							for (int i = 2; i < sha->varyDim; i++){
								vary[i] = a[i] + pAndQ[0] * (b[i] - a[i]) +
																	 pAndQ[1] * (c[i] - a[i]);
							}
							// right now, we perform calculations for pixel color while lookin
							// for depth, which is inefficient but oh well
							sha->colorPixel(sha->unifDim, unif, sha->texNum,
								              tex, sha->varyDim, vary, rgbd);
							// now depth is in rgbd[3], so check depth of current pixel (x0, x1)
							double curPixDepth = depthGetDepth(buf, x0, x1);
							if (0<=x0 && x0<512 && 0<=x1 && x1<512 && curPixDepth > rgbd[3]){
								// draw pixel and update depth if pixel is closer
								pixSetRGB(x0, x1, rgbd[0], rgbd[1], rgbd[2]);
								depthSetDepth(buf, x0, x1, rgbd[3]);
							}
							// otherwise do nothing
					}
				}
			}
}

/* Assumes that the 0th and 1th elements of a, b, c are the 'x' and 'y'
	 coordinates of the vertices, respectively (used in rasterization, and to
	 interpolate the other elements of a, b, c). */
void triRender(const shaShading *sha, depthBuffer *buf, const double unif[],
		const texTexture *tex[], const double a[], const double b[],
		const double c[]) {
			/* If a is leftmost, possibly tied with b or c. */
			if (a[0] <= b[0] && a[0] <= c[0]) {
				triRenderALeft(sha, buf, unif, tex, a, b, c);
			}
			/* If b is leftmost, possibly tied with c. */
			else if (b[0] <= a[0] && b[0] <= c[0]) {
				triRenderALeft(sha, buf, unif, tex, b, c, a);
			}
			/* If c is leftmost. */
			else {
				triRenderALeft(sha, buf, unif, tex, c, a, b);
			}
}
