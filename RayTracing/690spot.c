


/* A simple spot light. No attenuation. */
typedef struct spotLight spotLight;
struct spotLight {
	lightClass *class;
	double pLight[3];
	double cLight[3];
	double spotAngle;
	double dSpot[3];
};

lightResponse spotLighting(const void *light, const double world[3]) {
	spotLight *spot = (spotLight *)light;
	lightResponse result;
	vec3Set(0.0, 0.0, 0.0, result.cLight);
	vecSubtract(3, spot->pLight, world, result.dLight);
	result.distance = vecLength(3, result.dLight);
	//vecScale(3, 1.0 / result.distance, result.dLight, result.dLight);
	vecUnit(3, result.dLight, result.dLight);

	/* Algorithm for Spotlighting */
	double maxAngle = cos(spot->spotAngle/2);
	double lightSpotAngle = vecDot(3, result.dLight, spot->dSpot);
	if (lightSpotAngle >= maxAngle){
		vecCopy(3, spot->cLight, result.cLight);
	}
	return result;
}

lightClass spotClass = {spotLighting};
