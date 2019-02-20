


/*** Procedural interface ***/

/* This code in this section helps you compile and link OpenGL shader programs.
The process is simpler than it looks here. In pseudocode, the core idea is 10
steps:
	glCreateShader(vertexShader);
		glShaderSource(vertexShader, vertexCode);
		glCompileShader(vertexShader);
	glCreateShader(fragmentShader);
		glShaderSource(fragmentShader, fragmentCode);
		glCompileShader(fragmentShader);
	glCreateProgram(program);
		glAttachShader(vertexShader);
		glAttachShader(fragmentShader);
		glLinkProgram(program);
The code below is much longer than 10 steps, only because it has lots of error
checking. */

/* Compiles a shader from GLSL source code. type is either GL_VERTEX_SHADER or
GL_FRAGMENT_SHADER. If an error occurs, then returns 0. Otherwise, returns a
compiled shader, which the user must eventually deallocate with glDeleteShader.
(But usually this function is called from shaMakeProgram, which calls
glDeleteShader on the user's behalf.) */
GLuint shaMakeShader(GLenum type, const GLchar *shaderCode) {
	GLuint shader = glCreateShader(type);
	if (shader == 0) {
		fprintf(stderr, "shaMakeShader: glCreateShader failed\n");
		return 0;
	}
	glShaderSource(shader, 1, &shaderCode, NULL);
	glCompileShader(shader);
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE) {
		GLsizei length;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
		GLchar *infoLog = (GLchar *)malloc(length);
		glGetShaderInfoLog(shader, length, &length, infoLog);
		fprintf(stderr, "shaMakeShader: glGetShaderInfoLog:\n%s\n", infoLog);
		free(infoLog);
		glDeleteShader(shader);
		return 0;
	}
	return shader;
}

/* Compiles and links a shader program from two pieces of GLSL source code. If
an error occurs, then returns 0. Otherwise, returns a shader program, which the
user should eventually deallocate using glDeleteProgram. */
GLuint shaMakeProgram(const GLchar *vertexCode, const GLchar *fragmentCode) {
	GLuint vertexShader, fragmentShader, program;
	vertexShader = shaMakeShader(GL_VERTEX_SHADER, vertexCode);
	if (vertexShader == 0)
		return 0;
	fragmentShader = shaMakeShader(GL_FRAGMENT_SHADER, fragmentCode);
	if (fragmentShader == 0) {
		glDeleteShader(vertexShader);
		return 0;
	}
	program = glCreateProgram();
	if (program == 0) {
		fprintf(stderr, "error: shaMakeProgram: glCreateProgram failed\n");
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
		return 0;
	}
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) {
		GLsizei length;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
		GLchar *infoLog = (GLchar *)malloc(length);
		glGetProgramInfoLog(program, length, &length, infoLog);
		fprintf(stderr, "error: shaMakeProgram: glGetProgramInfoLog:\n%s\n",
			infoLog);
		free(infoLog);
    	glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
		glDeleteProgram(program);
		return 0;
    }
    /* Success. The shaders are built into the program and don't need to be
    remembered separately, so delete them. */
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return program;
}

/* Checks the validity of a shader program against the rest of the current
OpenGL state. If you choose to use this function, invoke it *after*
shaMakeProgram, setting up textures, etc. Returns 0 if okay, non-zero if error.
*/
int shaValidateProgram(GLuint program) {
	GLint validation;
    glValidateProgram(program);
	glGetProgramiv(program, GL_VALIDATE_STATUS, &validation);
	if (validation != GL_TRUE) {
		GLsizei length;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
		GLchar *infoLog = (GLchar *)malloc(length);
		glGetProgramInfoLog(program, length, &length, infoLog);
		fprintf(stderr, "shaValidateProgram: glGetProgramInfoLog:\n%s\n",
			infoLog);
		free(infoLog);
		return 1;
	}
	return 0;
}



/*** Object-oriented interface ***/

/* This data structure packages a compiled shader program with its locations. */

/* Feel free to read from this struct's members, but don't write to them except
through the accessor functions. */
typedef struct shaShading shaShading;
struct shaShading {
	GLuint program;
	int unifNum, attrNum;
	GLint *unifLocs, *attrLocs;
};

/* Frees the resources underlying the shading program. You must call this
function when you are done using the program. */
void shaDestroy(shaShading *sha) {
	glDeleteProgram(sha->program);
	free(sha->unifLocs);
}

/* Returns error code; 0 on success and non-zero on failure. Don't forget to
shaDestroy when you are done using the shader program. */
int shaInitialize(shaShading *sha, const GLchar *vertexCode,
		const GLchar *fragmentCode, int unifNum, const GLchar *unifNames[],
		int attrNum, const GLchar *attrNames[]) {
	sha->unifLocs = (GLint *)malloc((unifNum + attrNum) * sizeof(GLint));
	if (sha->unifLocs == NULL)
		return 1;
	sha->program = shaMakeProgram(vertexCode, fragmentCode);
	if (sha->program == 0) {
		free(sha->unifLocs);
		return 2;
	}
	sha->unifNum = unifNum;
	sha->attrNum = attrNum;
	sha->attrLocs = &(sha->unifLocs[unifNum]);
	glUseProgram(sha->program);
	int i;
	for (i = 0; i < unifNum; i += 1) {
		sha->unifLocs[i] = glGetUniformLocation(sha->program, unifNames[i]);
		if (sha->unifLocs[i] == -1) {
			fprintf(stderr,
				"error: shaInitialize: uniform location %s does not exist\n",
				unifNames[i]);
			shaDestroy(sha);
			return 3;
		}
	}
	for (i = 0; i < attrNum; i += 1) {
		sha->attrLocs[i] = glGetAttribLocation(sha->program, attrNames[i]);
		if (sha->attrLocs[i] == -1) {
			fprintf(stderr,
				"error: shaInitialize: attribute location %s does not exist\n",
				attrNames[i]);
			shaDestroy(sha);
			return 4;
		}
	}
	return 0;
}
