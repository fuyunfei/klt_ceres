/**********************************************************************
Finds the 150 best features in an image and tracks them through the 
next two images.  The sequential mode is set in order to speed
processing.  The features are stored in a feature table, which is then
saved to a text file; each feature list is also written to a PPM file.
**********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include "pnmio.h"
#include "klt.h"


void write_pre(KLT_FeatureTable,char* );
/* #define REPLACE */
#ifdef WIN32
int RunExample3()
#else
int main()
#endif
{
  unsigned char *img1, *img2;
  char fnamein[100], fnameout[100];
  KLT_TrackingContext tc;
  KLT_FeatureList fl;
  KLT_FeatureTable ft;
  int nFeatures = 150, nFrames = 10;
  int ncols, nrows;
  int i;

  tc = KLTCreateTrackingContext();
  fl = KLTCreateFeatureList(nFeatures);
  ft = KLTCreateFeatureTable(nFrames, nFeatures);
  tc->sequentialMode = TRUE;
  tc->writeInternalImages = FALSE;
  tc->affineConsistencyCheck = -1;  /* set this to 2 to turn on affine consistency check */
 
  img1 = pgmReadFile("img0.pgm", NULL, &ncols, &nrows);
  img2 = (unsigned char *) malloc(ncols*nrows*sizeof(unsigned char));

  KLTSelectGoodFeatures(tc, img1, ncols, nrows, fl);
  KLTStoreFeatureList(fl, ft, 0);
  KLTWriteFeatureListToPPM(fl, img1, ncols, nrows, "feat0.ppm");

  for (i = 1 ; i < nFrames ; i++)  {
    sprintf(fnamein, "img%d.pgm", i);
    pgmReadFile(fnamein, img2, &ncols, &nrows);
    KLTTrackFeatures(tc, img1, img2, ncols, nrows, fl);
#ifdef REPLACE
    KLTReplaceLostFeatures(tc, img2, ncols, nrows, fl);
#endif
    KLTStoreFeatureList(fl, ft, i);
    sprintf(fnameout, "feat%d.ppm", i);
    KLTWriteFeatureListToPPM(fl, img2, ncols, nrows, fnameout);
  }

    write_pre(ft,"pre.txt");
//  KLTWriteFeatureTable(ft, "features.txt", "%5.1f");
//  KLTWriteFeatureTable(ft, "features.ft", NULL);

  KLTFreeFeatureTable(ft);
  KLTFreeFeatureList(fl);
  KLTFreeTrackingContext(tc);
  free(img1);
  free(img2);

  return 0;
}

void write_pre(KLT_FeatureTable ft,char* fname){
    int i,j;
    FILE * fp;
    fp=fopen(fname,"wb");
   // Part 1: point_index camera_index feature_xy ;
    for (j = 0 ; j < ft->nFeatures ; j++)  {
      for (i = 0 ; i < ft->nFrames ; i++){
      if (ft->feature[j][i]->x>0){
      fprintf(fp, "%d ", j);
      fprintf(fp, "%d ", i);
      fprintf(fp, "%f %f", (float) ft->feature[j][i]->x, (float) ft->feature[j][i]->y);
      fprintf(fp, "\n");
      }
      }
    }
   // Part 2: intrinsic
    for (i = 0 ; i < ft->nFrames ; i++){

    int intrinsic[]={0,0,0,0,0,0,1781,0,0};
    for(j=0;j<9;j++){
        fprintf(fp,"%d \n",intrinsic[j]);}
    }
   // Part 3: 3D points based on img0
    for (j = 0 ; j < ft->nFeatures ; j++)  {
        float xj,yj;
         xj= (float) ft->feature[j][0]->x;
         yj= (float) ft->feature[j][0]->y;
        fprintf(fp, "%f \n%f \n%f \n",xj,yj,0.3 );
    }
}
