/*
Dawson d'Almeida and Justin T. Washington
February 5 2018
CS311 with Josh Davis

Written by Josh Davis for generating/handling landscapes for 140mainLandscape.c.
*/


/* This file contains functions for randomly generating landscapes. A landscape
is a rectangular array of doubles, with each one giving an elevation. To turn
the landscape into a mesh, use the appropriate mesh initializer functions. */

/* Makes a flat landscape with the given elevation. */
void landFlat(int width, int height, double *data, double elevation) {
	int i, j;
	for (i = 0; i < width; i += 1)
		for (j = 0; j < height; j += 1)
			data[i * width + j] = elevation;
}

/* Returns a random integer in [a, b]. Before using this function, call
srand(). Warning: This is a poor-quality generator. It is not suitable for
serious cryptographic or statistical applications. */
int landInt(int a, int b) {
	return rand() % (b - a + 1) + a;
}

/* Returns a random double in [a, b]. Before using this function, call srand().
Warning: This is a poor-quality generator. It is not suitable for serious
cryptographic or statistical applications. */
double landDouble(double a, double b) {
	return a + (b - a) * (double)rand() / RAND_MAX;
}

/* Randomly chooses a vertical fault and slips the landscape up and down on the
two sides of that fault. Before using this function, call srand(). */
void landFault(int width, int height, double *data, double magnitude) {
	int i, j, sign;
	double m, b;
	m = landDouble(-1.0, 1.0);
	sign = (2 * landInt(0, 1) - 1);
	if (landInt(0, 1) == 0) {
		// Make a line y = m x + b, such that it intersects the landscape.
		if (m > 0)
			b = landDouble(-m * (width - 1), height - 1);
		else
			b = landDouble(-m * (width - 1), height - 1 - m * (width - 1));
		// Raise points north of the line and lower points south of it.
		double raisingNorth = magnitude * landDouble(0.5, 1.5) * sign;
		for (j = 0; j < height; j += 1)
			for (i = 0; i < width; i += 1)
				if (j > m * i + b)
					data[i * width + j] += raisingNorth;
				else if (j < m * i + b)
					data[i * width + j] -= raisingNorth;
	} else {
		// Make a line x = m y + b, such that it intersects the landscape.
		if (m > 0)
			b = landDouble(-m * (height - 1), width - 1);
		else
			b = landDouble(-m * (height - 1), width - 1 - m * (height - 1));
		// Raise points east of the line and lower points west of it.
		double raisingEast = magnitude * landDouble(0.5, 1.5) * sign;
		for (i = 0; i < width; i += 1)
			for (j = 0; j < height; j += 1)
				if (i > m * j + b)
					data[i * width + j] += raisingEast;
				else if (i < m * j + b)
					data[i * width + j] -= raisingEast;
	}
}

/* Blurs each (non-border) elevation with the eight elevations around it.
Doesn't work with gigantic landscapes, because of the memory allocation. To
work with gigantic landscapes, rewrite to malloc/free the copy variable. */
void landBlur(int width, int height, double *data) {
	int i, j;
	double copy[width * height];
	for (i = 1; i < width - 1; i += 1)
		for (j = 1; j < height - 1; j += 1) {
			copy[i * width + j] =
				(data[(i) * width + (j)] +
				data[(i + 1) * width + (j)] +
				data[(i - 1) * width + (j)] +
				data[(i) * width + (j + 1)] +
				data[(i) * width + (j - 1)] +
				data[(i + 1) * width + (j + 1)] +
				data[(i + 1) * width + (j - 1)] +
				data[(i - 1) * width + (j + 1)] +
				data[(i - 1) * width + (j - 1)]) / 9.0;
		}
	for (i = 1; i < width - 1; i += 1)
		for (j = 1; j < height - 1; j += 1)
			data[i * width + j] = copy[i * width + j];
}

/* Computes the min, mean, and max of the elevations. */
void landStatistics(int width, int height, double *data, double *min,
		double *mean, double *max) {
	*min = data[0];
	*max = data[0];
	*mean = 0.0;
	int i, j;
	for (i = 0; i < width; i += 1)
		for (j = 0; j < height; j += 1) {
			*mean += data[i * width + j];
			if (data[i * width + j] < *min)
				*min = data[i * width + j];
			if (data[i * width + j] > *max)
				*max = data[i * width + j];
		}
	*mean = *mean / (width * height);
}
