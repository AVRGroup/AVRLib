/*
  Name:        avrMarker.h
  Version      0.1
  Author:      Felipe Caetano, Luiz Maurílio, Rodrigo L. S. Silva
  Date:        10/10/2012
  Last Update: 10/10/2012
  Description: Collection of the graphical functions of ARToolkit. Receive functions
               from gsub and config.
*/


#ifndef AVR_GRAPHICS_H
#define AVR_GRAPHICS_H

///config.h

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

#ifdef __cplusplus
extern "C" {
#endif


// ============================================================================
//	Public includes.
// ============================================================================

//#include <AR/config.h>
//#include <AR/param.h>
//#include <AR/ar.h>

#include <avrParameters.h> // << this include avrUtil

	// ============================================================================
//	Public types and defines.
// ============================================================================

// ============================================================================
//	Public globals.
// ============================================================================

/** \var int argDrawMode
* \brief define the draw configuration mode.
*
* Define the draw mode for display of the video background.
* The possible values are :
* - AR_DRAW_BY_GL_DRAW_PIXELS: use the GL_DRAW_PIXELS function
* - AR_DRAW_BY_TEXTURE_MAPPING: use a quad mesh with a texture mapping
* of the video.
* by default: DEFAULT_DRAW_MODE in config.h
* \rem choice and performance depends on your hardware and your openGL driver.
*/
extern int  argDrawMode;

/** \var int argTexmapMode
* \brief define the texture map configuration mode.
*
* If the draw mode is AR_DRAW_BY_TEXTURE_MAPPING, you can
* configure the copy mode of the texture mapping.
* The possible values are :
* - AR_DRAW_TEXTURE_FULL_IMAGE: texture mapping full resolution.
* - AR_DRAW_TEXTURE_HALF_IMAGE: texture mapping half resolution.
* by default: DEFAULT_DRAW_TEXTURE_IMAGE in config.h
*/
extern int  argTexmapMode;

// ============================================================================
//	Public functions.
// ============================================================================

/** \fn argInit( ARParam *cparam, double zoom, int fullFlag, int xwin, int ywin, int hmd_flag )
* \brief Initialise the gsub library
*
*  This function performs required initialisation of the gsub library.
*  It must be called before any other argl*() functions are called.
* \param cparam the intrinsics parameters of the camera (used to defined openGL perspective matrix)
* \param zoom defined a zoom parameter for the final result.
* \param fullFlag full screen mode (1 enable, 0 disable).
* \param xwin XXXBK. 0 if indifferent.
* \param ywin XXXBK. 0 if indifferent.
* \param hmd_flag enable stereo display mode (only interleaved configuration)
*/
void argInit( ARParam *cparam, double zoom, int fullFlag, int xwin, int ywin, int hmd_flag );

/** \fn void argLoadHMDparam( ARParam *lparam, ARParam *rparam )
* \brief initialize camera for HMD.
*
* Load in the display module the intrinsic parameters of the two view,
* i.e camera (identify to the eyes).
*
* \param lparam parameter of left camera
* \param rparam parameter of right camera
*/
void argLoadHMDparam( ARParam *lparam, ARParam *rparam );

/** \fn void argCleanup( void )
* \brief Close the gsub library.
*
* This function clean the rendering context (GLUT and openGL).
* Call in the exit of your program.
* \remark BE CAREFUL, THIS FUNCTION DOESN'T RESET PERSPECTIVE
* MATRIX AND CURRENT GL STATE TO DEFAULT
*/
void argCleanup( void );

/** \fn void argSwapBuffers( void )
* \brief swap the rendering buffer.
*
* Swap the back-buffer to the front-buffer. the
* pre-condition is that all the rendering functions have been
* called.
*/
void argSwapBuffers( void );

/** \fn void argMainLoop( void (*mouseFunc)(int button, int state, int x, int y),
                  void (*keyFunc)(unsigned char key, int x, int y),
                  void (*mainFunc)(void) )
* \brief start the program main loop with specified callback functions.
*
* This function is called in the entry block of a program. User
* specify the main callback of his program. Users should not
* put routines calls after this function, generally never accessible.
* \param mouseFunc the user mouse function can be NULL.
* \param keyFunc the user keyboard function can be NULL.
* \param mainFunc the user main update function can be NULL.
*/
void argMainLoop( void (*mouseFunc)(int button, int state, int x, int y),
                  void (*keyFunc)(unsigned char key, int x, int y),
                  void (*mainFunc)(void) );

/** \fn void argDrawMode2D( void )
* \brief switch the rendering context for 2D rendering mode.
*
* Update curent camera parameters (internal and external)
* for rendering 2D or 3D objects in the view plane (like text or 2D shape).
* This function define an orthographic projection in the image
* plane. It not define opengl state for rendering in image space (like
* for a bitmap copy).
*/
void argDrawMode2D( void );

/** \fn void argDraw2dLeft( void )
* \brief switch the rendering view to left eye  (in 2D space)
*
* Combine with argDrawMode2D for rendering the left view.
*/
void argDraw2dLeft( void );

/** \fn void argDraw2dRight( void )
* \brief switch the rendering view to right eye (in 2D space)
*
* Combine with argDrawMode2D for rendering the right view.
*/
void argDraw2dRight( void );

/** \fn void argDrawMode3D( void )
* \brief switch the rendering context for 3D rendering mode.
*
* Update curent camera parameters for rendering in 3D space.
* Generally call to reinializing model view matrix.
*/
void argDrawMode3D( void );

/** \fn void argDraw3dLeft( void )
* \brief switch the rendering view to left eye (in 3D space)
*
* Update curent internal camera parameters for rendering in 3D space
* for left eye.
* this function complements argDrawMode3D.
*/
void argDraw3dLeft( void );

/** \fn void argDraw3dRight( void )
* \brief switch the rendering view to right eye (in 3D space)
*
* Update curent internal camera parameters for rendering in 3D space
* for left eye.
* this function complements argDrawMode3D.
*/
void argDraw3dRight( void );

/** \fn void argDraw3dCamera( int xwin, int ywin )
* \brief switch the rendering view for 3D rendering mode.
*
* Update curent internal camera parameters for rendering in 3D space.
* this function complements argDrawMode3D.
* \param xwin length of rendering view (less than window length)
* \param ywin width of rendering view (less than window width)
*/
void argDraw3dCamera( int xwin, int ywin );


/** \fn void argConvGlpara( double para[3][4], double gl_para[16] )
* \brief transform ARToolKit matrix format to an openGL matrix format.
*
* simple conversion for the openGL matrix (16 values and homogeneous matrix).
* Returned value is generally use with a Model View Matrix.
* \param para the ARToolKit matrix
* \param gl_para the resulted openGL matrix
*/
void argConvGlpara( double para[3][4], double gl_para[16] );

/** \fn void argConvGLcpara( ARParam *param, double gnear, double gfar, double m[16] )
* \brief transform ARToolKit intrinsic camera parameters matrix format to an openGL matrix format.
*
* XXXBK: not be sure of this function:
* this function must just convert 3x4 matrix to classical perspective openGL matrix.
* But in the code, you used arParamDecompMat that seem decomposed K and R,t, aren't it ?
* why do this decomposition since we want just intrinsic parameters ? and if not what is arDecomp ?
*
* Returned value is generally use with a Projection Matrix.
* \param param
* \param gnear near clipping plane value
* \param gfar far clipping plane value
* \param m  the resulted openGL matrix
*/
void argConvGLcpara( ARParam *param, double gnear, double gfar, double m[16] );

/** \fn void argDispImage( ARUint8 *image, int xwin, int ywin )
* \brief display the video image.
*
* Display in the back-buffer the video image in argument.
* For doing AR video background, this function must be called before
* any rendering of 3D object.
* \remark According to your argDrawMode, argTexmapMode and the internal
* image format the openGL function called is different and less
* or more efficient.
* \remark with AR_DRAW_BY_GL_DRAW_PIXELS, unaffected by current camera
* parameters matrix but affected by glRasterPos3f.
* \remark with AR_DRAW_BY_TEXTURE_MAPPING, affected by current current camera
* parameters matrix. You need generally call argDrawMode2D before this function.
* \param image image to display
* \param xwin XXXBK
* \param ywin XXXBK
*/
void argDispImage( ARUint8 *image, int xwin, int ywin );

/** \fn void argDispHalfImage( ARUint8 *image, int xwin, int ywin )
* \brief display half of the video image.
*
* Idem of argDispImage except than a quarter of the image is display
* (first left top quadrant, so size/2 in x and y).
* \param image image to display
* \param xwin XXXBK
* \param ywin XXXBK
*/
void argDispHalfImage( ARUint8 *image, int xwin, int ywin );

/** \fn void argDrawSquare( double vertex[4][2], int xwin, int ywin )
* \brief draw a 2D square.
*
* Draw a square. The position of the square is affected by openGL
* model view matrix and call to argDrawMode2D argDrawMode3D.
* Generally call in a 2D mode (so after a argDrawMode2D).
*
* \param vertex corner of square.
* \param xwin XXXBK
* \param ywin XXXBK
*/
void argDrawSquare( double vertex[4][2], int xwin, int ywin );

/** \fn void argLineSeg( double x1, double y1, double x2, double y2, int xwin, int ywin )
* \brief Draw a line.
*
* Draw a segment.T The position of the line is affected by openGL
* model view matrix and call to argDrawMode2D argDrawMode3D.
* Generally call in a 2D mode (so after a argDrawMode2D).
* \param x1 x position of the first point.
* \param y1 y position of the first point.
* \param x2 x position of the second point.
* \param y2 y position of the second point.
* \param xwin XXXBK
* \param ywin XXXBK
*/
void argLineSeg( double x1, double y1, double x2, double y2, int xwin, int ywin );

/** \fn void argLineSegHMD( double x1, double y1, double x2, double y2 )
* \brief  Draw a line with HMD mode.
*
* Draw a segment in HMD mode.
* \param x1 x position of the first point.
* \param y1 y position of the first point.
* \param x2 x position of the second point.
* \param y2 y position of the second point.
*/
void argLineSegHMD( double x1, double y1, double x2, double y2 );

/** \fn argInqSetting( int *hmdMode,
                    int *gMiniXnum2, int *gMiniYnum2,
                    void (**mouseFunc)(int button, int state, int x, int y),
                    void (**keyFunc)(unsigned char key, int x, int y),
                    void (**mainFunc)(void) );
* \brief Get current configuration of gsub library.
*
* Retrieve current state of gsub library like the current callback functions.
* \param hmdMode the current hmdMode
* \param gMiniXnum2 XXXBK
* \param gMiniYnum2 XXXBK
* \param mouseFunc the current mouse function callback
* \param keyFunc the current key function callback
* \param mainFunc the current main function callback
*/
void argInqSetting( int *hmdMode,
                    int *gMiniXnum2, int *gMiniYnum2,
                    void (**mouseFunc)(int button, int state, int x, int y),
                    void (**keyFunc)(unsigned char key, int x, int y),
                    void (**mainFunc)(void) );

/*-------------------------*/

void argsInit( ARSParam *scparam, double zoom, int twinFlag, int fullFlag, int xwin, int ywin );
void argsDraw3dCamera( int xwin, int ywin, int LorR, int stencil_flag );
void argsConvGLcpara( ARSParam *sparam, double gnear, double gfar, double mL[16], double mR[16] );
void argsDispImage( ARUint8 *image, int LorR, int xwin, int ywin );
void argsDispHalfImage( ARUint8 *image, int LorR, int xwin, int ywin );
void argsLineSeg( double x1, double y1, double x2, double y2, int xwin, int ywin, int LorR );
void argsDrawSquare( double  vertex[4][2], int xwin, int ywin, int LorR );

void removeWarning();

///ar.h
/*
  Internal processing
*/

/**
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
ARInt16 *arLabeling( ARUint8 *image, int thresh,
                     int *label_num, int **area, double **pos, int **clip,
                     int **label_ref );

/**
 * \brief clean up static data allocated by arLabeling.
 *
 * In debug mode, arLabeling may allocate and use static storage.
 * This function deallocates this storage.
 */
 void arLabelingCleanup(void);

 /**
* \brief  XXXBK
*
*  XXXBK
* \param num XXXBK
* \param area XXXBK
* \param clip XXXBK
* \param pos XXXBK
*/
void arGetImgFeature( int *num, int **area, int **clip, double **pos );

void          arsGetImgFeature   ( int *num, int **area, int **clip, double **pos, int LorR );

ARInt16      *arsLabeling        ( ARUint8 *image, int thresh,
                                   int *label_num, int **area, double **pos, int **clip,
                                   int **label_ref, int LorR );


///ar.h

///gsub_lite.h

// ============================================================================
//	Public includes.
// ============================================================================

#ifdef __APPLE__
#  include <OpenGL/gl.h>
#else
#  ifdef _WIN32
#    include <windows.h>
#  endif
#  include <GL/gl.h>
#endif
//#include <AR/config.h>
//#include <AR/ar.h>		// ARUint8, AR_PIXEL_FORMAT, arDebug, arImage.
//#include <AR/param.h>	// ARParam, arParamDecompMat(), arParamObserv2Ideal()

///
#include <avrParameters.h>
///

// ============================================================================
//	Public types and definitions.
// ============================================================================

// Keep code nicely typed.
#ifndef TRUE
#  define TRUE 1
#endif
#ifndef FALSE
#  define FALSE 0
#endif

/*!
    @typedef ARGL_CONTEXT_SETTINGS_REF
    @abstract Opaque type to hold ARGL settings for a given OpenGL context.
    @discussion
		An OpenGL context is an implementation-defined structure which
		keeps track of OpenGL state, including textures and display lists.
		Typically, individual OpenGL windows will have distinct OpenGL
		contexts assigned to them by the host operating system.

		As gsub_lite uses textures and display lists, it must be able to
		track which OpenGL context a given texture or display list it is using
		belongs to. This is especially important when gsub_lite is being used to
		draw into more than one window (and therefore more than one context.)

		Basically, functions which depend on OpenGL state, will require an
		ARGL_CONTEXT_SETTINGS_REF to be passed to them. An ARGL_CONTEXT_SETTINGS_REF
		is generated by setting the current OpenGL context (e.g. if using GLUT,
		you might call glutSetWindow()) and then calling arglSetupForCurrentContext().
		When you have finished using ARGL in a given context, you should call
		arglCleanup(), passing in an ARGL_CONTEXT_SETTINGS_REF, to free the
		memory used by the settings structure.
	@availability First appeared in ARToolKit 2.68.
 */
typedef struct _ARGL_CONTEXT_SETTINGS *ARGL_CONTEXT_SETTINGS_REF;

// ============================================================================
//	Public globals.
// ============================================================================

#if defined(__APPLE__)
extern int arglAppleClientStorage;
extern int arglAppleTextureRange;
#endif // __APPLE__

// ============================================================================
//	Public functions.
// ============================================================================

/*!
    @function
    @abstract Initialise the gsub_lite library for the current OpenGL context.
    @discussion
		This function performs required setup of the gsub_lite library
		for the current OpenGL context and must be called before any other argl*()
		functions are called for this context.

		An OpenGL context holds all of the state of the OpenGL machine, including
		textures and display lists etc. There will usually be one OpenGL context
		for each window displaying OpenGL content.

		Other argl*() functions whose operation depends on OpenGL state will
		require an ARGL_CONTEXT_SETTINGS_REF. This is just so that
		they can keep track of per-context variables.

		You should call arglCleanup() passing in the ARGL_CONTEXT_SETTINGS_REF
		when you have finished with the library for this context.
    @result An ARGL_CONTEXT_SETTINGS_REF. See the documentation for this type for more info.
	@availability First appeared in ARToolKit 2.68.
*/
ARGL_CONTEXT_SETTINGS_REF arglSetupForCurrentContext(void);

/*!
    @function
    @abstract Free memory used by gsub_lite associated with the specified context.
    @discussion
		Should be called after no more argl* functions are needed, in order
		to prevent memory leaks etc.

		The library can be setup again for the context at a later time by calling
		arglSetupForCurrentContext() again.
	@param contextSettings A reference to ARGL's settings for an OpenGL
		context, as returned by arglSetupForCurrentContext().
	@availability First appeared in ARToolKit 2.68.
*/
void arglCleanup(ARGL_CONTEXT_SETTINGS_REF contextSettings);

/*!
    @function
    @abstract Create an OpenGL perspective projection matrix.
    @discussion
		Use this function to create a matrix suitable for passing to OpenGL
		to set the viewing projection.
    @param cparam Pointer to a set of ARToolKit camera parameters for the
		current video source.
	@param focalmax The maximum distance at which geometry will be rendered.
		Any geometry further away from the camera than this distance will be clipped
		and will not be appear in a rendered frame. Thus, this value should be
		set high enough to avoid clipping of any geometry you care about. However,
		the precision of the depth buffer is correlated with the ratio of focalmin
		to focalmax, thus you should not set focalmax any higher than it needs to be.
		This value should be specified in the same units as your OpenGL drawing.
	@param focalmin The minimum distance at which geometry will be rendered.
		Any geometry closer to the camera than this distance will be clipped
		and will not be appear in a rendered frame. Thus, this value should be
		set low enough to avoid clipping of any geometry you care about. However,
		the precision of the depth buffer is correlated with the ratio of focalmin
		to focalmax, thus you should not set focalmin any lower than it needs to be.
		Additionally, geometry viewed in a stereo projections that is too close to
		camera is difficult and tiring to view, so if you are rendering stereo
		perspectives you should set this value no lower than the near-point of
		the eyes. The near point in humans varies, but usually lies between 0.1 m
		0.3 m. This value should be specified in the same units as your OpenGL drawing.
	@param m_projection Pointer to a array of 16 GLdoubles, which will be filled
		out with a projection matrix suitable for passing to OpenGL. The matrix
		is specified in column major order.
	@availability First appeared in ARToolKit 2.68.
*/
void arglCameraFrustum(ARParam *cparam, const double focalmin, const double focalmax, GLdouble m_projection[16]);

/*!
    @function
    @abstract   (description)
    @discussion (description)
    @param      (name) (description)
    @result     (description)
*/
void arglCameraFrustumRH(ARParam *cparam, const double focalmin, const double focalmax, GLdouble m_projection[16]);

/*!
    @function
    @abstract Create an OpenGL viewing transformation matrix.
	@discussion
		Use this function to create a matrix suitable for passing to OpenGL
		to set the viewing transformation of the virtual camera.
	@param para Pointer to 3x4 matrix array of doubles which specify the
		position of an ARToolKit marker, as returned by arGetTransMat().
	@param m_modelview Pointer to a array of 16 GLdoubles, which will be filled
		out with a modelview matrix suitable for passing to OpenGL. The matrix
		is specified in column major order.
	@param scale Specifies a scaling between ARToolKit's
		units (usually millimeters) and OpenGL's coordinate system units.
		What you pass for the scalefactor parameter depends on what units you
		want to do your OpenGL drawing in. If you use a scalefactor of 1.0, then
		1.0 OpenGL unit will equal 1.0 millimetre (ARToolKit's default units).
		To use different OpenGL units, e.g. metres, then you would pass 0.001.
 	@availability First appeared in ARToolKit 2.68.
*/
void arglCameraView(const double para[3][4], GLdouble m_modelview[16], const double scale);

/*!
    @function
    @abstract   (description)
    @discussion (description)
    @param      (name) (description)
    @result     (description)
*/
void arglCameraViewRH(const double para[3][4], GLdouble m_modelview[16], const double scale);

/*!
    @function
    @abstract Display an ARVideo image, by drawing it using OpenGL.
    @discussion
		This function draws an image from an ARVideo source to the current
		OpenGL context. This operation is most useful in video see-through
		augmented reality applications for drawing the camera view as a
		background image, but can also be used in other ways.

		An undistorted image is drawn with the lower-left corner of the
		bottom-left-most pixel at OpenGL screen coordinates (0,0), and the
		upper-right corner of the top-right-most pixel at OpenGL screen
		coodinates (x * zoom, y * zoom), where x and y are the values of the
		fields cparam->xsize and cparam->ysize (see below) and zoom is the
		value of the parameter zoom (also see below). If cparam->dist_factor
		indicates that an un-warping correction should be applied, the actual
		coordinates will differ from the values specified here.

		OpenGL state: Drawing is performed with depth testing and lighting
		disabled, and thus leaves the the depth buffer (if any) unmodified. If
		pixel transfer is by texturing (see documentation for arglDrawMode),
		the drawing is done in replacement texture environment mode.
		The depth test enable and lighting enable state and the texture
		environment mode are restored before the function returns.
	@param image Pointer to the tightly-packed image data (as returned by
		arVideoGetImage()). The horizontal and vertical dimensions of the image
		data must exactly match the values specified in the fields cparam->xsize
		and cparam->ysize (see below).

		The first byte of image data corresponds to the first component of the
		top-left-most pixel in the image. The data continues with the remaining
		pixels of the first row, followed immediately by the pixels of the second
		row, and so on to the last byte of the image data, which corresponds to
		the last component of the bottom-right-most pixel.
	@param cparam Pointer to a set of ARToolKit camera parameters for the
		current video source. The size of the source image is taken from the
		fields xsize and ysize of the ARParam structure pointed to. Also, when
		the draw mode is AR_DRAW_BY_TEXTURE_MAPPING (see the documentation for
		the global variable arglDrawMode) the field dist_factor of the ARParam
		structure pointed to will be taken as the amount to un-warp the supplied
		image.
	@param zoom The amount to scale the video image up or down. To draw the video
		image double size, use a zoom value of 2.0. To draw the video image
		half size use a zoom value of 0.5.
	@param contextSettings A reference to ARGL's settings for the current OpenGL
		context, as returned by arglSetupForCurrentContext() for this context. It
		is the callers responsibility to make sure that the current context at the
		time arglDisplayImage() is called matches that under which contextSettings
		was created.
	@availability First appeared in ARToolKit 2.68.
*/
void arglDispImage(ARUint8 *image, const ARParam *cparam, const double zoom, ARGL_CONTEXT_SETTINGS_REF contextSettings);

/*!
	@function
    @abstract Display an ARVideo image, by drawing it using OpenGL, using and modifying current OpenGL state.
    @discussion
		This function is identical to arglDispImage except that whereas
		arglDispImage sets an orthographic 2D projection and the OpenGL state
		prior to drawing, this function does not. It also does not restore any
		changes made to OpenGL state.

		This allows you to do effects with your image, other than just drawing it
		2D and with the lower-left corner of the bottom-left-most pixel attached
		to the bottom-left (0,0) of the window. For example, you might use a
		perspective projection instead of an orthographic projection with a
		glLoadIdentity() / glTranslate() on the modelview matrix to place the
		lower-left corner of the bottom-left-most pixel somewhere other than 0,0
		and leave depth-testing enabled.

		See the documentation for arglDispImage() for more information.
	@availability First appeared in ARToolKit 2.68.2.
 */
void arglDispImageStateful(ARUint8 *image, const ARParam *cparam, const double zoom, ARGL_CONTEXT_SETTINGS_REF contextSettings);

/*!
    @function
    @abstract Set compensation for camera lens distortion in arglDispImage to off or on.
    @discussion
		By default, arglDispImage compensates for the distortion of the camera's
		acquired image caused by the lens when it draws. By calling this function
		with enabled = FALSE, this compensation will be disabled in the specified
		drawing context. It may be re-enabled at any time.
		This function is useful if you need to draw an image, but do not know the
		extent of the camera's lens distortion (such as during distortion calibration).
		While distortion compensation is disabled, the dist_factor[] array in a
		the camera cparam structure passed to arglDispImage is ignored.
	@param contextSettings A reference to ARGL's settings for the current OpenGL
		context, as returned by arglSetupForCurrentContext() for this context.
	@param enable TRUE to enabled distortion compensation, FALSE to disable it.
		The default state for new contexts is enable = TRUE.
	@result TRUE if the distortion value was set, FALSE if an error occurred.
	@availability First appeared in ARToolKit 2.71.
*/
int arglDistortionCompensationSet(ARGL_CONTEXT_SETTINGS_REF contextSettings, int enable);

/*!
    @function
	@abstract Enquire as to the enable state of camera lens distortion compensation in arglDispImage.
	@discussion
		By default, arglDispImage compensates for the distortion of the camera's
		acquired image caused by the lens when it draws. This function enquires
		as to whether arglDispImage is currently doing compensation or not.
	@param contextSettings A reference to ARGL's settings for the current OpenGL
		context, as returned by arglSetupForCurrentContext() for this context.
	@param enable Pointer to an integer value which will be set to TRUE if distortion
		compensation is enabled in the specified context, or FALSE if it is disabled.
	@result TRUE if the distortion value was retreived, FALSE if an error occurred.
	@availability First appeared in ARToolKit 2.71.
 */
int arglDistortionCompensationGet(ARGL_CONTEXT_SETTINGS_REF contextSettings, int *enable);

/*!
    @function
    @abstract Set the format of pixel data which will be passed to arglDispImage*()
    @discussion (description)
		In gsub_lite, the format of the pixels (i.e. the arrangement of components
		within each pixel) can be changed at runtime. Use this function to inform
		gsub_lite the format the pixels being passed to arglDispImage*() functions
		are in. This setting applies only to the context passed in parameter
		contextSettings. The default format is determined by
		the value of AR_DEFAULT_PIXEL_FORMAT at the time the library was built.
		Usually, image data is passed in directly from images generated by ARVideo,
		and so you should ensure that ARVideo is generating pixels of the same format.
	@param contextSettings A reference to ARGL's settings for the current OpenGL
		context, as returned by arglSetupForCurrentContext() for this context.
    @param format A symbolic constant for the pixel format being set. See
		AR_PIXEL_FORMAT in ar.h for a list of all possible formats.
	@result TRUE if the pixel format value was set, FALSE if an error occurred.
	@availability First appeared in ARToolKit 2.71.
*/
int arglPixelFormatSet(ARGL_CONTEXT_SETTINGS_REF contextSettings, AR_PIXEL_FORMAT format);

/*!
    @function
    @abstract Get the format of pixel data in which arglDispImage*() is expecting data to be passed.
    @discussion This function enquires as to the current format of pixel data being
		expected by the arglDispImage*() functions. The default format is determined by
		the value of AR_DEFAULT_PIXEL_FORMAT at the time the library was built.
	@param contextSettings A reference to ARGL's settings for the current OpenGL
		context, as returned by arglSetupForCurrentContext() for this context.
	@param format A symbolic constant for the pixel format in use. See
		AR_PIXEL_FORMAT in ar.h for a list of all possible formats.
	@param size The number of bytes of memory occupied per pixel, for the given format.
	@result TRUE if the pixel format and size values were retreived, FALSE if an error occurred.
	@availability First appeared in ARToolKit 2.71.
*/
int arglPixelFormatGet(ARGL_CONTEXT_SETTINGS_REF contextSettings, AR_PIXEL_FORMAT *format, int *size);

/*!
    @function
	@abstract Set method by which arglDispImage() will transfer pixels.
	@discussion
		This setting determines the method by which arglDispImage transfers pixels
		of an image to OpenGL for display. Setting this
		variable to a value of AR_DRAW_BY_GL_DRAW_PIXELS specifies the use of the
		OpenGL DrawPixels functions to do the transfer. Setting this variable to a value of
		AR_DRAW_BY_TEXTURE_MAPPING specifies the use of OpenGL TexImage2D functions to do the
		transfer. The DrawPixels method is guaranteed to be available on all
		implementations, but arglDispImage does not correct the image
		for camera lens distortion under this method. In contrast, TexImage2D is only
		available on some implementations, but allows arglDispImage() to apply a correction
		for camera lens distortion, and additionally offers greater potential for
		accelerated drawing on some implementations.

		The initial value is AR_DRAW_BY_TEXTURE_MAPPING.
	@availability First appeared in ARToolKit 2.72.
 */
void arglDrawModeSet(ARGL_CONTEXT_SETTINGS_REF contextSettings, const int mode);

/*!
    @function
	@abstract Get method by which arglDispImage() is transfering pixels.
	@discussion
		Enquires as to the current method by which arglDispImage() is
		transferring pixels to OpenGL for display. See arglDrawModeSet() for
		more information.
	@availability First appeared in ARToolKit 2.72.
 */
int arglDrawModeGet(ARGL_CONTEXT_SETTINGS_REF contextSettings);

/*!
    @function
	@abstract Determines use of full or half-resolution TexImage2D pixel-transfer in arglDispImage().
	@discussion
		When arglDrawModeSet(AR_DRAW_BY_TEXTURE_MAPPING) has been called, the value of this
		setting determines whether full or half-resolution data is transferred to the
		texture. Calling this function with a mode value of AR_DRAW_TEXTURE_FULL_IMAGE
		uses all available pixels in the source image data. A value of
		AR_DRAW_TEXTURE_HALF_IMAGE discards every second row
		in the source image data, defining a half-height texture which is then drawn stretched
		vertically to double its height.

		The latter method is well-suited to drawing interlaced images, as would be obtained
		from DV camera sources in interlaced mode or composite video sources.

		The initial value is AR_DRAW_TEXTURE_FULL_IMAGE.
	@availability First appeared in ARToolKit 2.72.
 */
void arglTexmapModeSet(ARGL_CONTEXT_SETTINGS_REF contextSettings, const int mode);

/*!
    @function
	@abstract Enquire whether full or half-resolution TexImage2D pixel-transfer is being used in arglDispImage().
	@discussion
		Enquires as to the current value of the TexmapMode setting. See arglTexmapModeSet()
		for more info.
	@availability First appeared in ARToolKit 2.72.
 */
int arglTexmapModeGet(ARGL_CONTEXT_SETTINGS_REF contextSettings);

/*!
    @function
	@abstract Determines use of rectangular TexImage2D pixel-transfer in arglDispImage().
	@discussion
		On implementations which support the OpenGL extension for rectangular textures (of
		non power-of-two size), and when arglDrawMode is set to AR_DRAW_BY_TEXTURE_MAPPING,
		the value of this variable determines whether rectangular textures or ordinary
		(power-of-two) textures are used by arglDispImage(). A value of TRUE specifies the
		use of rectangluar textures. A value of FALSE specifies the use of ordinary textures.

		If the OpenGL driver available at runtime does not support for rectangular textures,
		changing the value of this setting to TRUE will result calls to arglDispImage
		performing no drawing.
	@availability First appeared in ARToolKit 2.72.
 */
void arglTexRectangleSet(ARGL_CONTEXT_SETTINGS_REF contextSettings, const int state);

/*!
    @function
	@abstract Enquire as to use of rectangular TexImage2D pixel-transfer in arglDispImage().
	@discussion
		Enquires as to the current value of the TexRectangle setting. See arglTexRectangleSet()
		for more info.
	@availability First appeared in ARToolKit 2.72.
 */
int arglTexRectangleGet(ARGL_CONTEXT_SETTINGS_REF contextSettings);

///gsub_lite.h

///gsubUtil.h
/** \fn void argUtilCalibHMD( int targetId, int thresh2,
                      void (*postFunc)(ARParam *lpara, ARParam *rpara) )
* \brief utility function for calibrate an HMD.
*
* This function offers a full calibration run-time routines for an optical HMD (mono
* or stereo).
* It is useful for estimate transformation between user eye position and
* camera position. You will find more informations on the calibration routine
* on opticalcalibration.html .This function modify gsub state of left and right camera
* intrinsic parameters.
* \param targetId the target used for the calibration step.
* \param thresh2 lighting threshold value to use
* \param postFunc a callback function used to analysis computed internal camera
* parameters. if your application is mono display, only lpara contains a value.
* lpara and rpara are NULL if the calibration failed.
*/
void argUtilCalibHMD( int targetId, int thresh2,
                      void (*postFunc)(ARParam *lpara, ARParam *rpara) );
///gsubUtil.h

#ifdef __cplusplus
}
#endif
#endif
