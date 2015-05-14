/*
  Name:        avrParameters.h
  Version      1.0.1
  Author:      Felipe Caetano, Luiz Maurílio, Rodrigo L. S. Silva
  Date:        10/10/2012
  Last Update: 09/10/2014
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
//	Public types and defines.
// ============================================================================
//! \cond
/** \struct ARParam
* \brief ARToolKit struct, camera intrinsic parameters.
*
* This structure contains the main parameters for
* the intrinsic parameters of the camera
* representation. The camera used is a pinhole
* camera with standard parameters. User should
* consult a computer vision reference for more
* information. (e.g. Three-Dimensional Computer Vision
* (Artificial Intelligence) by Olivier Faugeras).
* \param xsize length of the image (in pixels).
* \param ysize height of the image (in pixels).
* \param mat perspective matrix (K).
* \param dist_factor radial distortions factor
*          dist_factor[0]=x center of distortion
*          dist_factor[1]=y center of distortion
*          dist_factor[2]=distortion factor
*          dist_factor[3]=scale factor
*/
typedef struct {
    int      xsize, ysize;
    double   mat[3][4];
    double   dist_factor[4];
} ARParam;

/** \struct ARSParam
*  \brief ARToolKit structure, similar to ARParam structure
*/
typedef struct {
    int      xsize, ysize;
    double   matL[3][4];
    double   matR[3][4];
    double   matL2R[3][4];
    double   dist_factorL[4];
    double   dist_factorR[4];
} ARSParam;

//! \endcond
// ============================================================================
//	Public globals.
// ============================================================================
/** \var ARParam arParam
* \brief internal intrinsic camera parameter
*
* internal variable for camera intrinsic parameters
*/
extern ARParam  arParam;
/** \var ARSParam arsParam
* \brief internal intrinsic camera parameter
*
* internal variable for camera intrinsic parameters
*/
extern ARSParam arsParam;

// ============================================================================
//	Public functions.
// ============================================================================
/** \fn int arInitCparam( ARParam *param )
* \brief initialize camera parameters.
*
* set the camera parameters specified in the camera parameters structure
* *param to static memory in the AR library. These camera parameters are
* typically read from a data file at program startup. In the video-see through
* AR applications, the default camera parameters are sufficient, no camera
* calibration is needed.
* \param param the camera parameter structure
* \return always 0
*/
int   arInitCparam( ARParam *param );

/** \fn arGetLine(int x_coord[], int y_coord[], int coord_num, int vertex[], double line[4][3], double v[4][2])
*  \fn int arsGetLine( int x_coord[], int y_coord[], int coord_num, int vertex[], double line[4][3], double v[4][2], int LorR)
* \brief estimate a line from a list of point.
*
* Compute a linear regression from a list of point.
* \return always 0
*/
int   arGetLine(int x_coord[], int y_coord[], int coord_num, int vertex[], double line[4][3], double v[4][2]);
/** \fn arGetLine(int x_coord[], int y_coord[], int coord_num, int vertex[], double line[4][3], double v[4][2])
*  \brief Similar to int arGetLine
*  \return always 0
*/
int   arsGetLine( int x_coord[], int y_coord[], int coord_num, int vertex[], double line[4][3], double v[4][2], int LorR);

/** \fn int arParamDecompMat( double source[3][4], double cpara[3][4], double trans[3][4] )
* \return always 0
*/
int   arParamDecompMat( double source[3][4], double cpara[3][4], double trans[3][4] );
/** \fn int int arParamIdeal2Observ( const double dist_factor[4], const double ix, const double iy, double *ox, double *oy )
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
int   arParamIdeal2Observ( const double dist_factor[4], const double ix, const double iy, double *ox, double *oy );
/** \fn int arParamObserv2Ideal( const double dist_factor[4], const double ox, const double oy, double *ix, double *iy )
* \brief Convert observed screen coordinates of a vertex to ideal ones.
*
* Ideal coordinates mean that the distortion of the camera is compensated (so a straight line looks straight).
* In observed coordinates the camera-distortion is not compensated and thus a straight line is not shown really straight.
* \param dist_factor distorsion factors of used camera
* \param ox x in observed screen coordinates
* \param oy y in observed screen coordinates
* \param ix resulted x in ideal screen coordinates
* \param iy resulted y in ideal screen coordinates
* \return 0 if success, -1 otherwise
*/
int   arParamObserv2Ideal( const double dist_factor[4], const double ox, const double oy, double *ix, double *iy );
/** \fn int arParamChangeSize( ARParam *source, int xsize, int ysize, ARParam *newparam )
* \brief change the camera size parameters.
*
* Change the size variable in camera intrinsic parameters.
* \param source name of the source parameters structure
* \param xsize new length size
* \param ysize new height size
* \param newparam name of the destination parameters structure.
* \return always 0
*/
int   arParamChangeSize( ARParam *source, int xsize, int ysize, ARParam *newparam );

/** \fn int  arParamGet( double global[][3], double screen[][2], int data_num, double mat[3][4] )
* \return 0 if success, -1 otherwise
*/
int   arParamGet( double global[][3], double screen[][2], int data_num, double mat[3][4] );
/** \fn int arParamSave( char *filename, int num, ARParam *param, ...)
* \brief save a camera intrinsic parameters.
*
* Save manipulated camera intrinsic parameters in a file.
* \param filename name of the parameters file.
* \param num number of variable arguments
* \param param parameters to save
* \return 0 if success, -1 if Error (file not found, file structure problem)
*/
int   arParamSave( char *filename, int num, ARParam *param, ...);
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
int   arParamLoad( const char *filename, int num, ARParam *param, ...);
/** \fn int arParamDisp( ARParam *param )
* \brief display parameters.
*
* Display the structure of the camera instrinsic parameters argument.
* \param param structure to display
* \return always 0
*/
int   arParamDisp( ARParam *param );

#ifdef __cplusplus
}
#endif
#endif

