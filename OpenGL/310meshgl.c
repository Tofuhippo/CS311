/*
Dawson d'Almeida and Justin T. Washington
February 15 2018
CS311 with Josh Davis

Creates and manages mesh's in openGL.
*/


#define meshglBUFFEROFFSET(bytes) ((GLubyte*) NULL + (bytes))

/* Feel free to read from this struct's members, but don't write to them except
through the accessor functions. */
typedef struct meshglMesh meshglMesh;
struct meshglMesh {
	GLuint triNum, vertNum, attrDim;
	GLuint buffers[2];
};

/* Initializes the mesh from a non-OpenGL base mesh. After this function
completes, the base mesh can be destroyed (because its data have been copied
into GPU memory). When you are done using the mesh, don't forget to deallocate
its resources using meshglDestroy. */
void meshglInitialize(meshglMesh *mesh, meshMesh *base) {
	mesh->triNum = base->triNum;
	mesh->vertNum = base->vertNum;
	mesh->attrDim = base->attrDim;
	/* Ask OpenGL for two buffers in GPU memory. Copy the attributes and
    triangles into them. GL_STATIC_DRAW is a performance hint. It means that we
    expect to render using these buffers many times, without editing them.
    Performance hints help OpenGL decide how to organize the data in memory. */
	glGenBuffers(2, mesh->buffers);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, mesh->vertNum * mesh->attrDim * sizeof(GLdouble),
		(GLvoid *)base->vert, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->buffers[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->triNum * 3 * sizeof(GLuint),
		(GLvoid *)base->tri, GL_STATIC_DRAW);
}

/* Renders the mesh. Immediately before calling this function, you must
'pre-render' with custom code to set up the attributes, such as
	glBindBuffer(GL_ARRAY_BUFFER, mesh->buffers[0]);
	glVertexPointer(3, GL_DOUBLE, mesh->attrDim * sizeof(GLdouble),
		meshglBUFFEROFFSET(0));
	glNormalPointer(GL_DOUBLE, mesh->attrDim * sizeof(GLdouble),
		meshglBUFFEROFFSET(0));
	glColorPointer(3, GL_DOUBLE, mesh->attrDim * sizeof(GLdouble),
		meshglBUFFEROFFSET(3 * sizeof(GLdouble))); */
void meshglRender(meshglMesh *mesh) {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->buffers[1]);
  glDrawElements(GL_TRIANGLES, mesh->triNum * 3, GL_UNSIGNED_INT,
		meshglBUFFEROFFSET(0));
}

/* Releases the resources backing the mesh. Invoke this function when you are
done using the mesh. */
void meshglDestroy(meshglMesh *mesh) {
	glDeleteBuffers(2, mesh->buffers);
}
