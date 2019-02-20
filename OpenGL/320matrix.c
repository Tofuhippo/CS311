/*
Dawson d'Almeida and Justin T. Washington
February 1 2018
CS311 with Josh Davis

General matrix functions.
*/

/*** 2 x 2 Matrices ***/

/* Pretty-prints the given matrix, with one line of text per row of matrix. */
void mat22Print(const GLdouble m[2][2]) {
	int i, j;
	for (i = 0; i < 2; i += 1) {
		for (j = 0; j < 2; j += 1)
			printf("%f    ", m[i][j]);
		printf("\n");
	}
}

/* Returns the determinant of the matrix m. If the determinant is 0.0, then the
matrix is not invertible, and mInv is untouched. If the determinant is not 0.0,
then the matrix is invertible, and its inverse is placed into mInv. The output
CANNOT safely alias the input. */
GLdouble mat22Invert(const GLdouble m[2][2], GLdouble mInv[2][2]) {
	GLdouble a = m[0][0];
	GLdouble b = m[0][1];
	GLdouble c = m[1][0];
	GLdouble d = m[1][1];
	GLdouble det = (a*d) - (b*c);
	if(det != 0){
		mInv[0][0] = d/det;
		mInv[0][1] = -b/det;
		mInv[1][0] = -c/det;
		mInv[1][1] = a/det;
	}
	return det;
}

/* Multiplies a 2x2 matrix m by a 2-column v, storing the result in mTimesV.
The output CANNOT safely alias the input. */
void mat221Multiply(const GLdouble m[2][2], const GLdouble v[2],
		GLdouble mTimesV[2]) {
			mTimesV[0] = m[0][0]*v[0] + m[0][1]*v[1];
			mTimesV[1] = m[1][0]*v[0] + m[1][1]*v[1];
}

/* Fills the matrix m from its two columns. The output CANNOT safely alias the
input. */
void mat22Columns(const GLdouble col0[2], const GLdouble col1[2], GLdouble m[2][2]) {
	m[0][0] = col0[0];
	m[1][0] = col0[1];
	m[0][1] = col1[0];
	m[1][1] = col1[1];
}

/* The theta parameter is an angle in radians. Sets the matrix m to the
rotation matrix corresponding to counterclockwise rotation of the plane through
the angle theta. */
void mat22Rotation(GLdouble theta, GLdouble m[2][2]) {

	GLdouble rotationMatrix[2][2] = {{cos(theta), -sin(theta)},
	                               {sin(theta), cos(theta)}};

	// Split into columns of m
	GLdouble col0[2] = {m[0][0], m[1][0]};
	GLdouble col1[2] = {m[1][0], m[1][1]};

	// Rotate the columns of m
	mat221Multiply(rotationMatrix, col0, col0);
	mat221Multiply(rotationMatrix, col1, col1);

	// Recombine the columns into m
	mat22Columns(col0, col1, m);

}

/* Multiplies the 3x3 matrix m by the 3x3 matrix n. The output CANNOT safely
alias the input. */
void mat333Multiply(const GLdouble m[3][3], const GLdouble n[3][3],
		GLdouble mTimesN[3][3]) {
			for (int colN = 0; colN < 3; colN++){
				for (int rowM = 0; rowM < 3; rowM++){
					mTimesN[rowM][colN] = m[rowM][0] * n[0][colN] +
					                      m[rowM][1] * n[1][colN] +
																m[rowM][2] * n[2][colN];
				}
			}
		}

/* Multiplies the 3x3 matrix m by the 3x1 matrix v. The output CANNOT safely
alias the input. */
void mat331Multiply(const GLdouble m[3][3], const GLdouble v[3],
		GLdouble mTimesV[3]) {
			for (int rowM = 0; rowM < 3; rowM++){
				mTimesV[rowM] = m[rowM][0] * v[0] +
												m[rowM][1] * v[1] +
												m[rowM][2] * v[2];
			}
		}

/* Builds a 3x3 matrix representing 2D rotation and translation in homogeneous
coordinates. More precisely, the transformation first rotates through the angle
theta (in radians, counterclockwise), and then translates by the vector t. */
void mat33Isometry(GLdouble theta, const GLdouble t[2], GLdouble isom[3][3]) {
	GLdouble rotationMatrix[3][3] = {{cos(theta), -sin(theta), 0},
	                               {sin(theta), cos(theta), 0},
								   {0, 0, 1}};
	GLdouble translation[3][3] = {{1, 0, t[0]},
                                {0, 1, t[1]},
								{0, 0, 1}};
	mat333Multiply(translation, rotationMatrix, isom);
}

/***************************************************/
/**** Below added in conversion from 100 to 120 ****/
/***************************************************/

/* Given a 3 by 3 matrix, multiply all values by a coefficient.
The output CAN safely alias the input. */
void matCoefficient(GLdouble co, GLdouble m[3][3], GLdouble mApplied[3][3]) {
		for (int i = 0; i < 3; i++){
			for (int j = 0; j < 3; j++){
				mApplied[i][j] = co * m[i][j];
			}
		}
	}

/* Given two 3x3 matrices, add their contents into another 3x3. The
output CAN safely alias the input. */
void mat33Add(GLdouble m[3][3], GLdouble n[3][3], GLdouble mPlusN[3][3]) {
	for (int i = 0; i < 3; i++){
		for (int j = 0; j < 3; j++){
			mPlusN[i][j] = m[i][j] + n[i][j];
		}
	}
}

/* Given a length-1 3D vector axis and an angle theta (in radians), builds the
rotation matrix for the rotation about that axis through that angle. */
void mat33AngleAxisRotation(GLdouble theta, const GLdouble axis[3],
		GLdouble rot[3][3]){
			GLdouble identity[3][3] = {{1,0,0},
															 {0,1,0},
															 {0,0,1}};
			GLdouble u[3][3] = {{0, -axis[2], axis[1]},
												{axis[2], 0, -axis[0]},
												{-axis[1], axis[0], 0}};
			GLdouble uSqr[3][3];
			mat333Multiply(u, u, uSqr);
			GLdouble uCo[3][3], uSqrCo[3][3];
			matCoefficient(sin(theta), u, uCo);
			matCoefficient(1-cos(theta), uSqr, uSqrCo);
			mat33Add(identity, uCo, rot);
			mat33Add(uSqrCo, rot, rot);
		}

/* Given a 3 3D vectors, fills the columns of matrix m with vectors m0, m1, m2.*/
void mat33FillColumns(const GLdouble m0[3], const GLdouble m1[3], const GLdouble m2[3],
	GLdouble m[3][3]) {
		for (int i = 0; i < 3; i++){
			m[i][0] = m0[i];
			m[i][1] = m1[i];
			m[i][2] = m2[i];
		}
	}

/* Given a 3D matrix, puts transpose of matrix into mT. */
void mat33Transpose(const GLdouble m[3][3], GLdouble mT[3][3]) {
	for (int i = 0; i < 3; i++){
		for (int j = 0; j < 3; j++){
			mT[i][j] = m[j][i];
		}
	}
}

/* Given two length-1 3D vectors u, v that are perpendicular to each other.
Given two length-1 3D vectors a, b that are perpendicular to each other. Builds
the rotation matrix that rotates u to a and v to b. */
void mat33BasisRotation(const GLdouble u[3], const GLdouble v[3],
		const GLdouble a[3], const GLdouble b[3], GLdouble rot[3][3]) {
			GLdouble w[3], c[3];
			vec3Cross(u, v, w);
			vec3Cross(a, b, c);
			GLdouble r[3][3], s[3][3], rT[3][3];
			mat33FillColumns(u, v, w, r);
			mat33Transpose(r, rT);
			mat33FillColumns(a, b, c, s);
			mat333Multiply(s, rT, rot);
		}

/* Multiplies m by n, placing the answer in mTimesN. The output CANNOT safely
alias the input. */
void mat444Multiply(const GLdouble m[4][4], const GLdouble n[4][4],
		GLdouble mTimesN[4][4]) {
			for (int colN = 0; colN < 4; colN++){
				for (int rowM = 0; rowM < 4; rowM++){
					mTimesN[rowM][colN] = m[rowM][0] * n[0][colN] +
					                      m[rowM][1] * n[1][colN] +
										            m[rowM][2] * n[2][colN] +
										            m[rowM][3] * n[3][colN];
				}
			}
		}

/* Multiplies m by v, placing the answer in mTimesV. The output CANNOT safely
alias the input. */
void mat441Multiply(const GLdouble m[4][4], const GLdouble v[4],
		GLdouble mTimesV[4]) {
			for (int rowM = 0; rowM < 4; rowM++){
				mTimesV[rowM] = m[rowM][0] * v[0] +
								        m[rowM][1] * v[1] +
								        m[rowM][2] * v[2] +
								        m[rowM][3] * v[3];
			}
		}

/* Given a rotation and a translation, forms the 4x4 homogeneous matrix
representing the rotation followed in time by the translation. */
void mat44Isometry(const GLdouble rot[3][3], const GLdouble trans[3],
		GLdouble isom[4][4]) {
			GLdouble rotationMatrix[4][4] = {{rot[0][0], rot[0][1], rot[0][2], 0},
	                               		 {rot[1][0], rot[1][1], rot[1][2], 0},
										                 {rot[2][0], rot[2][1], rot[2][2], 0},
										                 {0, 0, 0, 1}};
			GLdouble translation[4][4] = {{1, 0, 0, trans[0]},
                              		{0, 1, 0, trans[1]},
										              {0, 0, 1, trans[2]},
										              {0, 0, 0, 1}};
			mat444Multiply(translation, rotationMatrix, isom);
		}

/***************************************************/
/**** Below added in conversion from 120 to 140 ****/
/***************************************************/

/* Multiplies the transpose of the 3x3 matrix m by the 3x1 matrix v. To
clarify, in math notation it computes M^T v. The output CANNOT safely alias the
input. */
void mat331TransposeMultiply(const GLdouble m[3][3], const GLdouble v[3],
		GLdouble mTTimesV[3]) {
			for (int i = 0; i < 3; i ++){
				mTTimesV[i] = (m[0][i] * v[0] + m[1][i] * v[1] + m[2][i] * v[2]);
			}
		}

/* Sets its argument to the 4x4 zero matrix (which consists entirely of 0s). */
void mat44Zero(GLdouble m[4][4]) {
	for (int i = 0; i < 4; i++){
		for (int j = 0; j < 4; j++){
			m[i][j] = 0;
		}
	}
}


/* Builds a 4x4 matrix for a viewport with lower left (0, 0) and upper right
(width, height). This matrix maps a projected viewing volume
[-1, 1] x [-1, 1] x [-1, 1] to screen [0, w] x [0, h] x [0, 1] (each interval
in that order). */
void mat44Viewport(GLdouble width, GLdouble height, GLdouble view[4][4]) {
	mat44Zero(view);
	view[0][0] = view[0][3] = width / 2.0;
	view[1][1] = view[1][3] = height / 2.0;
	view[2][2] = view[2][3] = 1.0 / 2.0;
	view[3][3] = 1.0;
}

/* Inverse to mat44Viewport. */
void mat44InverseViewport(GLdouble width, GLdouble height, GLdouble view[4][4]) {
	mat44Zero(view);
	view[0][0] = 2.0 / width;
	view[0][3] = view[1][3] = view[2][3] = -1;
	view[1][1] = 2.0 / height;
	view[2][2] = 2.0;
	view[3][3] = 1.0;
}
