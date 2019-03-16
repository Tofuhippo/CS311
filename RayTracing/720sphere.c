

typedef struct sphereSphere sphereSphere;
struct sphereSphere {
	rayClass *class;
	isoIsometry isometry;
	double radius;
	texTexture *texture;
};

rayResponse sphereIntersection(const void *body, const rayQuery *query) {
	const sphereSphere *sphere = (const sphereSphere *)body;
	rayResponse result;
	/* Four Cases to test intersection:
	 			1. No intersection: 0
				2. Tangent to sphere: 0
				3. Intersection from outside: -1
				4. Intersection from inside: 1 */
	double eMinusC[3];
	double center[3];
	vecCopy(3, sphere->isometry.translation, center);
	vecSubtract(3, query->e, center, eMinusC);
	double a = vecDot(3, query->d, query->d);//pow(vecLength(3, d), 2)
	double b = 2 * vecDot(3, query->d, eMinusC);
	double c = vecDot(3, eMinusC, eMinusC) - (sphere->radius * sphere->radius);
	double discriminant = (b * b) - (4 * a * c);

	/* Pre-set intersected as 0. Handles Cases 1, 2, and any extraneous */
	result.intersected = 0;
	/* Case 3 */
	double negT = (-b - sqrt(discriminant)) / (2 * a);
	if (query->tStart <= negT && negT <= query->tEnd) {
		result.intersected = -1;
		result.t = negT;
	} else {
		/* Case 4 */
		double posT = (-b + sqrt(discriminant)) / (2 * a);
		if (query->tStart <= posT && posT <= query->tEnd) {
			result.intersected = 1;
			result.t = posT;
		}
	}
	return result;
}

void sphereTexCoords(const double xLocal[3], double st[2], double radius) {
	double rho, phi, theta;
	vec3Rectangular(xLocal, &radius, &phi, &theta);
	st[0] = phi/M_PI;
	st[1] = theta/(2*M_PI);
}

void sphereColor(const void *body, const rayQuery *query,
	const rayResponse *response, int bodyNum, const void *bodies[],
	int lightNum, const void *lights[], const double cAmbient[3],
	int recursionNum, double rgb[3]) {
		vec3Set(0.0, 0.0, 0.0, rgb);
		const sphereSphere *sphere = (const sphereSphere *)body;
		/* x = e + t d. */
		double xWorld[3], xLocal[3];
		vecScale(3, query->tEnd, query->d, xWorld);
		vecAdd(3, query->e, xWorld, xWorld);
		isoUntransformPoint(&(sphere->isometry), xWorld, xLocal);
		/* Sample texture to get diffuse surface color. */
		double texCoords[2];
		sphereTexCoords(xLocal, texCoords, sphere->radius);
		double cDiff[sphere->texture->texelDim];
		texSample(sphere->texture, texCoords[0], texCoords[1], cDiff);

		lightResponse light;
		lightClass **class;
		rayResponse checkShadowRay, mirrorResponse, transResponse;
		rayQuery shadowQuery, mirrorQuery, transQuery;
		double lightContrib[3], mirrorContrib[3], transContrib[3];
		for (int i = 0; i < lightNum; i++) {
			vec3Set(0.0, 0.0, 0.0, lightContrib);
			vec3Set(0.0, 0.0, 0.0, mirrorContrib);
			vec3Set(0.0, 0.0, 0.0, transContrib);
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
			// remember that inShadow is updated via side effect

			/* Do lighting calculations in local coordinates. */
			double cSpec[3] = {0.1, 0.1, 0.1}, shininess = 16.0;
			double cTrans[3] = {0.5, 0.5, 0.5};
			double dNormalLocal[3], dLightLocal[3];
			vecUnit(3, xLocal, dNormalLocal);
			isoUnrotateVector(&(sphere->isometry), light.dLight, dLightLocal);
			double pCameraLocal[3], dCameraLocal[3];
			isoUntransformPoint(&(sphere->isometry), query->e, pCameraLocal);
			vecSubtract(3, pCameraLocal, xLocal, dCameraLocal);
			vecUnit(3, dCameraLocal, dCameraLocal);
			if (inShadow == -1) { //no shadow -> no intersection
				rayDiffuseAndSpecular(dNormalLocal, dLightLocal, dCameraLocal, cDiff,
					cSpec, shininess, light.cLight, lightContrib);
				vecAdd(3, rgb, lightContrib, rgb);
			}
			if (recursionNum > 0) {
				/* Do reflection. */
				/* Get the reflected camera direction. */
				double twiceDot, dReflLocal[3], dRefl[3];
				twiceDot = 2.0 * vecDot(3, dNormalLocal, dCameraLocal);
				vecScale(3, twiceDot, dNormalLocal, dReflLocal);
				vecSubtract(3, dReflLocal, dCameraLocal, dReflLocal);
				isoRotateVector(&(sphere->isometry), dReflLocal, dRefl);

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

				/* Do refraction. */
				double dRefrLocal[3], dRefr[3];
				double indexInc = 1.0, indexRefr = 1.3;
				if (response->intersected == 1) { // If we are leaving sphere to air
					indexInc = 1.3;
					indexRefr = 1.0;
				}
				rayRefraction(dNormalLocal, indexInc, dCameraLocal, indexRefr,
					            dRefrLocal);
				isoRotateVector(&(sphere->isometry), dRefrLocal, dRefr); //World

				/* Set new refraction query. */
				vecCopy(3, dRefr, transQuery.d);
				vecCopy(3, xWorld, transQuery.e);
				transQuery.tStart = rayEPSILON;
				transQuery.tEnd = rayINFINITY;
				transResponse = rayColor(bodyNum, bodies, lightNum, lights, cAmbient,
																	&transQuery, recursionNum - 1, transContrib);
				transContrib[0] = transContrib[0] * cTrans[0];
				transContrib[1] = transContrib[1] * cTrans[1];
				transContrib[2] = transContrib[2] * cTrans[2];
				vecAdd(3, rgb, transContrib, rgb);
			}
		}
		/* Ambient light. */
		rgb[0] += cDiff[0] * cAmbient[0];
		rgb[1] += cDiff[1] * cAmbient[1];
		rgb[2] += cDiff[2] * cAmbient[2];
}

rayClass sphereClass = {sphereIntersection, sphereColor};
