


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
		const rayResponse *response, double rgb[3]);
};
