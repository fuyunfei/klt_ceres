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
#include <png.h>

int width, height;
png_byte color_type;
png_byte bit_depth;
png_bytep *row_pointers;

void write_pre(KLT_FeatureTable,char* );
void read_png_file(char *filename);

/* #define REPLACE */
#ifdef WIN32
int RunExample3()
#else
int main(int argc, char *argv[])
#endif
{
  read_png_file(argv[1]);
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
   // Part 4: colour
    for (j = 0 ; j < ft->nFeatures ; j++)  {
        float xj,yj;
         xj= (float) ft->feature[j][0]->x;
         yj= (float) ft->feature[j][0]->y;

         png_bytep row = row_pointers[(int)yj];
         png_bytep px = &(row[(int)xj * 4]);
         fprintf(fp,"%f, %4f = RGBA(%3d, %3d, %3d, %3d)\n", xj, yj, px[0], px[1], px[2], px[3]);
    }

}




void read_png_file(char *filename) {
  FILE *fp = fopen(filename, "rb");

  png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if(!png) abort();

  png_infop info = png_create_info_struct(png);
  if(!info) abort();

  if(setjmp(png_jmpbuf(png))) abort();

  png_init_io(png, fp);

  png_read_info(png, info);

  width      = png_get_image_width(png, info);
  height     = png_get_image_height(png, info);
  color_type = png_get_color_type(png, info);
  bit_depth  = png_get_bit_depth(png, info);

  // Read any color_type into 8bit depth, RGBA format.
  // See http://www.libpng.org/pub/png/libpng-manual.txt

  if(bit_depth == 16)
    png_set_strip_16(png);

  if(color_type == PNG_COLOR_TYPE_PALETTE)
    png_set_palette_to_rgb(png);

  // PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
  if(color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
    png_set_expand_gray_1_2_4_to_8(png);

  if(png_get_valid(png, info, PNG_INFO_tRNS))
    png_set_tRNS_to_alpha(png);

  // These color_type don't have an alpha channel then fill it with 0xff.
  if(color_type == PNG_COLOR_TYPE_RGB ||
     color_type == PNG_COLOR_TYPE_GRAY ||
     color_type == PNG_COLOR_TYPE_PALETTE)
    png_set_filler(png, 0xFF, PNG_FILLER_AFTER);

  if(color_type == PNG_COLOR_TYPE_GRAY ||
     color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
    png_set_gray_to_rgb(png);

  png_read_update_info(png, info);

  row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);
  int y ;
  for( y = 0; y < height; y++) {
    row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png,info));
  }

  png_read_image(png, row_pointers);

  fclose(fp);
}
