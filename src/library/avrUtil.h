/*
  Name:        avrUtil.h
  Version      1.0.1
  Author:      Felipe Caetano, Luiz Maurílio, Rodrigo L. S. Silva
  Date:        10/10/2012
  Last Update: 09/10/2014
  Description: Collection of the "util" functions of ARToolkit. Receive functions from:
               * arUtil
               * arMultiReadConfigFile
               * mAlloc, mAllocDup, mAllocInv, mAllocMul, mAllocTrans, mAllocUnit, mDisp, mDup, mFree
               * vAlloc, vDisp, vFree
*/

#ifndef AVR_UTIL_H
#define AVR_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

//! \cond

#define AR_PIXEL_FORMAT_RGB 1
#define AR_PIXEL_FORMAT_BGR 2
#define AR_PIXEL_FORMAT_RGBA 3
#define AR_PIXEL_FORMAT_BGRA 4
#define AR_PIXEL_FORMAT_ABGR 5
#define AR_PIXEL_FORMAT_MONO 6
#define AR_PIXEL_FORMAT_ARGB 7
#define AR_PIXEL_FORMAT_2vuy 8
#define AR_PIXEL_FORMAT_yuvs 9

/*--------------------------------------------------------------*/
/*                                                              */
/*  For Linux, you should define one of below 4 input method    */
/*    AR_INPUT_V4L:       use of standard Video4Linux Library   */
/*    AR_INPUT_GSTREAMER: use of GStreamer Media Framework      */
/*    AR_INPUT_DV:        use of DV Camera                      */
/*    AR_INPUT_1394CAM:   use of 1394 Digital Camera            */
/*                                                              */
/*--------------------------------------------------------------*/
#ifdef __linux
#undef  AR_INPUT_V4L
#undef  AR_INPUT_DV
#undef  AR_INPUT_1394CAM
#undef  AR_INPUT_GSTREAMER

#    define  AR_DEFAULT_PIXEL_FORMAT AR_PIXEL_FORMAT_BGR
/*
#  ifdef AR_INPUT_V4L
#    ifdef USE_EYETOY
#      define AR_DEFAULT_PIXEL_FORMAT AR_PIXEL_FORMAT_RGB
#    else
#      define AR_DEFAULT_PIXEL_FORMAT AR_PIXEL_FORMAT_BGR
#    endif
#  endif

#  ifdef AR_INPUT_DV
#    define  AR_DEFAULT_PIXEL_FORMAT AR_PIXEL_FORMAT_RGB
#  endif

#  ifdef AR_INPUT_1394CAM
#    define  AR_DEFAULT_PIXEL_FORMAT AR_PIXEL_FORMAT_RGB
#  endif

#  ifdef AR_INPUT_GSTREAMER
#    define  AR_DEFAULT_PIXEL_FORMAT AR_PIXEL_FORMAT_RGB
#  endif
*/
#  undef   AR_BIG_ENDIAN
#  define  AR_LITTLE_ENDIAN
#endif

/*------------------------------------------------------------*/
/*  For SGI                                                   */
/*------------------------------------------------------------*/
#ifdef __sgi
#  define  AR_BIG_ENDIAN
#  undef   AR_LITTLE_ENDIAN
#  define  AR_DEFAULT_PIXEL_FORMAT AR_PIXEL_FORMAT_ABGR
#endif

/*------------------------------------------------------------*/
/*  For Windows                                               */
/*------------------------------------------------------------*/
#ifdef _WIN32
#  undef   AR_BIG_ENDIAN
#  define  AR_LITTLE_ENDIAN
#  define  AR_DEFAULT_PIXEL_FORMAT AR_PIXEL_FORMAT_BGRA
#endif

/*------------------------------------------------------------*/
/*  For Mac OS X                                              */
/*------------------------------------------------------------*/
#ifdef __APPLE__
#  if defined(__BIG_ENDIAN__) // Check architecture endianess using gcc's macro.
#    define  AR_BIG_ENDIAN  // Most Significant Byte has greatest address in memory (ppc).
#    undef   AR_LITTLE_ENDIAN
#  elif defined (__LITTLE_ENDIAN__)
#    undef   AR_BIG_ENDIAN   // Least significant Byte has greatest address in memory (i386).
#    define  AR_LITTLE_ENDIAN
#  endif
#  define  AR_DEFAULT_PIXEL_FORMAT AR_PIXEL_FORMAT_ARGB
#endif
/*------------------------------------------------------------*/

#define  AR_DRAW_BY_GL_DRAW_PIXELS    0
#define  AR_DRAW_BY_TEXTURE_MAPPING   1
#define  AR_DRAW_TEXTURE_FULL_IMAGE   0
#define  AR_DRAW_TEXTURE_HALF_IMAGE   1
#define  AR_IMAGE_PROC_IN_FULL        0
#define  AR_IMAGE_PROC_IN_HALF        1
#define  AR_FITTING_TO_IDEAL          0
#define  AR_FITTING_TO_INPUT          1

#ifdef __linux
//#  ifdef AR_INPUT_V4L
#    define   VIDEO_MODE_PAL              0
#    define   VIDEO_MODE_NTSC             1
#    define   VIDEO_MODE_SECAM            2
#    define   DEFAULT_VIDEO_DEVICE        "/dev/video0"
#    define   DEFAULT_VIDEO_WIDTH         640
#    define   DEFAULT_VIDEO_HEIGHT        480
#    define   DEFAULT_VIDEO_CHANNEL       1
#    define   DEFAULT_VIDEO_MODE          VIDEO_MODE_NTSC
//#  endif

#  ifdef AR_INPUT_DV
/* Defines all moved into video.c now - they are not used anywhere else */
#  endif

#  ifdef AR_INPUT_1394CAM
/* Defines all moved into video.c now - they are not used anywhere else */
#  endif

#  define   DEFAULT_IMAGE_PROC_MODE     AR_IMAGE_PROC_IN_FULL
#  define   DEFAULT_FITTING_MODE        AR_FITTING_TO_IDEAL
#  define   DEFAULT_DRAW_MODE           AR_DRAW_BY_TEXTURE_MAPPING
#  define   DEFAULT_DRAW_TEXTURE_IMAGE  AR_DRAW_TEXTURE_HALF_IMAGE
#endif

#ifdef __sgi
#  define   VIDEO_FULL                  0
#  define   VIDEO_HALF                  1
#  define   DEFAULT_VIDEO_SIZE          VIDEO_FULL
#  define   DEFAULT_IMAGE_PROC_MODE     AR_IMAGE_PROC_IN_FULL
#  define   DEFAULT_FITTING_MODE        AR_FITTING_TO_INPUT
#  define   DEFAULT_DRAW_MODE           AR_DRAW_BY_GL_DRAW_PIXELS
#  define   DEFAULT_DRAW_TEXTURE_IMAGE  AR_DRAW_TEXTURE_HALF_IMAGE
#endif

#ifdef _WIN32
#  define   DEFAULT_IMAGE_PROC_MODE     AR_IMAGE_PROC_IN_FULL
#  define   DEFAULT_FITTING_MODE        AR_FITTING_TO_INPUT
#  define   DEFAULT_DRAW_MODE           AR_DRAW_BY_TEXTURE_MAPPING
#  define   DEFAULT_DRAW_TEXTURE_IMAGE  AR_DRAW_TEXTURE_FULL_IMAGE
#endif

#ifdef __APPLE__
#  define   DEFAULT_VIDEO_WIDTH         640
#  define   DEFAULT_VIDEO_HEIGHT        480
#  define   DEFAULT_IMAGE_PROC_MODE     AR_IMAGE_PROC_IN_FULL
#  define   DEFAULT_FITTING_MODE        AR_FITTING_TO_IDEAL
#  define   DEFAULT_DRAW_MODE           AR_DRAW_BY_TEXTURE_MAPPING
#  define   DEFAULT_DRAW_TEXTURE_IMAGE  AR_DRAW_TEXTURE_FULL_IMAGE
#undef    APPLE_TEXTURE_FAST_TRANSFER
#endif

/*  For NVIDIA OpenGL Driver  */
#undef    AR_OPENGL_TEXTURE_RECTANGLE

#if (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_ABGR) || (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_BGRA) || (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_RGBA) || (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_ARGB)
#  define AR_PIX_SIZE_DEFAULT      4
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_BGR) || (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_RGB)
#  define AR_PIX_SIZE_DEFAULT      3
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_2vuy) || (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_yuvs)
#  define AR_PIX_SIZE_DEFAULT      2
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_MONO)
#  define AR_PIX_SIZE_DEFAULT      1
#else
#  error Unknown default pixel format defined in config.h.
#endif

#define   AR_CHAIN_MAX     10000
#define   AR_PATT_SIZE_X   16
#define   AR_PATT_SIZE_Y   16

#define   AR_GL_CLIP_NEAR  50.0
#define   AR_GL_CLIP_FAR   5000.0

#define   AR_HMD_XSIZE     640
#define   AR_HMD_YSIZE     480

//config.h

//! \endcond
// ============================================================================
//	Public types and defines.
// ============================================================================
/** \def arMalloc(V,T,S)
* \brief allocation macro function
*
* allocate S elements of type T.
* \param V returned allocated area pointer
* \param T type of element
* \param S number of elements
*/
#define arMalloc(V,T,S)  \
{ if( ((V) = (T *)malloc( sizeof(T) * (S) )) == 0 ) \
{printf("malloc error!!\n"); exit(1);} }

/* overhead ARToolkit type*/
typedef char              ARInt8;
typedef short             ARInt16;
typedef int               ARInt32;
typedef unsigned char     ARUint8;
typedef unsigned short    ARUint16;
typedef unsigned int      ARUint32;

//! \cond
/** \typedef AR_PIXEL_FORMAT
	\brief ARToolKit pixel-format specifiers.

	ARToolKit functions can accept pixel data in a variety of formats.
	This enumerations provides a set of constants you can use to request
	data in a particular pixel format from an ARToolKit function that
	returns data to you, or to specify that data you are providing to an
	ARToolKit function is in a particular pixel format.

	AR_PIXEL_FORMAT_RGB
	Each pixel is represented by 24 bits. Eight bits per each Red, Green,
	and Blue component. This is the native 24 bit format for the Mac platform.

	AR_PIXEL_FORMAT_BGR
	Each pixel is represented by 24 bits. Eight bits per each Blue, Red, and
	Green component. This is the native 24 bit format for the Win32 platform.

	AR_PIXEL_FORMAT_RGBA
	Each pixel is represented by 32 bits. Eight bits per each Red, Green,
	Blue, and Alpha component.

	AR_PIXEL_FORMAT_BGRA
	Each pixel is represented by 32 bits. Eight bits per each Blue, Green,
	Red, and Alpha component. This is the native 32 bit format for the Win32
	platform.

	AR_PIXEL_FORMAT_ABGR
	Each pixel is represented by 32 bits. Eight bits per each Alpha, Blue,
	Green, and Red component. This is the native 32 bit format for the SGI
	platform.

	AR_PIXEL_FORMAT_ARGB
	Each pixel is represented by 32 bits. Eight bits per each Alpha, Red,
	Green, and Blue component. This is the native 32 bit format for the Mac
	platform.

	AR_PIXEL_FORMAT_MONO
	Each pixel is represented by 8 bits of luminance information.

	AR_PIXEL_FORMAT_2vuy
	8-bit 4:2:2 Component Y'CbCr format. Each 16 bit pixel is represented
	by an unsigned eight bit luminance component and two unsigned eight bit
	chroma components. Each pair of pixels shares a common set of chroma
	values. The components are ordered in memory; Cb, Y0, Cr, Y1. The
	luminance components have a range of [16, 235], while the chroma value
	has a range of [16, 240]. This is consistent with the CCIR601 spec.
	This format is fairly prevalent on both Mac and Win32 platforms.
	'2vuy' is the Apple QuickTime four-character code for this pixel format.
	The equivalent Microsoft fourCC is 'UYVY'.

	AR_PIXEL_FORMAT_yuvs
	8-bit 4:2:2 Component Y'CbCr format. Identical to the AR_PIXEL_FORMAT_2vuy except
	each 16 bit word has been byte swapped. This results in a component
	ordering of; Y0, Cb, Y1, Cr.
	This is most prevalent yuv 4:2:2 format on both Mac and Win32 platforms.
	'yuvs' is the Apple QuickTime four-character code for this pixel format.
	The equivalent Microsoft fourCC is 'YUY2'.
 */
typedef int AR_PIXEL_FORMAT;

/** \struct ARMarkerInfo
* \brief ARToolKit struct, main structure for detected marker of internal use.
*
* Store information after contour detection (in idea screen coordinate, after distorsion compensated).
* \remark lines are represented by 3 values a,b,c for ax+by+c=0
* \param area number of pixels in the labeled region
* \param id marker identitied number
* \param dir Direction that tells about the rotation about the marker (possible values are 0, 1, 2 or 3). This parameter makes it possible to tell about the line order of the detected marker (so which line is the first one) and so find the first vertex. This is important to compute the transformation matrix in arGetTransMat().
* \param cf confidence value (probability to be a marker)
* \param pos center of marker (in ideal screen coordinates)
* \param line line equations for four side of the marker (in ideal screen coordinates)
* \param vertex edge points of the marker (in ideal screen coordinates)
*/
typedef struct {
    int     area;
    int     id;
    int     dir;
    double  cf;
    double  pos[2];
    double  line[4][3];
    double  vertex[4][2];
} ARMarkerInfo;

/** \struct ARMarkerInfo2
* \brief ARToolKit struct, use internally for marker detection.
*
* Store information after contour detection (in observed screen coordinate, before distorsion correction).
* \param area number of pixels in the labeled region
* \param pos position of the center of the marker (in observed screen coordinates)
* \param coord_num numer of pixels in the contour.
* \param x_coord x coordinate of the pixels of contours (size limited by AR_CHAIN_MAX).
* \param y_coord y coordinate of the pixels of contours (size limited by AR_CHAIN_MAX).
* \param vertex position of the vertices of the marker. (in observed screen coordinates)
		 rem:the first vertex is stored again as the 5th entry in the array – for convenience of drawing a line-strip easier.
*
*/
typedef struct {
    int     area;
    double  pos[2];
    int     coord_num;
    int     x_coord[AR_CHAIN_MAX];
    int     y_coord[AR_CHAIN_MAX];
    int     vertex[5];
} ARMarkerInfo2;

/** \struct ARMultiEachMarkerInfoT
* \brief ARToolKit struct, multi-marker structure of internal use
*
* Structure for multi-marker tracking
* really similar to ARMarkerInfo
* \param patt_id identification of the pattern
* \param width width of the pattern (in mm)
* \param center center of the pattern (in mm)
* \param trans estimated position of the pattern
* \param itrans relative position of the pattern
* \param pos3d final position of the pattern
* \param visible boolean flag for visibility
* \param visibleR last state visibility
*/
typedef struct {
    int     patt_id;
    double  width;
    double  center[2];
    double  trans[3][4];
    double  itrans[3][4];
    double  pos3d[4][3];
    int     visible;
/*---*/
    int     visibleR;
} ARMultiEachMarkerInfoT;

/** \struct ARMultiMarkerInfoT
* \brief ARToolKit struct, global multi-marker structure of internal use
*
* Main structure for multi-marker tracking.
*
* \param marker list of markers of the multi-marker pattern
* \param marker_num number of markers used
* \param trans position of the multi-marker pattern (more precisely, the camera position in the multi-marker CS)
* \param prevF boolean flag for visibility
* \param transR last position
*/
typedef struct {
    ARMultiEachMarkerInfoT  *marker;
    int                     marker_num;
    double                  trans[3][4];
    int                     prevF;
/*---*/
    double                  transR[3][4];
} ARMultiMarkerInfoT;

//! \endcond

// ============================================================================
//	Public globals.
// ============================================================================
/** \var int arDebug
* \brief activate artoolkit debug mode
*
* control debug informations in ARToolKit.
* the possible values are:
* - 0: not in debug mode
* - 1: in debug mode
* by default: 0
*/
extern int        arDebug;
/** \var ARUint8 *arImage
* \brief internal image
*
* internal image used. (access only for debugging ARToolKit)
* by default: NULL
*/
extern ARUint8*   arImage;
/** \var int arFittingMode
* \brief fitting display mode use by ARToolkit.
*
* Correction mode for the distorsion of the camera.
* You can enable a correction with a texture mapping.
* the possible values are:
* - AR_FITTING_TO_INPUT: input image
* - AR_FITTING_TO_IDEAL: compensated image
* by default: DEFAULT_FITTING_MODE in config.h
*/
extern int        arFittingMode;
/** \var int arImageProcMode
* \brief define the image size mode for marker detection.
*
* Video image size for marker detection. This control
* if all the image is analyzed
* the possible values are :
* - AR_IMAGE_PROC_IN_FULL: full image uses.
* - AR_IMAGE_PROC_IN_HALF: half image uses.
* by default: DEFAULT_IMAGE_PROC_MODE in config.h
*/
extern int        arImageProcMode;
/** \var int arImXsize
* \brief internal image size in width.
*
* internal image size in width (generally initialize in arInitCparam)
*/
/** \var int arImYsize
* \brief internal image size in heigth
*
* internal image size in heigth (generally initialize in arInitCparam)
*/
extern int        arImXsize, arImYsize;
/** \var int arTemplateMatchingMode
*
* the possible values are :
* AR_TEMPLATE_MATCHING_COLOR: Color Template
* AR_TEMPLATE_MATCHING_BW: BW Template
* by default: DEFAULT_TEMPLATE_MATCHING_MODE in config.h
*/
extern int        arTemplateMatchingMode;
/** \var int arMatchingPCAMode
*
* the possible values are :
* -AR_MATCHING_WITHOUT_PCA: without PCA
* -AR_MATCHING_WITH_PCA: with PCA
* by default: DEFAULT_MATCHING_PCA_MODE in config.h
*/
extern int        arMatchingPCAMode;

// ============================================================================
//	Public functions.
// ============================================================================
/*
    Utility
*/
// ar,h
/** \fn int arUtilMatInv( double s[3][4], double d[3][4] )
* \brief Inverse a non-square matrix.
*
* Inverse a matrix in a non homogeneous format. The matrix
* need to be euclidian.
* \param s matrix input
* \param d resulted inverse matrix.
* \return 0 if the inversion success, -1 otherwise
* \remark input matrix can be also output matrix
*/
int      arUtilMatInv( double s[3][4], double d[3][4] );
/** \fn int arUtilMatMul( double s1[3][4], double s2[3][4], double d[3][4] )
* \brief Multiplication of two matrix.
*
* This procedure do a multiplication matrix between s1 and s2 and return
* the result in d : d=s1*s2. The precondition is the output matrix
* need to be different of input matrix. The precondition is euclidian matrix.
* \param s1 first matrix.
* \param s2 second matrix.
* \param d resulted multiplication matrix.
* \return 0 if the multiplication success, -1 otherwise
*/
int      arUtilMatMul( double s1[3][4], double s2[3][4], double d[3][4] );
/** \fn int arUtilMat2QuatPos( double m[3][4], double q[4], double p[3] )
* \brief extract a quaternion/position of matrix.
*
* Extract a rotation (quaternion format) and a position (vector format)
* from a transformation matrix. The precondition is an euclidian matrix.
* \param m source matrix
* \param q a rotation represented by a quaternion.
* \param p a translation represented by a vector.
* \return 0 if the extraction success, -1 otherwise (quaternion not normalize)
*/
int      arUtilMat2QuatPos( double m[3][4], double q[4], double p[3] );
/** \fn int arUtilQuatPos2Mat( double q[4], double p[3], double m[3][4] )
* \brief create a matrix with a quaternion/position.
*
* Create a transformation matrix from a quaternion rotation and a vector translation.
* \param q a rotation represented by a quaternion.
* \param p a translation represented by a vector.
* \param m destination matrix
* \return always 0
*/
int      arUtilQuatPos2Mat( double q[4], double p[3], double m[3][4] );

/** \fn double arUtilTimer(void)
* \brief get the time with the ARToolkit timer.
*
* Give the time elapsed since the reset of the timer.
* \return elapsed time (in milliseconds)
*/
double   arUtilTimer(void);
/** \fn void   arUtilTimerReset(void)
* \brief reset the internal timer of ARToolkit.
*
* Reset the internal timer used by ARToolKit.
* timer measurement (with arUtilTimer()).
*/
void     arUtilTimerReset(void);
/** \fn void   arUtilSleep( int msec )
* \brief sleep the actual thread.
*
* Sleep the actual thread.
* \param msec time to sleep (in millisecond)
*/
void     arUtilSleep( int msec );

//ar.h

//arMulti.h

/** \fn int arMultiActivate( ARMultiMarkerInfoT *config )
* \brief activate a multi-marker pattern on the recognition procedure.
*
* Activate a multi-marker for be checking during the template matching
* operation.
* \param config pointer to the multi-marker
* \return 0 if success, -1 if error
*/
int arMultiActivate( ARMultiMarkerInfoT *config );
/** \fn int arMultiDeactivate( ARMultiMarkerInfoT *config )
* \brief Desactivate a multi-marker pattern on the recognition procedure.
*
* Desactivate a multi-marker for not be checking during the template matching
* operation.
* \param config pointer to the multi-marker
* \return 0 if success, -1 if error
*/
int arMultiDeactivate( ARMultiMarkerInfoT *config );
/** \fn int arMultiFreeConfig( ARMultiMarkerInfoT *config )
* \brief remove a multi-marker pattern from memory.
*
* desactivate a pattern and remove it from memory. Post-condition
* of this function is unavailability of the multi-marker pattern.
* \param config pointer to the multi-marker
* \return 0 if success, -1 if error
*/
int arMultiFreeConfig( ARMultiMarkerInfoT *config );

//arMulti.h

//ar.h
/** \fn int arLoadPatt( const char *filename )
* \brief load markers description from a file
*
* load the bitmap pattern specified in the file filename into the pattern
* matching array for later use by the marker detection routines.
* \param filename name of the file containing the pattern bitmap to be loaded
* \return the identity number of the pattern loaded or –1 if the pattern load failed.
*/
int arLoadPatt( const char *filename );
/** \fn int arFreePatt( int patt_no )
* \brief remove a pattern from memory.
*
* desactivate a pattern and remove from memory. post-condition
* of this function is unavailability of the pattern.
* \param patt_no number of pattern to free
* \return return 1 in success, -1 if error
*/
int arFreePatt( int patt_no );
/** \fn int arActivatePatt( int pat_no )
* \brief activate a pattern on the recognition procedure.
*
* Activate a pattern to be check during the template matching
* operation.
* \param patt_no number of pattern to activate
* \return return 1 in success, -1 if error
*/
int arActivatePatt( int pat_no );
/** \fn int arDeactivatePatt( int pat_no )
* \brief desactivate a pattern on the recognition procedure.
*
* Desactivate a pattern for not be check during the template matching
* operation.
* \param patt_no number of pattern to desactivate
* \return return 1 in success, -1 if error
*/
int arDeactivatePatt( int pat_no );

/** \fn int arGetCode(ARUint8 *image, int *x_coord, int *y_coord, int *vertex, int *code, int *dir, double *cf )
* \return always 0
*/
int arGetCode( ARUint8 *image, int *x_coord, int *y_coord, int *vertex, int *code, int *dir, double *cf );
/** \fn int arGetPatt(ARUint8 *image, int *x_coord, int *y_coord, int *vertex, ARUint8 ext_pat[AR_PATT_SIZE_Y][AR_PATT_SIZE_X][3] )
* \brief Get a normalized pattern from a video image.
*
* This function returns a normalized pattern from a video image. The
* format is a table with AR_PATT_SIZE_X by AR_PATT_SIZE_Y
* \param image video input image
* \param ext_pat detected pattern.
* \return 0 if sucess, -1 otherwise
*/
int arGetPatt( ARUint8 *image, int *x_coord, int *y_coord, int *vertex, ARUint8 ext_pat[AR_PATT_SIZE_Y][AR_PATT_SIZE_X][3] );
/** \fn int arGetContour( ARInt16 *limage, int *label_ref, int label, int clip[4], ARMarkerInfo2 *marker_info2 )
* \return  0 if sucess, -1 otherwise
*/
int arGetContour( ARInt16 *limage, int *label_ref, int label, int clip[4], ARMarkerInfo2 *marker_info2 );

/** \fn int arSavePatt( ARUint8 *image, ARMarkerInfo *marker_info, char *filename )
* \brief save a marker.
*
* used in mk_patt to save a bitmap of the pattern of the currently detected marker.
* The saved image is a table of the normalized viewed pattern.
* \param image a pointer to the image containing the marker pattern to be trained.
* \param marker_info a pointer to the ARMarkerInfo structure of the pattern to be trained.
* \param filename The name of the file where the bitmap image is to be saved.
* \return 0 if the bitmap image is successfully saved, -1 otherwise.
*/
int arSavePatt( ARUint8 *image, ARMarkerInfo *marker_info, char *filename );

//ar.h

/*
  Internal processing
*/
//Labeling

/** \fn ARInt16 *arLabeling ( ARUint8 *image, int thresh,int *label_num, int **area, double **pos, int **clip, int **label_ref )
* \brief extract connected components from image.
*
* Label the input image, i.e. extract connected components from the
* input video image.
* \param image input image, as returned by arVideoGetImage()
* \param thresh lighting threshold
* \param label_num Ouput- number of detected components
* \param area On return, if label_num > 0, points to an array of ints, one for each detected component.
* \param pos On return, if label_num > 0, points to an array of doubles, one for each detected component.
* \param clip On return, if label_num > 0, points to an array of ints, one for each detected component.
* \param label_ref On return, if label_num > 0, points to an array of ints, one for each detected component.
* \return returns a pointer to the labeled output image, ready for passing onto the next stage of processing.
*/
ARInt16* arLabeling( ARUint8 *image, int thresh,int *label_num, int **area, double **pos, int **clip,int **label_ref );
/** \fn ARInt16 *arsLabeling( ARUint8 *image, int thresh,int *label_num, int **area, double **pos, int **clip,int **label_ref, int LorR )
*  \brief similar to arLabeling function
*  \return returns a pointer to the labeled output image, ready for passing onto the next stage of processing.
*/
ARInt16* arsLabeling( ARUint8 *image, int thresh,int *label_num, int **area, double **pos, int **clip,int **label_ref, int LorR );
/** \fn void arLabelingCleanup(void)
 * \brief clean up static data allocated by arLabeling.
 *
 * In debug mode, arLabeling may allocate and use static storage.
 * This function deallocates this storage.
 */
void     arLabelingCleanup(void);

//Labeling

/** \fn ARMultiMarkerInfoT *arMultiReadConfigFile( const char *filename )
* \brief loading multi-markers description from a file
*
* Load a configuration file for multi-markers tracking. The configuration
* file furnishs pointer to each pattern description.
*
* \param filename name of the pattern file
* \return a pattern structure, NULL if error
*/
ARMultiMarkerInfoT*  arMultiReadConfigFile( const char *filename );

#ifdef __cplusplus
}
#endif
#endif
