


/* The cylinder is infinitely long. It is centered on its local z-axis and has
the prescribed radius. Its class should be set to cylClass. */
typedef struct cylCylinder cylCylinder;
struct cylCylinder {
	rayClass *class;
	isoIsometry isometry;
	double radius;
	texTexture *texture;
};

rayResponse cylIntersection(const void *body, const rayQuery *query) {
	const cylCylinder *cyl = (const cylCylinder *)body;
	rayResponse result;
	/* Transform to local coordinates. */
	double eLocal[3], dLocal[3];
	isoUntransformPoint(&(cyl->isometry), query->e, eLocal);
	isoUnrotateVector(&(cyl->isometry), query->d, dLocal);
	/* Ignore the third dimension. */
	double eE, dE, dD, rSq, disc, t;
	eE = vecDot(2, eLocal, eLocal);
	dE = vecDot(2, dLocal, eLocal);
	dD = vecDot(2, dLocal, dLocal);
	disc = dE * dE - dD * (eE - cyl->radius * cyl->radius);
	if (disc <= 0) {
		result.intersected = 0;
		return result;
	}
	double sqrtDisc = sqrt(disc);
	t = (-dE - sqrtDisc) / dD;
	if (query->tStart <= t && t <= query->tEnd) {
		result.intersected = -1;
		result.t = t;
		return result;
	}
	t = (-dE + sqrtDisc) / dD;
	if (query->tStart <= t && t <= query->tEnd) {
		result.intersected = 1;
		result.t = t;
		return result;
	}
	result.intersected = 0;
	return result;
}

void cylTexCoords(const double xLocal[3], double st[2]) {
	/* Simply use cylindrical coordinates as texture coordinates. */
	double rho, phi, theta;
	st[0] = atan2(xLocal[1], xLocal[0]);
	if (st[0] < 0.0)
		st[0] += 2.0 * M_PI;
	st[0] = st[0] / (2.0 * M_PI);
	st[1] = xLocal[2];
}

void cylColor(const void *body, const rayQuery *query,
	const rayResponse *response, int bodyNum, const void *bodies[],
	int lightNum, const void *lights[], const double cAmbient[3],
	double rgb[3]) {
	const cylCylinder *cyl = (const cylCylinder *)body;
	/* x = e + t d. */
	double xWorld[3], xLocal[3];
	vecScale(3, query->tEnd, query->d, xWorld);
	vecAdd(3, query->e, xWorld, xWorld);
	isoUntransformPoint(&(cyl->isometry), xWorld, xLocal);
	/* Sample texture to get diffuse surface color. */
	double texCoords[2];
	cylTexCoords(xLocal, texCoords);
	double cDiff[cyl->texture->texelDim];
	texSample(cyl->texture, texCoords[0], texCoords[1], cDiff);

	lightResponse light;
	lightClass **class;
	double lightContrib[3];
	for (int i = 0; i < lightNum; i++) {
		vec3Set(0.0, 0.0, 0.0, lightContrib);
		/* Check light type and get response. */
		class = (lightClass **)(lights[i]);
		light = (*class)->lighting(lights[i], xWorld);

		/* Do lighting calculations in local coordinates. */
		double cSpec[3] = {0.5, 0.5, 0.5}, shininess = 16.0;
		double dNormalLocal[3], dLightLocal[3];
		vecUnit(2, xLocal, dNormalLocal);
		dNormalLocal[2] = 0.0;
		isoUnrotateVector(&(cyl->isometry), light.dLight, dLightLocal);
		double pCameraLocal[3], dCameraLocal[3];
		isoUntransformPoint(&(cyl->isometry), query->e, pCameraLocal);
		vecSubtract(3, pCameraLocal, xLocal, dCameraLocal);
		vecUnit(3, dCameraLocal, dCameraLocal);
		rayDiffuseAndSpecular(dNormalLocal, dLightLocal, dCameraLocal, cDiff,
			cSpec, shininess, light.cLight, rgb);
		vecAdd(3, rgb, lightContrib, rgb);
	}
	/* Ambient light. */
	rgb[0] += cDiff[0] * cAmbient[0];
	rgb[1] += cDiff[1] * cAmbient[1];
	rgb[2] += cDiff[2] * cAmbient[2];
}

rayClass cylClass = {cylIntersection, cylColor};
