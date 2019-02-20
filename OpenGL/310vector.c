/*
Dawson d'Almeida and Justin T. Washington
February 15 2018
CS311 with Josh Davis

General vector functions for openGL.
*/

/*** In general dimensions ***/

/* Copies the dim-dimensional vector v to the dim-dimensional vector copy. The
output can safely alias the input. */
void vecCopy(int dim, const GLdouble v[], GLdouble copy[]) {
	int i;
	for (i = 0; i < dim; i += 1)
		copy[i] = v[i];
}

/* Adds the dim-dimensional vectors v and w. The output can safely alias the
input. */
void vecAdd(int dim, const GLdouble v[], const GLdouble w[], GLdouble vPlusW[]) {
	for (int i = 0; i < dim; i += 1){
		vPlusW[i] = v[i] + w[i];
	}
}

/* Subtracts the dim-dimensional vectors v and w. The output can safely alias
the input. */
void vecSubtract(int dim, const GLdouble v[], const GLdouble w[],
		GLdouble vMinusW[]) {
			for (int i = 0; i < dim; i += 1){
				vMinusW[i] = v[i] - w[i];
			}
}

/* Scales the dim-dimensional vector w by the number c. The output can safely
alias the input.*/
void vecScale(int dim, GLdouble c, const GLdouble w[], GLdouble cTimesW[]) {
	for (int i = 0; i < dim; i += 1){
		cTimesW[i] = c * w[i];
	}
}



/*** In specific dimensions ***/

/* By the way, there is a way to write a single vecSet function that works in
all dimensions. The module stdarg.h, which is part of the C standard library,
lets you write variable-arity functions. The general vecSet would look like
	void vecSet(int dim, GLdouble a[], ...)
where the '...' represents dim numbers to be loaded into a. We're not going to
take this approach for two reasons. First, I try not to burden you with
learning a lot of C that isn't strictly necessary. Second, the variable-arity
feature is a bit dangerous, in that it provides no type checking. */

/* Copies three numbers into a three-dimensional vector. */
void vec3Set(GLdouble a0, GLdouble a1, GLdouble a2, GLdouble a[3]) {
	a[0] = a0;
	a[1] = a1;
	a[2] = a2;
}

/* Copies four numbers into a four-dimensional vector. */
void vec4Set(GLdouble a0, GLdouble a1, GLdouble a2, GLdouble a3, GLdouble a[4]) {
	a[0] = a0;
	a[1] = a1;
	a[2] = a2;
	a[3] = a3;
}

/* Copies eight numbers into a eight-dimensional vector. */
void vec8Set(GLdouble a0, GLdouble a1, GLdouble a2, GLdouble a3, GLdouble a4, GLdouble a5,
		GLdouble a6, GLdouble a7, GLdouble a[8]) {
			a[0] = a0;
			a[1] = a1;
			a[2] = a2;
			a[3] = a3;
			a[4] = a4;
			a[5] = a5;
			a[6] = a6;
			a[7] = a7;
}

/**** Below added in conversion from 30 to 120 ****/

/* Returns the dot product of the vectors v and w. */
GLdouble vecDot(int dim, const GLdouble v[], const GLdouble w[]) {
	GLdouble dotProduct = 0;
	for (int i = 0; i < dim; i++){
		dotProduct += v[i] * w[i];
	}
	return dotProduct;
}

/* Returns the length of the vector v. */
GLdouble vecLength(int dim, const GLdouble v[]) {
	GLdouble squaredValues = vecDot(dim, v, v);
	return sqrt(squaredValues);
}

/* Returns the length of the vector v. If the length is non-zero, then also
places a normalized (length-1) version of v into unit. The output can safely
alias the input. */
GLdouble vecUnit(int dim, const GLdouble v[], GLdouble unit[]) {
	GLdouble vectorLength = vecLength(dim, v);
	if (vectorLength != 0){
		for (int i = 0; i < dim; i++){
			unit[i] = v[i] / vectorLength;
		}
	}
	return vectorLength;
}

/* Computes the cross product of v and w, and places it into vCrossW. The
output CANNOT safely alias the input. */
void vec3Cross(const GLdouble v[3], const GLdouble w[3], GLdouble vCrossW[3]) {
	vCrossW[0] = (v[1] * w[2]) - (v[2] * w[1]);
	vCrossW[1] = (v[2] * w[0]) - (v[0] * w[2]);
	vCrossW[2] = (v[0] * w[1]) - (v[1] * w[0]);
}

/* Computes the vector v from its spherical coordinates. rho >= 0.0 is the
radius. 0 <= phi <= pi is the co-latitude. -pi <= theta <= pi is the longitude
or azimuth. */
void vec3Spherical(GLdouble rho, GLdouble phi, GLdouble theta, GLdouble v[3]) {
	v[0] = rho * sin(phi) * cos(theta);
	v[1] = rho * sin(phi) * sin(theta);
	v[2] = rho * cos(phi);
}
