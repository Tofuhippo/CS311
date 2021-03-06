/*
Dawson d'Almeida and Justin T. Washington
January 23 2018
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
