/*
Dawson d'Almeida and Justin T. Washington
January 21 2018
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
