


#define rayEPSILON 0.00000001
#define rayINFINITY 100000000

typedef struct rayQuery rayQuery;
struct rayQuery {
	/* The ray is parametrized as x(t) = e + t d for t in [tStart, tEnd]. The
	direction d is non-zero but not necessarily unit. */
	double e[3], d[3], tStart, tEnd;
};

typedef struct rayResponse rayResponse;
struct rayResponse {
	/* 0 for no intersection (or just tangency), -1 for entering the body, or 1
	for exiting the body. */
	int intersected;
	/* If the intersection code is non-zero, then the t member contains the
	time of intersection. */
	double t;
};

typedef struct rayClass rayClass;
struct rayClass {
	/* If the ray e + t d intersects the body for some t in [tStart, tEnd],
	then returns the least such t, along with an intersection code: -1 for
	entering the body and 1 for exiting it. If the ray does not intersect the
	body in [tStart, tEnd] (or is tangent to it), then returns an intersection
	code of 0. */
	rayResponse (*intersection)(const void *body, const rayQuery *query);
	/* Fills the rgb argument with the color of the body at a certain
	intersection point. The query that produced the intersection and the body's
	reponse to that query are included in the parameters. */
	void (*color)(const void *body, const rayQuery *query,
		const rayResponse *response, int bodyNum, const void *bodies[],
		int lightNum, const void *lights[], const double cAmbient[3],
		int recursionNum, double rgb[3]);
};

/* Directions can be local or global, as long as they're consistent. */
void rayDiffuseAndSpecular(const double dNormal[3], const double dLight[3],
		const double dCamera[3], const double cDiff[3], const double cSpec[3],
		double shininess, const double cLight[3], double rgb[3]) {
	/* Diffuse reflection. */
	double diffuse[3];
	double iDiff = vecDot(3, dNormal, dLight);
	if (iDiff <= 0.0)
		iDiff = 0.0;
	diffuse[0] = iDiff * cDiff[0] * cLight[0];
	diffuse[1] = iDiff * cDiff[1] * cLight[1];
	diffuse[2] = iDiff * cDiff[2] * cLight[2];
	/* Specular reflection. dRefl = 2 (dCam . dNorm) dNorm - dCam. */
	double twiceDot, dRefl[3];
	twiceDot = 2.0 * vecDot(3, dNormal, dCamera);
	vecScale(3, twiceDot, dNormal, dRefl);
	vecSubtract(3, dRefl, dCamera, dRefl);
	double iSpec = vecDot(3, dRefl, dLight);
	if (iDiff <= 0.0 || iSpec <= 0.0)
		iSpec = 0.0;
	iSpec = pow(iSpec, shininess);
	double specular[3];
	specular[0] = iSpec * cSpec[0] * cLight[0];
	specular[1] = iSpec * cSpec[1] * cLight[1];
	specular[2] = iSpec * cSpec[2] * cLight[2];
	/* Output. */
	vecAdd(3, diffuse, specular, rgb);
}

/* If the ray does not intersect the scene, then sets index to -1. If the ray
intersects the scene, then updates query->tEnd, outputs the index of the body
in the list of bodies, and returns the corresponding rayResponse. */
rayResponse rayIntersection(int bodyNum, const void *bodies[], rayQuery *query,
		int *index) {
			rayResponse candidate;
			rayClass **class;
			for (int k = 0; k < bodyNum; k += 1) {
				class = (rayClass **)(bodies[k]);
				candidate = (*class)->intersection(bodies[k], query);
				if (candidate.intersected) {
					query->tEnd = candidate.t;
					*index = k;
				}
			}
			return candidate;
}

/* Outputs the RGB color of the specified ray. If the ray hits nothing in the
scene, then outputs some fixed background color. Also returns the response to
the given query. */
rayResponse rayColor(int bodyNum, const void *bodies[], int lightNum,
		const void *lights[], const double cAmbient[3], rayQuery *query,
		int recursionNum, double rgb[3]) {
			/* Find body intersection. */
			int index = -1; // index of the body best intersection happens at (-1 = no intersection)
			rayResponse response = rayIntersection(bodyNum, bodies, query, &index);
			/* Note that index is set via side effect (hence the &index) */

			/* Set rgb. */
			rayClass **class;
			if (index >= 0) {
				class = (rayClass **)(bodies[index]);
				(*class)->color(bodies[index], query, &response, bodyNum, bodies,
					              lightNum, lights, cAmbient, recursionNum, rgb);
			} else
				vec3Set(0.0, 0.0, 0.0, rgb);

			return response;
}
