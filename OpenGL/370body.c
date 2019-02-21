


/* Feel free to read from this struct's members. Write to the isometry through
its accessors. Write to the other members only through the accessors here. */
typedef struct bodyBody bodyBody;
struct bodyBody {
	isoIsometry isometry;
	meshglMesh *mesh;
	GLuint auxDim, texNum;
	texTexture **tex;
	GLdouble *aux;
};

/* Returns error code, which is 0 if successful. Don't forget to free the
resources backing the body using bodyDestroy when you are finished with it. */
int bodyInitialize(bodyBody *body, GLuint auxDim, GLuint texNum) {
	body->aux = (GLdouble *)malloc(auxDim * sizeof(GLdouble) +
		texNum * sizeof(texTexture *));
	if (body->aux == NULL)
		return 1;
	body->tex = (texTexture **)&(body->aux[auxDim]);
	body->auxDim = auxDim;
	body->texNum = texNum;
	return 0;
}

/* Sets the body's mesh. */
void bodySetMesh(bodyBody *body, meshglMesh *mesh) {
	body->mesh = mesh;
}

/* Sets one of the body's textures. */
void bodySetTexture(bodyBody *body, GLuint index, texTexture *tex) {
	if (index < body->texNum)
		body->tex[index] = tex;
}

/* Sets one of the body's auxiliary values. */
void bodySetAuxiliary(bodyBody *body, GLuint index, GLdouble value) {
	if (index < body->auxDim)
		body->aux[index] = value;
}

/* Sets some of the body's auxiliary values. More precisely, count values are
set, starting at the given index. */
void bodySetAuxiliaries(bodyBody *body, GLuint index, GLuint count,
		const GLdouble *values) {
	if (index + count <= body->auxDim)
		vecCopy(count, values, &(body->aux[index]));
}

/* Frees the resources backing the body itself. Does not free the body's mesh or textures (because those might still be in use by other bodies). */
void bodyDestroy(bodyBody *body) {
	free(body->aux);
}
