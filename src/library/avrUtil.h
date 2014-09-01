/*
  Name:        avrUtil.h
  Version      0.1
  Author:      Felipe Caetano, Luiz Maurílio, Rodrigo L. S. Silva
  Date:        10/10/2012
  Last Update: 10/10/2012
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

#define resetMatrix3x4(m)\
    m[0][0] = 0; m[0][1] = 0; m[0][2] = 0; m[0][3] = 0;\
    m[1][0] = 0; m[1][1] = 0; m[1][2] = 0; m[1][3] = 0;\
    m[2][0] = 0; m[2][1] = 0; m[2][2] = 0; m[2][3] = 0;

#define loadIdentityMatrix3x4(m)\
    m[0][0] = 1; m[0][1] = 0; m[0][2] = 0; m[0][3] = 0;\
    m[1][0] = 0; m[1][1] = 1; m[1][2] = 0; m[1][3] = 0;\
    m[2][0] = 0; m[2][1] = 0; m[2][2] = 1; m[2][3] = 0;

#define multiMatrix3x4Vector3(dest,m,v)\
    dest[0] = m[0][0]*v[0] + m[0][1]*v[1] + m[0][2]*v[2] + m[0][3];\
	dest[1] = m[1][0]*v[0] + m[1][1]*v[1] + m[1][2]*v[2] + m[1][3];\
	dest[2] = m[2][0]*v[0] + m[2][1]*v[1] + m[2][2]*v[2] + m[2][3];

#define euclidianDistanceMatrix3x4(result,src1,src2)\
    result = 0;\
	result += pow(src1[0][3]-src2[0][3],2);\
    result += pow(src1[1][3]-src2[1][3],2);\
    result += pow(src1[2][3]-src2[2][3],2);\
	result = sqrt(result);

#define euclidianDistanceVector3(result,src1,src2)\
    result = 0;\
	result += pow(src1[0]-src2[0],2); \
	result += pow(src1[1]-src2[1],2); \
	result += pow(src1[2]-src2[2],2); \
    result = sqrt(result);

#define scalarProduct4(result,src1,src2)\
    result = 0;\
	result += src1[0]*src2[0];\
	result += src1[1]*src2[1];\
	result += src1[2]*src2[2];\
	result += src1[3]*src2[3];

#define copyMatrix3x4(dest,src)\
    dest[0][0] = src[0][0]; dest[0][1] = src[0][1]; dest[0][2] = src[0][2]; dest[0][3] = src[0][3];\
    dest[1][0] = src[1][0]; dest[1][1] = src[1][1]; dest[1][2] = src[1][2]; dest[1][3] = src[1][3];\
    dest[2][0] = src[2][0]; dest[2][1] = src[2][1]; dest[2][2] = src[2][2]; dest[2][3] = src[2][3];

//
// As of version 2.72, ARToolKit supports an OpenGL-like
// versioning system, with both header versions (for the version
// of the ARToolKit SDK installed) and runtime version reporting
// via arGetVersion().
//

// The MAJOR version number defines non-backwards compatible
// changes in the ARToolKit API. Range: [0-99].
#define AR_HEADER_VERSION_MAJOR		2

// The MINOR version number defines additions to the ARToolKit
// API, or (occsasionally) other significant backwards-compatible
// changes in runtime functionality. Range: [0-99].
#define AR_HEADER_VERSION_MINOR		72

// The TINY version number defines bug-fixes to existing
// functionality. Range: [0-99].
#define AR_HEADER_VERSION_TINY		0

// The BUILD version number will always be zero in releases,
// but may be non-zero in internal builds or in version-control
// repository-sourced code. Range: [0-99].
#define AR_HEADER_VERSION_BUILD		0

// The string representation below must match the major, minor
// and tiny release numbers.
#define AR_HEADER_VERSION_STRING	"2.72.0"

// The macros below are convenience macros to enable use
// of certain ARToolKit header functionality by the release
// version in which it appeared.
// Each time the major version number is incremented, all
// existing macros must be removed, and just one new one
// added for the new major version.
// Each time the minor version number is incremented, a new
// AR_HAVE_HEADER_VERSION_ macro definition must be added.
// Tiny version numbers (being bug-fix releases, by definition)
// are NOT included in the AR_HAVE_HEADER_VERSION_ system.
#define AR_HAVE_HEADER_VERSION_2
#define AR_HAVE_HEADER_VERSION_2_72

//
// End version definitions.
//

#define AR_PIXEL_FORMAT_RGB 1
#define AR_PIXEL_FORMAT_BGR 2
#define AR_PIXEL_FORMAT_RGBA 3
#define AR_PIXEL_FORMAT_BGRA 4
#define AR_PIXEL_FORMAT_ABGR 5
#define AR_PIXEL_FORMAT_MONO 6
#define AR_PIXEL_FORMAT_ARGB 7
#define AR_PIXEL_FORMAT_2vuy 8
#define AR_PIXEL_FORMAT_UYVY AR_PIXEL_FORMAT_2vuy
#define AR_PIXEL_FORMAT_yuvs 9
#define AR_PIXEL_FORMAT_YUY2 AR_PIXEL_FORMAT_yuvs

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

#define  AR_TEMPLATE_MATCHING_COLOR   0
#define  AR_TEMPLATE_MATCHING_BW      1
#define  AR_MATCHING_WITHOUT_PCA      0
#define  AR_MATCHING_WITH_PCA         1
#define  DEFAULT_TEMPLATE_MATCHING_MODE     AR_TEMPLATE_MATCHING_COLOR
#define  DEFAULT_MATCHING_PCA_MODE          AR_MATCHING_WITHOUT_PCA


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


#define   AR_GET_TRANS_MAT_MAX_LOOP_COUNT         5
#define   AR_GET_TRANS_MAT_MAX_FIT_ERROR          1.0
#define   AR_GET_TRANS_CONT_MAT_MAX_FIT_ERROR     1.0

#define   AR_AREA_MAX      100000
#define   AR_AREA_MIN          70


#define   AR_SQUARE_MAX        30
#define   AR_CHAIN_MAX      10000
#define   AR_PATT_NUM_MAX      50
#define   AR_PATT_SIZE_X       16
#define   AR_PATT_SIZE_Y       16
#define   AR_PATT_SAMPLE_NUM   64

#define   AR_GL_CLIP_NEAR      50.0
#define   AR_GL_CLIP_FAR     5000.0

#define   AR_HMD_XSIZE        640
#define   AR_HMD_YSIZE        480

#define   AR_PARAM_NMIN         6
#define   AR_PARAM_NMAX      1000
#define   AR_PARAM_C34        100.0

///config.h

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
* \brief main structure for detected marker.
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
* \brief internal structure use for marker detection.
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
* \brief multi-marker structure
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
* \brief global multi-marker structure
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

/** \struct ARParam
* \brief camera intrinsic parameters.
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

typedef struct {
    int      xsize, ysize;
    double   matL[3][4];
    double   matR[3][4];
    double   matL2R[3][4];
    double   dist_factorL[4];
    double   dist_factorR[4];
} ARSParam;


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
extern int      arDebug;

/** \var ARUint8 *arImage
* \brief internal image
*
* internal image used. (access only for debugging ARToolKit)
* by default: NULL
*/
extern ARUint8  *arImage;

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
extern int      arFittingMode;

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
extern int      arImageProcMode;

/** \var ARParam arParam
* \brief internal intrinsic camera parameter
*
* internal variable for camera intrinsic parameters
*/
extern ARParam  arParam;

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
extern int      arImXsize, arImYsize;

/** \var int arTemplateMatchingMode
* \brief XXXBK
*
* XXXBK
* the possible values are :
* AR_TEMPLATE_MATCHING_COLOR: Color Template
* AR_TEMPLATE_MATCHING_BW: BW Template
* by default: DEFAULT_TEMPLATE_MATCHING_MODE in config.h
*/
extern int      arTemplateMatchingMode;

/** \var int arMatchingPCAMode
* \brief XXXBK
*
* XXXBK
* the possible values are :
* -AR_MATCHING_WITHOUT_PCA: without PCA
* -AR_MATCHING_WITH_PCA: with PCA
* by default: DEFAULT_MATCHING_PCA_MODE in config.h
*/
extern int      arMatchingPCAMode;

/* === matrix definition ===

  <---- clm --->
  [ 10  20  30 ] ^
  [ 20  10  15 ] |
  [ 12  23  13 ] row
  [ 20  10  15 ] |
  [ 13  14  15 ] v

=========================== */

/** \struct ARMat
* \brief matrix structure.
*
* Defined the structure of the matrix type based on a dynamic allocation.
* The matrix format is :<br>
*  <---- clm ---><br>
*  [ 10  20  30 ] ^<br>
*  [ 20  10  15 ] |<br>
*  [ 12  23  13 ] row<br>
*  [ 20  10  15 ] |<br>
*  [ 13  14  15 ] v<br>
*
* \param m content of matrix
* \param row number of lines in matrix
* \param clm number of column in matrix
*/
typedef struct {
	double *m;
	int row;
	int clm;
} ARMat;

/** \struct ARVec
* \brief vector structure.
*
* The vector format is :<br>
*  <---- clm ---><br>
*  [ 10  20  30 ]<br>
* Defined the structure of the vector type based on a dynamic allocation.
* \param m content of vector
* \param clm number of column in matrix
*/
typedef struct {
        double *v;
        int    clm;
} ARVec;

/** \def ARELEM0(mat,r,c)
* \brief macro function that give direct access to an element (0 origin)
*
*
*/
/* 0 origin */
#define ARELEM0(mat,r,c) ((mat)->m[(r)*((mat)->clm)+(c)])

/** \def ARELEM1(mat,row,clm)
* \brief macro function that give direct access to an element (1 origin)
*
*
*/
/* 1 origin */
#define ARELEM1(mat,row,clm) ARELEM0(mat,row-1,clm-1)


///matrix.h
/** \fn ARMat *arMatrixAlloc(int row, int clm)
* \brief creates a new matrix.
*
* Allocate and initialize a new matrix structure.
* XXXBK initializing ?? to 0 m ??
* \param row number of line
* \param clm number of column
* \return the matrix structure, NULL if allocation is impossible
*/
ARMat  *arMatrixAlloc(int row, int clm);

/** \fn int arMatrixFree(ARMat *m)
* \brief deletes a matrix.
*
* Delete a matrix structure (deallocate used memory).
* \param m matrix to delete
* \return 0
*/
int    arMatrixFree(ARMat *m);

/** \fn int arMatrixDup(ARMat *dest, ARMat *source)
* \brief copy a matrix
*
* copy one matrix to another. The two ARMat must
* be allocated.
* \param dest the destination matrix of the copy
* \param source the original matrix source
* \return 0 if success, -1 if error (matrix with different size)
*/
int    arMatrixDup(ARMat *dest, ARMat *source);

/** \fn ARMat *arMatrixAllocDup(ARMat *source)
* \brief dumps a new matrix
*
* Allocates and recopy the original source matrix.
* \param source the source matrix to copy
* \return the matrix if success, NULL if error
*/
ARMat  *arMatrixAllocDup(ARMat *source);

/** \fn ARVec *arVecAlloc( int clm )
* \brief creates a new vector.
*
* Allocates and initializes new vector structure.
* \param clm dimension of vector
* \return the allocated vector, NULL if error (impossible allocation)
*/
ARVec  *arVecAlloc( int clm );

/** \fn int arVecFree( ARVec *v )
* \brief delete a vector.
*
* Delete a vector structure (deallocate used memory).
* \param v the vector to delete
* \return 0
*/
int    arVecFree( ARVec *v );

/** \fn int arVecDisp( ARVec *v )
* \brief display a vector.
*
* Display element of a vector.
* \param v the vector to display
* \return 0
*/
int    arVecDisp( ARVec *v );

///matrix.h



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
extern int      arDebug;

/** \var ARUint8 *arImage
* \brief internal image
*
* internal image used. (access only for debugging ARToolKit)
* by default: NULL
*/
extern ARUint8  *arImage;

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
extern int      arFittingMode;

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
extern int      arImageProcMode;

/** \var ARParam arParam
* \brief internal intrinsic camera parameter
*
* internal variable for camera intrinsic parameters
*/
extern ARParam  arParam;

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
extern int      arImXsize, arImYsize;

/** \var int arTemplateMatchingMode
* \brief XXXBK
*
* XXXBK
* the possible values are :
* AR_TEMPLATE_MATCHING_COLOR: Color Template
* AR_TEMPLATE_MATCHING_BW: BW Template
* by default: DEFAULT_TEMPLATE_MATCHING_MODE in config.h
*/
extern int      arTemplateMatchingMode;

/** \var int arMatchingPCAMode
* \brief XXXBK
*
* XXXBK
* the possible values are :
* -AR_MATCHING_WITHOUT_PCA: without PCA
* -AR_MATCHING_WITH_PCA: with PCA
* by default: DEFAULT_MATCHING_PCA_MODE in config.h
*/
extern int      arMatchingPCAMode;

// ============================================================================
//	Public functions.
// ============================================================================

/*
   Initialization
*/

/**
 * \brief Get the ARToolKit version information in numberic and string format.
 *
 * As of version 2.72, ARToolKit now allows querying of the version number
 * of the toolkit available at runtime. It is highly recommended that
 * any calling program that depends on features in a certain
 * ARToolKit version, check at runtime that it is linked to a version
 * of ARToolKit that can supply those features. It is NOT sufficient
 * to check the ARToolKit SDK header versions, since with ARToolKit implemented
 * in dynamically-loaded libraries, there is no guarantee that the
 * version of ARToolKit installed on the machine at run-time will as
 * recent as the version of the ARToolKit SDK which the host
 * program was compiled against.
 * The version information is reported in binary-coded decimal format,
 * and optionally in an ASCII string. See the config.h header
 * for more discussion of the definition of major, minor, tiny and build
 * version numbers.
 *
 * \param versionStringRef
 *	If non-NULL, the location pointed to will be filled
 *	with a pointer to a string containing the version information.
 *  Fields in the version string are separated by spaces. As of version
 *  2.72.0, there is only one field implemented, and this field
 *  contains the major, minor and tiny version numbers
 *	in dotted-decimal format. The string is guaranteed to contain
 *  at least this field in all future versions of the toolkit.
 *  Later versions of the toolkit may add other fields to this string
 *  to report other types of version information. The storage for the
 *  string is malloc'ed inside the function. The caller is responsible
 *  for free'ing the string.
 *
 * \return Returns the full version number of the ARToolKit in
 *	binary coded decimal (BCD) format.
 *  BCD format allows simple tests of version number in the caller
 *  e.g. if ((arGetVersion(NULL) >> 16) > 0x0272) printf("This release is later than 2.72\n");
 *	The major version number is encoded in the most-significant byte
 *  (bits 31-24), the minor version number in the second-most-significant
 *	byte (bits 23-16), the tiny version number in the third-most-significant
 *  byte (bits 15-8), and the build version number in the least-significant
 *	byte (bits 7-0).
 */
ARUint32 arGetVersion(char **versionStringRef);

/**
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
int arInitCparam( ARParam *param );

/*
    Utility
*/

/**
* \brief Inverse a non-square matrix.
*
* Inverse a matrix in a non homogeneous format. The matrix
* need to be euclidian.
* \param s matrix input
* \param d resulted inverse matrix.
* \return 0 if the inversion success, -1 otherwise
* \remark input matrix can be also output matrix
*/
int    arUtilMatInv( double s[3][4], double d[3][4] );

/**
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
int    arUtilMatMul( double s1[3][4], double s2[3][4], double d[3][4] );

/**
* \brief extract a quaternion/position of matrix.
*
* Extract a rotation (quaternion format) and a position (vector format)
* from a transformation matrix. The precondition is an euclidian matrix.
* \param m source matrix
* \param q a rotation represented by a quaternion.
* \param p a translation represented by a vector.
* \return 0 if the extraction success, -1 otherwise (quaternion not normalize)
*/
int    arUtilMat2QuatPos( double m[3][4], double q[4], double p[3] );

/**
* \brief create a matrix with a quaternion/position.
*
* Create a transformation matrix from a quaternion rotation and a vector translation.
* \param q a rotation represented by a quaternion.
* \param p a translation represented by a vector.
* \param m destination matrix
* \return always 0
*/
int    arUtilQuatPos2Mat( double q[4], double p[3], double m[3][4] );

/**
* \brief get the time with the ARToolkit timer.
*
* Give the time elapsed since the reset of the timer.
* \return elapsed time (in milliseconds)
*/
double arUtilTimer(void);

/**
* \brief reset the internal timer of ARToolkit.
*
* Reset the internal timer used by ARToolKit.
* timer measurement (with arUtilTimer()).
*/
void   arUtilTimerReset(void);

/**
* \brief sleep the actual thread.
*
* Sleep the actual thread.
* \param msec time to sleep (in millisecond)
*/
void   arUtilSleep( int msec );

/**
* \brief estimate a line from a list of point.
*
* Compute a linear regression from a list of point.
* \param x_coord X coordinate of points
* \param y_coord Y coordinate of points
* \param coord_num number of points
* \param vertex XXXBK
* \param line XXXBK
* \param v XXXBK
* \return  XXXBK
*/
int arGetLine(int x_coord[], int y_coord[], int coord_num,
              int vertex[], double line[4][3], double v[4][2]);

/*------------------------------------*/

extern ARUint8  *arImageL;
extern ARUint8  *arImageR;
extern ARSParam arsParam;
extern double   arsMatR2L[3][4];

int           arsInitCparam      ( ARSParam *sparam );

int           arsGetLine         ( int x_coord[], int y_coord[], int coord_num,
                                   int vertex[], double line[4][3], double v[4][2], int LorR);

///ar.h

#ifdef __cplusplus
}
#endif
#endif
