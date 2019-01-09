


/*** In general dimensions ***/

/* Copies the dim-dimensional vector v to the dim-dimensional vector copy. The 
output can safely alias the input. */
void vecCopy(int dim, const double v[], double copy[]) {
	int i;
	for (i = 0; i < dim; i += 1)
		copy[i] = v[i];
}

/* Adds the dim-dimensional vectors v and w. The output can safely alias the 
input. */
void vecAdd(int dim, const double v[], const double w[], double vPlusW[]) {
	
}

/* Subtracts the dim-dimensional vectors v and w. The output can safely alias 
the input. */
void vecSubtract(int dim, const double v[], const double w[], 
		double vMinusW[]) {
	
}

/* Scales the dim-dimensional vector w by the number c. The output can safely 
alias the input.*/
void vecScale(int dim, double c, const double w[], double cTimesW[]) {
	
}



/*** In specific dimensions ***/

/* By the way, there is a way to write a single vecSet function that works in 
all dimensions. The module stdarg.h, which is part of the C standard library, 
lets you write variable-arity functions. The general vecSet would look like
	void vecSet(int dim, double a[], ...)
where the '...' represents dim numbers to be loaded into a. We're not going to 
take this approach for two reasons. First, I try not to burden you with 
learning a lot of C that isn't strictly necessary. Second, the variable-arity 
feature is a bit dangerous, in that it provides no type checking. */

/* Copies three numbers into a three-dimensional vector. */
void vec3Set(double a0, double a1, double a2, double a[3]) {
	a[0] = a0;
	a[1] = a1;
	a[2] = a2;
}

/* Copies four numbers into a four-dimensional vector. */
void vec4Set(double a0, double a1, double a2, double a3, double a[4]) {
	
}

/* Copies eight numbers into a eight-dimensional vector. */
void vec8Set(double a0, double a1, double a2, double a3, double a4, double a5, 
		double a6, double a7, double a[8]) {
	
}


