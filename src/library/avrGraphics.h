/*
  Name:        avrGraphics.h
  Version      1.0.1
  Author:      Felipe Caetano, Luiz Maurílio, Rodrigo L. S. Silva
  Date:        10/10/2012
  Last Update: 09/10/2014
  Description: Collection of the graphical functions of ARToolkit. Receive functions
               from gsub and config.
*/

#ifndef AVR_GRAPHICS_H
#define AVR_GRAPHICS_H

#include <avrParameters.h> // << this include avrUtil
#include <avrUtil.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __APPLE__
#  include <OpenGL/gl.h>
#else
#  ifdef _WIN32
#    include <windows.h>
#  endif
#  include <GL/gl.h>
#endif

//config.h

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
* \param xwin number of mini windows to be created to left the video (0, 1, 2 if indifferent).
* \param ywin number of mini windows to be created below the video (0 if indifferent).
* \param hmd_flag enable stereo display mode (only interleaved configuration)
*/
void argInit(ARParam *cparam, double zoom, int fullFlag, int xwin, int ywin, int hmd_flag);
/** \fn void argCleanup( void )
* \brief Close the gsub library.
*
* This function clean the rendering context (GLUT and openGL).
* Call in the exit of your program.
* \remark BE CAREFUL, THIS FUNCTION DOESN'T RESET PERSPECTIVE
* MATRIX AND CURRENT GL STATE TO DEFAULT
*/
void argCleanup(void);
/** \fn void argSwapBuffers( void )
* \brief swap the rendering buffer.
*
* Swap the back-buffer to the front-buffer. the
* pre-condition is that all the rendering functions have been
* called.
*/
void argSwapBuffers(void);

/** \fn void argDrawMode2D( void )
* \brief switch the rendering context for 2D rendering mode.
*
* Update curent camera parameters (internal and external)
* for rendering 2D or 3D objects in the view plane (like text or 2D shape).
* This function define an orthographic projection in the image
* plane. It not define opengl state for rendering in image space (like
* for a bitmap copy).
*/
void argDrawMode2D(void);
/** \fn void argDraw2dLeft( void )
* \brief switch the rendering view to left eye  (in 2D space)
*
* Combine with argDrawMode2D for rendering the left view.
*/
void argDraw2dLeft(void);
/** \fn void argDraw2dRight( void )
* \brief switch the rendering view to right eye (in 2D space)
*
* Combine with argDrawMode2D for rendering the right view.
*/
void argDraw2dRight(void);
/** \fn void argDrawMode3D( void )
* \brief switch the rendering context for 3D rendering mode.
*
* Update curent camera parameters for rendering in 3D space.
* Generally call to reinializing model view matrix.
*/
void argDrawMode3D(void);
/** \fn void argDraw3dLeft( void )
* \brief switch the rendering view to left eye (in 3D space)
*
* Update curent internal camera parameters for rendering in 3D space
* for left eye.
* this function complements argDrawMode3D.
*/
void argDraw3dLeft(void);
/** \fn void argDraw3dRight( void )
* \brief switch the rendering view to right eye (in 3D space)
*
* Update curent internal camera parameters for rendering in 3D space
* for left eye.
* this function complements argDrawMode3D.
*/
void argDraw3dRight(void);
/** \fn void argDraw3dCamera( int xwin, int ywin )
* \brief switch the rendering view for 3D rendering mode.
*
* Update curent internal camera parameters for rendering in 3D space.
* this function complements argDrawMode3D.
* \param xwin length of rendering view (less than window length)
* \param ywin width of rendering view (less than window width)
*/
void argDraw3dCamera(int xwin, int ywin);

/** \fn void argReshapeCallback(int width, int height)
*  \brief default glut reshape callback, called when window is resized
*  \param width the new horizontal size of the window
*  \param height the new vertical size of the window
*/
void argReshapeCallback(int width, int height);
/** \fn void argConvGlpara( double para[3][4], double gl_para[16] )
* \brief transform ARToolKit matrix format to an openGL matrix format.
*
* simple conversion for the openGL matrix (16 values and homogeneous matrix).
* Returned value is generally use with a Model View Matrix.
* \param para the ARToolKit matrix
* \param gl_para the resulted openGL matrix
*/
void argConvGlpara(double para[3][4], double gl_para[16]);
/** \fn void argConvGLcpara( ARParam *param, double gnear, double gfar, double m[16] )
* \brief transform ARToolKit intrinsic camera parameters matrix format to an openGL matrix format.
*
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
void argConvGLcpara(ARParam *param, double gnear, double gfar, double m[16]);

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
* \param xwin mini window to left
* \param ywin mini window below
*/
void argDispImage(ARUint8 *image, int xwin, int ywin);
/** \fn void argDispHalfImage( ARUint8 *image, int xwin, int ywin )
* \brief display half of the video image.
*
* Idem of argDispImage except than a quarter of the image is display
* (first left top quadrant, so size/2 in x and y).
* \param image image to display
* \param xwin mini window to left
* \param ywin mini window below
*/
void argDispHalfImage(ARUint8 *image, int xwin, int ywin);

/** \fn void argDrawSquare( double vertex[4][2], int xwin, int ywin )
* \brief draw a 2D square.
*
* Draw a square. The position of the square is affected by openGL
* model view matrix and call to argDrawMode2D argDrawMode3D.
* Generally call in a 2D mode (so after a argDrawMode2D).
*
* \param vertex corner of square.
* \param xwin mini window to left
* \param ywin mini window below
*/
void argDrawSquare(double vertex[4][2], int xwin, int ywin);

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
* \param xwin mini window to left
* \param ywin mini window below
*/
void argLineSeg(double x1, double y1, double x2, double y2, int xwin, int ywin);

/*
*   Gets
*/
//! gets the horizontal size of the window
int argGetWindowWidth();
//! gets the vertical size of the window
int argGetWindowHeight();
//! gets the horizontal size of the video image
int argGetImageWidth();
//! gets the vertical size of the video image
int argGetImageHeight();

/*
*   Sets
*/
//! it defines a name of the glut window
void argSetWindowName(const char* name);

#ifdef __cplusplus
}
#endif
#endif
