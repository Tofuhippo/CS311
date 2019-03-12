/*
Dawson d'Almeida and Justin T. Washington
March 8 2018
CS311 with Josh Davis

Main abstracted file that implements ray tracing with better ABSTRACTION IT'S
POLYMORPHISM BABY.
*/


/* On macOS, compile with...
    clang 700mainShadows.c 000pixel.o -lglfw -framework OpenGL
*/

#include <stdio.h>
#include <math.h>
#include <GLFW/glfw3.h>
#include "000pixel.h"

#include "610vector.c"
#include "140matrix.c"
#include "040texture.c"
#include "610isometry.c"
#include "600camera.c"
#include "700ray.c"
#include "680light.c"


#define SCREENWIDTH 512
#define SCREENHEIGHT 512
#define BODYNUM 6
#define LIGHTNUM 2

double cAmbient[3] = {0.2, 0.2, 0.2};

#include "700cylinder.c"
#include "700sphere.c"
#include "700plane.c"
#include "680omnidirectional.c"
#include "690spot.c"


camCamera camera;
double cameraTarget[3] = {0.2, 0.2, 0.2};
double cameraRho = 10.0, cameraPhi = M_PI / 3.0, cameraTheta = -M_PI / 3.0;
int cameraMode = 0;

/* Make textures. */
texTexture textureRed, textureGreen, textureBlue, textureNathan;
/* Make bodies. */
cylCylinder cylRed, cylGreen, cylBlue;
sphereSphere sphereSmall, sphereBig;
plaPlane plane;
const void *bodies[BODYNUM] = {&cylRed, &cylGreen, &cylBlue,
	                       &sphereSmall, &sphereBig, &plane};
/* Make lights. */
omniLight omLight;
spotLight spot;
const void *lights[LIGHTNUM] = {&omLight, &spot};


/* Rendering ******************************************************************/



void render(void) {
	double homog[4][4], world[4], rgb[3];
	double screen[4] = {0.0, 0.0, 0.0, 1.0};
	rayQuery query;
	rayResponse response;
	/* Compute the position e of the camera. */
	vecCopy(3, camera.isometry.translation, query.e);
	camWorldFromScreenHomogeneous(&camera, SCREENWIDTH, SCREENHEIGHT, homog);
	for (int i = 0; i < SCREENWIDTH; i += 1) {
		screen[0] = i;
		for (int j = 0; j < SCREENHEIGHT; j += 1) {
			screen[1] = j;
			/* Compute the direction d from the camera to the pixel. */
			mat441Multiply(homog, screen, world);
			vecScale(4, 1.0 / world[3], world, world);
			vecSubtract(3, world, query.e, query.d);
			/* Query the scene to find the intersection, if any. */
			query.tStart = rayEPSILON;
			query.tEnd = rayINFINITY;

			/* Revised version of depricated query Scene (new in 700) */
			int index = -1; // index of the body best intersection happens at (-1 = no intersection)
			response = rayIntersection(BODYNUM, bodies, &query, &index);
			/* Note that index is set via side effect (hence the &index) */

			/* Color the pixel. */
			rayClass **class;
			if (index >= 0) {
				class = (rayClass **)(bodies[index]);
				(*class)->color(bodies[index], &query, &response, BODYNUM, bodies,
					              LIGHTNUM, lights, cAmbient, rgb);
			} else
				vec3Set(0.0, 0.0, 0.0, rgb);
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
	else if (key == GLFW_KEY_Q)
		cameraRho *= 1.1;
	camSetFrustum(&camera, M_PI / 6.0, cameraRho, 10.0, SCREENWIDTH,
		SCREENHEIGHT);
	camLookAt(&camera, cameraTarget, cameraRho, cameraPhi, cameraTheta);
}


void handleKeyAny(int key, int shiftIsDown, int controlIsDown,
		int altOptionIsDown, int superCommandIsDown) {
	if (key == GLFW_KEY_W || key == GLFW_KEY_A || key == GLFW_KEY_S ||
			key == GLFW_KEY_D || key == GLFW_KEY_E || key == GLFW_KEY_Q)
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

void initializeCylinder(cylCylinder *cyl, double radius,
		const double rot[3][3], const double transl[3], texTexture *texture) {
	cyl->class = &cylClass;
	isoSetRotation(&(cyl->isometry), rot);
	isoSetTranslation(&(cyl->isometry), transl);
	cyl->radius = radius;
	cyl->texture = texture;
}

void initializeSphere(sphereSphere *sphere, double radius,
		const double rot[3][3], const double transl[3], texTexture *texture) {
	sphere->class = &sphereClass;
	isoSetRotation(&(sphere->isometry), rot);
	isoSetTranslation(&(sphere->isometry), transl);
	sphere->radius = radius;
	sphere->texture = texture;
}

void initializePlane(plaPlane *plane, const double rot[3][3],
		const double transl[3], texTexture *texture) {
	plane->class = &planeClass;
	isoSetRotation(&(plane->isometry), rot);
	isoSetTranslation(&(plane->isometry), transl);
	plane->texture = texture;
}

void initializeOmniLight(omniLight *omLight, const double pLight[3],
		const double cLight[3]) {
	omLight->class = &omniClass;
	vecCopy(3, pLight, omLight->pLight);
	vecCopy(3, cLight, omLight->cLight);
}

void initializeSpotLight(spotLight *spot, const double pLight[3],
		const double cLight[3], const double spotAngle, const double dSpot[3]) {
	spot->class = &spotClass;
	vecCopy(3, pLight, spot->pLight);
	vecCopy(3, cLight, spot->cLight);
	spot->spotAngle = spotAngle;
	vecUnit(3, dSpot, spot->dSpot);
	vecScale(3, -1, spot->dSpot, spot->dSpot);
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
		double rgb[3] = {1.0, 0.0, 0.0};
		if (texInitializeSolid(&textureRed, 4, 4, 3, rgb) != 0)
			return 2;
		vec3Set(0.0, 1.0, 0.0, rgb);
		if (texInitializeSolid(&textureGreen, 4, 4, 3, rgb) != 0)
			return 3;
		vec3Set(0.0, 0.0, 1.0, rgb);
		if (texInitializeSolid(&textureBlue, 4, 4, 3, rgb) != 0)
			return 4;
		if (texInitializeFile(&textureNathan, "nathan_mannes.jpg") != 0)
			return 5;
		/* Initialize Bodies. */
		double center[3] = {0.0, 0.0, 0.0}, rot[3][3];
		double axis[3] = {1.0 / sqrt(3.0), 1.0 / sqrt(3.0), 1.0 / sqrt(3.0)};
		mat33AngleAxisRotation(2.0 * M_PI / 3.0, axis, rot);
		initializeCylinder(&cylRed, 0.1, rot, center, &textureRed);
		mat33AngleAxisRotation(-2.0 * M_PI / 3.0, axis, rot);
		initializeCylinder(&cylGreen, 0.1, rot, center, &textureGreen);
		mat33AngleAxisRotation(0.0, axis, rot);
		initializeCylinder(&cylBlue, 0.1, rot, center, &textureBlue);

		double centerBigSphere[3] = {4.0, 1.0, 1.0};
		double centerSmallSphere[3] = {2.0, 5.0, 0.0};
		initializeSphere(&sphereBig, 1.5, rot, centerBigSphere, &textureNathan);
		initializeSphere(&sphereSmall, 1.0, rot, centerSmallSphere, &textureNathan);

		initializePlane(&plane, rot, center, &textureNathan);

		double pLightOmni[3] = {1.0, 1.0, 5.0};
		double cLightOmni[3] = {1.0, 0.0, 0.0};
		initializeOmniLight(&omLight, pLightOmni, cLightOmni);

		double pLightSpot[3] = {-5.0, -1.0, 4.0};
		double cLightSpot[3] = {0.0, 1.0, 1.0};
		double spotAngle = 0.8;
		double dSpot[3] = {5.0, 1.0, -4.0}; // doesn't need to be declared unit (see initilaization)
		initializeSpotLight(&spot, pLightSpot, cLightSpot, spotAngle, dSpot);

		/* Initialize and run the user interface. */
		pixSetKeyDownHandler(handleKeyDown);
		pixSetKeyRepeatHandler(handleKeyAny);
		pixSetKeyUpHandler(handleKeyAny);
		pixSetTimeStepHandler(handleTimeStep);
		pixRun();
		/* Clean up. */
		texDestroy(&textureRed);
		texDestroy(&textureGreen);
		texDestroy(&textureBlue);
		texDestroy(&textureNathan);
		return 0;
	}
}
