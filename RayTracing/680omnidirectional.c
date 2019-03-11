


/* A simple omnidirectional light. No attenuation. */
typedef struct omniLight omniLight;
struct omniLight {
	lightClass *class;
	double pLight[3];
	double cLight[3];
};

lightResponse omniLighting(const void *light, const double world[3]) {
	const omniLight *omni = (const omniLight *)light;
	lightResponse result;
	vecCopy(3, omni->cLight, result.cLight);
	vecSubtract(3, omni->pLight, world, result.dLight);
	result.distance = vecLength(3, result.dLight);
	vecScale(3, 1.0 / result.distance, result.dLight, result.dLight);
	return result;
}

lightClass omniClass = {omniLighting};
