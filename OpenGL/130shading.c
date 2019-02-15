/*
Dawson d'Almeida and Justin T. Washington
January 27 2018
CS311 with Josh Davis

Contains struct for pixel shading attributes.
*/


typedef struct shaShading shaShading;

struct shaShading{
  int unifDim, attrDim, texNum, varyDim;
  void (*colorPixel)(int unifDim, const double unif[], int texNum,
			const texTexture *tex[], int varyDim, const double vary[],
			double rgbd[4]);
  void (*transformVertex)(int unifDim, const double unif[], int attrDim,
      const double attr[], int varyDim, double vary[]);
};
