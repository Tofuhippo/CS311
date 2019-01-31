/*
Dawson d'Almeida and Justin T. Washington
February 1 2018
CS311 with Josh Davis

General matrix functions.
*/

/*** 2 x 2 Matrices ***/

/* Pretty-prints the given matrix, with one line of text per row of matrix. */
void mat22Print(const double m[2][2]) {
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
double mat22Invert(const double m[2][2], double mInv[2][2]) {
	double a = m[0][0];
	double b = m[0][1];
	double c = m[1][0];
	double d = m[1][1];
	double det = (a*d) - (b*c);
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
void mat221Multiply(const double m[2][2], const double v[2],
		double mTimesV[2]) {
			mTimesV[0] = m[0][0]*v[0] + m[0][1]*v[1];
			mTimesV[1] = m[1][0]*v[0] + m[1][1]*v[1];
}

/* Fills the matrix m from its two columns. The output CANNOT safely alias the
input. */
void mat22Columns(const double col0[2], const double col1[2], double m[2][2]) {
	m[0][0] = col0[0];
	m[1][0] = col0[1];
	m[0][1] = col1[0];
	m[1][1] = col1[1];
}

/* The theta parameter is an angle in radians. Sets the matrix m to the
rotation matrix corresponding to counterclockwise rotation of the plane through
the angle theta. */
void mat22Rotation(double theta, double m[2][2]) {

	double rotationMatrix[2][2] = {{cos(theta), -sin(theta)},
	                               {sin(theta), cos(theta)}};

	// Split into columns of m
	double col0[2] = {m[0][0], m[1][0]};
	double col1[2] = {m[1][0], m[1][1]};

	// Rotate the columns of m
	mat221Multiply(rotationMatrix, col0, col0);
	mat221Multiply(rotationMatrix, col1, col1);

	// Recombine the columns into m
	mat22Columns(col0, col1, m);

}

/* Multiplies the 3x3 matrix m by the 3x3 matrix n. The output CANNOT safely
alias the input. */
void mat333Multiply(const double m[3][3], const double n[3][3],
		double mTimesN[3][3]) {
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
void mat331Multiply(const double m[3][3], const double v[3],
		double mTimesV[3]) {
			for (int rowM = 0; rowM < 3; rowM++){
				mTimesV[rowM] = m[rowM][0] * v[0] +
												m[rowM][1] * v[1] +
												m[rowM][2] * v[2];
			}
		}

/* Builds a 3x3 matrix representing 2D rotation and translation in homogeneous
coordinates. More precisely, the transformation first rotates through the angle
theta (in radians, counterclockwise), and then translates by the vector t. */
void mat33Isometry(double theta, const double t[2], double isom[3][3]) {
	double rotationMatrix[3][3] = {{cos(theta), -sin(theta), 0},
	                               {sin(theta), cos(theta), 0},
								   {0, 0, 1}};
	double translation[3][3] = {{1, 0, t[0]},
                                {0, 1, t[1]},
								{0, 0, 1}};
	mat333Multiply(translation, rotationMatrix, isom);
}

/***************************************************/
/**** Below added in conversion from 100 to 120 ****/
/***************************************************/

/* Given a 3 by 3 matrix, multiply all values by a coefficient.
The output CAN safely alias the input. */
void matCoefficient(double co, double m[3][3], double mApplied[3][3]) {
		for (int i = 0; i < 3; i++){
			for (int j = 0; j < 3; j++){
				mApplied[i][j] = co * m[i][j];
			}
		}
	}

/* Given two 3x3 matrices, add their contents into another 3x3. The
output CAN safely alias the input. */
void mat33Add(double m[3][3], double n[3][3], double mPlusN[3][3]) {
	for (int i = 0; i < 3; i++){
		for (int j = 0; j < 3; j++){
			mPlusN[i][j] = m[i][j] + n[i][j];
		}
	}
}

/* Given a length-1 3D vector axis and an angle theta (in radians), builds the
rotation matrix for the rotation about that axis through that angle. */
void mat33AngleAxisRotation(double theta, const double axis[3],
		double rot[3][3]){
			double identity[3][3] = {{1,0,0},
															 {0,1,0},
															 {0,0,1}};
			double u[3][3] = {{0, -axis[2], axis[1]},
												{axis[2], 0, -axis[0]},
												{-axis[1], axis[0], 0}};
			double uSqr[3][3];
			mat333Multiply(u, u, uSqr);
			double uCo[3][3], uSqrCo[3][3];
			matCoefficient(sin(theta), u, uCo);
			matCoefficient(1-cos(theta), uSqr, uSqrCo);
			mat33Add(identity, uCo, rot);
			mat33Add(uSqrCo, rot, rot);
		}

/* Given a 3 3D vectors, fills the columns of matrix m with vectors m0, m1, m2.*/
void mat33FillColumns(const double m0[3], const double m1[3], const double m2[3],
	double m[3][3]) {
		for (int i = 0; i < 3; i++){
			m[i][0] = m0[i];
			m[i][1] = m1[i];
			m[i][2] = m2[i];
		}
	}

/* Given a 3D matrix, puts transpose of matrix into mT. */
void mat33Transpose(const double m[3][3], double mT[3][3]) {
	for (int i = 0; i < 3; i++){
		for (int j = 0; j < 3; j++){
			mT[i][j] = m[j][i];
		}
	}
}

/* Given two length-1 3D vectors u, v that are perpendicular to each other.
Given two length-1 3D vectors a, b that are perpendicular to each other. Builds
the rotation matrix that rotates u to a and v to b. */
void mat33BasisRotation(const double u[3], const double v[3],
		const double a[3], const double b[3], double rot[3][3]) {
			double w[3], c[3];
			vec3Cross(u, v, w);
			vec3Cross(a, b, c);
			double r[3][3], s[3][3], rT[3][3];
			mat33FillColumns(u, v, w, r);
			mat33FillColumns(a, b, c, s);
			mat333Multiply(s, rT, rot);
		}

/* Multiplies m by n, placing the answer in mTimesN. The output CANNOT safely
alias the input. */
void mat444Multiply(const double m[4][4], const double n[4][4],
		double mTimesN[4][4]) {
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
void mat441Multiply(const double m[4][4], const double v[4],
		double mTimesV[4]) {
			for (int rowM = 0; rowM < 4; rowM++){
				mTimesV[rowM] = m[rowM][0] * v[0] +
								        m[rowM][1] * v[1] +
								        m[rowM][2] * v[2] +
								        m[rowM][3] * v[3];
			}
		}

/* Given a rotation and a translation, forms the 4x4 homogeneous matrix
representing the rotation followed in time by the translation. */
void mat44Isometry(const double rot[3][3], const double trans[3],
		double isom[4][4]) {
			double rotationMatrix[4][4] = {{rot[0][0], rot[0][1], rot[0][2], 0},
	                               		 {rot[1][0], rot[1][1], rot[1][2], 0},
										                 {rot[2][0], rot[2][1], rot[2][2], 0},
										                 {0, 0, 0, 1}};
			double translation[4][4] = {{1, 0, 0, trans[0]},
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
void mat331TransposeMultiply(const double m[3][3], const double v[3],
		double mTTimesV[3]) {
			mTTimesV[0] = (m[0][0] * v[0] + m[1][0] * v[1] + m[2][0] * v[2]);
			mTTimesV[1] = (m[0][1] * v[0] + m[1][1] * v[1] + m[2][1] * v[2]);
			mTTimesV[2] = (m[0][2] * v[0] + m[1][2] * v[1] + m[2][2] * v[2]);
		}

/* Sets its argument to the 4x4 zero matrix (which consists entirely of 0s). */
void mat44Zero(double m[4][4]) {
	for (int i = 0; i < 4; i++){
		for (int j = 0; j < 4; j++){
			m[i][j] = 0;
		}
	}
}
