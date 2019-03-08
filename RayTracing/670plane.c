


/* The cylinder is infinitely long. It is centered on its local z-axis and has
the prescribed radius. Its class should be set to cylClass. */
typedef struct plaPlane plaPlane;
struct plaPlane {
	rayClass *class;
	isoIsometry isometry;
	texTexture *texture;
};

rayResponse planeIntersection(const void *body, const rayQuery *query) {
	const plaPlane *plane = (const plaPlane *)body;
	rayResponse result;
	/* Transform e, d to local coords. */
	double eLocal[3], dLocal[3];
	isoUnrotateVector(&(plane->isometry), query->d, dLocal);
	isoUntransformPoint(&(plane->isometry), query->e, eLocal);
	if (dLocal[2] == 0) {
		result.intersected = 0;
		result.t = INFINITY;
		return result;
	}
	double t = -eLocal[2]/dLocal[2];
	if (query->tStart <= t && t <= query->tEnd) {
		if (dLocal[2] > 0) { // intersecting from below
			result.intersected = 1;
			result.t = t;
			return result;
		}
		if (dLocal[2] < 0) {// intersecting from above
			result.intersected = -1;
			result.t = t;
			return result;
		}
	}
	result.intersected = 0;
	return result;
}

void planeTexCoords(const double xLocal[3], double st[2]) {
	st[0] = xLocal[0]/2 - floor(xLocal[0]/2);
	st[1] = xLocal[1]/2 - floor(xLocal[1]/2);
}

void planeColor(const void *body, const rayQuery *query,
		const rayResponse *response, double rgb[3]) {
	const plaPlane *plane = (const plaPlane *)body;
	/* x = e + t d. */
	double xWorld[3], xLocal[3];
	vecScale(3, query->tEnd, query->d, xWorld);
	vecAdd(3, query->e, xWorld, xWorld);
	isoUntransformPoint(&(plane->isometry), xWorld, xLocal);
	/* Sample texture to get diffuse surface color. */
	double texCoords[2];
	planeTexCoords(xLocal, texCoords);
	double cDiff[plane->texture->texelDim];
	texSample(plane->texture, texCoords[0], texCoords[1], cDiff);


	double cSpec[3] = {0.5, 0.5, 0.5}, shininess = 16.0;
	/* Do lighting calculations in world coordinates. */
	double dNormalLocal[3] = {0.0, 0.0, 1.0};
	double dNormal[3];
	isoRotateVector(&(plane->isometry), dNormalLocal, dNormal);
	vecUnit(3, dNormal, dNormal); // Got dNormal (world)
	double dLight[3];
	vecScale(3, -1, dLight, dLight); // Get direction TOWARDS the light, World
	vecUnit(3, dLight, dLight); // Normalize dLight
	double pCamera[3], dCamera[3];
	vecCopy(3, query->e, pCamera);
	vecSubtract(3, pCamera, xWorld, dCamera); // Got dCamera (world)
	vecUnit(3, dCamera, dCamera);

	diffuseAndSpecular(dNormal, dLight, dCamera, cDiff,
		cSpec, shininess, cLight, rgb);

	/* Ambient light. */
	rgb[0] += 4 * cDiff[0] * cAmbient[0];
	rgb[1] += 4 * cDiff[1] * cAmbient[1];
	rgb[2] += 4 * cDiff[2] * cAmbient[2];
}

rayClass planeClass = {planeIntersection, planeColor};
