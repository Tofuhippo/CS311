

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
	const rayResponse *response, int bodyNum, const void *bodies[],
	int lightNum, const void *lights[], const double cAmbient[3],
	int recursionNum, double rgb[3]) {
		vec3Set(0.0, 0.0, 0.0, rgb);
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

		lightResponse light;
		lightClass **class;
		rayResponse checkShadowRay, mirrorResponse;
		rayQuery shadowQuery, mirrorQuery;
		double lightContrib[3], mirrorContrib[3];
		for (int i = 0; i < lightNum; i++) {
			vec3Set(0.0, 0.0, 0.0, lightContrib);
			vec3Set(0.0, 0.0, 0.0, mirrorContrib);
			/* Check light type and get response. */
			class = (lightClass **)(lights[i]);
			light = (*class)->lighting(lights[i], xWorld);

			/* Cast a ray toward the light. Incorporate the light's contribution iff
			the ray does not intersect the scene. */
			shadowQuery.tStart = rayEPSILON;
			shadowQuery.tEnd = light.distance; //position of light - xWorld == len(dLight)
			vecCopy(3, light.dLight, shadowQuery.d);
			vecCopy(3, xWorld, shadowQuery.e);
			int inShadow = -1; //refers to the index of the body it insersects; -1 means no intersection
			checkShadowRay = rayIntersection(bodyNum, bodies, &shadowQuery, &inShadow);
			//remember that inShadow is updated via side effect

			/* Do lighting calculations in local coordinates. */
			double cSpec[3] = {0.5, 0.5, 0.5}, shininess = 16.0;
			double dNormalLocal[3], dLightLocal[3];
			vec3Set(0.0, 0.0, 1.0, dNormalLocal);  // Plane normal
			isoUnrotateVector(&(plane->isometry), light.dLight, dLightLocal);
			double pCameraLocal[3], dCameraLocal[3];
			isoUntransformPoint(&(plane->isometry), query->e, pCameraLocal);
			vecSubtract(3, pCameraLocal, xLocal, dCameraLocal);
			vecUnit(3, dCameraLocal, dCameraLocal);
			if (inShadow == -1) { //no shadow -> no intersection
				rayDiffuseAndSpecular(dNormalLocal, dLightLocal, dCameraLocal, cDiff,
					cSpec, shininess, light.cLight, lightContrib);
					vecAdd(3, rgb, lightContrib, rgb);
			}
			if (recursionNum > 0) {
				/* Get the reflected camera direction. */
				double twiceDot, dReflLocal[3], dRefl[3];
				twiceDot = 2.0 * vecDot(3, dNormalLocal, dCameraLocal);
				vecScale(3, twiceDot, dNormalLocal, dReflLocal);
				vecSubtract(3, dReflLocal, dCameraLocal, dReflLocal);
				isoRotateVector(&(plane->isometry), dReflLocal, dRefl);

				/* Set new mirror query. */
				vecCopy(3, dRefl, mirrorQuery.d);
				vecCopy(3, xWorld, mirrorQuery.e);
				mirrorQuery.tStart = rayEPSILON;
				mirrorQuery.tEnd = rayINFINITY;

				mirrorResponse = rayColor(bodyNum, bodies, lightNum, lights, cAmbient,
																	&mirrorQuery, recursionNum - 1, mirrorContrib);
				mirrorContrib[0] = mirrorContrib[0] * cSpec[0];
				mirrorContrib[1] = mirrorContrib[1] * cSpec[1];
				mirrorContrib[2] = mirrorContrib[2] * cSpec[2];
				vecAdd(3, rgb, mirrorContrib, rgb);
			}
		}
		/* Ambient light. */
		rgb[0] += cDiff[0] * cAmbient[0];
		rgb[1] += cDiff[1] * cAmbient[1];
		rgb[2] += cDiff[2] * cAmbient[2];
}

rayClass planeClass = {planeIntersection, planeColor};
