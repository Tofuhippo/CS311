/*
Dawson d'Almeida and Justin T. Washington
March 4 2018
CS311 with Josh Davis

Main abstracted file that implements ray tracing.
*/

/* On macOS, compile with...
    clang 630mainShadows.c 000pixel.o -lglfw -framework OpenGL
*/

#include <stdio.h>
#include <math.h>
#include <stdarg.h>
#include "000pixel.h"
#include <GLFW/glfw3.h>

#include "610vector.c"
#include "140matrix.c"
#include "610isometry.c"
#include "600camera.c"
#include "040texture.c"

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

/* Light stuff. */
// Diffuse
double dLight[3] = {10.0, 10.0, 0.0}; // World coords
double cLight[3] = {1.0, 1.0, 1.0};
// Specular
double shininess = 10.0;
double cSpec[3] = {1.0, 1.0, 1.0};
// Ambient
double cAmbient[3] = {0.2, 0.2, 0.2};


/* Initialize texture. */
texTexture tex;

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

/* Fills the RGB color with the color sampled from the specified texture. */
void sphereColor(const isoIsometry *isom, double radius, const double e[3],
		const double d[3], double tEnd, const texTexture *tex, double rgb[3]) {
			double xWorld[3], xLocal[3];
			double dScaled[3];
			vecScale(3, tEnd, d, dScaled);
			vecAdd(3, e, dScaled, xWorld);
			isoUntransformPoint(isom, xWorld, xLocal);

			double rho, phi, theta;
			vec3Rectangular(xLocal, &radius, &phi, &theta);

			double cDiff[3]; // get cDiff from textures
			texSample(tex, phi/M_PI, theta/(2*M_PI), cDiff);

			/* Apply Lighting AND Shadowing. */
			double dNormal[3];
			vecSubtract(3, xWorld, isom->translation, dNormal);
			vecUnit(3, dNormal, dNormal); // Got dNormal (world)
			vecUnit(3, dLight, dLight); // Make dLight unit lol (world)
			double iDiff = vecDot(3, dLight, dNormal);
			if (iDiff < 0) {
				iDiff = 0;
			}

			/* Shadowing. If the normal is the same direction as dlight +- 90, there
			is no diffuse, and therefore no need to test shadowing */
			double shadowed = 0; // True = 1/False = 0
			double dtoLight[3];
			double tStartShadow = EPSILON;
			double tEndShadow = INFINITY;
			vecScale(3, -1, dLight, dtoLight); // Get direction TOWARDS the light
			/* Only send ray for shadowing if iDiff > 0. */
			if (iDiff > 0) {
				// Send new ray towards light. e is now xWorld, d is now dtoLight
			}

			/* Apply diffuse and specular IFF iDiff > 0 because we set iSpec to 0
			if iDiff <= 0. */
			if (iDiff > 0 && shadowed) {
				/* Add diffuse lighting. */
				rgb[0] = iDiff * cDiff[0] * cLight[0];
				rgb[1] = iDiff * cDiff[1] * cLight[1];
				rgb[2] = iDiff * cDiff[2] * cLight[2];

				/* Now do specular reflection. */
				double dCamera[3];
				vecSubtract(3, camera.isometry.translation, xWorld, dCamera); // Got dCamera (world)
				double dRefl[3], dReflMidStep[3]; // Reflection of dCamera across dNormal
				double dReflMultiplier = 2 * vecDot(3, dCamera, dNormal);
				vecScale(3, dReflMultiplier, dNormal, dReflMidStep);
				vecSubtract(3, dReflMidStep, dCamera, dRefl);
				vecUnit(3, dRefl, dRefl); // Got dRefl
				double iSpec = vecDot(3, dRefl, dLight);
				if (iSpec < 0) {
					iSpec = 0;
				}
				iSpec = pow(iSpec, shininess);

				/* Add specular lighting. */
				rgb[0] += iSpec * cSpec[0] * cLight[0];
				rgb[1] += iSpec * cSpec[1] * cLight[1];
				rgb[2] += iSpec * cSpec[2] * cLight[2];
			}
			rgb[0] += cDiff[0] * cAmbient[0];
			rgb[1] += cDiff[1] * cAmbient[1];
			rgb[2] += cDiff[2] * cAmbient[2];
			//printf("RGB[0] %f, RGB[1] %f, RGB[2] %f\n", rgb[0], rgb[1], rgb[2]);
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
	for (i = 0; i < SCREENWIDTH; i += 1) {
		screen[0] = i;
		for (j = 0; j < SCREENHEIGHT; j += 1) {
			screen[1] = j;

			/* Compute the direction d from the camera to the pixel. */
			double ppixel[4];
			mat441Multiply(homog, screen, ppixel);
			/* Apply homogenous division. */
			for (int x = 0; x < 4; x++){
				ppixel[x] = ppixel[x] / ppixel[3];
			}
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
			vec3Set(0.0, 0.0, 0.0, rgb); // rgb is set to 0, 0, 0
			if (tEnd == recA.t) {
				sphereColor(&isomA, radiusA, e, d, tEnd, &tex, rgb);
				//vecCopy(3, colorA, rgb);
			} else if (tEnd == recB.t) {
				sphereColor(&isomB, radiusB, e, d, tEnd, &tex, rgb);
				//vecCopy(3, colorB, rgb);
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
	else if (key == GLFW_KEY_1)
		dLight[0] += 0.1;
	else if (key == GLFW_KEY_2)
		dLight[0] -= 0.1;
	camSetFrustum(&camera, M_PI / 6.0, cameraRho, 10.0, SCREENWIDTH,
		SCREENHEIGHT);
	camLookAt(&camera, cameraTarget, cameraRho, cameraPhi, cameraTheta);
}

void handleKeyAny(int key, int shiftIsDown, int controlIsDown,
		int altOptionIsDown, int superCommandIsDown) {
	if (key == GLFW_KEY_W || key == GLFW_KEY_A || key == GLFW_KEY_S ||
			key == GLFW_KEY_D || key == GLFW_KEY_E || key == GLFW_KEY_C ||
			key == GLFW_KEY_1 || key == GLFW_KEY_2)
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
		texInitializeFile(&tex, "nathan_mannes.jpg");
		pixRun();
		return 0;
	}
}
