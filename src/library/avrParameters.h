/*
  Name:        avrParameters.h
  Version      0.1
  Author:      Felipe Caetano, Luiz Maur�lio, Rodrigo L. S. Silva
  Date:        10/10/2012
  Last Update: 10/10/2012
  Description: Collection of the "parameters" functions of ARToolkit. Receive functions
               from paramChangeSize, paramDecomp, paramDisp, paramDistortion
               paramFile and paramGet.
*/

#ifndef AVR_PARAMETERS_H
#define AVR_PARAMETERS_H
#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
//	Public includes.
// ============================================================================

//#include <AR/config.h>

#include <avrUtil.h>

// ============================================================================
//	Public types and defines.
// ============================================================================

// ============================================================================
//	Public globals.
// ============================================================================

// ============================================================================
//	Public functions.
// ============================================================================

/** \fn int  arParamGet( double global[][3], double screen[][2], int data_num,
                 double mat[3][4] )
* \brief  XXXBK
*
*  XXXBK
* \param global XXXBK
* \param screen XXXBK
* \param data_num XXXBK
* \param mat XXXBK
* \return  XXXBK
*/
int  arParamGet( double global[][3], double screen[][2], int data_num,
                 double mat[3][4] );

/** \fn int  arParamDecomp( ARParam *source, ARParam *icpara, double trans[3][4] )
* \brief  XXXBK
*
*  XXXBK
* \param source XXXBK
* \param icpara XXXBK
* \param trans XXXBK
* \return  XXXBK
*/
int  arParamDecomp( ARParam *source, ARParam *icpara, double trans[3][4] );

/** \fn int arParamDecompMat( double source[3][4], double cpara[3][4], double trans[3][4] )
* \brief  XXXBK
*
*  XXXBK
* \param source input camera matrix
* \param cpara camera parameter to be set
* \param trans XXXBK
* \return  XXXBK
*/
int  arParamDecompMat( double source[3][4], double cpara[3][4], double trans[3][4] );

/** \fn int int arParamIdeal2Observ( const double dist_factor[4], const double ix, const double iy,
						 double *ox, double *oy )

* \brief  Convert ideal screen coordinates of a vertex to observed ones.
*
* Ideal coordinates mean that the distortion of the camera is compensated (so a straight line looks straight).
* In observed coordinates the camera-distortion is not compensated and thus a straight line is not shown really straight.
* \param dist_factor distorsion factors of used camera
* \param ix x in ideal screen coordinates
* \param iy y in ideal screen coordinates
* \param ox resulted x in observed screen coordinates
* \param oy resulted y in observed screen coordinates
* \return 0 if success, -1 otherwise
*/
int arParamIdeal2Observ( const double dist_factor[4], const double ix, const double iy,
                         double *ox, double *oy );

/** \fn int arParamObserv2Ideal( const double dist_factor[4], const double ox, const double oy,
                         double *ix, double *iy )

* \brief Convert observed screen coordinates of a vertex to ideal ones.

* Ideal coordinates mean that the distortion of the camera is compensated (so a straight line looks straight).
* In observed coordinates the camera-distortion is not compensated and thus a straight line is not shown really straight.
* \param dist_factor distorsion factors of used camera
* \param ox x in observed screen coordinates
* \param oy y in observed screen coordinates
* \param ix resulted x in ideal screen coordinates
* \param iy resulted y in ideal screen coordinates
* \return 0 if success, -1 otherwise
*/
int arParamObserv2Ideal( const double dist_factor[4], const double ox, const double oy,
                         double *ix, double *iy );

/** \fn int arParamChangeSize( ARParam *source, int xsize, int ysize, ARParam *newparam )
* \brief change the camera size parameters.
*
* Change the size variable in camera intrinsic parameters.
* \param source name of the source parameters structure
* \param xsize new length size
* \param ysize new height size
* \param newparam name of the destination parameters structure.
* \return 0
*/
int arParamChangeSize( ARParam *source, int xsize, int ysize, ARParam *newparam );

/** \fn int arParamSave( char *filename, int num, ARParam *param, ...)
* \brief save a camera intrinsic parameters.
*
* Save manipulated camera intrinsic parameters in a file.
* \param filename name of the parameters file.
* \param num number of variable arguments
* \param param parameters to save
* \return 0 if success, -1 if Error (file not found, file structure problem)
*/
int    arParamSave( char *filename, int num, ARParam *param, ...);

/** \fn int arParamLoad( const char *filename, int num, ARParam *param, ...)
* \brief load the camera intrinsic parameters.
*
* Load camera intrinsic parameters in the ARToolkit Library from
* a file (itself, an output of the calibration step).
* \param filename name of the parameters file.
* \param num number of variable arguments
* \param param result of the loaded parameters
* \return 0 if success, -1 if Error (file not found, file structure problem)
*/
int    arParamLoad( const char *filename, int num, ARParam *param, ...);

/** \fn int arParamDisp( ARParam *param )
* \brief display parameters.
*
* Display the structure of the camera instrinsic parameters argument.
* \param param structure to display
* \return 0
*/
int    arParamDisp( ARParam *param );

/*-------------------*/

int    arsParamChangeSize( ARSParam *source, int xsize, int ysize, ARSParam *newparam );

int    arsParamSave( char *filename, ARSParam *sparam );

int    arsParamLoad( char *filename, ARSParam *sparam );

int    arsParamDisp( ARSParam *sparam );

int    arsParamGetMat( double matL[3][4], double matR[3][4],
                       double cparaL[3][4], double cparaR[3][4], double matL2R[3][4] );

#ifdef __cplusplus
}
#endif
#endif

