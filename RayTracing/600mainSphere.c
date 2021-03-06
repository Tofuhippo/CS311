/*
Dawson d'Almeida and Justin T. Washington
March 4 2018
CS311 with Josh Davis

Main abstracted file that implements ray tracing.
*/

/* On macOS, compile with...
    clang 600mainSphere.c 000pixel.o -lglfw -framework OpenGL
*/

#include <stdio.h>
#include <math.h>
#include <stdarg.h>
#include "000pixel.h"
#include <GLFW/glfw3.h>

#include "120vector.c"
#include "140matrix.c"
#include "140isometry.c"
#include "600camera.c"

#define SCREENWIDTH 512
#define SCREENHEIGHT 512

#define EPSILON 0.00000001
//#define INFINITY 100000000

camCamera camera;
double cameraTarget[3] = {0.0, 0.0, 0.0};
double cameraRho = 10.0, cameraPhi = M_PI / 3.0, cameraTheta = -M_PI / 3.0;
int cameraMode = 0;

/* These are our two sphere bodies. */
isoIsometry isomA, isomB;
double radiusA = 1.0, radiusB = 1.5;
double colorA[3] = {1.0, 0.0, 1.0}, colorB[3] = {1.0, 1.0, 0.0};

/* Rendering ******************************************************************/

/* The intersected member contains a code for how a ray (first) intersects the
surface of a body: 0 if it doesn't intersect (or is tangent), 1 if it
intersects leaving the body, or -1 if it intersects entering the body. If the
intersected code is not 0, then time records the time of the intersection. */
typedef struct rayRecord rayRecord;
struct rayRecord {
	int intersected;
	double t;
};

/* Given the isometry and radius of a spherical body. Given a starting time and
an ending time. Returns the ray record describing how the ray first intersects
the sphere. */
rayRecord sphereIntersection(const isoIsometry *iso, double radius,
		const double e[3], const double d[3], double tStart, double tEnd) {
	rayRecord result;
	/* Four Cases to test intersection:
	 			1. No intersection: 0
				2. Tangent to sphere: 0
				3. Intersection from outside: -1
				4. Intersection from inside: 1 */
	double eMinusC[3];
	double center[3];
	vecCopy(3, iso->translation, center);
	vecSubtract(3, e, center, eMinusC);
	double a = vecDot(3, d, d);//pow(vecLength(3, d), 2)
	double b = 2 * vecDot(3, d, eMinusC);
	double c = vecDot(3, eMinusC, eMinusC) - (radius * radius);
	double discriminant = (b * b) - (4 * a * c);

	/* Pre-set intersected as 0. Handles Cases 1, 2, and any extraneous */
	result.intersected = 0;
	/* Case 3 */
	double negT = (-b - sqrt(discriminant)) / (2 * a);
	if (tStart <= negT && negT <= tEnd) {
		result.intersected = -1;
		result.t = negT;
	} else {
		/* Case 4 */
		double posT = (-b + sqrt(discriminant)) / (2 * a);
		if (tStart <= posT && posT <= tEnd) {
			result.intersected = 1;
			result.t = posT;
		}
	}
	return result;
}

void render(void) {
	double homog[4][4], screen[4], world[4], e[3], d[3], rgb[3];
	double tStart, tEnd;
	rayRecord recA, recB;
	int i, j;
	pixClearRGB(0.0, 0.0, 0.0);

	/* Get camera world position e and transformation from screen to world. */
	vecCopy(3, camera.isometry.translation, e);
	camWorldFromScreenHomogeneous(&camera, SCREENWIDTH, SCREENHEIGHT, homog);

	/* Each screen point is arbitrarily chosen on the near plane. */
	screen[2] = 0.0;
	screen[3] = 1.0;
	/* Orthographic camera. */
	//double eS[4];
	//eS[2] = 10.0;
	//eS[3] = 1.0;

	for (i = 0; i < SCREENWIDTH; i += 1) {
		screen[0] = i;
		//eS[0] = i;
		for (j = 0; j < SCREENHEIGHT; j += 1) {
			screen[1] = j;
			//eS[1] = j;

			/* Compute the direction d from the camera to the pixel. */
			double ppixel[4];
			mat441Multiply(homog, screen, ppixel);
			//mat441Multiply(homog, eS, e);
			/* Apply homogenous division. */
			for (int x = 0; x < 4; x++){
				ppixel[x] = ppixel[x] / ppixel[3];
				//e[x] = e[x] / e[3];
			}
			/* Calculate d. */
			vecSubtract(3, ppixel, e, d);

			/* Prepare to loop over all bodies. */
			tStart = EPSILON;
			tEnd = INFINITY;
			/* Test the first sphere. */
			recA = sphereIntersection(&isomA, radiusA, e, d, tStart, tEnd);
			if (recA.intersected) {
				tEnd = recA.t;
			}
			/* Test the second sphere. */
			recB = sphereIntersection(&isomB, radiusB, e, d, tStart, tEnd);
			if (recB.intersected) {
				tEnd = recB.t;
			}
			/* Choose the winner. */
			vec3Set(0.0, 0.0, 0.0, rgb);
			if (tEnd == recA.t) {
				vecCopy(3, colorA, rgb);
			} else if (tEnd == recB.t) {
				vecCopy(3, colorB, rgb);
			}
			pixSetRGB(i, j, rgb[0], rgb[1], rgb[2]);
		}
	}
}

/* User interface *************************************************************/

void handleKeyCamera(int key) {
	if (key == GLFW_KEY_W)
		cameraPhi -= 0.1;
	else if (key == GLFW_KEY_A)
		cameraTheta -= 0.1;
	else if (key == GLFW_KEY_S)
		cameraPhi += 0.1;
	else if (key == GLFW_KEY_D)
		cameraTheta += 0.1;
	else if (key == GLFW_KEY_E)
		cameraRho *= 0.9;
	else if (key == GLFW_KEY_C)
		cameraRho *= 1.1;
	camSetFrustum(&camera, M_PI / 6.0, cameraRho, 10.0, SCREENWIDTH,
		SCREENHEIGHT);
	camLookAt(&camera, cameraTarget, cameraRho, cameraPhi, cameraTheta);
}

void handleKeyAny(int key, int shiftIsDown, int controlIsDown,
		int altOptionIsDown, int superCommandIsDown) {
	if (key == GLFW_KEY_W || key == GLFW_KEY_A || key == GLFW_KEY_S ||
			key == GLFW_KEY_D || key == GLFW_KEY_E || key == GLFW_KEY_C)
		handleKeyCamera(key);
}

void handleKeyDown(int key, int shiftIsDown, int controlIsDown,
		int altOptionIsDown, int superCommandIsDown) {
	if (key == GLFW_KEY_Q)
		cameraMode = 1 - cameraMode;
	else
		handleKeyAny(key, shiftIsDown, controlIsDown, altOptionIsDown,
			superCommandIsDown);
}

void handleTimeStep(double oldTime, double newTime) {
	if (floor(newTime) - floor(oldTime) >= 1.0)
		printf("handleTimeStep: %f frames/s\n", 1.0 / (newTime - oldTime));
	render();
}

int main(void) {
	if (pixInitialize(SCREENWIDTH, SCREENHEIGHT, "Ray Tracing") != 0)
		return 1;
	else {
		/* Initialize the scene. */
		camSetProjectionType(&camera, camPERSPECTIVE);
		camSetFrustum(&camera, M_PI / 6.0, cameraRho, 10.0, SCREENWIDTH,
			SCREENHEIGHT);
		camLookAt(&camera, cameraTarget, cameraRho, cameraPhi, cameraTheta);
		double center[3] = {0.0, 0.0, 0.0}, axis[3] = {1.0, 0.0, 0.0}, r[3][3];
		mat33AngleAxisRotation(0.0, axis, r);
		isoSetTranslation(&isomA, center);
		isoSetRotation(&isomA, r);
		vec3Set(1.0, 0.0, 1.0, center);
		isoSetTranslation(&isomB, center);
		isoSetRotation(&isomB, r);
		/* Initialize and run the user interface. */
		pixSetKeyDownHandler(handleKeyDown);
		pixSetKeyRepeatHandler(handleKeyAny);
		pixSetKeyUpHandler(handleKeyAny);
		pixSetTimeStepHandler(handleTimeStep);
		pixRun();
		return 0;
	}
}
