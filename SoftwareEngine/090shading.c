/*
Dawson d'Almeida and Justin T. Washington
January 17 2018
CS311 with Josh Davis

Contains struct for pixel shading attributes.
*/


typedef struct shaShading shaShading;

struct shaShading{
  int unifDim, texNum, attrDim, varyDim;
  void (*colorPixel)(int unifDim, const double unif[], int texNum,
  		const texTexture *tex[], int varyDim, const double vary[],
  		double rgb[3]);
  void (*transformVertex)(int unifDim, const double unif[], int attrDim,
      const double attr[], int varyDim, double vary[]);
};
