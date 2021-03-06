/*
Dawson d'Almeida and Justin T. Washington
February 1 2018
CS311 with Josh Davis

*/


/* Describes an isometry as a rotation followed by a translation. Can be used
to describe the position and orientation of a rigid body. If the position is
the translation, and the columns of the rotation are the local coordinate axes
in global coordinates, then the isometry takes local coordinates to global. */

/* Feel free to read from, but not write to, this struct's members. */
typedef struct isoIsometry isoIsometry;
struct isoIsometry {
	GLdouble translation[3];
	GLdouble rotation[3][3];
};

/* Sets the rotation. */
void isoSetRotation(isoIsometry *iso, const GLdouble rot[3][3]) {
	vecCopy(9, (GLdouble *)rot, (GLdouble *)(iso->rotation));
}

/* Sets the translation. */
void isoSetTranslation(isoIsometry *iso, const GLdouble transl[3]) {
	vecCopy(3, transl, iso->translation);
}

/* Applies the rotation and translation to a point. The output CANNOT safely
alias the input. */
void isoTransformPoint(isoIsometry *iso, const GLdouble p[3], GLdouble isoP[3]) {
	mat331Multiply(iso->rotation, p, isoP);
	vecAdd(3, isoP, iso->translation, isoP);
}

/* Applies the inverse of the isometry to a point. If you transform a point and
then untransform the result, then you recover the original point. Similarly, if
you untransform a point and then transform the result, then you recover the
original point. The output CANNOT safely alias the input. */
void isoUntransformPoint(isoIsometry *iso, const GLdouble isoP[3], GLdouble p[3]) {
	//first undo the translation
	GLdouble isoPWithoutTranslation[3];
	vecSubtract(3, isoP, iso->translation, isoPWithoutTranslation);
	//second undo the rotation
	//because the rotation matrix is orthogonal, R^T = R^-1
	mat331TransposeMultiply(iso->rotation, isoPWithoutTranslation, p);
}

/* Applies the rotation to a vector. The output CANNOT safely alias the input.
*/
void isoRotateVector(isoIsometry *iso, const GLdouble v[3], GLdouble rotV[3]) {
	mat331Multiply(iso->rotation, v, rotV);
}

/* Applies the inverse rotation to a vector. The output CANNOT safely alias the
input. */
void isoUnrotateVector(const isoIsometry *iso, const GLdouble rotV[3],
		GLdouble v[3]) {
	mat331TransposeMultiply(iso->rotation, rotV, v);
}

/* Fills homog with the homogeneous version of the isometry. */
void isoGetHomogeneous(const isoIsometry *iso, GLdouble homog[4][4]) {
	mat44Isometry(iso->rotation, iso->translation, homog);
}

/* Fills homogInv with the homogeneous version of the inverse isometry. That is,
the product of this matrix and the one from isoGetHomogeneous is the identity
matrix. */
void isoGetInverseHomogeneous(const isoIsometry *iso, GLdouble homogInv[4][4]) {
	GLdouble rotTranspose[3][3];
	mat33Transpose(iso->rotation, rotTranspose);
	homogInv[0][0] = rotTranspose[0][0];
	homogInv[0][1] = rotTranspose[0][1];
	homogInv[0][2] = rotTranspose[0][2];
	homogInv[0][3] = -(rotTranspose[0][0]*iso->translation[0]) +
	                 -(rotTranspose[0][1]*iso->translation[1]) +
									 -(rotTranspose[0][2]*iso->translation[2]);

	homogInv[1][0] = rotTranspose[1][0];
	homogInv[1][1] = rotTranspose[1][1];
	homogInv[1][2] = rotTranspose[1][2];
	homogInv[1][3] = -(rotTranspose[1][0]*iso->translation[0]) +
	                 -(rotTranspose[1][1]*iso->translation[1]) +
									 -(rotTranspose[1][2]*iso->translation[2]);

	homogInv[2][0] = rotTranspose[2][0];
	homogInv[2][1] = rotTranspose[2][1];
	homogInv[2][2] = rotTranspose[2][2];
	homogInv[2][3] = -(rotTranspose[2][0]*iso->translation[0]) +
	                 -(rotTranspose[2][1]*iso->translation[1]) +
									 -(rotTranspose[2][2]*iso->translation[2]);

	homogInv[3][0] = 0;
	homogInv[3][1] = 0;
	homogInv[3][2] = 0;
	homogInv[3][3] = 1;
}
