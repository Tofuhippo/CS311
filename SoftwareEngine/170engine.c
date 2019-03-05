


/*
On macOS, compile with...
	clang -c 170engine.c
...and then link with a main program by...
	clang main.c 000pixel.o 170engine.o -lglfw -framework OpenGL
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "000pixel.h"

#include "120vector.c"
#include "140matrix.c"
#include "040texture.c"
#include "130shading.c"
#include "130depth.c"
#include "130triangle.c"
#include "160mesh.c"
#include "140isometry.c"
#include "150camera.c"
#include "140landscape.c"


