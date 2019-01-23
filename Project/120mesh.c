/*
Dawson d'Almeida and Justin T. Washington
January 25 2018
CS311 with Josh Davis

Creates and manages mesh's of any number of triangles.
*/


/*** Creating and destroying ***/

/* Feel free to read the struct's members, but don't write them, except through
the accessors below such as meshSetTriangle, meshSetVertex. */
typedef struct meshMesh meshMesh;
struct meshMesh {
	int triNum, vertNum, attrDim;
	int *tri;						/* triNum * 3 ints */
	double *vert;					/* vertNum * attrDim doubles */
};

/* Initializes a mesh with enough memory to hold its triangles and vertices.
Does not actually fill in those triangles or vertices with useful data. When
you are finished with the mesh, you must call meshDestroy to deallocate its
backing resources. */
int meshInitialize(meshMesh *mesh, int triNum, int vertNum, int attrDim) {
	mesh->tri = (int *)malloc(triNum * 3 * sizeof(int) +
		vertNum * attrDim * sizeof(double));
	if (mesh->tri != NULL) {
		mesh->vert = (double *)&(mesh->tri[triNum * 3]);
		mesh->triNum = triNum;
		mesh->vertNum = vertNum;
		mesh->attrDim = attrDim;
	}
	return (mesh->tri == NULL);
}

/* Sets the trith triangle to have vertex indices i, j, k. */
void meshSetTriangle(meshMesh *mesh, int tri, int i, int j, int k) {
	if (0 <= tri && tri < mesh->triNum) {
		mesh->tri[3 * tri] = i;
		mesh->tri[3 * tri + 1] = j;
		mesh->tri[3 * tri + 2] = k;
	}
}

/* Returns a pointer to the trith triangle. For example:
	int *triangle13 = meshGetTrianglePointer(&mesh, 13);
	printf("%d, %d, %d\n", triangle13[0], triangle13[1], triangle13[2]); */
int *meshGetTrianglePointer(const meshMesh *mesh, int tri) {
	if (0 <= tri && tri < mesh->triNum)
		return &mesh->tri[tri * 3];
	else
		return NULL;
}

/* Sets the vertth vertex to have attributes attr. */
void meshSetVertex(meshMesh *mesh, int vert, const double attr[]) {
	int k;
	if (0 <= vert && vert < mesh->vertNum)
		for (k = 0; k < mesh->attrDim; k += 1)
			mesh->vert[mesh->attrDim * vert + k] = attr[k];
}

/* Returns a pointer to the vertth vertex. For example:
	double *vertex13 = meshGetVertexPointer(&mesh, 13);
	printf("x = %f, y = %f\n", vertex13[0], vertex13[1]); */
double *meshGetVertexPointer(const meshMesh *mesh, int vert) {
	if (0 <= vert && vert < mesh->vertNum)
		return &mesh->vert[vert * mesh->attrDim];
	else
		return NULL;
}

/* Deallocates the resources backing the mesh. This function must be called
when you are finished using a mesh. */
void meshDestroy(meshMesh *mesh) {
	free(mesh->tri);
}



/*** Writing and reading files ***/

/* Helper function for meshInitializeFile. */
int meshFileError(meshMesh *mesh, FILE *file, const char *cause,
		const int line) {
	fprintf(stderr, "error: meshInitializeFile: %s at line %d\n", cause, line);
	fclose(file);
	meshDestroy(mesh);
	return 3;
}

/* Initializes a mesh from a mesh file. The file format is simple and custom
(not any industry standard). It is documented at meshSaveFile. This function
does not do as much error checking as one might like. Use it only on trusted,
non-corrupted files, such as ones that you have recently created using
meshSaveFile. Returns 0 on success, non-zero on failure. Don't forget to invoke
meshDestroy when you are done using the mesh. */
int meshInitializeFile(meshMesh *mesh, const char *path) {
	FILE *file = fopen(path, "r");
	if (file == NULL) {
		fprintf(stderr, "error: meshInitializeFile: fopen failed\n");
		return 1;
	}
	int year, month, day, triNum, vertNum, attrDim;
	// Future work: Check version.
	if (fscanf(file, "Carleton College CS 311 mesh version %d/%d/%d\n", &year,
			&month, &day) != 3) {
		fprintf(stderr, "error: meshInitializeFile: bad header at line 1\n");
		fclose(file);
		return 1;
	}
	if (fscanf(file, "triNum %d\n", &triNum) != 1) {
		fprintf(stderr, "error: meshInitializeFile: bad triNum at line 2\n");
		fclose(file);
		return 2;
	}
	if (fscanf(file, "vertNum %d\n", &vertNum) != 1) {
		fprintf(stderr, "error: meshInitializeFile: bad vertNum at line 3\n");
		fclose(file);
		return 3;
	}
	if (fscanf(file, "attrDim %d\n", &attrDim) != 1) {
		fprintf(stderr, "error: meshInitializeFile: bad attrDim at line 4\n");
		fclose(file);
		return 4;
	}
	if (meshInitialize(mesh, triNum, vertNum, attrDim) != 0) {
		fclose(file);
		return 5;
	}
	int line = 5, *tri, j, check;
	if (fscanf(file, "%d Triangles:\n", &check) != 1 || check != triNum)
		return meshFileError(mesh, file, "bad header", line);
	for (line = 6; line < triNum + 6; line += 1) {
		tri = meshGetTrianglePointer(mesh, line - 6);
		if (fscanf(file, "%d %d %d\n", &tri[0], &tri[1], &tri[2]) != 3)
			return meshFileError(mesh, file, "bad triangle", line);
		if (0 > tri[0] || tri[0] >= vertNum || 0 > tri[1] || tri[1] >= vertNum
				|| 0 > tri[2] || tri[2] >= vertNum)
			return meshFileError(mesh, file, "bad index", line);
	}
	double *vert;
	if (fscanf(file, "%d Vertices:\n", &check) != 1 || check != vertNum)
		return meshFileError(mesh, file, "bad header", line);
	for (line = triNum + 7; line < triNum + 7 + vertNum; line += 1) {
		vert = meshGetVertexPointer(mesh, line - (triNum + 7));
		for (j = 0; j < attrDim; j += 1) {
			if (fscanf(file, "%lf ", &vert[j]) != 1)
				return meshFileError(mesh, file, "bad vertex", line);
		}
		if (fscanf(file, "\n") != 0)
			return meshFileError(mesh, file, "bad vertex", line);
	}
	// Future work: Check EOF.
	fclose(file);
	return 0;
}

/* Saves a mesh to a file in a simple custom format (not any industry
standard). Returns 0 on success, non-zero on failure. The first line is a
comment of the form 'Carleton College CS 311 mesh version YYYY/MM/DD'.

I now describe version 2019/01/15. The second line says 'triNum [triNum]',
where the latter is an integer value. The third and fourth lines do the same
for vertNum and attrDim. The fifth line says '[triNum] Triangles:'. Then there
are triNum lines, each holding three integers between 0 and vertNum - 1
(separated by a space). Then there is a line that says '[vertNum] Vertices:'.
Then there are vertNum lines, each holding attrDim floating-point numbers
(terminated by a space). */
int meshSaveFile(const meshMesh *mesh, const char *path) {
	FILE *file = fopen(path, "w");
	if (file == NULL) {
		fprintf(stderr, "error: meshSaveFile: fopen failed\n");
		return 1;
	}
	fprintf(file, "Carleton College CS 311 mesh version 2019/01/15\n");
	fprintf(file, "triNum %d\n", mesh->triNum);
	fprintf(file, "vertNum %d\n", mesh->vertNum);
	fprintf(file, "attrDim %d\n", mesh->attrDim);
	fprintf(file, "%d Triangles:\n", mesh->triNum);
	int i, j;
	int *tri;
	for (i = 0; i < mesh->triNum; i += 1) {
		tri = meshGetTrianglePointer(mesh, i);
		fprintf(file, "%d %d %d\n", tri[0], tri[1], tri[2]);
	}
	fprintf(file, "%d Vertices:\n", mesh->vertNum);
	double *vert;
	for (i = 0; i < mesh->vertNum; i += 1) {
		vert = meshGetVertexPointer(mesh, i);
		for (j = 0; j < mesh->attrDim; j += 1)
			fprintf(file, "%f ", vert[j]);
		fprintf(file, "\n");
	}
	fclose(file);
	return 0;
}



/*** Convenience initializers: 2D ***/

/* Initializes a mesh to two triangles forming a rectangle of the given sides.
The four attributes are X, Y, S, T. Do not call meshInitialize separately; it
is called inside this function. Don't forget to call meshDestroy when done. */
int meshInitializeRectangle(meshMesh *mesh, double left, double right,
		double bottom, double top) {
	int error = meshInitialize(mesh, 2, 4, 2 + 2);
	if (error == 0) {
		meshSetTriangle(mesh, 0, 0, 1, 2);
		meshSetTriangle(mesh, 1, 0, 2, 3);
		double attr[4];
		vec4Set(left, bottom, 0.0, 0.0, attr);
		meshSetVertex(mesh, 0, attr);
		vec4Set(right, bottom, 1.0, 0.0, attr);
		meshSetVertex(mesh, 1, attr);
		vec4Set(right, top, 1.0, 1.0, attr);
		meshSetVertex(mesh, 2, attr);
		vec4Set(left, top, 0.0, 1.0, attr);
		meshSetVertex(mesh, 3, attr);
	}
	return error;
}

/* Initializes a mesh to sideNum triangles forming an ellipse of the given
center (x, y) and radii rx, ry. The four attributes are X, Y, S, T. Do not call
meshInitialize separately; it is called inside this function. Don't forget to
call meshDestroy when done. */
int meshInitializeEllipse(meshMesh *mesh, double x, double y, double rx,
		double ry, int sideNum) {
	int i, error;
	double theta, cosTheta, sinTheta, attr[4] = {x, y, 0.5, 0.5};
	error = meshInitialize(mesh, sideNum, sideNum + 1, 2 + 2);
	if (error == 0) {
		meshSetVertex(mesh, 0, attr);
		for (i = 0; i < sideNum; i += 1) {
			meshSetTriangle(mesh, i, 0, i + 1, (i + 1) % sideNum + 1);
			theta = i * 2.0 * M_PI / sideNum;
			cosTheta = cos(theta);
			sinTheta = sin(theta);
			vec4Set(x + rx * cosTheta, y + ry * sinTheta,
				0.5 * cosTheta + 0.5, 0.5 * sinTheta + 0.5, attr);
			meshSetVertex(mesh, i + 1, attr);
		}
	}
	return error;
}



/*** Convenience initializers: 3D ***/

/* Assumes that attributes 0, 1, 2 are XYZ. Assumes that the vertices of the
triangle are in counter-clockwise order when viewed from 'outside' the
triangle. Computes the outward-pointing unit normal vector for the triangle.
The output CANNOT safely alias the input. */
void meshTrueNormal(const double a[], const double b[], const double c[],
		double normal[3]) {
	double bMinusA[3], cMinusA[3];
	vecSubtract(3, b, a, bMinusA);
	vecSubtract(3, c, a, cMinusA);
	vec3Cross(bMinusA, cMinusA, normal);
	vecUnit(3, normal, normal);
}

/* Assumes that attributes 0, 1, 2 are XYZ. Sets attributes n, n + 1, n + 2 to
flat-shaded normals. If a vertex belongs to more than triangle, then some
unspecified triangle's normal wins. */
void meshFlatNormals(meshMesh *mesh, int n) {
	int i, *tri;
	double *a, *b, *c, normal[3];
	for (i = 0; i < mesh->triNum; i += 1) {
		tri = meshGetTrianglePointer(mesh, i);
		a = meshGetVertexPointer(mesh, tri[0]);
		b = meshGetVertexPointer(mesh, tri[1]);
		c = meshGetVertexPointer(mesh, tri[2]);
		meshTrueNormal(a, b, c, normal);
		vecCopy(3, normal, &a[n]);
		vecCopy(3, normal, &b[n]);
		vecCopy(3, normal, &c[n]);
	}
}

/* Assumes that attributes 0, 1, 2 are XYZ. Sets attributes n, n + 1, n + 2 to
smooth-shaded normals. Does not do anything special to handle multiple vertices
with the same coordinates. */
void meshSmoothNormals(meshMesh *mesh, int n) {
	int i, *tri;
	double *a, *b, *c, normal[3] = {0.0, 0.0, 0.0};
	/* Zero the normals. */
	for (i = 0; i < mesh->vertNum; i += 1) {
		a = meshGetVertexPointer(mesh, i);
		vecCopy(3, normal, &a[n]);
	}
	/* For each triangle, add onto the normal at each of its vertices. */
	for (i = 0; i < mesh->triNum; i += 1) {
		tri = meshGetTrianglePointer(mesh, i);
		a = meshGetVertexPointer(mesh, tri[0]);
		b = meshGetVertexPointer(mesh, tri[1]);
		c = meshGetVertexPointer(mesh, tri[2]);
		meshTrueNormal(a, b, c, normal);
		vecAdd(3, normal, &a[n], &a[n]);
		vecAdd(3, normal, &b[n], &b[n]);
		vecAdd(3, normal, &c[n], &c[n]);
	}
	/* Normalize the normals. */
	for (i = 0; i < mesh->vertNum; i += 1) {
		a = meshGetVertexPointer(mesh, i);
		vecUnit(3, &a[n], &a[n]);
	}
}

/* Builds a mesh for a parallelepiped (box) of the given size. The attributes
are XYZ position, ST texture, and NOP unit normal vector. The normals are
discontinuous at the edges (flat shading, not smooth). To facilitate this, some
vertices have equal XYZ but different NOP, for 24 vertices in all. Don't forget
to meshDestroy when finished. */
int meshInitializeBox(meshMesh *mesh, double left, double right, double bottom,
		double top, double base, double lid) {
	int error = meshInitialize(mesh, 12, 24, 3 + 2 + 3);
	if (error == 0) {
		/* Make the triangles. */
		meshSetTriangle(mesh, 0, 0, 2, 1);
		meshSetTriangle(mesh, 1, 0, 3, 2);
		meshSetTriangle(mesh, 2, 4, 5, 6);
		meshSetTriangle(mesh, 3, 4, 6, 7);
		meshSetTriangle(mesh, 4, 8, 10, 9);
		meshSetTriangle(mesh, 5, 8, 11, 10);
		meshSetTriangle(mesh, 6, 12, 13, 14);
		meshSetTriangle(mesh, 7, 12, 14, 15);
		meshSetTriangle(mesh, 8, 16, 18, 17);
		meshSetTriangle(mesh, 9, 16, 19, 18);
		meshSetTriangle(mesh, 10, 20, 21, 22);
		meshSetTriangle(mesh, 11, 20, 22, 23);
		/* Make the vertices after 0, using vertex 0 as temporary storage. */
		double *v = mesh->vert;
		vec8Set(right, bottom, base, 1.0, 0.0, 0.0, 0.0, -1.0, v);
		meshSetVertex(mesh, 1, v);
		vec8Set(right, top, base, 1.0, 1.0, 0.0, 0.0, -1.0, v);
		meshSetVertex(mesh, 2, v);
		vec8Set(left, top, base, 0.0, 1.0, 0.0, 0.0, -1.0, v);
		meshSetVertex(mesh, 3, v);
		vec8Set(left, bottom, lid, 0.0, 0.0, 0.0, 0.0, 1.0, v);
		meshSetVertex(mesh, 4, v);
		vec8Set(right, bottom, lid, 1.0, 0.0, 0.0, 0.0, 1.0, v);
		meshSetVertex(mesh, 5, v);
		vec8Set(right, top, lid, 1.0, 1.0, 0.0, 0.0, 1.0, v);
		meshSetVertex(mesh, 6, v);
		vec8Set(left, top, lid, 0.0, 1.0, 0.0, 0.0, 1.0, v);
		meshSetVertex(mesh, 7, v);
		vec8Set(left, top, base, 0.0, 1.0, 0.0, 1.0, 0.0, v);
		meshSetVertex(mesh, 8, v);
		vec8Set(right, top, base, 1.0, 1.0, 0.0, 1.0, 0.0, v);
		meshSetVertex(mesh, 9, v);
		vec8Set(right, top, lid, 1.0, 1.0, 0.0, 1.0, 0.0, v);
		meshSetVertex(mesh, 10, v);
		vec8Set(left, top, lid, 0.0, 1.0, 0.0, 1.0, 0.0, v);
		meshSetVertex(mesh, 11, v);
		vec8Set(left, bottom, base, 0.0, 0.0, 0.0, -1.0, 0.0, v);
		meshSetVertex(mesh, 12, v);
		vec8Set(right, bottom, base, 1.0, 0.0, 0.0, -1.0, 0.0, v);
		meshSetVertex(mesh, 13, v);
		vec8Set(right, bottom, lid, 1.0, 0.0, 0.0, -1.0, 0.0, v);
		meshSetVertex(mesh, 14, v);
		vec8Set(left, bottom, lid, 0.0, 0.0, 0.0, -1.0, 0.0, v);
		meshSetVertex(mesh, 15, v);
		vec8Set(right, top, base, 1.0, 1.0, 1.0, 0.0, 0.0, v);
		meshSetVertex(mesh, 16, v);
		vec8Set(right, bottom, base, 1.0, 0.0, 1.0, 0.0, 0.0, v);
		meshSetVertex(mesh, 17, v);
		vec8Set(right, bottom, lid, 1.0, 0.0, 1.0, 0.0, 0.0, v);
		meshSetVertex(mesh, 18, v);
		vec8Set(right, top, lid, 1.0, 1.0, 1.0, 0.0, 0.0, v);
		meshSetVertex(mesh, 19, v);
		vec8Set(left, top, base, 0.0, 1.0, -1.0, 0.0, 0.0, v);
		meshSetVertex(mesh, 20, v);
		vec8Set(left, bottom, base, 0.0, 0.0, -1.0, 0.0, 0.0, v);
		meshSetVertex(mesh, 21, v);
		vec8Set(left, bottom, lid, 0.0, 0.0, -1.0, 0.0, 0.0, v);
		meshSetVertex(mesh, 22, v);
		vec8Set(left, top, lid, 0.0, 1.0, -1.0, 0.0, 0.0, v);
		meshSetVertex(mesh, 23, v);
		/* Now make vertex 0 for realsies. */
		vec8Set(left, bottom, base, 0.0, 0.0, 0.0, 0.0, -1.0, v);
	}
	return error;
}

/* Rotates a 2-dimensional vector through an angle. The output can safely alias
the input. */
void meshRotateVector(double theta, const double v[2], double vRot[2]) {
	double cosTheta = cos(theta);
	double sinTheta = sin(theta);
	double vRot0 = cosTheta * v[0] - sinTheta * v[1];
	vRot[1] = sinTheta * v[0] + cosTheta * v[1];
	vRot[0] = vRot0;
}

/* Rotate a curve about the Z-axis. Can be used to make a sphere, spheroid,
capsule, circular cone, circular cylinder, box, etc. The z-values should be in
ascending order --- or at least the first z should be less than the last. The
first and last r-values should be 0.0, and no others. Probably the t-values
should be in ascending or descending order. The sideNum parameter controls the
fineness of the mesh. The attributes are XYZ position, ST texture, and NOP unit
normal vector. The normals are smooth. Don't forget to meshDestroy when
finished. */
int meshInitializeRevolution(meshMesh *mesh, int zNum, const double z[],
		const double r[], const double t[], int sideNum) {
	int i, j, error;
	error = meshInitialize(mesh, (zNum - 2) * sideNum * 2,
		(zNum - 2) * (sideNum + 1) + 2, 3 + 2 + 3);
	if (error == 0) {
		/* Make the bottom triangles. */
		for (i = 0; i < sideNum; i += 1)
			meshSetTriangle(mesh, i, 0, i + 2, i + 1);
		/* Make the top triangles. */
		for (i = 0; i < sideNum; i += 1)
			meshSetTriangle(mesh, sideNum + i, mesh->vertNum - 1,
				mesh->vertNum - 1 - (sideNum + 1) + i,
				mesh->vertNum - 1 - (sideNum + 1) + i + 1);
		/* Make the middle triangles. */
		for (j = 1; j <= zNum - 3; j += 1)
			for (i = 0; i < sideNum; i += 1) {
				meshSetTriangle(mesh, 2 * sideNum * j + 2 * i,
					(j - 1) * (sideNum + 1) + 1 + i,
					j * (sideNum + 1) + 1 + i + 1,
					j * (sideNum + 1) + 1 + i);
				meshSetTriangle(mesh, 2 * sideNum * j + 2 * i + 1,
					(j - 1) * (sideNum + 1) + 1 + i,
					(j - 1) * (sideNum + 1) + 1 + i + 1,
					j * (sideNum + 1) + 1 + i + 1);
			}
		/* Make the vertices, using vertex 0 as temporary storage. */
		double *v = mesh->vert;
		double p[3], q[3], o[3];
		for (j = 1; j <= zNum - 2; j += 1) {
			// Form the sideNum + 1 vertices in the jth layer.
			vec3Set(z[j + 1] - z[j], 0.0, r[j] - r[j + 1], p);
			vecUnit(3, p, p);
			vec3Set(z[j] - z[j - 1], 0.0, r[j - 1] - r[j], q);
			vecUnit(3, q, q);
			vecAdd(3, p, q, o);
			vecUnit(3, o, o);
			vec8Set(r[j], 0.0, z[j], 1.0, t[j], o[0], o[1], o[2], v);
			meshSetVertex(mesh, j * (sideNum + 1), v);
			v[3] = 0.0;
			meshSetVertex(mesh, (j - 1) * (sideNum + 1) + 1, v);
			for (i = 1; i < sideNum; i += 1) {
				meshRotateVector(2 * M_PI / sideNum, v, v);
				v[3] += 1.0 / sideNum;
				meshRotateVector(2 * M_PI / sideNum, &v[5], &v[5]);
				meshSetVertex(mesh, (j - 1) * (sideNum + 1) + 1 + i, v);
			}
		}
		/* Form the top vertex. */
		vec8Set(0.0, 0.0, z[zNum - 1], 0.0, 0.0, 0.0, 0.0, 1.0, v);
		meshSetVertex(mesh, mesh->vertNum - 1, v);
		/* Finally form the bottom vertex. */
		vec8Set(0.0, 0.0, z[0], 0.0, 0.0, 0.0, 0.0, -1.0, v);
	}
	return error;
}

/* Builds a mesh for a sphere, centered at the origin, of radius r. The sideNum
and layerNum parameters control the fineness of the mesh. The attributes are
XYZ position, ST texture, and NOP unit normal vector. The normals are smooth.
Don't forget to meshDestroy when finished. */
int meshInitializeSphere(meshMesh *mesh, double r, int layerNum, int sideNum) {
	int error, i;
	double *ts = (double *)malloc((layerNum + 1) * 3 * sizeof(double));
	if (ts == NULL)
		return 1;
	else {
		double *zs = &ts[layerNum + 1];
		double *rs = &ts[2 * layerNum + 2];
		for (i = 0; i <= layerNum; i += 1) {
			ts[i] = (double)i / layerNum;
			zs[i] = -r * cos(ts[i] * M_PI);
			rs[i] = r * sin(ts[i] * M_PI);
		}
		error = meshInitializeRevolution(mesh, layerNum + 1, zs, rs, ts,
			sideNum);
		free(ts);
		return error;
	}
}

/* Builds a mesh for a circular cylinder with spherical caps, centered at the
origin, of radius r and length l > 2 * r. The sideNum and layerNum parameters
control the fineness of the mesh. The attributes are XYZ position, ST texture,
and NOP unit normal vector. The normals are smooth. Don't forget to meshDestroy
when finished. */
int meshInitializeCapsule(meshMesh *mesh, double r, double l, int layerNum,
		int sideNum) {
	int error, i;
	double theta;
	double *ts = (double *)malloc((2 * layerNum + 2) * 3 * sizeof(double));
	if (ts == NULL)
		return 1;
	else {
		double *zs = &ts[2 * layerNum + 2];
		double *rs = &ts[4 * layerNum + 4];
		zs[0] = -l / 2.0;
		rs[0] = 0.0;
		ts[0] = 0.0;
		for (i = 1; i <= layerNum; i += 1) {
			theta = M_PI / 2.0 * (3 + i / (double)layerNum);
			zs[i] = -l / 2.0 + r + r * sin(theta);
			rs[i] = r * cos(theta);
			ts[i] = (zs[i] + l / 2.0) / l;
		}
		for (i = 0; i < layerNum; i += 1) {
			theta = M_PI / 2.0 * i / (double)layerNum;
			zs[layerNum + 1 + i] = l / 2.0 - r + r * sin(theta);
			rs[layerNum + 1 + i] = r * cos(theta);
			ts[layerNum + 1 + i] = (zs[layerNum + 1 + i] + l / 2.0) / l;
		}
		zs[2 * layerNum + 1] = l / 2.0;
		rs[2 * layerNum + 1] = 0.0;
		ts[2 * layerNum + 1] = 1.0;
		error = meshInitializeRevolution(mesh, 2 * layerNum + 2, zs, rs, ts,
			sideNum);
		free(ts);
		return error;
	}
}

/* Builds a non-closed 'landscape' mesh based on a grid of Z-values. There are
width * height Z-values, which arrive in the data parameter. The mesh is made
of (width - 1) * (height - 1) squares, each made of two triangles. The spacing
parameter controls the spacing of the X- and Y-coordinates of the vertices. The
attributes are XYZ position, ST texture, and NOP unit normal vector. Don't
forget to call meshDestroy when finished with the mesh. To understand the exact
layout of the data, try this example code:
double zs[3][4] = {
	{10.0, 9.0, 7.0, 6.0},
	{6.0, 5.0, 3.0, 1.0},
	{4.0, 3.0, -1.0, -2.0}};
int error = meshInitializeLandscape(&mesh, 3, 4, 20.0, (double *)zs); */
int meshInitializeLandscape(meshMesh *mesh, int width, int height,
		double spacing, const double *data) {
	int i, j, error;
	int a, b, c, d;
	double *vert, diffSWNE, diffSENW;
	error = meshInitialize(mesh, 2 * (width - 1) * (height - 1),
		width * height, 3 + 2 + 3);
	if (error == 0) {
		/* Build the vertices with normals set to 0. */
		for (i = 0; i < width; i += 1)
			for (j = 0; j < height; j += 1) {
				vert = meshGetVertexPointer(mesh, i * height + j);
				vec8Set(i * spacing, j * spacing, data[i * height + j],
					(double)i, (double)j, 0.0, 0.0, 0.0, vert);
			}
		/* Build the triangles. */
		for (i = 0; i < width - 1; i += 1)
			for (j = 0; j < height - 1; j += 1) {
				int index = 2 * (i * (height - 1) + j);
				a = i * height + j;
				b = (i + 1) * height + j;
				c = (i + 1) * height + (j + 1);
				d = i * height + (j + 1);
				diffSWNE = fabs(meshGetVertexPointer(mesh, a)[2] -
					meshGetVertexPointer(mesh, c)[2]);
				diffSENW = fabs(meshGetVertexPointer(mesh, b)[2] -
					meshGetVertexPointer(mesh, d)[2]);
				if (diffSENW < diffSWNE) {
					meshSetTriangle(mesh, index, d, a, b);
					meshSetTriangle(mesh, index + 1, b, c, d);
				} else {
					meshSetTriangle(mesh, index, a, b, c);
					meshSetTriangle(mesh, index + 1, a, c, d);
				}
			}
		/* Set the normals. */
		meshSmoothNormals(mesh, 5);
	}
	return error;
}

/* Given a landscape, such as that built by meshInitializeLandscape. Builds a
new landscape mesh by extracting triangles based on how horizontal they are. If
noMoreThan is true, then triangles are kept that deviate from horizontal by no more than angle. If noMoreThan is false, then triangles are kept that deviate
from horizontal by more than angle. Don't forget to call meshDestroy when
finished. Warning: May contain extraneous vertices not used by any triangle. */
int meshInitializeDissectedLandscape(meshMesh *mesh, const meshMesh *land,
		double angle, int noMoreThan) {
	int error, i, j = 0, triNum = 0;
	int *tri, *newTri;
	double normal[3];
	/* Count the triangles that are nearly horizontal. */
	for (i = 0; i < land->triNum; i += 1) {
		tri = meshGetTrianglePointer(land, i);
		meshTrueNormal(meshGetVertexPointer(land, tri[0]),
			meshGetVertexPointer(land, tri[1]),
			meshGetVertexPointer(land, tri[2]), normal);
		if ((noMoreThan && normal[2] >= cos(angle)) ||
				(!noMoreThan && normal[2] < cos(angle)))
			triNum += 1;
	}
	error = meshInitialize(mesh, triNum, land->vertNum, 3 + 2 + 3);
	if (error == 0) {
		/* Copy all of the vertices. */
		vecCopy(land->vertNum * (3 + 2 + 3), land->vert, mesh->vert);
		/* Copy just the horizontal triangles. */
		for (i = 0; i < land->triNum; i += 1) {
			tri = meshGetTrianglePointer(land, i);
			meshTrueNormal(meshGetVertexPointer(land, tri[0]),
				meshGetVertexPointer(land, tri[1]),
				meshGetVertexPointer(land, tri[2]), normal);
			if ((noMoreThan && normal[2] >= cos(angle)) ||
					(!noMoreThan && normal[2] < cos(angle))) {
				newTri = meshGetTrianglePointer(mesh, j);
				newTri[0] = tri[0];
				newTri[1] = tri[1];
				newTri[2] = tri[2];
				j += 1;
			}
		}
		/* Reset the normals, to make the cliff edges appear sharper. */
		meshSmoothNormals(mesh, 5);
	}
	return error;
}



/*** Rendering ***/

/* Renders the mesh. But if the mesh and the shading have differing values for
attrDim, then prints an error message and does not render anything. */
void meshRender(const meshMesh *mesh, const shaShading *sha,
		const double unif[], const texTexture *tex[]) {
			// Make sure that the mesh->attrDim and sha->attrDim are the same
			if (mesh->attrDim != sha->attrDim){
				printf("meshMesh and shaShading have differing values.\n");
				return;
			}
			// Make temp holders for each [triangle in the mesh]'s values to be held
			int *tempTriangle;
			double *tempVertexA;
			double *tempVertexB;
			double *tempVertexC;
			// Iterate through the triangles in mesh using mesh->triNum as index
			for (int i = 0; i < mesh->triNum; i++){
				// Get a pointer to the current triangle at index i in the mesh, which
				// is the first vertex of the triangle
				tempTriangle = meshGetTrianglePointer(mesh, i);
				// Get pointers to each of the vertexes of the ith triangle
				tempVertexA = meshGetVertexPointer(mesh, tempTriangle[0]);
				tempVertexB = meshGetVertexPointer(mesh, tempTriangle[1]);
				tempVertexC = meshGetVertexPointer(mesh, tempTriangle[2]);
				// Transform?
				double varyA[sha->varyDim], varyB[sha->varyDim], varyC[sha->varyDim];
				sha->transformVertex(sha->unifDim, unif, sha->attrDim, tempVertexA,
												     sha->varyDim, varyA);
				sha->transformVertex(sha->unifDim, unif, sha->attrDim, tempVertexB,
					                   sha->varyDim, varyB);
				sha->transformVertex(sha->unifDim, unif, sha->attrDim, tempVertexC,
					                   sha->varyDim, varyC);
				// Render each triangle using the vertexes we retrieved and infomration
				// from other structs we have been using.
				printf("varyA: %f, %f\n", varyA[0], varyA[1]);
				printf("varyB: %f, %f\n", varyB[0], varyB[1]);
				printf("varyC: %f, %f\n", varyC[0], varyC[1]);
				triRender(sha, unif, tex, varyA, varyB, varyC);
			}
}
