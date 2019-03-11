


typedef struct lightResponse lightResponse;
struct lightResponse {
	/* cLight is the color of the light that arrives at a specified point in
	the world. It may be as simple as a constant light color. In more
	complicated examples, it can incorporate spot light angle, attenuation,
	light mapping, etc. */
	double cLight[3];
	/* dLight is the unit direction from the specified point toward the light.
	This quantity is typically used in lighting calculations, including shadow
	casting. */
	double dLight[3];
	/* distance is the distance from the specified point to the light. It can
	be useful for setting tEnd in shadow rays. It can be rayINFINITY to signal
	a distant, directional light source. */
	double distance;
};

typedef struct lightClass lightClass;
struct lightClass {
	/* Returns the lighting data for a body point at the given world
	coordinates. */
	lightResponse (*lighting)(const void *light, const double world[3]);
};
