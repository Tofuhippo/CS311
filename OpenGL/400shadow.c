/*
Dawson d'Almeida and Justin T. Washington
February 25 2018
CS311 with Josh Davis

Define shadowMap struct written by Josh Davis.
*/


/* Feel free to read from this struct's members, but don't write to them,
except through the accessors. */
typedef struct shadowMap shadowMap;
struct shadowMap {
	GLint oldViewport[4];
	GLuint width, height;
	GLuint texture, fbo;
	camCamera camera;
};

/* The application needs one shadow map per shadow-casting light. The width and
height must be powers of 2. Returns 0 on success, non-zero on failure. On
success, the user must call shadowDestroy when finished with the shadow map. */
int shadowInitialize(shadowMap *map, GLuint width, GLuint height) {
	/* Create a texture. */
	map->width = width;
	map->height = height;
	glGenTextures(1, &(map->texture));
	glBindTexture(GL_TEXTURE_2D, map->texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0,
		GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	/* Set the filtering, comparison, and wrapping modes. */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE,
		GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	/* We are finished configuring the texture. */
	glBindTexture(GL_TEXTURE_2D, 0);
	/* Create a framebuffer object, attach the texture, and disable color. */
	glGenFramebuffers(1, &(map->fbo));
	glBindFramebuffer(GL_FRAMEBUFFER, map->fbo);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, map->texture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
    	fprintf(stderr,
    		"error: shadowInitialize: glCheckFramebufferStatus %d\n", status);
    	return 1;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return 0;
}

/* Deallocates the resources backing the shadow map. */
void shadowDestroy(shadowMap *map) {
	glDeleteTextures(1, &(map->texture));
	glDeleteFramebuffers(1, &(map->fbo));
}

/* Prepares a shadow map for the first rendering pass. */
void shadowRenderFirst(shadowMap *map) {
	glGetIntegerv(GL_VIEWPORT, map->oldViewport);
	glViewport(0, 0, map->width, map->height);
	glBindFramebuffer(GL_FRAMEBUFFER, map->fbo);
	//glClear(GL_DEPTH_BUFFER_BIT);
	/* Use OpenGL's polygon offset feature to push the depth values slightly
	away from the light. This prevents a lit triangle from fighting with itself
	to be lit. It might cause slight over-lighting in some places. */
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(2.0, 4.0);
	/* Another trick is to switch from culling backfaces to culling frontfaces.
	That causes the z-fighting to happen on faces that aren't lit anyway. But
	it assumes closed surfaces. */
	//glCullFace(GL_FRONT);
}

/* Cleans up after the first rendering pass. */
void shadowUnrenderFirst(shadowMap *map) {
	glViewport(map->oldViewport[0], map->oldViewport[1], map->oldViewport[2],
		map->oldViewport[3]);
	glDisable(GL_POLYGON_OFFSET_FILL);
	//glCullFace(GL_BACK);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

/* Prepares for the second rendering pass. */
void shadowRenderSecond(shadowMap *map, GLenum textureUnit,
	GLint textureUnitIndex, GLint textureLoc) {
	glActiveTexture(textureUnit);
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, map->texture);
  glUniform1i(textureLoc, textureUnitIndex);
}

/* Cleans up after the second rendering pass. */
void shadowUnrenderSecond(GLenum textureUnit) {
	glActiveTexture(textureUnit);
  glDisable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, 0);
}
