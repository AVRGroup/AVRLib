/*
gsub
*/

#include <stdio.h>
#include <stdlib.h>
#if defined(_WIN32)
#include <windows.h>
#include <string.h>
#endif
#ifndef __APPLE__
#  include <GL/glut.h>
#  ifdef GL_VERSION_1_2
#    include <GL/glext.h>
#  endif
#else
#  include <GLUT/glut.h>
#  include <OpenGL/glext.h>
#endif
//#include <AR/config.h>
//#include <AR/param.h>
//#include <AR/ar.h>

///
#include <avrMath.h>       // inclui avrUtil
#include <avrGraphics.h>   // inclui avrParameters que inclui avrUtil
#include <avrVideo.h>      // inclui avrUtil
#include <avrMarker.h>
///

#define  CALIB_POS1_NUM     5
#define  CALIB_POS2_NUM     2

#ifndef GL_ABGR
#  define GL_ABGR GL_ABGR_EXT
#endif
#ifndef GL_BGRA
#  define GL_BGRA GL_BGRA_EXT
#endif
#ifndef GL_BGR
#  define GL_BGR GL_BGR_EXT
#endif
#ifndef GL_RGBA
#  define GL_RGBA GL_RGBA_EXT
#endif
#ifndef GL_RGB
#  define GL_RGB GL_RGB_EXT
#endif

#ifdef AR_OPENGL_TEXTURE_RECTANGLE
#  if defined(GL_TEXTURE_RECTANGLE_EXT)
#    define AR_TEXTURE_RECTANGLE   GL_TEXTURE_RECTANGLE_EXT
#  elif defined(GL_TEXTURE_RECTANGLE_NV)
#    define AR_TEXTURE_RECTANGLE   GL_TEXTURE_RECTANGLE_NV
#  else
#    undef AR_OPENGL_TEXTURE_RECTANGLE
#  endif
#endif

#ifdef _WIN32
#  include <windows.h>
#  define put_zero(p,s) ZeroMemory(p, s)
#else
#  include <string.h>
#  define put_zero(p,s) memset((void *)p, 0, s)
#endif

#define USE_OPTIMIZATIONS
#define WORK_SIZE   1024*32

/*****************************************************************************/
// BUG in ARToolkit 2.65
// Hardcoded buffer (600*500) is too small for full-size DV-PAL/NTSC resolutions of
// 720x576 and 720x480, respectively. Results in segment faults.
/*
static ARInt16      l_imageL[640*500];
static ARInt16      l_imageR[640*500];
*/

#define HARDCODED_BUFFER_WIDTH  1024
#define HARDCODED_BUFFER_HEIGHT 1024

#define   MINIWIN_MAX    8
#define   REVERSE_LR     1
#define   LEFTEYE        1
#define   RIGHTEYE       2
#define   GMINI          2

int  argDrawMode   = DEFAULT_DRAW_MODE;
int  argTexmapMode = DEFAULT_DRAW_TEXTURE_IMAGE;


static ARParam  gCparam;
static ARSParam gsCparam;
static double   gl_cpara[16];
//static double   gl_cparaL[16];
//static double   gl_cparaR[16];
static double   gl_lpara[16];
static double   gl_rpara[16];
static int      gl_hmd_flag      = 0;
static int      gl_hmd_para_flag = 0;
static int      gl_stereo_flag   = 0;
//static int      gl_twin_flag     = 0;

static double   gZoom;
static int      gXsize, gYsize;
static int      gMiniXnum,  gMiniYnum;
static int      gMiniXsize, gMiniYsize;
static int      gWinXsize, gWinYsize;
static int      gImXsize, gImYsize;
static int      win;
static GLuint   glid[4];

static void (*gMouseFunc)(int button, int state, int x, int y);
static void (*gKeyFunc)(unsigned char key, int x, int y);
static void (*gMainFunc)(void);


static void argInit2( int fullFlag );
static void argInitLoop(void);
static void argInitStencil(void);
static void argSetStencil( int flag );
static void argConvGLcpara2( double cparam[3][4], int width, int height, double gnear, double gfar, double m[16] );


static int    useTextureRectangle = 0;
static GLint  maxRectangleTextureSize = 0;
static int    tex1Xsize1 = 1;
static int    tex1Xsize2 = 1;
static int    tex1Ysize  = 1;
static int    tex2Xsize  = 1;
static int    tex2Ysize  = 1;
#ifdef AR_OPENGL_TEXTURE_RECTANGLE
static void   argDispImageTexRectangle( ARUint8 *image, int xwin, int ywin, int mode );
#endif
static void   argDispImageTex3( ARUint8 *image, int xwin, int ywin, int mode );
static void   argDispImageTex4( ARUint8 *image, int xwin, int ywin, int mode );
static void   argDispHalfImageTex( ARUint8 *image, int xwin, int ywin, int mode );
static void   argDispImageDrawPixels( ARUint8 *image, int xwin, int ywin );
static void   argDispHalfImageDrawPixels( ARUint8 *image, int xwin, int ywin );

void argInqSetting( int *hmdMode,
                    int *gMiniXnum2, int *gMiniYnum2,
                    void (**mouseFunc)(int button, int state, int x, int y),
                    void (**keyFunc)(unsigned char key, int x, int y),
                    void (**mainFunc)(void) )
{
    *hmdMode    = gl_hmd_flag;
    *gMiniXnum2 = gMiniXnum;
    *gMiniYnum2 = gMiniYnum;
    *mouseFunc = gMouseFunc;
    *keyFunc   = gKeyFunc;
    *mainFunc  = gMainFunc;
}

void argInit( ARParam *cparam, double zoom, int fullFlag, int xwin, int ywin, int hmd_flag )
{
    int       i;

    gl_hmd_flag = hmd_flag;
    gZoom  = zoom;
    gImXsize = cparam->xsize;
    gImYsize = cparam->ysize;
    if( gl_hmd_flag == 0 ) {
        gXsize = (double)cparam->xsize * gZoom;
        gYsize = (double)cparam->ysize * gZoom;
    }
    else {
        gXsize = AR_HMD_XSIZE;
        gYsize = AR_HMD_YSIZE;
    }
    gMiniXsize = (double)cparam->xsize * gZoom / GMINI;
    gMiniYsize = (double)cparam->ysize * gZoom / GMINI;

    if( xwin * ywin > MINIWIN_MAX ) {
        if( xwin > MINIWIN_MAX ) xwin = MINIWIN_MAX;
        ywin = MINIWIN_MAX / xwin;
    }
    gMiniXnum = xwin;
    gMiniYnum = ywin;
    gWinXsize = (gMiniXsize*gMiniXnum > gXsize)?
                     gMiniXsize*gMiniXnum: gXsize;
    gWinYsize = gYsize + gMiniYsize*gMiniYnum;

    gCparam = *cparam;
    for( i = 0; i < 4; i++ ) {
        gCparam.mat[1][i] = (gCparam.ysize-1)*(gCparam.mat[2][i]) - gCparam.mat[1][i];
    }
    argConvGLcpara( &gCparam, AR_GL_CLIP_NEAR, AR_GL_CLIP_FAR, gl_cpara );

    argInit2( fullFlag );
}

static void argInit2( int fullFlag )
{
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(gWinXsize, gWinYsize);
    win = glutCreateWindow("");
    if( fullFlag ) {
        glutFullScreen();
        gWinXsize = glutGet(GLUT_SCREEN_WIDTH);
        gWinYsize = glutGet(GLUT_SCREEN_HEIGHT);
    }

#ifdef AR_OPENGL_TEXTURE_RECTANGLE
#if defined(GL_TEXTURE_RECTANGLE_EXT)
    if( glutExtensionSupported("GL_EXT_texture_rectangle") ) {
        useTextureRectangle = 1;
        glGetIntegerv(GL_MAX_RECTANGLE_TEXTURE_SIZE_EXT, &maxRectangleTextureSize);
    }
#elif defined(GL_TEXTURE_RECTANGLE_NV)
    if( glutExtensionSupported("GL_NV_texture_rectangle") ) {
        useTextureRectangle = 1;
        glGetIntegerv(GL_MAX_RECTANGLE_TEXTURE_SIZE_NV, &maxRectangleTextureSize);
    }
#endif
#endif

    gMouseFunc = NULL;
    gKeyFunc   = NULL;
    gMainFunc  = NULL;


    glGenTextures(4, glid);
    glBindTexture( GL_TEXTURE_2D, glid[0] );
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL );
    glBindTexture( GL_TEXTURE_2D, glid[1] );
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL );
    glBindTexture( GL_TEXTURE_2D, glid[2] );
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL );

    if( gImXsize > 512 ) {
        tex1Xsize1 = 512;
        tex1Xsize2 = 1;
        while( tex1Xsize2 < gImXsize - tex1Xsize1 ) tex1Xsize2 *= 2;
    }
    else {
        tex1Xsize1 = 1;
        while( tex1Xsize1 < gImXsize ) tex1Xsize1 *= 2;
    }
    tex1Ysize  = 1;
    while( tex1Ysize < gImYsize ) tex1Ysize *= 2;

    tex2Xsize = 1;
    while( tex2Xsize < gImXsize/2 ) tex2Xsize *= 2;
    tex2Ysize = 1;
    while( tex2Ysize < gImYsize/2 ) tex2Ysize *= 2;
}

void argCleanup( void )
{
/*
    glutDestroyWindow( win );
*/
}

void argSwapBuffers( void )
{
    glutSwapBuffers();
}

void argMainLoop( void (*mouseFunc)(int button, int state, int x, int y),
                  void (*keyFunc)(unsigned char key, int x, int y),
                  void (*mainFunc)(void) )
{
    gMouseFunc = mouseFunc;
    gKeyFunc   = keyFunc;
    gMainFunc  = mainFunc;

    glutDisplayFunc( argInitLoop );
    glutMainLoop();
}

static void argInitLoop(void)
{
    arUtilSleep( 500 );

    argDrawMode2D();
    if( gl_hmd_flag || gl_stereo_flag ) {
        glClearColor( 0.0, 0.0, 0.0, 0.0 );
        glClear(GL_COLOR_BUFFER_BIT);
        argInitStencil();
        argSwapBuffers();
    }

    glClearColor( 0.0, 0.0, 0.0, 0.0 );
    glClear(GL_COLOR_BUFFER_BIT);
    argSwapBuffers();
    glClear(GL_COLOR_BUFFER_BIT);
    argSwapBuffers();

    glutKeyboardFunc( gKeyFunc );
    glutMouseFunc( gMouseFunc );
    glutDisplayFunc( gMainFunc );
    glutIdleFunc( gMainFunc );
}

void argDrawMode2D( void )
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, gWinXsize, 0, gWinYsize, -1.0, 1.0);
    glViewport(0, 0, gWinXsize, gWinYsize);

    argSetStencil( 0 );
}

void argDraw2dLeft( void )
{
    if( gl_hmd_flag == 0 && gl_stereo_flag == 0 ) return;

    argSetStencil( LEFTEYE );
}

void argDraw2dRight( void )
{
    if( gl_hmd_flag == 0 && gl_stereo_flag == 0 ) return;

    argSetStencil( RIGHTEYE );
}

void argDrawMode3D( void )
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void argDraw3dLeft( void )
{
    if( gl_hmd_flag == 0 || gl_hmd_para_flag == 0 ) return;

    glViewport(0, gWinYsize-AR_HMD_YSIZE, AR_HMD_XSIZE, AR_HMD_YSIZE);
    argSetStencil( LEFTEYE );
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixd( gl_lpara );
}

void argDraw3dRight( void )
{
    if( gl_hmd_flag == 0 || gl_hmd_para_flag == 0 ) return;

    glViewport(0, gWinYsize-AR_HMD_YSIZE, AR_HMD_XSIZE, AR_HMD_YSIZE);
    argSetStencil( RIGHTEYE );
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixd( gl_rpara );
}


void argDraw3dCamera( int xwin, int ywin )
{
    if( xwin == 0 && ywin == 0 ) {
        glViewport(0, gWinYsize-(int)(gZoom*gImYsize),
                   (int)(gZoom*gImXsize), (int)(gZoom*gImYsize));
    }
    else {
        glViewport((xwin-1)*gMiniXsize, gWinYsize-gYsize-ywin*gMiniYsize,
					gMiniXsize, gMiniYsize);
    }

    argSetStencil( 0 );

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixd( gl_cpara );
}


void argConvGlpara( double para[3][4], double gl_para[16] )
{
    int     i, j;

    for( j = 0; j < 3; j++ ) {
        for( i = 0; i < 4; i++ ) {
            gl_para[i*4+j] = para[j][i];
        }
    }
    gl_para[0*4+3] = gl_para[1*4+3] = gl_para[2*4+3] = 0.0;
    gl_para[3*4+3] = 1.0;
}


void argDispImage( ARUint8 *image, int xwin, int ywin )
{
    if( argDrawMode == AR_DRAW_BY_GL_DRAW_PIXELS ) {
        argDispImageDrawPixels( image, xwin, ywin );
    }
    else {
        if( xwin == 0 && ywin == 0 ) {
            glScissor(0, gWinYsize-(int)(gZoom*gImYsize),
                      (int)(gZoom*gImXsize), (int)(gZoom*gImYsize));
        }
        else {
            glScissor((xwin-1)*gMiniXsize, gWinYsize-gYsize-ywin*gMiniYsize,
                       gMiniXsize, gMiniYsize);
        }
        glEnable( GL_SCISSOR_TEST );
        /* glDisable( GL_DEPTH_TEST ); */
        if( useTextureRectangle
         && gImXsize < maxRectangleTextureSize
         && gImYsize < maxRectangleTextureSize ) {
#ifdef AR_OPENGL_TEXTURE_RECTANGLE
            argDispImageTexRectangle( image, xwin, ywin, 0 );
#endif
        }
        else {
            if( gImXsize > tex1Xsize1 )
                argDispImageTex3( image, xwin, ywin, 0 );
            else
                argDispImageTex4( image, xwin, ywin, 0 );
        }
        glDisable( GL_SCISSOR_TEST );
    }
}


static void argDispImageDrawPixels( ARUint8 *image, int xwin, int ywin )
{
    float    sx, sy;
    GLfloat  zoom;

    if( xwin == 0 && ywin == 0 ) {
	zoom = gZoom;
        sx = 0;
        sy = gWinYsize - 0.5;
    }
    else if( xwin == 1 && ywin == 0 ) {
	zoom = gZoom;
        sx = gXsize;
        sy = gWinYsize - 0.5;
    }
    else {
        zoom = gZoom / (double)GMINI;
        sx = (xwin-1)*gMiniXsize;
        sy = gWinYsize - gYsize - (ywin-1)*gMiniYsize - 0.5;
    }
	glDisable(GL_TEXTURE_2D);
    glPixelZoom( zoom, -zoom);
    glRasterPos3f( sx, sy, -1.0 );

#if (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_ARGB)
#  ifdef AR_BIG_ENDIAN
    glDrawPixels( gImXsize, gImYsize, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, image );
#  else
    glDrawPixels( gImXsize, gImYsize, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8, image );
#  endif
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_ABGR)
    glDrawPixels( gImXsize, gImYsize, GL_ABGR, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_BGRA)
    glDrawPixels( gImXsize, gImYsize, GL_BGRA, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_BGR)
    glDrawPixels( gImXsize, gImYsize, GL_BGR, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_RGBA)
    glDrawPixels( gImXsize, gImYsize, GL_RGBA, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_RGB)
    glDrawPixels( gImXsize, gImYsize, GL_RGB, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_MONO)
    glDrawPixels( gImXsize, gImYsize, GL_LUMINANCE, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_2vuy)
#  ifdef AR_BIG_ENDIAN
    glDrawPixels( gImXsize, gImYsize, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_REV_APPLE, image );
#  else
    glDrawPixels( gImXsize, gImYsize, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_APPLE, image );
#  endif
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_yuvs)
#  ifdef AR_BIG_ENDIAN
	glDrawPixels( gImXsize, gImYsize, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_APPLE, image );
#  else
	glDrawPixels( gImXsize, gImYsize, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_REV_APPLE, image );
#  endif
#else
#  error Unknown default pixel format defined in config.h
#endif
}

#ifdef AR_OPENGL_TEXTURE_RECTANGLE
static void argDispImageTexRectangle( ARUint8 *image, int xwin, int ywin, int mode )
{
    static int      initf = 1;
    static int      flag[3][MINIWIN_MAX+2][2];
    static int      listIndex[3][MINIWIN_MAX+2][2];
    static int      old_size_adjust_factor = -1;
    double   *dist_factor;
    double   px, py, qy, z;
    double   x1, x2;
    double   y1, y2;
    double   xx1, xx2;
    double   yy1, yy2;
    int      size_adjust_factor;
    int      list, win;
    int      i, j;

    switch( mode ) {
		case 0: dist_factor = &(gCparam.dist_factor[0]);   break;
		case 1: dist_factor = &(gsCparam.dist_factorL[0]); break;
		case 2: dist_factor = &(gsCparam.dist_factorR[0]); break;
		default: return;
    }

    if( initf ) {
        for(j=0;j<3;j++) {
            for(i=0;i<MINIWIN_MAX+2;i++) flag[j][i][0] = flag[j][i][1] = 1;
        }
        initf = 0;
    }
    if( argTexmapMode == AR_DRAW_TEXTURE_HALF_IMAGE ) {
        size_adjust_factor = 2;
        list = 1;
    }
    else {
        size_adjust_factor = 1;
        list = 0;
    }
    if( xwin == 0 && ywin == 0 )      win = 0;
    else if( xwin == 1 && ywin == 0 ) win = 1;
    else win = gMiniXnum * (ywin-1) + xwin + 1;

    glEnable( AR_TEXTURE_RECTANGLE );
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);


    glBindTexture( AR_TEXTURE_RECTANGLE, glid[3] );
#ifdef APPLE_TEXTURE_FAST_TRANSFER
    glTexParameterf(AR_TEXTURE_RECTANGLE, GL_TEXTURE_PRIORITY, 0.0);
    glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, 1);
#endif
    glPixelStorei( GL_UNPACK_ROW_LENGTH, gImXsize*size_adjust_factor );
    if( size_adjust_factor == old_size_adjust_factor ) {
#if (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_ARGB)
#  ifdef AR_BIG_ENDIAN
        glTexSubImage2D( AR_TEXTURE_RECTANGLE, 0, 0, 0, gImXsize, gImYsize/size_adjust_factor, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, image );
#  else
        glTexSubImage2D( AR_TEXTURE_RECTANGLE, 0, 0, 0, gImXsize, gImYsize/size_adjust_factor, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8, image );
#  endif
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_ABGR)
        glTexSubImage2D( AR_TEXTURE_RECTANGLE, 0, 0, 0, gImXsize, gImYsize/size_adjust_factor, GL_ABGR, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_BGRA)
        glTexSubImage2D( AR_TEXTURE_RECTANGLE, 0, 0, 0, gImXsize, gImYsize/size_adjust_factor, GL_BGRA, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_BGR)
        glTexSubImage2D( AR_TEXTURE_RECTANGLE, 0, 0, 0, gImXsize, gImYsize/size_adjust_factor, GL_BGR, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_RGBA)
        glTexSubImage2D( AR_TEXTURE_RECTANGLE, 0, 0, 0, gImXsize, gImYsize/size_adjust_factor, GL_RGBA, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_RGB)
        glTexSubImage2D( AR_TEXTURE_RECTANGLE, 0, 0, 0, gImXsize, gImYsize/size_adjust_factor, GL_RGB, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_MONO)
        glTexSubImage2D( AR_TEXTURE_RECTANGLE, 0, 0, 0, gImXsize, gImYsize/size_adjust_factor, GL_LUMINANCE, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_2vuy)
#  ifdef AR_BIG_ENDIAN
        glTexSubImage2D( AR_TEXTURE_RECTANGLE, 0, 0, 0, gImXsize, gImYsize/size_adjust_factor, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_REV_APPLE, image );
#  else
        glTexSubImage2D( AR_TEXTURE_RECTANGLE, 0, 0, 0, gImXsize, gImYsize/size_adjust_factor, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_APPLE, image );
#  endif
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_yuvs)
#  ifdef AR_BIG_ENDIAN
        glTexSubImage2D( AR_TEXTURE_RECTANGLE, 0, 0, 0, gImXsize, gImYsize/size_adjust_factor, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_APPLE, image );
#  else
        glTexSubImage2D( AR_TEXTURE_RECTANGLE, 0, 0, 0, gImXsize, gImYsize/size_adjust_factor, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_REV_8_8_APPLE, image );
#  endif
#else
#  error Unknown default pixel format defined in config.h
#endif
    }
    else {
#if (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_ARGB)
#  ifdef AR_BIG_ENDIAN
        glTexImage2D( AR_TEXTURE_RECTANGLE, 0, GL_RGBA, gImXsize, gImYsize/size_adjust_factor, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, image );
#  else
        glTexImage2D( AR_TEXTURE_RECTANGLE, 0, GL_RGBA, gImXsize, gImYsize/size_adjust_factor, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8, image );
#  endif
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_ABGR)
        glTexImage2D( AR_TEXTURE_RECTANGLE, 0, GL_RGBA, gImXsize, gImYsize/size_adjust_factor, 0, GL_ABGR, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_BGRA)
        glTexImage2D( AR_TEXTURE_RECTANGLE, 0, GL_RGB, gImXsize, gImYsize/size_adjust_factor, 0, GL_BGRA, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_BGR)
        glTexImage2D( AR_TEXTURE_RECTANGLE, 0, GL_RGB, gImXsize, gImYsize/size_adjust_factor, 0, GL_BGR, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_RGBA)
        glTexImage2D( AR_TEXTURE_RECTANGLE, 0, GL_RGBA, gImXsize, gImYsize/size_adjust_factor, 0, GL_RGBA, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_RGB)
        glTexImage2D( AR_TEXTURE_RECTANGLE, 0, GL_RGB, gImXsize, gImYsize/size_adjust_factor, 0, GL_RGB, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_MONO)
        glTexImage2D( AR_TEXTURE_RECTANGLE, 0, GL_LUMINANCE, gImXsize, gImYsize/size_adjust_factor, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_2vuy)
#  ifdef AR_BIG_ENDIAN
        glTexImage2D( AR_TEXTURE_RECTANGLE, 0, GL_RGB, gImXsize, gImYsize/size_adjust_factor, 0, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_REV_APPLE, image );
#  else
        glTexImage2D( AR_TEXTURE_RECTANGLE, 0, GL_RGB, gImXsize, gImYsize/size_adjust_factor, 0, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_APPLE, image );
#  endif
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_yuvs)
#  ifdef AR_BIG_ENDIAN
        glTexImage2D( AR_TEXTURE_RECTANGLE, 0, GL_RGB, gImXsize, gImYsize/size_adjust_factor, 0, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_APPLE, image );
#  else
        glTexImage2D( AR_TEXTURE_RECTANGLE, 0, GL_RGB, gImXsize, gImYsize/size_adjust_factor, 0, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_REV_APPLE, image );
#  endif
#else
#  error Unknown default pixel format defined in config.h
#endif
        old_size_adjust_factor = size_adjust_factor;
    }

    if( flag[mode][win][list] ) {
        listIndex[mode][win][list] = glGenLists(1);
        glNewList(listIndex[mode][win][list], GL_COMPILE_AND_EXECUTE);

        z = -1.0;
        qy = gImYsize   * 0 / 20.0;
        for( j = 1; j <= 20; j++ ) {
            py = qy;
            qy = gImYsize * j / 20.0;

            glBegin( GL_QUAD_STRIP );
            for( i = 0; i <= 20; i++ ) {
                px = gImXsize * i / 20.0;

                arParamObserv2Ideal( dist_factor, px, py, &x1, &y1 );
                arParamObserv2Ideal( dist_factor, px, qy, &x2, &y2 );

                if( xwin == 0 && ywin == 0 ) {
                    xx1 = x1 * gZoom;
                    yy1 = gWinYsize - y1 * gZoom;
                    xx2 = x2 * gZoom;
                    yy2 = gWinYsize - y2 * gZoom;
                }
                else if( xwin == 1 && ywin == 0 ) {
                    xx1 = gXsize + x1 * gZoom;
                    yy1 = gWinYsize - y1 * gZoom;
                    xx2 = gXsize + x2 * gZoom;
                    yy2 = gWinYsize - y2 * gZoom;
                }
                else {
                    xx1 = (xwin-1)*gMiniXsize + x1*gZoom/(double)GMINI;
                    xx2 = (xwin-1)*gMiniXsize + x2*gZoom/(double)GMINI;
                    yy1 = gWinYsize-gYsize-(ywin-1)*gMiniYsize - y1*gZoom/(double)GMINI;
                    yy2 = gWinYsize-gYsize-(ywin-1)*gMiniYsize - y2*gZoom/(double)GMINI;
                }

                glTexCoord2d( px, py/size_adjust_factor ); glVertex3d( xx1, yy1, z );
                glTexCoord2d( px, qy/size_adjust_factor ); glVertex3d( xx2, yy2, z );
            }
            glEnd();
        }
        glEndList();
        flag[mode][win][list] = 0;
    }
    else {
        glCallList( listIndex[mode][win][list] );
    }

    glBindTexture( AR_TEXTURE_RECTANGLE, 0 );
    glDisable( AR_TEXTURE_RECTANGLE );
    glPixelStorei( GL_UNPACK_ROW_LENGTH, 0 );
}
#endif

#ifndef _WIN32
static void argDispImageTex4( ARUint8 *image, int xwin, int ywin, int mode )
#else
static void argDispImageTex4( ARUint8 *wimage, int xwin, int ywin, int mode )
#endif
{
    static int      initf = 1;
    static int      flag[3][MINIWIN_MAX+2][2];
    static int      listIndex[3][MINIWIN_MAX+2][2];
    static int      old_size_adjust_factor = -1;
#ifdef _WIN32
    static ARUint8  *image = NULL;
#endif
    double   *dist_factor;
    double   tsx, tsy, tex, tey;
    double   px, py, qx, qy, z;
    double   x1, x2, x3, x4;
    double   y1, y2, y3, y4;
    double   xx1, xx2, xx3, xx4;
    double   yy1, yy2, yy3, yy4;
    int      size_adjust_factor;
    int      list, win;
    int      i, j;

    switch( mode ) {
		case 0: dist_factor = &(gCparam.dist_factor[0]);   break;
		case 1: dist_factor = &(gsCparam.dist_factorL[0]); break;
		case 2: dist_factor = &(gsCparam.dist_factorR[0]); break;
		default: return;
    }

#ifdef _WIN32
    if( image == NULL ) {
        arMalloc(image,ARUint8,gImXsize*tex1Ysize*AR_PIX_SIZE_DEFAULT);
    }
    memcpy(image, wimage, gImXsize*gImYsize*AR_PIX_SIZE_DEFAULT);
#endif

    if( initf ) {
        for(j=0;j<3;j++) {
            for(i=0;i<MINIWIN_MAX+2;i++) flag[j][i][0] = flag[j][i][1] = 1;
        }
        initf = 0;
    }
    if( argTexmapMode == AR_DRAW_TEXTURE_HALF_IMAGE ) {
        size_adjust_factor = 2;
        list = 1;
    }
    else {
        size_adjust_factor = 1;
        list = 0;
    }
    if( xwin == 0 && ywin == 0 )      win = 0;
    else if( xwin == 1 && ywin == 0 ) win = 1;
    else win = gMiniXnum * (ywin-1) + xwin + 1;

    glEnable( GL_TEXTURE_2D );
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);

    glBindTexture( GL_TEXTURE_2D, glid[0] );
#ifdef APPLE_TEXTURE_FAST_TRANSFER
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_PRIORITY, 0.0);
    glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, 1);
#endif
    glPixelStorei( GL_UNPACK_ROW_LENGTH, gImXsize*size_adjust_factor );

    if( size_adjust_factor == old_size_adjust_factor ) {
#if (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_ARGB)
#  ifdef AR_BIG_ENDIAN
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, tex1Xsize1, tex1Ysize/size_adjust_factor, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, image );
#  else
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, tex1Xsize1, tex1Ysize/size_adjust_factor, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8, image );
#  endif
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_ABGR)
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, tex1Xsize1, tex1Ysize/size_adjust_factor, GL_ABGR, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_BGRA)
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, tex1Xsize1, tex1Ysize/size_adjust_factor, GL_BGRA, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_BGR)
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, tex1Xsize1, tex1Ysize/size_adjust_factor, GL_BGR, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_RGBA)
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, tex1Xsize1, tex1Ysize/size_adjust_factor, GL_RGBA, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_RGB)
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, tex1Xsize1, tex1Ysize/size_adjust_factor, GL_RGB, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_MONO)
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, tex1Xsize1, tex1Ysize/size_adjust_factor, GL_LUMINANCE, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_2vuy)
#  ifdef AR_BIG_ENDIAN
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, tex1Xsize1, tex1Ysize/size_adjust_factor, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_REV_APPLE, image );
#  else
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, tex1Xsize1, tex1Ysize/size_adjust_factor, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_APPLE, image );
#  endif
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_yuvs)
#  ifdef AR_BIG_ENDIAN
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, tex1Xsize1, tex1Ysize/size_adjust_factor, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_APPLE, image );
#  else
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, tex1Xsize1, tex1Ysize/size_adjust_factor, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_REV_APPLE, image );
#  endif
#else
#  error Unknown default pixel format defined in config.h
#endif
    }
    else {
#if (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_ARGB)
#  ifdef AR_BIG_ENDIAN
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, tex1Xsize1, tex1Ysize/size_adjust_factor, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, image );
#  else
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, tex1Xsize1, tex1Ysize/size_adjust_factor, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8, image );
#  endif
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_ABGR)
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, tex1Xsize1, tex1Ysize/size_adjust_factor, 0, GL_ABGR, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_BGRA)
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, tex1Xsize1, tex1Ysize/size_adjust_factor, 0, GL_BGRA, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_BGR)
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, tex1Xsize1, tex1Ysize/size_adjust_factor, 0, GL_BGR, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_RGBA)
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, tex1Xsize1, tex1Ysize/size_adjust_factor, 0, GL_RGBA, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_RGB)
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, tex1Xsize1, tex1Ysize/size_adjust_factor, 0, GL_RGB, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_MONO)
        glTexImage2D( GL_TEXTURE_2D, 0, GL_LUMINANCE, tex1Xsize1, tex1Ysize/size_adjust_factor, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_2vuy)
#  ifdef AR_BIG_ENDIAN
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, tex1Xsize1, tex1Ysize/size_adjust_factor, 0, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_REV_APPLE, image );
#  else
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, tex1Xsize1, tex1Ysize/size_adjust_factor, 0, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_APPLE, image );
#  endif
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_yuvs)
#  ifdef AR_BIG_ENDIAN
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, tex1Xsize1, tex1Ysize/size_adjust_factor, 0, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_APPLE, image );
#  else
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, tex1Xsize1, tex1Ysize/size_adjust_factor, 0, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_REV_APPLE, image );
#  endif
#else
#  error Unknown default pixel format defined in config.h
#endif
        old_size_adjust_factor = size_adjust_factor;
    }

    if( flag[mode][win][list] ) {
        listIndex[mode][win][list] = glGenLists(1);
        glNewList(listIndex[mode][win][list], GL_COMPILE_AND_EXECUTE);

        z = -1.0;
        qy = gImYsize   * 0 / 20.0;
        tey = ((double)gImYsize / (double)tex1Ysize) * (double)0 / 20.0;
        for( j = 1; j <= 20; j++ ) {
            py = qy;
            tsy = tey;
            qy = gImYsize   * j / 20.0;
            tey = ((double)gImYsize / (double)tex1Ysize) * (double)j / 20.0;

            qx = gImXsize * 0 / 20.0;
            tex = ((double)gImXsize / (double)(tex1Xsize1)) * (double)0 / 20.0;
            for( i = 1; i <= 20; i++ ) {
                px = qx;
                tsx = tex;
                qx = gImXsize * i / 20.0;
                tex = ((double)gImXsize / (double)(tex1Xsize1)) * (double)i / 20.0;

                arParamObserv2Ideal( dist_factor, px, py, &x1, &y1 );
                arParamObserv2Ideal( dist_factor, qx, py, &x2, &y2 );
                arParamObserv2Ideal( dist_factor, qx, qy, &x3, &y3 );
                arParamObserv2Ideal( dist_factor, px, qy, &x4, &y4 );

                if( xwin == 0 && ywin == 0 ) {
                    xx1 = x1 * gZoom ;
                    yy1 = gWinYsize - y1 * gZoom;
                    xx2 = x2 * gZoom ;
                    yy2 = gWinYsize - y2 * gZoom;
                    xx3 = x3 * gZoom ;
                    yy3 = gWinYsize - y3 * gZoom;
                    xx4 = x4 * gZoom ;
                    yy4 = gWinYsize - y4 * gZoom;
                }
                else if( xwin == 1 && ywin == 0 ) {
                    xx1 = gXsize + x1 * gZoom;
                    yy1 = gWinYsize - y1 * gZoom;
                    xx2 = gXsize + x2 * gZoom;
                    yy2 = gWinYsize - y2 * gZoom;
                    xx3 = gXsize + x3 * gZoom;
                    yy3 = gWinYsize - y3 * gZoom;
                    xx4 = gXsize + x4 * gZoom;
                    yy4 = gWinYsize - y4 * gZoom;
                }
                else {
                    xx1 = (xwin-1)*gMiniXsize + x1*gZoom/(double)GMINI;
                    xx2 = (xwin-1)*gMiniXsize + x2*gZoom/(double)GMINI;
                    xx3 = (xwin-1)*gMiniXsize + x3*gZoom/(double)GMINI;
                    xx4 = (xwin-1)*gMiniXsize + x4*gZoom/(double)GMINI;
                    yy1 = gWinYsize-gYsize-(ywin-1)*gMiniYsize - y1*gZoom/(double)GMINI;
                    yy2 = gWinYsize-gYsize-(ywin-1)*gMiniYsize - y2*gZoom/(double)GMINI;
                    yy3 = gWinYsize-gYsize-(ywin-1)*gMiniYsize - y3*gZoom/(double)GMINI;
                    yy4 = gWinYsize-gYsize-(ywin-1)*gMiniYsize - y4*gZoom/(double)GMINI;
                }

                glBegin( GL_QUADS );
                glTexCoord2d( tsx, tsy ); glVertex3d( xx1, yy1, z );
                glTexCoord2d( tex, tsy ); glVertex3d( xx2, yy2, z );
                glTexCoord2d( tex, tey ); glVertex3d( xx3, yy3, z );
                glTexCoord2d( tsx, tey ); glVertex3d( xx4, yy4, z );
                glEnd();
            }
        }
        glEndList();
        flag[mode][win][list] = 0;
    }
    else {
        glCallList( listIndex[mode][win][list] );
    }

    glBindTexture( GL_TEXTURE_2D, 0 );
    glDisable( GL_TEXTURE_2D );
    glPixelStorei( GL_UNPACK_ROW_LENGTH, 0 );
}

#ifndef _WIN32
static void argDispImageTex3( ARUint8 *image, int xwin, int ywin, int mode )
#else
static void argDispImageTex3( ARUint8 *wimage, int xwin, int ywin, int mode )
#endif
{
    static int      initf = 1;
    static int      flag[3][MINIWIN_MAX+2][2];
    static int      listIndex[3][MINIWIN_MAX+2][2];
    static int      old_size_adjust_factor = -1;
#ifdef _WIN32
    static ARUint8  *image = NULL;
#endif
    double   *dist_factor;
    double   tsx, tsy, tex, tey;
    double   px, py, qx, qy, z;
    double   x1, x2, x3, x4;
    double   y1, y2, y3, y4;
    double   xx1, xx2, xx3, xx4;
    double   yy1, yy2, yy3, yy4;
    int      size_adjust_factor;
    int      win, list;
    int      i, j;

    switch( mode ) {
		case 0: dist_factor = &(gCparam.dist_factor[0]);   break;
		case 1: dist_factor = &(gsCparam.dist_factorL[0]); break;
		case 2: dist_factor = &(gsCparam.dist_factorR[0]); break;
		default: return;
    }

#ifdef _WIN32
    if( image == NULL ) {
        arMalloc(image,ARUint8,gImXsize*tex1Ysize*AR_PIX_SIZE_DEFAULT);
    }
    memcpy(image, wimage, gImXsize*gImYsize*AR_PIX_SIZE_DEFAULT);
#endif

    if( initf ) {
        for(j=0;j<3;j++) {
            for(i=0;i<=MINIWIN_MAX;i++) flag[j][i][0] = flag[j][i][1] = 1;
        }
        initf = 0;
    }
    if( argTexmapMode == AR_DRAW_TEXTURE_HALF_IMAGE ) {
        size_adjust_factor = 2;
        list = 1;
    }
    else {
        size_adjust_factor = 1;
        list = 0;
    }
    if( xwin == 0 && ywin == 0 )      win = 0;
    else if( xwin == 1 && ywin == 0 ) win = 1;
    else win = gMiniXnum * (ywin-1) + xwin + 1;

    glEnable( GL_TEXTURE_2D );
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);

    glBindTexture( GL_TEXTURE_2D, glid[0] );
#ifdef APPLE_TEXTURE_FAST_TRANSFER
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_PRIORITY, 0.0);
    glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, 1);
#endif
    glPixelStorei( GL_UNPACK_ROW_LENGTH, gImXsize*size_adjust_factor );

    if( size_adjust_factor == old_size_adjust_factor ) {
#if (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_ARGB)
#  ifdef AR_BIG_ENDIAN
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, tex1Xsize1, tex1Ysize/size_adjust_factor, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, image );
#  else
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, tex1Xsize1, tex1Ysize/size_adjust_factor, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8, image );
#  endif
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_ABGR)
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, tex1Xsize1, tex1Ysize/size_adjust_factor, GL_ABGR, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_BGRA)
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, tex1Xsize1, tex1Ysize/size_adjust_factor, GL_BGRA, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_BGR)
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, tex1Xsize1, tex1Ysize/size_adjust_factor, GL_BGR, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_RGBA)
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, tex1Xsize1, tex1Ysize/size_adjust_factor, GL_RGBA, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_RGB)
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, tex1Xsize1, tex1Ysize/size_adjust_factor, GL_RGB, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_MONO)
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, tex1Xsize1, tex1Ysize/size_adjust_factor, GL_LUMINANCE, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_2vuy)
#  ifdef AR_BIG_ENDIAN
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, tex1Xsize1, tex1Ysize/size_adjust_factor, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_REV_APPLE, image );
#  else
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, tex1Xsize1, tex1Ysize/size_adjust_factor, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_APPLE, image );
#  endif
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_yuvs)
#  ifdef AR_BIG_ENDIAN
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, tex1Xsize1, tex1Ysize/size_adjust_factor, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_APPLE, image );
#  else
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, tex1Xsize1, tex1Ysize/size_adjust_factor, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_REV_APPLE, image );
#  endif
#else
#  error Unknown default pixel format defined in config.h
#endif
    }
    else {
#if (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_ARGB)
#  ifdef AR_BIG_ENDIAN
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, tex1Xsize1, tex1Ysize/size_adjust_factor, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, image );
#  else
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, tex1Xsize1, tex1Ysize/size_adjust_factor, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8, image );
#  endif
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_ABGR)
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, tex1Xsize1, tex1Ysize/size_adjust_factor, 0, GL_ABGR, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_BGRA)
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, tex1Xsize1, tex1Ysize/size_adjust_factor, 0, GL_BGRA, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_BGR)
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, tex1Xsize1, tex1Ysize/size_adjust_factor, 0, GL_BGR, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_RGBA)
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, tex1Xsize1, tex1Ysize/size_adjust_factor, 0, GL_RGBA, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_RGB)
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, tex1Xsize1, tex1Ysize/size_adjust_factor, 0, GL_RGB, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_MONO)
        glTexImage2D( GL_TEXTURE_2D, 0, GL_LUMINANCE, tex1Xsize1, tex1Ysize/size_adjust_factor, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_2vuy)
#  ifdef AR_BIG_ENDIAN
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, tex1Xsize1, tex1Ysize/size_adjust_factor, 0, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_REV_APPLE, image );
#  else
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, tex1Xsize1, tex1Ysize/size_adjust_factor, 0, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_APPLE, image );
#  endif
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_yuvs)
#  ifdef AR_BIG_ENDIAN
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, tex1Xsize1, tex1Ysize/size_adjust_factor, 0, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_APPLE, image );
#  else
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, tex1Xsize1, tex1Ysize/size_adjust_factor, 0, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_REV_APPLE, image );
#  endif
#else
#  error Unknown default pixel format defined in config.h
#endif
    }

    if( flag[mode][win][list] ) {
        listIndex[mode][win][list] = glGenLists(2);
        glNewList(listIndex[mode][win][list], GL_COMPILE_AND_EXECUTE);

        z = -1.0;
        qy = gImYsize   * 0 / 20.0;
        tey = ((double)gImYsize / (double)tex1Ysize) * (double)0 / 20.0;
        for( j = 1; j <= 20; j++ ) {
            py = qy;
            tsy = tey;
            qy = gImYsize   * j / 20.0;
            tey = ((double)gImYsize / (double)tex1Ysize) * (double)j / 20.0;

            qx = tex1Xsize1 * 0 / 16.0;
            tex = (double)0 / 16.0;
            for( i = 1; i <= 16; i++ ) {
                px = qx;
                tsx = tex;
                qx = tex1Xsize1 * i / 16.0;
                tex = (double)i / 16.0;

                arParamObserv2Ideal( dist_factor, px, py, &x1, &y1 );
                arParamObserv2Ideal( dist_factor, qx, py, &x2, &y2 );
                arParamObserv2Ideal( dist_factor, qx, qy, &x3, &y3 );
                arParamObserv2Ideal( dist_factor, px, qy, &x4, &y4 );

                if( x2 < x1 ) continue;
                if( x4 > x3 ) continue;
                if( y4 < y1 ) continue;
                if( y3 < y2 ) continue;
                if( x2 < 0 || x3 < 0 ) continue;
                if( x1 > gImXsize || x4 > gImXsize ) continue;
                if( y4 < 0 || y3 < 0 ) continue;
                if( y1 > gImYsize || y2 > gImXsize ) continue;

                if( xwin == 0 && ywin == 0 ) {
                    xx1 = x1 * gZoom;
                    yy1 = gWinYsize - y1 * gZoom;
                    xx2 = x2 * gZoom;
                    yy2 = gWinYsize - y2 * gZoom;
                    xx3 = x3 * gZoom;
                    yy3 = gWinYsize - y3 * gZoom;
                    xx4 = x4 * gZoom;
                    yy4 = gWinYsize - y4 * gZoom;
                }
                else if( xwin == 1 && ywin == 0 ) {
                    xx1 = gXsize + x1 * gZoom;
                    yy1 = gWinYsize - y1 * gZoom;
                    xx2 = gXsize + x2 * gZoom;
                    yy2 = gWinYsize - y2 * gZoom;
                    xx3 = gXsize + x3 * gZoom;
                    yy3 = gWinYsize - y3 * gZoom;
                    xx4 = gXsize + x4 * gZoom;
                    yy4 = gWinYsize - y4 * gZoom;
                }
                else {
                    xx1 = (xwin-1)*gMiniXsize + x1*gZoom/(double)GMINI;
                    xx2 = (xwin-1)*gMiniXsize + x2*gZoom/(double)GMINI;
                    xx3 = (xwin-1)*gMiniXsize + x3*gZoom/(double)GMINI;
                    xx4 = (xwin-1)*gMiniXsize + x4*gZoom/(double)GMINI;
                    yy1 = gWinYsize-gYsize-(ywin-1)*gMiniYsize - y1*gZoom/(double)GMINI;
                    yy2 = gWinYsize-gYsize-(ywin-1)*gMiniYsize - y2*gZoom/(double)GMINI;
                    yy3 = gWinYsize-gYsize-(ywin-1)*gMiniYsize - y3*gZoom/(double)GMINI;
                    yy4 = gWinYsize-gYsize-(ywin-1)*gMiniYsize - y4*gZoom/(double)GMINI;
                }

                glBegin( GL_QUADS );
                glTexCoord2d( tsx, tsy ); glVertex3d( xx1, yy1, z );
                glTexCoord2d( tex, tsy ); glVertex3d( xx2, yy2, z );
                glTexCoord2d( tex, tey ); glVertex3d( xx3, yy3, z );
                glTexCoord2d( tsx, tey ); glVertex3d( xx4, yy4, z );
                glEnd();
            }
        }
        glEndList();
    }
    else {
        glCallList( listIndex[mode][win][list] );
    }

    glBindTexture( GL_TEXTURE_2D, glid[1] );
#ifdef APPLE_TEXTURE_FAST_TRANSFER
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_PRIORITY, 0.0);
    glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, 1);
#endif

    if( size_adjust_factor == old_size_adjust_factor ) {
#if (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_ARGB)
#  ifdef AR_BIG_ENDIAN
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, tex1Xsize2, tex1Ysize/size_adjust_factor, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, image+tex1Xsize1*AR_PIX_SIZE_DEFAULT );
#  else
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, tex1Xsize2, tex1Ysize/size_adjust_factor, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8, image+tex1Xsize1*AR_PIX_SIZE_DEFAULT );
#  endif
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_ABGR)
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, tex1Xsize2, tex1Ysize/size_adjust_factor, GL_ABGR, GL_UNSIGNED_BYTE, image+tex1Xsize1*AR_PIX_SIZE_DEFAULT );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_BGRA)
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, tex1Xsize2, tex1Ysize/size_adjust_factor, GL_BGRA, GL_UNSIGNED_BYTE, image+tex1Xsize1*AR_PIX_SIZE_DEFAULT );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_BGR)
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, tex1Xsize2, tex1Ysize/size_adjust_factor, GL_BGR, GL_UNSIGNED_BYTE, image+tex1Xsize1*AR_PIX_SIZE_DEFAULT );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_RGBA)
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, tex1Xsize2, tex1Ysize/size_adjust_factor, GL_RGBA, GL_UNSIGNED_BYTE, image+tex1Xsize1*AR_PIX_SIZE_DEFAULT );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_RGB)
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, tex1Xsize2, tex1Ysize/size_adjust_factor, GL_RGB, GL_UNSIGNED_BYTE, image+tex1Xsize1*AR_PIX_SIZE_DEFAULT );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_MONO)
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, tex1Xsize2, tex1Ysize/size_adjust_factor, GL_LUMINANCE, GL_UNSIGNED_BYTE, image+tex1Xsize1*AR_PIX_SIZE_DEFAULT );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_2vuy)
#  ifdef AR_BIG_ENDIAN
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, tex1Xsize2, tex1Ysize/size_adjust_factor, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_REV_APPLE, image+tex1Xsize1*AR_PIX_SIZE_DEFAULT );
#  else
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, tex1Xsize2, tex1Ysize/size_adjust_factor, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_APPLE, image+tex1Xsize1*AR_PIX_SIZE_DEFAULT );
#  endif
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_yuvs)
#  ifdef AR_BIG_ENDIAN
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, tex1Xsize2, tex1Ysize/size_adjust_factor, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_APPLE, image+tex1Xsize1*AR_PIX_SIZE_DEFAULT );
#  else
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, tex1Xsize2, tex1Ysize/size_adjust_factor, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_REV_APPLE, image+tex1Xsize1*AR_PIX_SIZE_DEFAULT );
#  endif
#else
#  error Unknown default pixel format defined in config.h
#endif
    }
    else {
#if (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_ARGB)
#  ifdef AR_BIG_ENDIAN
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, tex1Xsize2, tex1Ysize/size_adjust_factor, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, image+tex1Xsize1*AR_PIX_SIZE_DEFAULT );
#  else
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, tex1Xsize2, tex1Ysize/size_adjust_factor, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8, image+tex1Xsize1*AR_PIX_SIZE_DEFAULT );
#  endif
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_ABGR)
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, tex1Xsize2, tex1Ysize/size_adjust_factor, 0, GL_ABGR, GL_UNSIGNED_BYTE, image+tex1Xsize1*AR_PIX_SIZE_DEFAULT );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_BGRA)
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, tex1Xsize2, tex1Ysize/size_adjust_factor, 0, GL_BGRA, GL_UNSIGNED_BYTE, image+tex1Xsize1*AR_PIX_SIZE_DEFAULT );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_BGR)
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, tex1Xsize2, tex1Ysize/size_adjust_factor, 0, GL_BGR, GL_UNSIGNED_BYTE, image+tex1Xsize1*AR_PIX_SIZE_DEFAULT );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_RGBA)
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, tex1Xsize2, tex1Ysize/size_adjust_factor, 0, GL_RGBA, GL_UNSIGNED_BYTE, image+tex1Xsize1*AR_PIX_SIZE_DEFAULT );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_RGB)
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, tex1Xsize2, tex1Ysize/size_adjust_factor, 0, GL_RGB, GL_UNSIGNED_BYTE, image+tex1Xsize1*AR_PIX_SIZE_DEFAULT );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_MONO)
        glTexImage2D( GL_TEXTURE_2D, 0, GL_LUMINANCE, tex1Xsize2, tex1Ysize/size_adjust_factor, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, image+tex1Xsize1*AR_PIX_SIZE_DEFAULT );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_2vuy)
#  ifdef AR_BIG_ENDIAN
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, tex1Xsize2, tex1Ysize/size_adjust_factor, 0, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_REV_APPLE, image+tex1Xsize1*AR_PIX_SIZE_DEFAULT );
#  else
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, tex1Xsize2, tex1Ysize/size_adjust_factor, 0, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_APPLE, image+tex1Xsize1*AR_PIX_SIZE_DEFAULT );
#  endif
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_yuvs)
#  ifdef AR_BIG_ENDIAN
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, tex1Xsize2, tex1Ysize/size_adjust_factor, 0, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_APPLE, image+tex1Xsize1*AR_PIX_SIZE_DEFAULT );
#  else
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, tex1Xsize2, tex1Ysize/size_adjust_factor, 0, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_REV_APPLE, image+tex1Xsize1*AR_PIX_SIZE_DEFAULT );
#  endif
#else
#  error Unknown default pixel format defined in config.h
#endif
        old_size_adjust_factor = size_adjust_factor;
    }

    if( flag[mode][win][list] ) {
        glNewList(listIndex[mode][win][list]+1, GL_COMPILE_AND_EXECUTE);

        z = -1.0;
        qy = gImYsize   * 0 / 20.0;
        tey = ((double)gImYsize / (double)tex1Ysize) * (double)0 / 20.0;
        for( j = 1; j <= 20; j++ ) {
            py = qy;
            tsy = tey;
            qy = gImYsize   * j / 20.0;
            tey = ((double)gImYsize / (double)tex1Ysize) * (double)j / 20.0;

            qx = tex1Xsize1 + (gImXsize-tex1Xsize1) * 0 / 4.0;
            tex = ((double)(gImXsize-tex1Xsize1) / (double)tex1Xsize2) * 0 / 4.0;
            for( i = 1; i <= 4; i++ ) {
                px = qx;
                tsx = tex;
                qx = tex1Xsize1 + (gImXsize-tex1Xsize1) * i / 4.0;
                tex = ((double)(gImXsize-tex1Xsize1) / (double)tex1Xsize2) * i / 4.0;

                arParamObserv2Ideal( dist_factor, px, py, &x1, &y1 );
                arParamObserv2Ideal( dist_factor, qx, py, &x2, &y2 );
                arParamObserv2Ideal( dist_factor, qx, qy, &x3, &y3 );
                arParamObserv2Ideal( dist_factor, px, qy, &x4, &y4 );

                if( x2 < x1 ) continue;
                if( x4 > x3 ) continue;
                if( y4 < y1 ) continue;
                if( y3 < y2 ) continue;
                if( x2 < 0 || x3 < 0 ) continue;
                if( x1 > gImXsize || x4 > gImXsize ) continue;
                if( y4 < 0 || y3 < 0 ) continue;
                if( y1 > gImYsize || y2 > gImXsize ) continue;

                if( xwin == 0 && ywin == 0 ) {
                    xx1 = x1 * gZoom;
                    yy1 = gWinYsize - y1 * gZoom;
                    xx2 = x2 * gZoom;
                    yy2 = gWinYsize - y2 * gZoom;
                    xx3 = x3 * gZoom;
                    yy3 = gWinYsize - y3 * gZoom;
                    xx4 = x4 * gZoom;
                    yy4 = gWinYsize - y4 * gZoom;
                }
                else if( xwin == 1 && ywin == 0 ) {
                    xx1 = gXsize + x1 * gZoom;
                    yy1 = gWinYsize - y1 * gZoom;
                    xx2 = gXsize + x2 * gZoom;
                    yy2 = gWinYsize - y2 * gZoom;
                    xx3 = gXsize + x3 * gZoom;
                    yy3 = gWinYsize - y3 * gZoom;
                    xx4 = gXsize + x4 * gZoom;
                    yy4 = gWinYsize - y4 * gZoom;
                }
                else {
                    xx1 = (xwin-1)*gMiniXsize + x1*gZoom/(double)GMINI;
                    xx2 = (xwin-1)*gMiniXsize + x2*gZoom/(double)GMINI;
                    xx3 = (xwin-1)*gMiniXsize + x3*gZoom/(double)GMINI;
                    xx4 = (xwin-1)*gMiniXsize + x4*gZoom/(double)GMINI;
                    yy1 = gWinYsize-gYsize-(ywin-1)*gMiniYsize - y1*gZoom/(double)GMINI;
                    yy2 = gWinYsize-gYsize-(ywin-1)*gMiniYsize - y2*gZoom/(double)GMINI;
                    yy3 = gWinYsize-gYsize-(ywin-1)*gMiniYsize - y3*gZoom/(double)GMINI;
                    yy4 = gWinYsize-gYsize-(ywin-1)*gMiniYsize - y4*gZoom/(double)GMINI;
                }

                glBegin( GL_QUADS );
                glTexCoord2d( tsx, tsy ); glVertex3d( xx1, yy1, z );
                glTexCoord2d( tex, tsy ); glVertex3d( xx2, yy2, z );
                glTexCoord2d( tex, tey ); glVertex3d( xx3, yy3, z );
                glTexCoord2d( tsx, tey ); glVertex3d( xx4, yy4, z );
                glEnd();
            }
        }
        glEndList();
        flag[mode][win][list] = 0;
    }
    else {
        glCallList( listIndex[mode][win][list]+1 );
    }

    glBindTexture( GL_TEXTURE_2D, 0 );
    glDisable( GL_TEXTURE_2D );
    glPixelStorei( GL_UNPACK_ROW_LENGTH, 0 );
}

void argDispHalfImage( ARUint8 *image, int xwin, int ywin )
{
    if( argDrawMode == AR_DRAW_BY_GL_DRAW_PIXELS ) {
        argDispHalfImageDrawPixels( image, xwin, ywin );
    }
    else {
        if( xwin == 0 && ywin == 0 ) {
            glScissor(0, gWinYsize-(int)(gZoom*gImYsize),
                      (int)(gZoom*gImXsize), (int)(gZoom*gImYsize));
        }
        else {
            glScissor((xwin-1)*gMiniXsize, gWinYsize-gYsize-ywin*gMiniYsize,
                       gMiniXsize, gMiniYsize);
        }
        glEnable( GL_SCISSOR_TEST );
        /* glDisable( GL_DEPTH_TEST ); */
        argDispHalfImageTex( image, xwin, ywin, 0 );
        glDisable( GL_SCISSOR_TEST );
    }
}

static void argDispHalfImageDrawPixels( ARUint8 *image, int xwin, int ywin )
{
    float    sx, sy;
    GLfloat  zoom;

    if( xwin == 0 && ywin == 0 ) {
	zoom = gZoom * 2.0;
        sx = 0;
        sy = gWinYsize - 0.5;
    }
    if( xwin == 1 && ywin == 0 ) {
	zoom = gZoom * 2.0;
        sx = gXsize;
        sy = gWinYsize - 0.5;
    }
    else {
        zoom = gZoom / (double)GMINI * 2.0;
        sx = (xwin-1)*gMiniXsize;
        sy = gWinYsize - gYsize - (ywin-1)*gMiniYsize - 0.5;
    }
    glPixelZoom( zoom, -zoom);
    glRasterPos3f( sx, sy, -1.0 );

#if (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_ARGB)
#  ifdef AR_BIG_ENDIAN
    glDrawPixels( gImXsize/2, gImYsize/2, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, image );
#  else
    glDrawPixels( gImXsize/2, gImYsize/2, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8, image );
#  endif
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_ABGR)
    glDrawPixels( gImXsize/2, gImYsize/2, GL_ABGR, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_BGRA)
    glDrawPixels( gImXsize/2, gImYsize/2, GL_BGRA, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_BGR)
    glDrawPixels( gImXsize/2, gImYsize/2, GL_BGR, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_RGBA)
    glDrawPixels( gImXsize/2, gImYsize/2, GL_RGBA, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_RGB)
    glDrawPixels( gImXsize/2, gImYsize/2, GL_RGB, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_MONO)
    glDrawPixels( gImXsize/2, gImYsize/2, GL_LUMINANCE, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_2vuy)
#  ifdef AR_BIG_ENDIAN
	glDrawPixels( gImXsize/2, gImYsize/2, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_REV_APPLE, image );
#  else
	glDrawPixels( gImXsize/2, gImYsize/2, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_APPLE, image );
#  endif
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_yuvs)
#  ifdef AR_BIG_ENDIAN
	glDrawPixels( gImXsize/2, gImYsize/2, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_APPLE, image );
#  else
	glDrawPixels( gImXsize/2, gImYsize/2, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_REV_APPLE, image );
#  endif
#else
#  error Unknown default pixel format defined in config.h
#endif
}

#ifndef _WIN32
static void argDispHalfImageTex( ARUint8 *image, int xwin, int ywin, int mode )
#else
static void argDispHalfImageTex( ARUint8 *wimage, int xwin, int ywin, int mode )
#endif
{
    static int      initf = 1;
    static int      flag[3][MINIWIN_MAX+2];
    static int      listIndex[3][MINIWIN_MAX+2];
#ifdef _WIN32
    static ARUint8  *image = NULL;
#endif
    double   *dist_factor;
    double   tsx, tsy, tex, tey;
    double   px, py, qx, qy, z;
    double   x1, x2, x3, x4;
    double   y1, y2, y3, y4;
    double   xx1, xx2, xx3, xx4;
    double   yy1, yy2, yy3, yy4;
    int      win;
    int      i, j;

    switch( mode ) {
		case 0: dist_factor = &(gCparam.dist_factor[0]);   break;
		case 1: dist_factor = &(gsCparam.dist_factorL[0]); break;
		case 2: dist_factor = &(gsCparam.dist_factorR[0]); break;
		default: return;
    }

#ifdef _WIN32
    if( image == NULL ) {
        arMalloc(image,ARUint8,tex2Xsize*tex2Ysize*AR_PIX_SIZE_DEFAULT);
    }
    memcpy(image, wimage, gImXsize*gImYsize*AR_PIX_SIZE_DEFAULT/4);
#endif

    if( initf ) {
        for(j=0;j<3;j++) {
            for(i=0;i<=MINIWIN_MAX;i++) flag[j][i] = 1;
        }
    }
    if( xwin == 0 && ywin == 0 )      win = 0;
    else if( xwin == 1 && ywin == 0 ) win = 1;
    else win = gMiniXnum * (ywin-1) + xwin + 1;

    glEnable( GL_TEXTURE_2D );
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);

    glBindTexture( GL_TEXTURE_2D, glid[2] );
#ifdef APPLE_TEXTURE_FAST_TRANSFER
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_PRIORITY, 0.0);
    glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, 1);
#endif
    glPixelStorei( GL_UNPACK_ROW_LENGTH, gImXsize/2 );

    if( initf == 0 ) {
#if (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_ARGB)
#  ifdef AR_BIG_ENDIAN
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, tex2Xsize, tex2Ysize, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, image );
#  else
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, tex2Xsize, tex2Ysize, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8, image );
#  endif
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_ABGR)
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, tex2Xsize, tex2Ysize, GL_ABGR, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_BGRA)
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, tex2Xsize, tex2Ysize, GL_BGRA, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_BGR)
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, tex2Xsize, tex2Ysize, GL_BGR, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_RGBA)
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, tex2Xsize, tex2Ysize, GL_RGBA, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_RGB)
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, tex2Xsize, tex2Ysize, GL_RGB, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_MONO)
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, tex2Xsize, tex2Ysize, GL_LUMINANCE, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_2vuy)
#  ifdef AR_BIG_ENDIAN
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, tex2Xsize, tex2Ysize, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_REV_APPLE, image );
#  else
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, tex2Xsize, tex2Ysize, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_APPLE, image );
#  endif
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_yuvs)
#  ifdef AR_BIG_ENDIAN
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, tex2Xsize, tex2Ysize, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_APPLE, image );
#  else
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, tex2Xsize, tex2Ysize, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_REV_APPLE, image );
#  endif
#else
#  error Unknown default pixel format defined in config.h
#endif
    }
    else {
#if (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_ARGB)
#  ifdef AR_BIG_ENDIAN
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, tex2Xsize, tex2Ysize, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, image );
#  else
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, tex2Xsize, tex2Ysize, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8, image );
#  endif
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_ABGR)
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, tex2Xsize, tex2Ysize, 0, GL_ABGR, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_BGRA)
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, tex2Xsize, tex2Ysize, 0, GL_BGRA, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_BGR)
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, tex2Xsize, tex2Ysize, 0, GL_BGR, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_RGBA)
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, tex2Xsize, tex2Ysize, 0, GL_RGBA, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_RGB)
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, tex2Xsize, tex2Ysize, 0, GL_RGB, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_MONO)
        glTexImage2D( GL_TEXTURE_2D, 0, GL_LUMINANCE, tex2Xsize, tex2Ysize, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, image );
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_2vuy)
#  ifdef AR_BIG_ENDIAN
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, tex2Xsize, tex2Ysize, 0, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_REV_APPLE, image );
#  else
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, tex2Xsize, tex2Ysize, 0, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_APPLE, image );
#  endif
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_yuvs)
#  ifdef AR_BIG_ENDIAN
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, tex2Xsize, tex2Ysize, 0, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_APPLE, image );
#  else
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, tex2Xsize, tex2Ysize, 0, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_REV_APPLE, image );
#  endif
#else
#  error Unknown default pixel format defined in config.h
#endif
    }

    if( flag[mode][win] ) {
        listIndex[mode][win] = glGenLists(1);
        glNewList(listIndex[mode][win], GL_COMPILE_AND_EXECUTE);

        z = -1.0;
        qy = gImYsize * 0 / 20.0;
        tey = ((double)gImYsize / (double)(tex2Ysize*2.0)) * (double)0 / 20.0;
        for( j = 1; j <= 20; j++ ) {
            py = qy;
            tsy = tey;
            qy = gImYsize * j / 20.0;
            tey = ((double)gImYsize / (double)(tex2Ysize*2.0)) * (double)j / 20.0;

            qx = gImXsize * 0 / 20.0;
            tex = ((double)gImXsize / (double)(tex2Xsize*2.0)) * (double)0 / 20.0;
            for( i = 1; i <= 20; i++ ) {
                px = qx;
                tsx = tex;
                qx = gImXsize * i / 20.0;
                tex = ((double)gImXsize / (double)(tex2Xsize*2.0)) * (double)i / 20.0;

                arParamObserv2Ideal( dist_factor, px, py, &x1, &y1 );
                arParamObserv2Ideal( dist_factor, qx, py, &x2, &y2 );
                arParamObserv2Ideal( dist_factor, qx, qy, &x3, &y3 );
                arParamObserv2Ideal( dist_factor, px, qy, &x4, &y4 );

                if( xwin == 0 && ywin == 0 ) {
                    xx1 = x1 * gZoom;
                    yy1 = gWinYsize - y1 * gZoom;
                    xx2 = x2 * gZoom;
                    yy2 = gWinYsize - y2 * gZoom;
                    xx3 = x3 * gZoom;
                    yy3 = gWinYsize - y3 * gZoom;
                    xx4 = x4 * gZoom;
                    yy4 = gWinYsize - y4 * gZoom;
                }
                else if( xwin == 1 && ywin == 0 ) {
                    xx1 = gXsize + x1 * gZoom;
                    yy1 = gWinYsize - y1 * gZoom;
                    xx2 = gXsize + x2 * gZoom;
                    yy2 = gWinYsize - y2 * gZoom;
                    xx3 = gXsize + x3 * gZoom;
                    yy3 = gWinYsize - y3 * gZoom;
                    xx4 = gXsize + x4 * gZoom;
                    yy4 = gWinYsize - y4 * gZoom;
                }
                else {
                    xx1 = (xwin-1)*gMiniXsize + x1*gZoom/(double)GMINI;
                    xx2 = (xwin-1)*gMiniXsize + x2*gZoom/(double)GMINI;
                    xx3 = (xwin-1)*gMiniXsize + x3*gZoom/(double)GMINI;
                    xx4 = (xwin-1)*gMiniXsize + x4*gZoom/(double)GMINI;
                    yy1 = gWinYsize-gYsize-(ywin-1)*gMiniYsize - y1*gZoom/(double)GMINI;
                    yy2 = gWinYsize-gYsize-(ywin-1)*gMiniYsize - y2*gZoom/(double)GMINI;
                    yy3 = gWinYsize-gYsize-(ywin-1)*gMiniYsize - y3*gZoom/(double)GMINI;
                    yy4 = gWinYsize-gYsize-(ywin-1)*gMiniYsize - y4*gZoom/(double)GMINI;
                }

                glBegin( GL_QUADS );
                glTexCoord2d( tsx, tsy ); glVertex3d( xx1, yy1, z );
                glTexCoord2d( tex, tsy ); glVertex3d( xx2, yy2, z );
                glTexCoord2d( tex, tey ); glVertex3d( xx3, yy3, z );
                glTexCoord2d( tsx, tey ); glVertex3d( xx4, yy4, z );
                glEnd();
            }
        }
        glEndList();
        flag[mode][win] = 0;
    }
    else {
        glCallList( listIndex[mode][win] );
    }

    initf = 0;

    glBindTexture( GL_TEXTURE_2D, 0 );
    glDisable( GL_TEXTURE_2D );
    glPixelStorei( GL_UNPACK_ROW_LENGTH, 0 );
}

void argDrawSquare( double  vertex[4][2], int xwin, int ywin )
{
    argLineSeg( vertex[0][0], vertex[0][1],
                vertex[1][0], vertex[1][1], xwin, ywin );
    argLineSeg( vertex[1][0], vertex[1][1],
                vertex[2][0], vertex[2][1], xwin, ywin );
    argLineSeg( vertex[2][0], vertex[2][1],
                vertex[3][0], vertex[3][1], xwin, ywin );
    argLineSeg( vertex[3][0], vertex[3][1],
                vertex[0][0], vertex[0][1], xwin, ywin );
}

void argLineSeg( double x1, double y1, double x2, double y2, int xwin, int ywin )
{
    float   ox, oy;
    double  xx1, yy1, xx2, yy2;

    if( argDrawMode == AR_DRAW_BY_TEXTURE_MAPPING ) {
        xx1 = x1;  yy1 = y1;
        xx2 = x2;  yy2 = y2;
    }
    else {
        arParamIdeal2Observ( gCparam.dist_factor, x1, y1, &xx1, &yy1 );
        arParamIdeal2Observ( gCparam.dist_factor, x2, y2, &xx2, &yy2 );
    }

    xx1 *= gZoom; yy1 *= gZoom;
    xx2 *= gZoom; yy2 *= gZoom;

    if( xwin == 0 && ywin == 0 ) {
        ox = 0;
        oy = gWinYsize-1;
        glBegin(GL_LINES);
          glVertex2f( ox+xx1, oy-yy1 );
          glVertex2f( ox+xx2, oy-yy2 );
        glEnd();
    }
    else {
        ox = (xwin-1)*gMiniXsize;
        oy = gWinYsize - gYsize -(ywin-1)*gMiniYsize - 1;
        glBegin(GL_LINES);
          glVertex2f( ox+xx1/GMINI, oy-yy1/GMINI );
          glVertex2f( ox+xx2/GMINI, oy-yy2/GMINI );
        glEnd();
    }

    glFlush();
}

void argLineSegHMD( double x1, double y1, double x2, double y2 )
{
    float   ox, oy;

    ox = 0;
    oy = gWinYsize - gYsize;
    glBegin(GL_LINES);
      glVertex2f( ox+x1, oy+y1 );
      glVertex2f( ox+x2, oy+y2 );
    glEnd();
}


static void argInitStencil(void)
{
    int     offset;
    int     i;

    glEnable(GL_STENCIL_TEST);
    glClearStencil(0);
    glClear(GL_STENCIL_BUFFER_BIT);
    glLineWidth(1.0);

    offset = gWinYsize - gYsize;

#if REVERSE_LR
    glStencilFunc(GL_ALWAYS, LEFTEYE, LEFTEYE);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
    glBegin(GL_LINES);
    for( i = 0; i < gYsize; i+=2 ) {
        glVertex2f( 0.0,       (float)(i+offset) );
        glVertex2f( gWinXsize, (float)(i+offset) );
    }
    glEnd();

    glStencilFunc(GL_ALWAYS, RIGHTEYE, RIGHTEYE);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
    glBegin(GL_LINES);
    for( i = 1; i < gYsize; i+=2 ) {
        glVertex2f( 0.0,       (float)(i+offset) );
        glVertex2f( gWinXsize, (float)(i+offset) );
    }
    glEnd();
#else
    glStencilFunc(GL_ALWAYS, LEFTEYE, LEFTEYE);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
    glBegin(GL_LINES);
    for( i = 1; i < gYsize; i+=2 ) {
        glVertex2f( 0.0,       (float)(i+offset) );
        glVertex2f( gWinXsize, (float)(i+offset) );
    }
    glEnd();

    glStencilFunc(GL_ALWAYS, RIGHTEYE, RIGHTEYE);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
    glBegin(GL_LINES);
    for( i = 0; i < gYsize; i+=2 ) {
        glVertex2f( 0.0,       (float)(i+offset) );
        glVertex2f( gWinXsize, (float)(i+offset) );
    }
    glEnd();
#endif

    glStencilFunc (GL_ALWAYS, 0, 0);
    glStencilOp (GL_KEEP, GL_KEEP, GL_KEEP);
    glDisable(GL_STENCIL_TEST);
}

void argLoadHMDparam( ARParam *lparam, ARParam *rparam )
{
    argConvGLcpara( lparam, AR_GL_CLIP_NEAR, AR_GL_CLIP_FAR, gl_lpara );
    argConvGLcpara( rparam, AR_GL_CLIP_NEAR, AR_GL_CLIP_FAR, gl_rpara );

    gl_hmd_para_flag = 1;
}


static void argSetStencil( int flag )
{
    if( flag == 0 ) {
        glDisable(GL_STENCIL_TEST);
        glStencilFunc (GL_ALWAYS, 0, 0);
        glStencilOp (GL_KEEP, GL_KEEP, GL_KEEP);
    }
    else {
        glEnable(GL_STENCIL_TEST);
        glStencilFunc (GL_EQUAL, flag, flag);
        glStencilOp (GL_KEEP, GL_KEEP, GL_KEEP);
    }
}

void argConvGLcpara( ARParam *param, double gnear, double gfar, double m[16] )
{
    argConvGLcpara2( param->mat, param->xsize, param->ysize, gnear, gfar, m );
}

static void argConvGLcpara2( double cparam[3][4], int width, int height, double gnear, double gfar, double m[16] )
{
    double   icpara[3][4];
    double   trans[3][4];
    double   p[3][3], q[4][4];
    int      i, j;

    if( arParamDecompMat(cparam, icpara, trans) < 0 ) {
        printf("gConvGLcpara: Parameter error!!\n");
        exit(0);
    }

    for( i = 0; i < 3; i++ ) {
        for( j = 0; j < 3; j++ ) {
            p[i][j] = icpara[i][j] / icpara[2][2];
        }
    }
    q[0][0] = (2.0 * p[0][0] / width);
    q[0][1] = (2.0 * p[0][1] / width);
    q[0][2] = ((2.0 * p[0][2] / width)  - 1.0);
    q[0][3] = 0.0;

    q[1][0] = 0.0;
    q[1][1] = (2.0 * p[1][1] / height);
    q[1][2] = ((2.0 * p[1][2] / height) - 1.0);
    q[1][3] = 0.0;

    q[2][0] = 0.0;
    q[2][1] = 0.0;
    q[2][2] = (gfar + gnear)/(gfar - gnear);
    q[2][3] = -2.0 * gfar * gnear / (gfar - gnear);

    q[3][0] = 0.0;
    q[3][1] = 0.0;
    q[3][2] = 1.0;
    q[3][3] = 0.0;

    for( i = 0; i < 4; i++ ) {
        for( j = 0; j < 3; j++ ) {
            m[i+j*4] = q[i][0] * trans[0][j]
                     + q[i][1] * trans[1][j]
                     + q[i][2] * trans[2][j];
        }
        m[i+3*4] = q[i][0] * trans[0][3]
                 + q[i][1] * trans[1][3]
                 + q[i][2] * trans[2][3]
                 + q[i][3];
    }
}

void removeWarning()
{
	glutInit(0, NULL);
}

static ARInt16      l_imageL[HARDCODED_BUFFER_WIDTH*HARDCODED_BUFFER_HEIGHT];
static ARInt16      l_imageR[HARDCODED_BUFFER_WIDTH*HARDCODED_BUFFER_HEIGHT];
/*****************************************************************************/


static int          workL[WORK_SIZE];
static int          workR[WORK_SIZE];
static int          work2L[WORK_SIZE*7];
static int          work2R[WORK_SIZE*7];

static int          wlabel_numL;
static int          wlabel_numR;
static int          wareaL[WORK_SIZE];
static int          wareaR[WORK_SIZE];
static int          wclipL[WORK_SIZE*4];
static int          wclipR[WORK_SIZE*4];
static double       wposL[WORK_SIZE*2];
static double       wposR[WORK_SIZE*2];

static ARInt16 *labeling2( ARUint8 *image, int thresh,
                           int *label_num, int **area, double **pos, int **clip,
                           int **label_ref, int LorR );
static ARInt16 *labeling3( ARUint8 *image, int thresh,
                           int *label_num, int **area, double **pos, int **clip,
                           int **label_ref, int LorR );

void arGetImgFeature( int *num, int **area, int **clip, double **pos )
{
    *num  = wlabel_numL;
    *area = wareaL;
    *clip = wclipL;
    *pos  = wposL;

    return;
}

ARInt16 *arLabeling( ARUint8 *image, int thresh,
                     int *label_num, int **area, double **pos, int **clip,
                     int **label_ref )
{
    if( arDebug ) {
        return( labeling3(image, thresh, label_num,
                          area, pos, clip, label_ref, 1) );
    } else {
        return( labeling2(image, thresh, label_num,
                          area, pos, clip, label_ref, 1) );
    }
}

void arsGetImgFeature( int *num, int **area, int **clip, double **pos, int LorR )
{
    if (LorR) {
        *num  = wlabel_numL;
        *area = wareaL;
        *clip = wclipL;
        *pos  = wposL;
    } else {
        *num  = wlabel_numR;
        *area = wareaR;
        *clip = wclipR;
        *pos  = wposR;
    }

    return;
}

ARInt16 *arsLabeling( ARUint8 *image, int thresh,
                      int *label_num, int **area, double **pos, int **clip,
                      int **label_ref, int LorR )
{
    if( arDebug ) {
        return( labeling3(image, thresh, label_num,
                          area, pos, clip, label_ref, LorR) );
    } else {
        return( labeling2(image, thresh, label_num,
                          area, pos, clip, label_ref, LorR) );
    }
}

static ARInt16 *labeling2( ARUint8 *image, int thresh,
                           int *label_num, int **area, double **pos, int **clip,
                           int **label_ref, int LorR )
{
    ARUint8   *pnt;                     /*  image pointer       */
    ARInt16   *pnt1, *pnt2;             /*  image pointer       */
    int       *wk;                      /*  pointer for work    */
    int       wk_max;                   /*  work                */
    int       m,n;                      /*  work                */
    int       i,j,k;                    /*  for loop            */
    int       lxsize, lysize;
    int       poff;
    ARInt16   *l_image;
    int       *work, *work2;
    int       *wlabel_num;
    int       *warea;
    int       *wclip;
    double    *wpos;
#ifdef USE_OPTIMIZATIONS
	int		  pnt2_index;   // [tp]
#endif
	int		  thresht3 = thresh * 3;

	if (LorR) {
        l_image = &l_imageL[0];
        work    = &workL[0];
        work2   = &work2L[0];
        wlabel_num = &wlabel_numL;
        warea   = &wareaL[0];
        wclip   = &wclipL[0];
        wpos    = &wposL[0];
    } else {
        l_image = &l_imageR[0];
        work    = &workR[0];
        work2   = &work2R[0];
        wlabel_num = &wlabel_numR;
        warea   = &wareaR[0];
        wclip   = &wclipR[0];
        wpos    = &wposR[0];
    }

    if (arImageProcMode == AR_IMAGE_PROC_IN_HALF) {
        lxsize = arImXsize / 2;
        lysize = arImYsize / 2;
    } else {
        lxsize = arImXsize;
        lysize = arImYsize;
    }

    pnt1 = &l_image[0]; // Leftmost pixel of top row of image.
    pnt2 = &l_image[(lysize - 1)*lxsize]; // Leftmost pixel of bottom row of image.

#ifndef USE_OPTIMIZATIONS
	for(i = 0; i < lxsize; i++) {
        *(pnt1++) = *(pnt2++) = 0;
    }
#else
// 4x loop unrolling
	for (i = 0; i < lxsize - (lxsize%4); i += 4) {
        *(pnt1++) = *(pnt2++) = 0;
        *(pnt1++) = *(pnt2++) = 0;
        *(pnt1++) = *(pnt2++) = 0;
        *(pnt1++) = *(pnt2++) = 0;
    }
#endif
    pnt1 = &l_image[0]; // Leftmost pixel of top row of image.
    pnt2 = &l_image[lxsize - 1]; // Rightmost pixel of top row of image.

#ifndef USE_OPTIMIZATIONS
    for(i = 0; i < lysize; i++) {
        *pnt1 = *pnt2 = 0;
        pnt1 += lxsize;
        pnt2 += lxsize;
    }
#else
// 4x loop unrolling
    for (i = 0; i < lysize - (lysize%4); i += 4) {
		*pnt1 = *pnt2 = 0;
        pnt1 += lxsize;
        pnt2 += lxsize;

		*pnt1 = *pnt2 = 0;
        pnt1 += lxsize;
        pnt2 += lxsize;

		*pnt1 = *pnt2 = 0;
        pnt1 += lxsize;
        pnt2 += lxsize;

		*pnt1 = *pnt2 = 0;
        pnt1 += lxsize;
        pnt2 += lxsize;
    }
#endif

    wk_max = 0;
    pnt2 = &(l_image[lxsize+1]);
    if (arImageProcMode == AR_IMAGE_PROC_IN_HALF) {
        pnt = &(image[(arImXsize*2+2)*AR_PIX_SIZE_DEFAULT]);
        poff = AR_PIX_SIZE_DEFAULT*2;
    } else {
        pnt = &(image[(arImXsize+1)*AR_PIX_SIZE_DEFAULT]);
        poff = AR_PIX_SIZE_DEFAULT;
    }
    for (j = 1; j < lysize - 1; j++, pnt += poff*2, pnt2 += 2) {
        for(i = 1; i < lxsize-1; i++, pnt+=poff, pnt2++) {
#if (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_ARGB)
            if( *(pnt+1) + *(pnt+2) + *(pnt+3) <= thresht3 )
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_ABGR)
            if( *(pnt+1) + *(pnt+2) + *(pnt+3) <= thresht3 )
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_BGRA)
            if( *(pnt+0) + *(pnt+1) + *(pnt+2) <= thresht3 )
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_BGR)
            if( *(pnt+0) + *(pnt+1) + *(pnt+2) <= thresht3 )
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_RGBA)
            if( *(pnt+0) + *(pnt+1) + *(pnt+2) <= thresht3 )
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_RGB)
            if( *(pnt+0) + *(pnt+1) + *(pnt+2) <= thresht3 )
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_MONO)
			if( *(pnt) <= thresh )
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_2vuy)
			if( *(pnt+1) <= thresh )
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_yuvs)
			if( *(pnt+0) <= thresh )
#else
#  error Unknown default pixel format defined in config.h
#endif
			{
                pnt1 = &(pnt2[-lxsize]);
                if( *pnt1 > 0 ) {
                    *pnt2 = *pnt1;

#ifndef USE_OPTIMIZATIONS
					// ORIGINAL CODE
					work2[((*pnt2)-1)*7+0] ++;
                    work2[((*pnt2)-1)*7+1] += i;
                    work2[((*pnt2)-1)*7+2] += j;
                    work2[((*pnt2)-1)*7+6] = j;
#else
					// OPTIMIZED CODE [tp]
					// ((*pnt2)-1)*7 should be treated as constant, since
					//  work2[n] (n=0..xsize*ysize) cannot overwrite (*pnt2)
					pnt2_index = ((*pnt2)-1) * 7;
                    work2[pnt2_index+0]++;
                    work2[pnt2_index+1]+= i;
                    work2[pnt2_index+2]+= j;
                    work2[pnt2_index+6] = j;
					// --------------------------------
#endif
                }
                else if( *(pnt1+1) > 0 ) {
                    if( *(pnt1-1) > 0 ) {
                        m = work[*(pnt1+1)-1];
                        n = work[*(pnt1-1)-1];
                        if( m > n ) {
                            *pnt2 = n;
                            wk = &(work[0]);
                            for(k = 0; k < wk_max; k++) {
                                if( *wk == m ) *wk = n;
                                wk++;
                            }
                        }
                        else if( m < n ) {
                            *pnt2 = m;
                            wk = &(work[0]);
                            for(k = 0; k < wk_max; k++) {
                                if( *wk == n ) *wk = m;
                                wk++;
                            }
                        }
                        else *pnt2 = m;

#ifndef USE_OPTIMIZATIONS
						// ORIGINAL CODE
						work2[((*pnt2)-1)*7+0] ++;
                        work2[((*pnt2)-1)*7+1] += i;
                        work2[((*pnt2)-1)*7+2] += j;
                        work2[((*pnt2)-1)*7+6] = j;
#else
						// PERFORMANCE OPTIMIZATION:
						pnt2_index = ((*pnt2)-1) * 7;
						work2[pnt2_index+0]++;
						work2[pnt2_index+1]+= i;
						work2[pnt2_index+2]+= j;
						work2[pnt2_index+6] = j;
#endif

                    }
                    else if( *(pnt2-1) > 0 ) {
                        m = work[*(pnt1+1)-1];
                        n = work[*(pnt2-1)-1];
                        if( m > n ) {
                            *pnt2 = n;
                            wk = &(work[0]);
                            for(k = 0; k < wk_max; k++) {
                                if( *wk == m ) *wk = n;
                                wk++;
                            }
                        }
                        else if( m < n ) {
                            *pnt2 = m;
                            wk = &(work[0]);
                            for(k = 0; k < wk_max; k++) {
                                if( *wk == n ) *wk = m;
                                wk++;
                            }
                        }
                        else *pnt2 = m;

#ifndef USE_OPTIMIZATIONS
						// ORIGINAL CODE
                        work2[((*pnt2)-1)*7+0] ++;
                        work2[((*pnt2)-1)*7+1] += i;
                        work2[((*pnt2)-1)*7+2] += j;
#else
						// PERFORMANCE OPTIMIZATION:
						pnt2_index = ((*pnt2)-1) * 7;
						work2[pnt2_index+0]++;
						work2[pnt2_index+1]+= i;
						work2[pnt2_index+2]+= j;
#endif

                    }
                    else {
                        *pnt2 = *(pnt1+1);

#ifndef USE_OPTIMIZATIONS
						// ORIGINAL CODE
                        work2[((*pnt2)-1)*7+0] ++;
                        work2[((*pnt2)-1)*7+1] += i;
                        work2[((*pnt2)-1)*7+2] += j;
                        if( work2[((*pnt2)-1)*7+3] > i ) work2[((*pnt2)-1)*7+3] = i;
                        work2[((*pnt2)-1)*7+6] = j;
#else
						// PERFORMANCE OPTIMIZATION:
						pnt2_index = ((*pnt2)-1) * 7;
						work2[pnt2_index+0]++;
						work2[pnt2_index+1]+= i;
						work2[pnt2_index+2]+= j;
                        if( work2[pnt2_index+3] > i ) work2[pnt2_index+3] = i;
						work2[pnt2_index+6] = j;
#endif
                    }
                }
                else if( *(pnt1-1) > 0 ) {
                    *pnt2 = *(pnt1-1);

#ifndef USE_OPTIMIZATIONS
						// ORIGINAL CODE
                    work2[((*pnt2)-1)*7+0] ++;
                    work2[((*pnt2)-1)*7+1] += i;
                    work2[((*pnt2)-1)*7+2] += j;
                    if( work2[((*pnt2)-1)*7+4] < i ) work2[((*pnt2)-1)*7+4] = i;
                    work2[((*pnt2)-1)*7+6] = j;
#else
					// PERFORMANCE OPTIMIZATION:
					pnt2_index = ((*pnt2)-1) * 7;
					work2[pnt2_index+0]++;
					work2[pnt2_index+1]+= i;
					work2[pnt2_index+2]+= j;
                    if( work2[pnt2_index+4] < i ) work2[pnt2_index+4] = i;
					work2[pnt2_index+6] = j;
#endif
                }
                else if( *(pnt2-1) > 0) {
                    *pnt2 = *(pnt2-1);

#ifndef USE_OPTIMIZATIONS
						// ORIGINAL CODE
                    work2[((*pnt2)-1)*7+0] ++;
                    work2[((*pnt2)-1)*7+1] += i;
                    work2[((*pnt2)-1)*7+2] += j;
                    if( work2[((*pnt2)-1)*7+4] < i ) work2[((*pnt2)-1)*7+4] = i;
#else
					// PERFORMANCE OPTIMIZATION:
					pnt2_index = ((*pnt2)-1) * 7;
					work2[pnt2_index+0]++;
					work2[pnt2_index+1]+= i;
					work2[pnt2_index+2]+= j;
                    if( work2[pnt2_index+4] < i ) work2[pnt2_index+4] = i;
#endif
				}
                else {
                    wk_max++;
                    if( wk_max > WORK_SIZE ) {
                        return(0);
                    }
                    work[wk_max-1] = *pnt2 = wk_max;
                    work2[(wk_max-1)*7+0] = 1;
                    work2[(wk_max-1)*7+1] = i;
                    work2[(wk_max-1)*7+2] = j;
                    work2[(wk_max-1)*7+3] = i;
                    work2[(wk_max-1)*7+4] = i;
                    work2[(wk_max-1)*7+5] = j;
                    work2[(wk_max-1)*7+6] = j;
                }
            }
            else {
                *pnt2 = 0;
            }
        }
        if (arImageProcMode == AR_IMAGE_PROC_IN_HALF) pnt += arImXsize*AR_PIX_SIZE_DEFAULT;
    }

    j = 1;
    wk = &(work[0]);
    for(i = 1; i <= wk_max; i++, wk++) {
        *wk = (*wk==i)? j++: work[(*wk)-1];
    }
    *label_num = *wlabel_num = j - 1;
    if( *label_num == 0 ) {
        return( l_image );
    }

    put_zero( (ARUint8 *)warea, *label_num *     sizeof(int) );
    put_zero( (ARUint8 *)wpos,  *label_num * 2 * sizeof(double) );
    for(i = 0; i < *label_num; i++) {
        wclip[i*4+0] = lxsize;
        wclip[i*4+1] = 0;
        wclip[i*4+2] = lysize;
        wclip[i*4+3] = 0;
    }
    for(i = 0; i < wk_max; i++) {
        j = work[i] - 1;
        warea[j]    += work2[i*7+0];
        wpos[j*2+0] += work2[i*7+1];
        wpos[j*2+1] += work2[i*7+2];
        if( wclip[j*4+0] > work2[i*7+3] ) wclip[j*4+0] = work2[i*7+3];
        if( wclip[j*4+1] < work2[i*7+4] ) wclip[j*4+1] = work2[i*7+4];
        if( wclip[j*4+2] > work2[i*7+5] ) wclip[j*4+2] = work2[i*7+5];
        if( wclip[j*4+3] < work2[i*7+6] ) wclip[j*4+3] = work2[i*7+6];
    }

    for( i = 0; i < *label_num; i++ ) {
        wpos[i*2+0] /= warea[i];
        wpos[i*2+1] /= warea[i];
    }

    *label_ref = work;
    *area      = warea;
    *pos       = wpos;
    *clip      = wclip;
    return (l_image);
}

static ARInt16 *labeling3( ARUint8 *image, int thresh,
                           int *label_num, int **area, double **pos, int **clip,
                           int **label_ref, int LorR )
{
    ARUint8   *pnt;                     /*  image pointer       */
    ARInt16   *pnt1, *pnt2;             /*  image pointer       */
    int       *wk;                      /*  pointer for work    */
    int       wk_max;                   /*  work                */
    int       m,n;                      /*  work                */
    int       i,j,k;                    /*  for loop            */
    int       lxsize, lysize;
    int       poff;
    ARUint8   *dpnt;
    ARInt16   *l_image;
    int       *work, *work2;
    int       *wlabel_num;
    int       *warea;
    int       *wclip;
    double    *wpos;
	int		  thresht3 = thresh * 3;
	static int imageProcModePrev = -1;
	static int imXsizePrev = -1;
	static int imYsizePrev = -1;

	// Ensure that the debug image is correct size.
	// If size has changed, debug image will need to be re-allocated.
	if (imageProcModePrev != arImageProcMode || imXsizePrev != arImXsize || imYsizePrev != arImYsize) {
		arLabelingCleanup();
		imageProcModePrev = arImageProcMode;
		imXsizePrev = arImXsize;
		imYsizePrev = arImYsize;
	}

    if( arImageProcMode == AR_IMAGE_PROC_IN_HALF ) {
        lxsize = arImXsize / 2;
        lysize = arImYsize / 2;
    }
    else {
        lxsize = arImXsize;
        lysize = arImYsize;
    }

    if( LorR ) {
        l_image = &l_imageL[0];
        work    = &workL[0];
        work2   = &work2L[0];
        wlabel_num = &wlabel_numL;
        warea   = &wareaL[0];
        wclip   = &wclipL[0];
        wpos    = &wposL[0];
        if( arImageL == NULL ) {
#if 0
            int texXsize = 1;
            int texYsize = 1;
            while( texXsize < arImXsize ) texXsize *= 2;
            if( texXsize > 512 ) texXsize = 512;
            while( texYsize < arImYsize ) texYsize *= 2;
            arMalloc( arImageL, ARUint8, texXsize*texYsize*AR_PIX_SIZE_DEFAULT );
#else
            arMalloc( arImageL, ARUint8, arImXsize*arImYsize*AR_PIX_SIZE_DEFAULT );
#endif
            put_zero( arImageL, lxsize*lysize*AR_PIX_SIZE_DEFAULT );
            arImage = arImageL;
        }
    }
    else {
        l_image = &l_imageR[0];
        work    = &workR[0];
        work2   = &work2R[0];
        wlabel_num = &wlabel_numR;
        warea   = &wareaR[0];
        wclip   = &wclipR[0];
        wpos    = &wposR[0];
        if( arImageR == NULL ) {
#if 0
            int texXsize = 1;
            int texYsize = 1;
            while( texXsize < arImXsize ) texXsize *= 2;
            if( texXsize > 512 ) texXsize = 512;
            while( texYsize < arImYsize ) texYsize *= 2;
            arMalloc( arImageR, ARUint8, texXsize*texYsize*AR_PIX_SIZE_DEFAULT );
#else
            arMalloc( arImageR, ARUint8, arImXsize*arImYsize*AR_PIX_SIZE_DEFAULT );
#endif
            put_zero( arImageR, lxsize*lysize*AR_PIX_SIZE_DEFAULT );
        }
    }

    pnt1 = &l_image[0];
    pnt2 = &l_image[(lysize-1)*lxsize];
    for(i = 0; i < lxsize; i++) {
        *(pnt1++) = *(pnt2++) = 0;
    }

    pnt1 = &l_image[0];
    pnt2 = &l_image[lxsize-1];
    for(i = 0; i < lysize; i++) {
        *pnt1 = *pnt2 = 0;
        pnt1 += lxsize;
        pnt2 += lxsize;
    }

    wk_max = 0;
    pnt2 = &(l_image[lxsize+1]);
    if( LorR ) dpnt = &(arImageL[(lxsize+1)*AR_PIX_SIZE_DEFAULT]);
    else       dpnt = &(arImageR[(lxsize+1)*AR_PIX_SIZE_DEFAULT]);
    if( arImageProcMode == AR_IMAGE_PROC_IN_HALF ) {
        pnt = &(image[(arImXsize*2+2)*AR_PIX_SIZE_DEFAULT]);
        poff = AR_PIX_SIZE_DEFAULT*2;
    }
    else {
        pnt = &(image[(arImXsize+1)*AR_PIX_SIZE_DEFAULT]);
        poff = AR_PIX_SIZE_DEFAULT;
    }
    for(j = 1; j < lysize-1; j++, pnt+=poff*2, pnt2+=2, dpnt+=AR_PIX_SIZE_DEFAULT*2) {
        for(i = 1; i < lxsize-1; i++, pnt+=poff, pnt2++, dpnt+=AR_PIX_SIZE_DEFAULT) {
#if (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_ARGB)
            if( *(pnt+1) + *(pnt+2) + *(pnt+3) <= thresht3 ) {
                *(dpnt+1) = *(dpnt+2) = *(dpnt+3) = 255;
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_ABGR)
            if( *(pnt+1) + *(pnt+2) + *(pnt+3) <= thresht3 ) {
                *(dpnt+1) = *(dpnt+2) = *(dpnt+3) = 255;
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_BGRA)
            if( *(pnt+0) + *(pnt+1) + *(pnt+2) <= thresht3 ) {
                *(dpnt+0) = *(dpnt+1) = *(dpnt+2) = 255;
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_BGR)
            if( *(pnt+0) + *(pnt+1) + *(pnt+2) <= thresht3 ) {
                *(dpnt+0) = *(dpnt+1) = *(dpnt+2) = 255;
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_RGBA)
            if( *(pnt+0) + *(pnt+1) + *(pnt+2) <= thresht3 ) {
                *(dpnt+0) = *(dpnt+1) = *(dpnt+2) = 255;
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_RGB)
            if( *(pnt+0) + *(pnt+1) + *(pnt+2) <= thresht3 ) {
                *(dpnt+0) = *(dpnt+1) = *(dpnt+2) = 255;
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_MONO)
			if( *(pnt) <= thresh ) {
				*(dpnt) = 255;
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_2vuy)
			if( *(pnt+1) <= thresh ) {
				*(dpnt+0) = 128; *(dpnt+1) = 235; // *(dpnt+0) is chroma, set to 128 to maintain black & white debug image.
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_yuvs)
			if( *(pnt+0) <= thresh ) {
				*(dpnt+0) = 235; *(dpnt+1) = 128; // *(dpnt+1) is chroma, set to 128 to maintain black & white debug image.
#else
#  error Unknown default pixel format defined in config.h
#endif
						pnt1 = &(pnt2[-lxsize]);
                if( *pnt1 > 0 ) {
                    *pnt2 = *pnt1;
                    work2[((*pnt2)-1)*7+0] ++;
                    work2[((*pnt2)-1)*7+1] += i;
                    work2[((*pnt2)-1)*7+2] += j;
                    work2[((*pnt2)-1)*7+6] = j;
                }
                else if( *(pnt1+1) > 0 ) {
                    if( *(pnt1-1) > 0 ) {
                        m = work[*(pnt1+1)-1];
                        n = work[*(pnt1-1)-1];
                        if( m > n ) {
                            *pnt2 = n;
                            wk = &(work[0]);
                            for(k = 0; k < wk_max; k++) {
                                if( *wk == m ) *wk = n;
                                wk++;
                            }
                        }
                        else if( m < n ) {
                            *pnt2 = m;
                            wk = &(work[0]);
                            for(k = 0; k < wk_max; k++) {
                                if( *wk == n ) *wk = m;
                                wk++;
                            }
                        }
                        else *pnt2 = m;
                        work2[((*pnt2)-1)*7+0] ++;
                        work2[((*pnt2)-1)*7+1] += i;
                        work2[((*pnt2)-1)*7+2] += j;
                        work2[((*pnt2)-1)*7+6] = j;
                    }
                    else if( *(pnt2-1) > 0 ) {
                        m = work[*(pnt1+1)-1];
                        n = work[*(pnt2-1)-1];
                        if( m > n ) {
                            *pnt2 = n;
                            wk = &(work[0]);
                            for(k = 0; k < wk_max; k++) {
                                if( *wk == m ) *wk = n;
                                wk++;
                            }
                        }
                        else if( m < n ) {
                            *pnt2 = m;
                            wk = &(work[0]);
                            for(k = 0; k < wk_max; k++) {
                                if( *wk == n ) *wk = m;
                                wk++;
                            }
                        }
                        else *pnt2 = m;
                        work2[((*pnt2)-1)*7+0] ++;
                        work2[((*pnt2)-1)*7+1] += i;
                        work2[((*pnt2)-1)*7+2] += j;
                    }
                    else {
                        *pnt2 = *(pnt1+1);
                        work2[((*pnt2)-1)*7+0] ++;
                        work2[((*pnt2)-1)*7+1] += i;
                        work2[((*pnt2)-1)*7+2] += j;
                        if( work2[((*pnt2)-1)*7+3] > i ) work2[((*pnt2)-1)*7+3] = i;
                        work2[((*pnt2)-1)*7+6] = j;
                    }
                }
                else if( *(pnt1-1) > 0 ) {
                    *pnt2 = *(pnt1-1);
                    work2[((*pnt2)-1)*7+0] ++;
                    work2[((*pnt2)-1)*7+1] += i;
                    work2[((*pnt2)-1)*7+2] += j;
                    if( work2[((*pnt2)-1)*7+4] < i ) work2[((*pnt2)-1)*7+4] = i;
                    work2[((*pnt2)-1)*7+6] = j;
                }
                else if( *(pnt2-1) > 0) {
                    *pnt2 = *(pnt2-1);
                    work2[((*pnt2)-1)*7+0] ++;
                    work2[((*pnt2)-1)*7+1] += i;
                    work2[((*pnt2)-1)*7+2] += j;
                    if( work2[((*pnt2)-1)*7+4] < i ) work2[((*pnt2)-1)*7+4] = i;
                }
                else {
                    wk_max++;
                    if( wk_max > WORK_SIZE ) {
                        return(0);
                    }
                    work[wk_max-1] = *pnt2 = wk_max;
                    work2[(wk_max-1)*7+0] = 1;
                    work2[(wk_max-1)*7+1] = i;
                    work2[(wk_max-1)*7+2] = j;
                    work2[(wk_max-1)*7+3] = i;
                    work2[(wk_max-1)*7+4] = i;
                    work2[(wk_max-1)*7+5] = j;
                    work2[(wk_max-1)*7+6] = j;
                }
            }
            else {
                *pnt2 = 0;
#if (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_ARGB)
                *(dpnt+1) = *(dpnt+2) = *(dpnt+3) = 0; }
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_ABGR)
                *(dpnt+1) = *(dpnt+2) = *(dpnt+3) = 0; }
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_BGRA)
                *(dpnt+0) = *(dpnt+1) = *(dpnt+2) = 0; }
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_BGR)
                *(dpnt+0) = *(dpnt+1) = *(dpnt+2) = 0; }
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_RGBA)
                *(dpnt+0) = *(dpnt+1) = *(dpnt+2) = 0; }
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_RGB)
                *(dpnt+0) = *(dpnt+1) = *(dpnt+2) = 0; }
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_MONO)
                *(dpnt) = 0; }
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_2vuy)
                *(dpnt+0) = 128; *(dpnt+1) = 16; } // *(dpnt+0) is chroma, set to 128 to maintain black & white debug image.
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_yuvs)
                *(dpnt+0) = 16; *(dpnt+1) = 128; } // *(dpnt+1) is chroma, set to 128 to maintain black & white debug image.
#else
#  error Unknown default pixel format defined in config.h
#endif
        }
        if (arImageProcMode == AR_IMAGE_PROC_IN_HALF) pnt += arImXsize*AR_PIX_SIZE_DEFAULT;
    }

    j = 1;
    wk = &(work[0]);
    for(i = 1; i <= wk_max; i++, wk++) {
        *wk = (*wk==i)? j++: work[(*wk)-1];
    }
    *label_num = *wlabel_num = j - 1;
    if( *label_num == 0 ) {
        return( l_image );
    }

    put_zero( (ARUint8 *)warea, *label_num *     sizeof(int) );
    put_zero( (ARUint8 *)wpos,  *label_num * 2 * sizeof(double) );
    for(i = 0; i < *label_num; i++) {
        wclip[i*4+0] = lxsize;
        wclip[i*4+1] = 0;
        wclip[i*4+2] = lysize;
        wclip[i*4+3] = 0;
    }
    for(i = 0; i < wk_max; i++) {
        j = work[i] - 1;
        warea[j]    += work2[i*7+0];
        wpos[j*2+0] += work2[i*7+1];
        wpos[j*2+1] += work2[i*7+2];
        if( wclip[j*4+0] > work2[i*7+3] ) wclip[j*4+0] = work2[i*7+3];
        if( wclip[j*4+1] < work2[i*7+4] ) wclip[j*4+1] = work2[i*7+4];
        if( wclip[j*4+2] > work2[i*7+5] ) wclip[j*4+2] = work2[i*7+5];
        if( wclip[j*4+3] < work2[i*7+6] ) wclip[j*4+3] = work2[i*7+6];
    }

    for( i = 0; i < *label_num; i++ ) {
        wpos[i*2+0] /= warea[i];
        wpos[i*2+1] /= warea[i];
    }

    *label_ref = work;
    *area      = warea;
    *pos       = wpos;
    *clip      = wclip;
    return( l_image );
}

void arLabelingCleanup(void)
{
	if (arImageL) {
		free (arImageL);
		arImageL = NULL;
		arImage = NULL;
	}
	if (arImageR) {
		free (arImageR);
		arImageR = NULL;
	}
}

///gsub_lite.c

// ============================================================================
//	Private types and defines.
// ============================================================================
#ifdef _MSC_VER
#  pragma warning (disable:4068)	// Disable MSVC warnings about unknown pragmas.
#endif

// Make sure that required OpenGL constant definitions are available at compile-time.
// N.B. These should not be used unless the renderer indicates (at run-time) that it supports them.

// Define constants for extensions which became core in OpenGL 1.2
#ifndef GL_VERSION_1_2
#  if GL_EXT_bgra
#    define GL_BGR							GL_BGR_EXT
#    define GL_BGRA							GL_BGRA_EXT
#  else
#    define GL_BGR							0x80E0
#    define GL_BGRA							0x80E1
#  endif
#  ifndef GL_APPLE_packed_pixels
#    define GL_UNSIGNED_INT_8_8_8_8			0x8035
#    define GL_UNSIGNED_INT_8_8_8_8_REV		0x8367
#  endif
#  if GL_SGIS_texture_edge_clamp
#    define GL_CLAMP_TO_EDGE				GL_CLAMP_TO_EDGE_SGIS
#  else
#    define GL_CLAMP_TO_EDGE				0x812F
#  endif
#endif

// Define constants for extensions (not yet core).
#ifndef GL_APPLE_ycbcr_422
#  define GL_YCBCR_422_APPLE				0x85B9
#  define GL_UNSIGNED_SHORT_8_8_APPLE		0x85BA
#  define GL_UNSIGNED_SHORT_8_8_REV_APPLE	0x85BB
#endif
#ifndef GL_EXT_abgr
#  define GL_ABGR_EXT						0x8000
#endif
#if GL_NV_texture_rectangle
#  define GL_TEXTURE_RECTANGLE				GL_TEXTURE_RECTANGLE_NV
#  define GL_PROXY_TEXTURE_RECTANGLE		GL_PROXY_TEXTURE_RECTANGLE_NV
#  define GL_MAX_RECTANGLE_TEXTURE_SIZE		GL_MAX_RECTANGLE_TEXTURE_SIZE_NV
#elif GL_EXT_texture_rectangle
#  define GL_TEXTURE_RECTANGLE				GL_TEXTURE_RECTANGLE_EXT
#  define GL_PROXY_TEXTURE_RECTANGLE		GL_PROXY_TEXTURE_RECTANGLE_EXT
#  define GL_MAX_RECTANGLE_TEXTURE_SIZE		GL_MAX_RECTANGLE_TEXTURE_SIZE_EXT
#else
#  define GL_TEXTURE_RECTANGLE				0x84F5
#  define GL_PROXY_TEXTURE_RECTANGLE		0x84F7
#  define GL_MAX_RECTANGLE_TEXTURE_SIZE		0x84F8
#endif
#ifndef GL_MESA_ycbcr_texture
#  define GL_YCBCR_MESA						0x8757
#  define GL_UNSIGNED_SHORT_8_8_MESA		0x85BA
#  define GL_UNSIGNED_SHORT_8_8_REV_MESA	0x85BB
#endif

//#define ARGL_DEBUG

struct _ARGL_CONTEXT_SETTINGS {
	int		texturePow2CapabilitiesChecked;
	GLuint	texturePow2;
	GLuint	listPow2;
	int		initedPow2;
	int		textureRectangleCapabilitiesChecked;
	GLuint	textureRectangle;
	GLuint	listRectangle;
	int		initedRectangle;
	int		initPlease;		// Set to TRUE to request re-init of texture etc.
	int		asInited_texmapScaleFactor;
	float	asInited_zoom;
	int		asInited_xsize;
	int		asInited_ysize;
	GLsizei	texturePow2SizeX;
	GLsizei	texturePow2SizeY;
	GLenum	texturePow2WrapMode;
	int		disableDistortionCompensation;
	GLenum	pixIntFormat;
	GLenum	pixFormat;
	GLenum	pixType;
	GLenum	pixSize;
	int	arglDrawMode;
	int	arglTexmapMode;
	int arglTexRectangle;
};
typedef struct _ARGL_CONTEXT_SETTINGS ARGL_CONTEXT_SETTINGS;

// ============================================================================
//	Public globals.
// ============================================================================

// It'd be nice if we could wrap these in accessor functions!
// These items relate to Apple's fast texture transfer support.
//#define ARGL_USE_TEXTURE_RANGE	// Commented out due to conflicts with GL_APPLE_ycbcr_422 extension.
#if defined(__APPLE__) && defined(APPLE_TEXTURE_FAST_TRANSFER)
int arglAppleClientStorage = TRUE; // TRUE | FALSE .
#  ifdef ARGL_USE_TEXTURE_RANGE
int arglAppleTextureRange = TRUE; // TRUE | FALSE .
GLuint arglAppleTextureRangeStorageHint = GL_STORAGE_SHARED_APPLE; // GL_STORAGE_PRIVATE_APPLE | GL_STORAGE_SHARED_APPLE | GL_STORAGE_CACHED_APPLE .
#  else
int arglAppleTextureRange = FALSE; // TRUE | FALSE .
GLuint arglAppleTextureRangeStorageHint = GL_STORAGE_PRIVATE_APPLE; // GL_STORAGE_PRIVATE_APPLE | GL_STORAGE_SHARED_APPLE | GL_STORAGE_CACHED_APPLE .
#  endif // ARGL_USE_TEXTURE_RANGE
#endif // __APPLE__ && APPLE_TEXTURE_FAST_TRANSFER

// ============================================================================
//	Private globals.
// ============================================================================


//#pragma mark -
// ============================================================================
//	Private functions.
// ============================================================================

//
//  Provide a gluCheckExtension() function, since some platforms don't have GLU version 1.3 or later.
//
GLboolean arglGluCheckExtension(const GLubyte* extName, const GLubyte *extString)
{
	const GLubyte *start;
	GLubyte *where, *terminator;

	// Extension names should not have spaces.
	where = (GLubyte *)strchr((const char *)extName, ' ');
	if (where || *extName == '\0')
		return GL_FALSE;
	// It takes a bit of care to be fool-proof about parsing the
	//	OpenGL extensions string. Don't be fooled by sub-strings, etc.
	start = extString;
	for (;;) {
		where = (GLubyte *) strstr((const char *)start, (const char *)extName);
		if (!where)
			break;
		terminator = where + strlen((const char *)extName);
		if (where == start || *(where - 1) == ' ')
			if (*terminator == ' ' || *terminator == '\0')
				return GL_TRUE;
		start = terminator;
	}
	return GL_FALSE;
}

//
//  Checks for the presence of an OpenGL capability by version or extension.
//  Reports whether the current OpenGL driver's OpenGL implementation version
//  meets or exceeds a minimum value passed in in minVersion (represented as a binary-coded
//  decimal i.e. version 1.0 is represented as 0x0100). If minVersion is zero, the
//  version test will always fail. Alternately, the test is satisfied if an OpenGL extension
//  identifier passed in as a character string
//  is non-NULL, and is found in the current driver's list of supported extensions.
//  Returns: TRUE If either of the tests passes, or FALSE if both fail.
//
static int arglGLCapabilityCheck(const unsigned short minVersion, const unsigned char *extension)
{
	const GLubyte * strRenderer;
	const GLubyte * strVersion;
	const GLubyte * strVendor;
	const GLubyte * strExtensions;
	short j, shiftVal;
	unsigned short version = 0; // binary-coded decimal gl version (ie. 1.4 is 0x0140).

	strRenderer = glGetString(GL_RENDERER);
	strVendor = glGetString(GL_VENDOR);
	strVersion = glGetString(GL_VERSION);
	j = 0;
	shiftVal = 8;
	// Construct BCD version.
	while (((strVersion[j] <= '9') && (strVersion[j] >= '0')) || (strVersion[j] == '.')) { // Get only basic version info (until first non-digit or non-.)
		if ((strVersion[j] <= '9') && (strVersion[j] >= '0')) {
			version += (strVersion[j] - '0') << shiftVal;
			shiftVal -= 4;
		}
		j++;
	}
	strExtensions = glGetString(GL_EXTENSIONS);

	if (0 < minVersion && version >= minVersion) return (TRUE);
	if (extension && arglGluCheckExtension(extension, strExtensions)) return (TRUE);
	return (FALSE);
}

static int arglDispImageTexPow2CapabilitiesCheck(const ARParam *cparam, ARGL_CONTEXT_SETTINGS_REF contextSettings)
{
	GLint format;
	GLint texture1SizeMax;

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &texture1SizeMax);
	if (cparam->xsize > texture1SizeMax || cparam->ysize > texture1SizeMax) {
		return (FALSE);
	}

	// Work out how big textures needs to be.
	contextSettings->texturePow2SizeX = contextSettings->texturePow2SizeY = 1;
	while (contextSettings->texturePow2SizeX < cparam->xsize) {
		contextSettings->texturePow2SizeX *= 2;
		if (contextSettings->texturePow2SizeX > texture1SizeMax) {
			return (FALSE); // Too big to handle.
		}
	}
	while (contextSettings->texturePow2SizeY < cparam->ysize) {
		contextSettings->texturePow2SizeY *= 2;
		if (contextSettings->texturePow2SizeY > texture1SizeMax) {
			return (FALSE); // Too big to handle.
		}
	}

	// Now check that the renderer can accomodate a texture of this size.
#ifdef APPLE_TEXTURE_FAST_TRANSFER
	// Can't use client storage or texture range.
#  ifdef ARGL_USE_TEXTURE_RANGE
	glTextureRangeAPPLE(GL_TEXTURE_2D, 0, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_STORAGE_HINT_APPLE, GL_STORAGE_PRIVATE_APPLE);
#  endif // ARGL_USE_TEXTURE_RANGE
	glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, FALSE);
#endif // APPLE_TEXTURE_FAST_TRANSFER
	glTexImage2D(GL_PROXY_TEXTURE_2D, 0, contextSettings->pixIntFormat, contextSettings->texturePow2SizeX, contextSettings->texturePow2SizeY, 0, contextSettings->pixFormat, contextSettings->pixType, NULL);
	glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &format);
	if (!format) {
		return (FALSE);
	}

	// Decide whether we can use GL_CLAMP_TO_EDGE.
	if (arglGLCapabilityCheck(0x0120, (unsigned char *)"GL_SGIS_texture_edge_clamp")) {
		contextSettings->texturePow2WrapMode = GL_CLAMP_TO_EDGE;
	} else {
		contextSettings->texturePow2WrapMode = GL_REPEAT;
	}

	return (TRUE);
}

static int arglCleanupTexPow2(ARGL_CONTEXT_SETTINGS_REF contextSettings)
{
	if (!contextSettings->initedPow2) return (FALSE);

	glDeleteTextures(1, &(contextSettings->texturePow2));
	glDeleteLists(contextSettings->listPow2, 1);
	contextSettings->texturePow2CapabilitiesChecked = FALSE;
	contextSettings->initedPow2 = FALSE;
	return (TRUE);
}

//
// Blit an image to the screen using OpenGL power-of-two texturing.
//
static void arglDispImageTexPow2(ARUint8 *image, const ARParam *cparam, const float zoom, ARGL_CONTEXT_SETTINGS_REF contextSettings, const int texmapScaleFactor)
{
    float	tsx, tsy, tex, tey;
    float	px, py, qx, qy;
    double	x1, x2, x3, x4, y1, y2, y3, y4;
    float	xx1, xx2, xx3, xx4, yy1, yy2, yy3, yy4;
    int		i, j;

    if(!contextSettings->initedPow2 || contextSettings->initPlease) {

		contextSettings->initPlease = FALSE;
		// Delete previous texture and list, unless this is our first time here.
		if (contextSettings->initedPow2) arglCleanupTexPow2(contextSettings);

		// If we have not done so, check texturing capabilities. If they have already been
		// checked, and we got to here, then obviously the capabilities were insufficient,
		// so just return without doing anything.
		if (!contextSettings->texturePow2CapabilitiesChecked) {
			contextSettings->texturePow2CapabilitiesChecked = TRUE;
			if (!arglDispImageTexPow2CapabilitiesCheck(cparam, contextSettings)) {
				printf("argl error: Your OpenGL implementation and/or hardware's texturing capabilities are insufficient.\n"); // Windows bug: when running multi-threaded, can't write to stderr!
				return;
			}
		} else {
			return;
		}

		// Set up the texture object.
		glGenTextures(1, &(contextSettings->texturePow2));
		glBindTexture(GL_TEXTURE_2D, contextSettings->texturePow2);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, contextSettings->texturePow2WrapMode);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, contextSettings->texturePow2WrapMode);

#ifdef APPLE_TEXTURE_FAST_TRANSFER
#  ifdef ARGL_USE_TEXTURE_RANGE
		// Can't use client storage or texture range.
		glTextureRangeAPPLE(GL_TEXTURE_2D, 0, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_STORAGE_HINT_APPLE, GL_STORAGE_PRIVATE_APPLE);
#  endif // ARGL_USE_TEXTURE_RANGE
		glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, FALSE);
#endif // APPLE_TEXTURE_FAST_TRANSFER

		// Request OpenGL allocate memory for a power-of-two texture of the appropriate size.
		if (texmapScaleFactor == 2) {
			// If texmapScaleFactor is 2, pretend lines in the source image are
			// twice as long as they are; glTexImage2D will read only the first
			// half of each line, effectively discarding every second line in the source image.
			glPixelStorei(GL_UNPACK_ROW_LENGTH, cparam->xsize*texmapScaleFactor);
		}
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, contextSettings->pixIntFormat, contextSettings->texturePow2SizeX, contextSettings->texturePow2SizeY/texmapScaleFactor, 0, contextSettings->pixFormat, contextSettings->pixType, NULL);
		if (texmapScaleFactor == 2) {
			glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		}

		// Set up the surface which we will texture upon.
		contextSettings->listPow2 = glGenLists(1);
		glNewList(contextSettings->listPow2, GL_COMPILE); // NB Texture not specified yet so don't execute.
		glEnable(GL_TEXTURE_2D);
		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);

		if (contextSettings->disableDistortionCompensation) {
			glBegin(GL_QUADS);
			glTexCoord2f(0.0f, (float)cparam->ysize/(float)contextSettings->texturePow2SizeY);
			glVertex2f(0.0f, 0.0f);
			glTexCoord2f((float)cparam->xsize/(float)contextSettings->texturePow2SizeX, (float)cparam->ysize/(float)contextSettings->texturePow2SizeY);
			glVertex2f((float)cparam->xsize * zoom, 0.0f);
			glTexCoord2f((float)cparam->xsize/(float)contextSettings->texturePow2SizeX, 0.0f);
			glVertex2f((float)cparam->xsize * zoom, (float)cparam->ysize * zoom);
			glTexCoord2f(0.0f, 0.0f);
			glVertex2f(0.0f, (float)cparam->ysize * zoom);
			glEnd();
		} else {
			qy = 0.0f;
			tey = 0.0f;
			for(j = 1; j <= 20; j++) {	// Do 20 rows.
				py = qy;
				tsy = tey;
				qy = cparam->ysize * j / 20.0f;
				tey = qy / contextSettings->texturePow2SizeY;

				qx = 0.0f;
				tex = 0.0f;
				for(i = 1; i <= 20; i++) {	// Draw 20 columns.
					px = qx;
					tsx = tex;
					qx = cparam->xsize * i / 20.0f;
					tex = qx / contextSettings->texturePow2SizeX;

					arParamObserv2Ideal(cparam->dist_factor, (double)px, (double)py, &x1, &y1);
					arParamObserv2Ideal(cparam->dist_factor, (double)qx, (double)py, &x2, &y2);
					arParamObserv2Ideal(cparam->dist_factor, (double)qx, (double)qy, &x3, &y3);
					arParamObserv2Ideal(cparam->dist_factor, (double)px, (double)qy, &x4, &y4);

					xx1 = (float)x1 * zoom;
					yy1 = (cparam->ysize - (float)y1) * zoom;
					xx2 = (float)x2 * zoom;
					yy2 = (cparam->ysize - (float)y2) * zoom;
					xx3 = (float)x3 * zoom;
					yy3 = (cparam->ysize - (float)y3) * zoom;
					xx4 = (float)x4 * zoom;
					yy4 = (cparam->ysize - (float)y4) * zoom;

					glBegin(GL_QUADS);
					glTexCoord2f(tsx, tsy); glVertex2f(xx1, yy1);
					glTexCoord2f(tex, tsy); glVertex2f(xx2, yy2);
					glTexCoord2f(tex, tey); glVertex2f(xx3, yy3);
					glTexCoord2f(tsx, tey); glVertex2f(xx4, yy4);
					glEnd();
				} // columns.
			} // rows.
		}
		glDisable(GL_TEXTURE_2D);
        glEndList();

		contextSettings->asInited_ysize = cparam->ysize;
		contextSettings->asInited_xsize = cparam->xsize;
		contextSettings->asInited_zoom = zoom;
        contextSettings->asInited_texmapScaleFactor = texmapScaleFactor;
		contextSettings->initedPow2 = TRUE;
	}

    glBindTexture(GL_TEXTURE_2D, contextSettings->texturePow2);
#ifdef APPLE_TEXTURE_FAST_TRANSFER
#  ifdef ARGL_USE_TEXTURE_RANGE
	// Can't use client storage or texture range.
	glTextureRangeAPPLE(GL_TEXTURE_2D, 0, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_STORAGE_HINT_APPLE, GL_STORAGE_PRIVATE_APPLE);
#endif // ARGL_USE_TEXTURE_RANGE
	glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, FALSE);
#endif // APPLE_TEXTURE_FAST_TRANSFER
	if (texmapScaleFactor == 2) {
		// If texmapScaleFactor is 2, pretend lines in the source image are
		// twice as long as they are; glTexImage2D will read only the first
		// half of each line, effectively discarding every second line in the source image.
		glPixelStorei(GL_UNPACK_ROW_LENGTH, cparam->xsize*texmapScaleFactor);
	}
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, cparam->xsize, cparam->ysize/texmapScaleFactor, contextSettings->pixFormat, contextSettings->pixType, image);
	glCallList(contextSettings->listPow2);
	if (texmapScaleFactor == 2) {
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	}
    glBindTexture(GL_TEXTURE_2D, 0);
}

static int arglDispImageTexRectangleCapabilitiesCheck(const ARParam *cparam, ARGL_CONTEXT_SETTINGS_REF contextSettings)
{
	GLint textureRectangleSizeMax;
	GLint format;

    if (!arglGLCapabilityCheck(0, (unsigned char *)"GL_NV_texture_rectangle")) {
		if (!arglGLCapabilityCheck(0, (unsigned char *)"GL_EXT_texture_rectangle")) { // Alternate name.
			return (FALSE);
		}
	}
    glGetIntegerv(GL_MAX_RECTANGLE_TEXTURE_SIZE, &textureRectangleSizeMax);
	if (cparam->xsize > textureRectangleSizeMax || cparam->ysize > textureRectangleSizeMax) {
		return (FALSE);
	}

	// Now check that the renderer can accomodate a texture of this size.
	glTexImage2D(GL_PROXY_TEXTURE_RECTANGLE, 0, contextSettings->pixIntFormat, cparam->xsize, cparam->ysize, 0, contextSettings->pixFormat, contextSettings->pixType, NULL);
	glGetTexLevelParameteriv(GL_PROXY_TEXTURE_RECTANGLE, 0, GL_TEXTURE_INTERNAL_FORMAT, &format);
	if (!format) {
		return (FALSE);
	}

	return (TRUE);
}

static int arglCleanupTexRectangle(ARGL_CONTEXT_SETTINGS_REF contextSettings)
{
	if (!contextSettings->initedRectangle) return (FALSE);

	glDeleteTextures(1, &(contextSettings->textureRectangle));
	glDeleteLists(contextSettings->listRectangle, 1);
	contextSettings->textureRectangleCapabilitiesChecked = FALSE;
	contextSettings->initedRectangle = FALSE;
	return (TRUE);
}

//
// Blit an image to the screen using OpenGL rectangle texturing.
//
static void arglDispImageTexRectangle(ARUint8 *image, const ARParam *cparam, const float zoom, ARGL_CONTEXT_SETTINGS_REF contextSettings, const int texmapScaleFactor)
{
	float	px, py, py_prev;
    double	x1, x2, y1, y2;
    float	xx1, xx2, yy1, yy2;
	int		i, j;

    if(!contextSettings->initedRectangle || contextSettings->initPlease) {

		contextSettings->initPlease = FALSE;
		// Delete previous texture and list, unless this is our first time here.
		if (contextSettings->initedRectangle) arglCleanupTexRectangle(contextSettings);

		// If we have not done so, check texturing capabilities. If they have already been
		// checked, and we got to here, then obviously the capabilities were insufficient,
		// so just return without doing anything.
		if (!contextSettings->textureRectangleCapabilitiesChecked) {
			contextSettings->textureRectangleCapabilitiesChecked = TRUE;
			if (!arglDispImageTexRectangleCapabilitiesCheck(cparam, contextSettings)) {
				printf("argl error: Your OpenGL implementation and/or hardware's texturing capabilities are insufficient to support rectangle textures.\n"); // Windows bug: when running multi-threaded, can't write to stderr!
				// Fall back to power of 2 texturing.
				contextSettings->arglTexRectangle = FALSE;
				arglDispImageTexPow2(image, cparam, zoom, contextSettings, texmapScaleFactor);
				return;
			}
		} else {
			return;
		}

		// Set up the rectangle texture object.
		glGenTextures(1, &(contextSettings->textureRectangle));
		glBindTexture(GL_TEXTURE_RECTANGLE, contextSettings->textureRectangle);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

#ifdef APPLE_TEXTURE_FAST_TRANSFER
#  ifdef ARGL_USE_TEXTURE_RANGE
		if (arglAppleTextureRange) {
			glTextureRangeAPPLE(GL_TEXTURE_RECTANGLE, cparam->xsize * cparam->ysize * contextSettings->pixSize, image);
			glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_STORAGE_HINT_APPLE, arglAppleTextureRangeStorageHint);
		} else {
			glTextureRangeAPPLE(GL_TEXTURE_RECTANGLE, 0, NULL);
			glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_STORAGE_HINT_APPLE, GL_STORAGE_PRIVATE_APPLE);
		}
#endif // ARGL_USE_TEXTURE_RANGE
		glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, arglAppleClientStorage);
#endif // APPLE_TEXTURE_FAST_TRANSFER

		// Specify the texture to OpenGL.
		if (texmapScaleFactor == 2) {
			// If texmapScaleFactor is 2, pretend lines in the source image are
			// twice as long as they are; glTexImage2D will read only the first
			// half of each line, effectively discarding every second line in the source image.
			glPixelStorei(GL_UNPACK_ROW_LENGTH, cparam->xsize*texmapScaleFactor);
		}
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Our image data is tightly packed.
		glTexImage2D(GL_TEXTURE_RECTANGLE, 0, contextSettings->pixIntFormat, cparam->xsize, cparam->ysize/texmapScaleFactor, 0, contextSettings->pixFormat, contextSettings->pixType, image);
		if (texmapScaleFactor == 2) {
			glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		}

		// Set up the surface which we will texture upon.
		contextSettings->listRectangle = glGenLists(1);
		glNewList(contextSettings->listRectangle, GL_COMPILE);
		glEnable(GL_TEXTURE_RECTANGLE);
		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);

		if (contextSettings->disableDistortionCompensation) {
			glBegin(GL_QUADS);
			glTexCoord2f(0.0f, (float)(cparam->ysize/texmapScaleFactor)); glVertex2f(0.0f, 0.0f);
			glTexCoord2f((float)(cparam->xsize), (float)(cparam->ysize/texmapScaleFactor)); glVertex2f(cparam->xsize * zoom, 0.0f);
			glTexCoord2f((float)(cparam->xsize), 0.0f); glVertex2f(cparam->xsize * zoom, cparam->ysize * zoom);
			glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, cparam->ysize * zoom);
			glEnd();
		} else {
			py_prev = 0.0f;
			for(j = 1; j <= 20; j++) {	// Do 20 rows.
				py = py_prev;
				py_prev = cparam->ysize * j / 20.0f;

				glBegin(GL_QUAD_STRIP);
				for(i = 0; i <= 20; i++) {	// Draw 21 pairs of vertices per row to make 20 columns.
					px = cparam->xsize * i / 20.0f;

					arParamObserv2Ideal(cparam->dist_factor, (double)px, (double)py, &x1, &y1);
					arParamObserv2Ideal(cparam->dist_factor, (double)px, (double)py_prev, &x2, &y2);

					xx1 = (float)x1 * zoom;
					yy1 = (cparam->ysize - (float)y1) * zoom;
					xx2 = (float)x2 * zoom;
					yy2 = (cparam->ysize - (float)y2) * zoom;

					glTexCoord2f(px, py/texmapScaleFactor); glVertex2f(xx1, yy1);
					glTexCoord2f(px, py_prev/texmapScaleFactor); glVertex2f(xx2, yy2);
				}
				glEnd();
			}
		}
		glDisable(GL_TEXTURE_RECTANGLE);
		glEndList();

		contextSettings->asInited_ysize = cparam->ysize;
		contextSettings->asInited_xsize = cparam->xsize;
		contextSettings->asInited_zoom = zoom;
        contextSettings->asInited_texmapScaleFactor = texmapScaleFactor;
        contextSettings->initedRectangle = TRUE;
    }

    glBindTexture(GL_TEXTURE_RECTANGLE, contextSettings->textureRectangle);
#ifdef APPLE_TEXTURE_FAST_TRANSFER
#  ifdef ARGL_USE_TEXTURE_RANGE
	if (arglAppleTextureRange) {
		glTextureRangeAPPLE(GL_TEXTURE_RECTANGLE, cparam->xsize * cparam->ysize * contextSettings->pixSize, image);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_STORAGE_HINT_APPLE, arglAppleTextureRangeStorageHint);
	} else {
		glTextureRangeAPPLE(GL_TEXTURE_RECTANGLE, 0, NULL);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_STORAGE_HINT_APPLE, GL_STORAGE_PRIVATE_APPLE);
	}
#endif // ARGL_USE_TEXTURE_RANGE
	glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, arglAppleClientStorage);
#endif // APPLE_TEXTURE_FAST_TRANSFER
	if (texmapScaleFactor == 2) {
		glPixelStorei(GL_UNPACK_ROW_LENGTH, cparam->xsize*texmapScaleFactor);
	}
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexSubImage2D(GL_TEXTURE_RECTANGLE, 0, 0, 0, cparam->xsize, cparam->ysize/texmapScaleFactor, contextSettings->pixFormat, contextSettings->pixType, image);
	glCallList(contextSettings->listRectangle);
	if (texmapScaleFactor == 2) {
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	}
    glBindTexture(GL_TEXTURE_RECTANGLE, 0);
}

//#pragma mark -
// ============================================================================
//	Public functions.
// ============================================================================

ARGL_CONTEXT_SETTINGS_REF arglSetupForCurrentContext(void)
{
	ARGL_CONTEXT_SETTINGS_REF contextSettings;

	contextSettings = (ARGL_CONTEXT_SETTINGS_REF)calloc(1, sizeof(ARGL_CONTEXT_SETTINGS));
	// Use default pixel format handed to us by <AR/config.h>.
	if (!arglPixelFormatSet(contextSettings, AR_DEFAULT_PIXEL_FORMAT)) {
		printf("Unknown default pixel format defined in config.h.\n"); // Windows bug: when running multi-threaded, can't write to stderr!
		return (NULL);
	}
	arglDrawModeSet(contextSettings, AR_DRAW_BY_TEXTURE_MAPPING);
	arglTexmapModeSet(contextSettings, AR_DRAW_TEXTURE_FULL_IMAGE);
	arglTexRectangleSet(contextSettings, TRUE);

	return (contextSettings);
}

void arglCleanup(ARGL_CONTEXT_SETTINGS_REF contextSettings)
{
	arglCleanupTexRectangle(contextSettings);
	arglCleanupTexPow2(contextSettings);
	free(contextSettings);
}

//
// Convert a camera parameter structure into an OpenGL projection matrix.
//
void arglCameraFrustum(ARParam *cparam, const double focalmin, const double focalmax, GLdouble m_projection[16])
{
	double   icpara[3][4];
    double   trans[3][4];
    double   p[3][3], q[4][4];
	int      width, height;
    int      i, j;

    width  = cparam->xsize;
    height = cparam->ysize;

    if (arParamDecompMat(cparam->mat, icpara, trans) < 0) {
        printf("arglCameraFrustum(): arParamDecompMat() indicated parameter error.\n"); // Windows bug: when running multi-threaded, can't write to stderr!
        return;
    }
	for (i = 0; i < 4; i++) {
        icpara[1][i] = (height - 1)*(icpara[2][i]) - icpara[1][i];
    }

    for(i = 0; i < 3; i++) {
        for(j = 0; j < 3; j++) {
            p[i][j] = icpara[i][j] / icpara[2][2];
        }
    }
    q[0][0] = (2.0 * p[0][0] / (width - 1));
    q[0][1] = (2.0 * p[0][1] / (width - 1));
    q[0][2] = ((2.0 * p[0][2] / (width - 1))  - 1.0);
    q[0][3] = 0.0;

    q[1][0] = 0.0;
    q[1][1] = (2.0 * p[1][1] / (height - 1));
    q[1][2] = ((2.0 * p[1][2] / (height - 1)) - 1.0);
    q[1][3] = 0.0;

    q[2][0] = 0.0;
    q[2][1] = 0.0;
    q[2][2] = (focalmax + focalmin)/(focalmax - focalmin);
    q[2][3] = -2.0 * focalmax * focalmin / (focalmax - focalmin);

    q[3][0] = 0.0;
    q[3][1] = 0.0;
    q[3][2] = 1.0;
    q[3][3] = 0.0;

    for (i = 0; i < 4; i++) { // Row.
		// First 3 columns of the current row.
        for (j = 0; j < 3; j++) { // Column.
            m_projection[i + j*4] = q[i][0] * trans[0][j] +
									q[i][1] * trans[1][j] +
									q[i][2] * trans[2][j];
        }
		// Fourth column of the current row.
        m_projection[i + 3*4] = q[i][0] * trans[0][3] +
								q[i][1] * trans[1][3] +
								q[i][2] * trans[2][3] +
								q[i][3];
    }
}

void arglCameraFrustumRH(ARParam *cparam, const double focalmin, const double focalmax, GLdouble m_projection[16])
{
	double   icpara[3][4];
    double   trans[3][4];
    double   p[3][3], q[4][4];
	int      width, height;
    int      i, j;

    width  = cparam->xsize;
    height = cparam->ysize;

    if (arParamDecompMat(cparam->mat, icpara, trans) < 0) {
        printf("arglCameraFrustum(): arParamDecompMat() indicated parameter error.\n"); // Windows bug: when running multi-threaded, can't write to stderr!
        return;
    }
	for (i = 0; i < 4; i++) {
        icpara[1][i] = (height - 1)*(icpara[2][i]) - icpara[1][i];
    }

    for(i = 0; i < 3; i++) {
        for(j = 0; j < 3; j++) {
            p[i][j] = icpara[i][j] / icpara[2][2];
        }
    }
    q[0][0] = (2.0 * p[0][0] / (width - 1));
    q[0][1] = (2.0 * p[0][1] / (width - 1));
    q[0][2] = -((2.0 * p[0][2] / (width - 1))  - 1.0);
    q[0][3] = 0.0;

    q[1][0] = 0.0;
    q[1][1] = -(2.0 * p[1][1] / (height - 1));
    q[1][2] = -((2.0 * p[1][2] / (height - 1)) - 1.0);
    q[1][3] = 0.0;

    q[2][0] = 0.0;
    q[2][1] = 0.0;
    q[2][2] = (focalmax + focalmin)/(focalmin - focalmax);
    q[2][3] = 2.0 * focalmax * focalmin / (focalmin - focalmax);

    q[3][0] = 0.0;
    q[3][1] = 0.0;
    q[3][2] = -1.0;
    q[3][3] = 0.0;

    for (i = 0; i < 4; i++) { // Row.
		// First 3 columns of the current row.
        for (j = 0; j < 3; j++) { // Column.
            m_projection[i + j*4] = q[i][0] * trans[0][j] +
			q[i][1] * trans[1][j] +
			q[i][2] * trans[2][j];
        }
		// Fourth column of the current row.
        m_projection[i + 3*4] = q[i][0] * trans[0][3] +
								q[i][1] * trans[1][3] +
								q[i][2] * trans[2][3] +
								q[i][3];
    }
}

void arglCameraView(const double para[3][4], GLdouble m_modelview[16], const double scale)
{
	m_modelview[0 + 0*4] = para[0][0]; // R1C1
	m_modelview[0 + 1*4] = para[0][1]; // R1C2
	m_modelview[0 + 2*4] = para[0][2];
	m_modelview[0 + 3*4] = para[0][3];
	m_modelview[1 + 0*4] = para[1][0]; // R2
	m_modelview[1 + 1*4] = para[1][1];
	m_modelview[1 + 2*4] = para[1][2];
	m_modelview[1 + 3*4] = para[1][3];
	m_modelview[2 + 0*4] = para[2][0]; // R3
	m_modelview[2 + 1*4] = para[2][1];
	m_modelview[2 + 2*4] = para[2][2];
	m_modelview[2 + 3*4] = para[2][3];
	m_modelview[3 + 0*4] = 0.0;
	m_modelview[3 + 1*4] = 0.0;
	m_modelview[3 + 2*4] = 0.0;
	m_modelview[3 + 3*4] = 1.0;
	if (scale != 0.0) {
		m_modelview[12] *= scale;
		m_modelview[13] *= scale;
		m_modelview[14] *= scale;
	}
}

void arglCameraViewRH(const double para[3][4], GLdouble m_modelview[16], const double scale)
{
	m_modelview[0 + 0*4] = para[0][0]; // R1C1
	m_modelview[0 + 1*4] = para[0][1]; // R1C2
	m_modelview[0 + 2*4] = para[0][2];
	m_modelview[0 + 3*4] = para[0][3];
	m_modelview[1 + 0*4] = -para[1][0]; // R2
	m_modelview[1 + 1*4] = -para[1][1];
	m_modelview[1 + 2*4] = -para[1][2];
	m_modelview[1 + 3*4] = -para[1][3];
	m_modelview[2 + 0*4] = -para[2][0]; // R3
	m_modelview[2 + 1*4] = -para[2][1];
	m_modelview[2 + 2*4] = -para[2][2];
	m_modelview[2 + 3*4] = -para[2][3];
	m_modelview[3 + 0*4] = 0.0;
	m_modelview[3 + 1*4] = 0.0;
	m_modelview[3 + 2*4] = 0.0;
	m_modelview[3 + 3*4] = 1.0;
	if (scale != 0.0) {
		m_modelview[12] *= scale;
		m_modelview[13] *= scale;
		m_modelview[14] *= scale;
	}
}

void arglDispImage(ARUint8 *image, const ARParam *cparam, const double zoom, ARGL_CONTEXT_SETTINGS_REF contextSettings)
{
	GLint texEnvModeSave;
	GLboolean lightingSave;
	GLboolean depthTestSave;
#ifdef ARGL_DEBUG
	GLenum			err;
	const GLubyte	*errs;
#endif // ARGL_DEBUG

	if (!image) return;

	// Prepare an orthographic projection, set camera position for 2D drawing, and save GL state.
	glGetTexEnviv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, &texEnvModeSave); // Save GL texture environment mode.
	if (texEnvModeSave != GL_REPLACE) glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	lightingSave = glIsEnabled(GL_LIGHTING);			// Save enabled state of lighting.
	if (lightingSave == GL_TRUE) glDisable(GL_LIGHTING);
	depthTestSave = glIsEnabled(GL_DEPTH_TEST);		// Save enabled state of depth test.
	if (depthTestSave == GL_TRUE) glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, cparam->xsize, 0, cparam->ysize);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	if (arDebug) { // Globals from ar.h: arDebug, arImage, arImageProcMode.
		if (arImage) {
			if (arImageProcMode == AR_IMAGE_PROC_IN_HALF) {
				ARParam cparamScaled = *cparam;
				cparamScaled.xsize /= 2;
				cparamScaled.ysize /= 2;
				arglDispImageStateful(arImage, &cparamScaled, zoom * 2.0, contextSettings);
			} else {
				arglDispImageStateful(arImage, cparam, zoom, contextSettings);
			}
		}
	} else {
		arglDispImageStateful(image, cparam, zoom, contextSettings);
	}

	// Restore previous projection, camera position, and GL state.
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	if (depthTestSave == GL_TRUE) glEnable(GL_DEPTH_TEST);			// Restore enabled state of depth test.
	if (lightingSave == GL_TRUE) glEnable(GL_LIGHTING);			// Restore enabled state of lighting.
	if (texEnvModeSave != GL_REPLACE) glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, texEnvModeSave); // Restore GL texture environment mode.

#ifdef ARGL_DEBUG
	// Report any errors we generated.
	while ((err = glGetError()) != GL_NO_ERROR) {
		errs = gluErrorString(err);	// fetch error code
		fprintf(stderr, "GL error: %s (%i)\n", errs, (int)err);	// write err code and number to stderr
	}
#endif // ARGL_DEBUG

}

void arglDispImageStateful(ARUint8 *image, const ARParam *cparam, const double zoom, ARGL_CONTEXT_SETTINGS_REF contextSettings)
{
	float zoomf;
	int texmapScaleFactor, params[4];

	zoomf = (float)zoom;
	texmapScaleFactor = contextSettings->arglTexmapMode + 1;
	if (contextSettings->arglDrawMode == AR_DRAW_BY_GL_DRAW_PIXELS) {
		glDisable(GL_TEXTURE_2D);
		glGetIntegerv(GL_VIEWPORT, (GLint *)params);
		glPixelZoom(zoomf * ((float)(params[2]) / (float)(cparam->xsize)),
				   -zoomf * ((float)(params[3]) / (float)(cparam->ysize)));
		glRasterPos2f(0.0f, (float)(cparam->ysize));
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glDrawPixels(cparam->xsize, cparam->ysize, contextSettings->pixFormat, contextSettings->pixType, image);
	} else {
		// Check whether any settings in globals/parameters have changed.
		// N.B. We don't check cparam->dist_factor[], but this is unlikely to change!
		if ((texmapScaleFactor != contextSettings->asInited_texmapScaleFactor) ||
			(zoomf != contextSettings->asInited_zoom) ||
			(cparam->xsize != contextSettings->asInited_xsize) ||
			(cparam->ysize != contextSettings->asInited_ysize)) {
			contextSettings->initPlease = TRUE;
		}

		if (contextSettings->arglTexRectangle) {
			arglDispImageTexRectangle(image, cparam, zoomf, contextSettings, texmapScaleFactor);
		} else {
			arglDispImageTexPow2(image, cparam, zoomf, contextSettings, texmapScaleFactor);
		}
	}
}

int arglDistortionCompensationSet(ARGL_CONTEXT_SETTINGS_REF contextSettings, int enable)
{
	if (!contextSettings) return (FALSE);
	contextSettings->disableDistortionCompensation = !enable;
	contextSettings->initPlease = TRUE;
	return (TRUE);
}

int arglDistortionCompensationGet(ARGL_CONTEXT_SETTINGS_REF contextSettings, int *enable)
{
	if (!contextSettings) return (FALSE);
	*enable = !contextSettings->disableDistortionCompensation;
	return (TRUE);
}

int arglPixelFormatSet(ARGL_CONTEXT_SETTINGS_REF contextSettings, AR_PIXEL_FORMAT format)
{
	if (!contextSettings) return (FALSE);
	switch (format) {
		case AR_PIXEL_FORMAT_RGBA:
			contextSettings->pixIntFormat = GL_RGBA;
			contextSettings->pixFormat = GL_RGBA;
			contextSettings->pixType = GL_UNSIGNED_BYTE;
			contextSettings->pixSize = 4;
			break;
		case AR_PIXEL_FORMAT_ABGR:	// SGI.
			if (arglGLCapabilityCheck(0, (unsigned char *)"GL_EXT_abgr")) {
				contextSettings->pixIntFormat = GL_RGBA;
				contextSettings->pixFormat = GL_ABGR_EXT;
				contextSettings->pixType = GL_UNSIGNED_BYTE;
				contextSettings->pixSize = 4;
			} else {
				return (FALSE);
			}
			break;
		case AR_PIXEL_FORMAT_BGRA:	// Windows.
			if (arglGLCapabilityCheck(0x0120, (unsigned char *)"GL_EXT_bgra")) {
				contextSettings->pixIntFormat = GL_RGBA;
				contextSettings->pixFormat = GL_BGRA;
				contextSettings->pixType = GL_UNSIGNED_BYTE;
				contextSettings->pixSize = 4;
			} else {
				return (FALSE);
			}
			break;
		case AR_PIXEL_FORMAT_ARGB:	// Mac.
			if (arglGLCapabilityCheck(0x0120, (unsigned char *)"GL_EXT_bgra")
				&& arglGLCapabilityCheck(0x0120, (unsigned char *)"GL_APPLE_packed_pixels")) {
				contextSettings->pixIntFormat = GL_RGBA;
				contextSettings->pixFormat = GL_BGRA;
#ifdef AR_BIG_ENDIAN
				contextSettings->pixType = GL_UNSIGNED_INT_8_8_8_8_REV;
#else
				contextSettings->pixType = GL_UNSIGNED_INT_8_8_8_8;
#endif
				contextSettings->pixSize = 4;
			} else {
				return (FALSE);
			}
			break;
		case AR_PIXEL_FORMAT_RGB:
			contextSettings->pixIntFormat = GL_RGB;
			contextSettings->pixFormat = GL_RGB;
			contextSettings->pixType = GL_UNSIGNED_BYTE;
			contextSettings->pixSize = 3;
			break;
		case AR_PIXEL_FORMAT_BGR:
			if (arglGLCapabilityCheck(0x0120, (unsigned char *)"GL_EXT_bgra")) {
				contextSettings->pixIntFormat = GL_RGB;
				contextSettings->pixFormat = GL_BGR;
				contextSettings->pixType = GL_UNSIGNED_BYTE;
				contextSettings->pixSize = 3;
			} else {
				return (FALSE);
			}
			break;
		case AR_PIXEL_FORMAT_MONO:
			contextSettings->pixIntFormat = GL_LUMINANCE;
			contextSettings->pixFormat = GL_LUMINANCE;
			contextSettings->pixType = GL_UNSIGNED_BYTE;
			contextSettings->pixSize = 1;
			break;
		case AR_PIXEL_FORMAT_2vuy:
			if (arglGLCapabilityCheck(0, (unsigned char *)"GL_APPLE_ycbcr_422")) {
				contextSettings->pixIntFormat = GL_RGB;
				contextSettings->pixFormat = GL_YCBCR_422_APPLE;
#ifdef AR_BIG_ENDIAN
				contextSettings->pixType = GL_UNSIGNED_SHORT_8_8_REV_APPLE;
#else
				contextSettings->pixType = GL_UNSIGNED_SHORT_8_8_APPLE;
#endif
			} else if (arglGLCapabilityCheck(0, (unsigned char *)"GL_MESA_ycbcr_texture")) {
				contextSettings->pixIntFormat = GL_YCBCR_MESA;
				contextSettings->pixFormat = GL_YCBCR_MESA;
#ifdef AR_BIG_ENDIAN
				contextSettings->pixType = GL_UNSIGNED_SHORT_8_8_REV_MESA;
#else
				contextSettings->pixType = GL_UNSIGNED_SHORT_8_8_MESA;
#endif
			} else {
				return (FALSE);
			}
			contextSettings->pixSize = 2;
			break;
		case AR_PIXEL_FORMAT_yuvs:
			if (arglGLCapabilityCheck(0, (unsigned char *)"GL_APPLE_ycbcr_422")) {
				contextSettings->pixIntFormat = GL_RGB;
				contextSettings->pixFormat = GL_YCBCR_422_APPLE;
#ifdef AR_BIG_ENDIAN
				contextSettings->pixType = GL_UNSIGNED_SHORT_8_8_APPLE;
#else
				contextSettings->pixType = GL_UNSIGNED_SHORT_8_8_REV_APPLE;
#endif
			} else if (arglGLCapabilityCheck(0, (unsigned char *)"GL_MESA_ycbcr_texture")) {
				contextSettings->pixIntFormat = GL_YCBCR_MESA;
				contextSettings->pixFormat = GL_YCBCR_MESA;
#ifdef AR_BIG_ENDIAN
				contextSettings->pixType = GL_UNSIGNED_SHORT_8_8_MESA;
#else
				contextSettings->pixType = GL_UNSIGNED_SHORT_8_8_REV_MESA;
#endif
			} else {
				return (FALSE);
			}
			contextSettings->pixSize = 2;
			break;
		default:
			return (FALSE);
			break;
	}
	contextSettings->initPlease = TRUE;
	return (TRUE);
}

int arglPixelFormatGet(ARGL_CONTEXT_SETTINGS_REF contextSettings, AR_PIXEL_FORMAT *format, int *size)
{
	if (!contextSettings) return (FALSE);
	switch (contextSettings->pixFormat) {
		case GL_RGBA:
			*format = AR_PIXEL_FORMAT_RGBA;
			*size = 4;
			break;
		case GL_ABGR_EXT:
			*format = AR_PIXEL_FORMAT_ABGR;
			*size = 4;
			break;
		case GL_BGRA:
			if (contextSettings->pixType == GL_UNSIGNED_BYTE) *format = AR_PIXEL_FORMAT_BGRA;
#ifdef AR_BIG_ENDIAN
			else if (contextSettings->pixType == GL_UNSIGNED_INT_8_8_8_8_REV) *format = AR_PIXEL_FORMAT_ARGB;
#else
			else if (contextSettings->pixType == GL_UNSIGNED_INT_8_8_8_8) *format = AR_PIXEL_FORMAT_ARGB;
#endif
			else  return (FALSE);
			*size = 4;
			break;
		case GL_RGB:
			*format = AR_PIXEL_FORMAT_RGB;
			*size = 3;
			break;
		case GL_BGR:
			*format = AR_PIXEL_FORMAT_BGR;
			*size = 3;
			break;
		case GL_YCBCR_422_APPLE:
		case GL_YCBCR_MESA:
#ifdef AR_BIG_ENDIAN
			if (contextSettings->pixType == GL_UNSIGNED_SHORT_8_8_REV_APPLE) *format = AR_PIXEL_FORMAT_2vuy; // N.B.: GL_UNSIGNED_SHORT_8_8_REV_APPLE = GL_UNSIGNED_SHORT_8_8_REV_MESA
			else if (contextSettings->pixType == GL_UNSIGNED_SHORT_8_8_APPLE) *format = AR_PIXEL_FORMAT_yuvs; // GL_UNSIGNED_SHORT_8_8_APPLE = GL_UNSIGNED_SHORT_8_8_MESA
#else
			if (contextSettings->pixType == GL_UNSIGNED_SHORT_8_8_APPLE) *format = AR_PIXEL_FORMAT_2vuy;
			else if (contextSettings->pixType == GL_UNSIGNED_SHORT_8_8_REV_APPLE) *format = AR_PIXEL_FORMAT_yuvs;
#endif
			else return (FALSE);
			*size = 2;
			break;
		case GL_LUMINANCE:
			*format = AR_PIXEL_FORMAT_MONO;
			*size = 1;
			break;
		default:
			return (FALSE);
			break;
	}
	return (TRUE);
}

void arglDrawModeSet(ARGL_CONTEXT_SETTINGS_REF contextSettings, const int mode)
{
	if (!contextSettings || mode < 0 || mode > 1) return; // Sanity check.
	contextSettings->arglDrawMode = mode;
}

int arglDrawModeGet(ARGL_CONTEXT_SETTINGS_REF contextSettings)
{
	if (!contextSettings) return (-1); // Sanity check.
	return (contextSettings->arglDrawMode);
}

void arglTexmapModeSet(ARGL_CONTEXT_SETTINGS_REF contextSettings, const int mode)
{
	if (!contextSettings || mode < 0 || mode > 1) return; // Sanity check.
	contextSettings->arglTexmapMode = mode;
}

int arglTexmapModeGet(ARGL_CONTEXT_SETTINGS_REF contextSettings)
{
	if (!contextSettings) return (-1); // Sanity check.
	return (contextSettings->arglTexmapMode);
}

void arglTexRectangleSet(ARGL_CONTEXT_SETTINGS_REF contextSettings, const int state)
{
	if (!contextSettings) return; // Sanity check.
	contextSettings->arglTexRectangle = state;
}

int arglTexRectangleGet(ARGL_CONTEXT_SETTINGS_REF contextSettings)
{
	if (!contextSettings) return (-1); // Sanity check.
	return (contextSettings->arglTexRectangle);
}

///gsub_lite.c

///gsubUtil.c
static double   calib_pos[CALIB_POS1_NUM][2] = { { 160, 120 },
                                                 { 480, 120 },
                                                 { 320, 240 },
                                                 { 160, 360 },
                                                 { 480, 360 } };
static double   calib_pos2d[CALIB_POS1_NUM][CALIB_POS2_NUM][2];
static double   calib_pos3d[CALIB_POS1_NUM][CALIB_POS2_NUM][3];
static int      co1;
static int      co2;
static int      left_right;
static double   target_trans[3][4];
static int      target_id;
static int      target_visible;
static double   target_center[2] = { 0.0, 0.0 };
static double   target_width     =  80.0;

static ARParam  hmd_param[2];
static int      thresh;
static int      arFittingModeBak;

static int      hmdMode;
//static int      gMiniXnum,  gMiniYnum;
//static void     (*gMouseFunc)(int button, int state, int x, int y);
//static void     (*gKeyFunc)(unsigned char key, int x, int y);
//static void     (*gMainFunc)(void);
static void     (*gCalibPostFunc)(ARParam *lpara, ARParam *rpara);

static void argCalibMouseFunc(int button, int state, int x, int y);
static void argCalibMainFunc(void);
static int  argDrawAttention(double pos[2], int color);

void argUtilCalibHMD( int targetId, int thresh2,
                      void (*postFunc)(ARParam *lpara, ARParam *rpara) )
{
    argInqSetting( &hmdMode, &gMiniXnum, &gMiniYnum,
                   &gMouseFunc, &gKeyFunc, &gMainFunc );

    if( hmdMode == 0 ) return;

    target_id = targetId;
    thresh = thresh2;
    gCalibPostFunc = postFunc;
    arFittingModeBak = arFittingMode;

    arFittingMode = AR_FITTING_TO_IDEAL;
    co1 = 0;
    co2 = 0;
    left_right = 0;
    target_visible = 0;

    glutKeyboardFunc( NULL );
    glutMouseFunc( argCalibMouseFunc );
    glutIdleFunc( argCalibMainFunc );
    glutDisplayFunc( argCalibMainFunc );
}

static void argCalibMouseFunc(int button, int state, int x, int y)
{
    if( button == GLUT_LEFT_BUTTON  && state == GLUT_DOWN ) {
        if( target_visible ) {
            calib_pos3d[co1][co2][0] = target_trans[0][3];
            calib_pos3d[co1][co2][1] = target_trans[1][3];
            calib_pos3d[co1][co2][2] = target_trans[2][3];
            calib_pos2d[co1][co2][0] = calib_pos[co1][0];
            calib_pos2d[co1][co2][1] = calib_pos[co1][1];
            co2++;
            if( co2 == CALIB_POS2_NUM ) {
                co1++;
                co2 = 0;
            }

            if( co1 == CALIB_POS1_NUM ) {
                hmd_param[left_right].xsize = AR_HMD_XSIZE;
                hmd_param[left_right].ysize = AR_HMD_YSIZE;
                hmd_param[left_right].dist_factor[0] = AR_HMD_XSIZE / 2.0;
                hmd_param[left_right].dist_factor[1] = AR_HMD_YSIZE / 2.0;
                hmd_param[left_right].dist_factor[2] = 0.0;
                hmd_param[left_right].dist_factor[3] = 1.0;
                if( arParamGet( (double (*)[3])calib_pos3d, (double (*)[2])calib_pos2d,
                                 CALIB_POS1_NUM*CALIB_POS2_NUM, hmd_param[left_right].mat) < 0 ) {
                    (*gCalibPostFunc)( NULL, NULL );
                    arFittingMode = arFittingModeBak;
                    glutKeyboardFunc( gKeyFunc );
                    glutMouseFunc( gMouseFunc );
                    glutIdleFunc( gMainFunc );
                    glutDisplayFunc( gMainFunc );
                    return;
                }

                co1 = 0;
                co2 = 0;
                left_right++;
                if( left_right == 2 ) {
                    argLoadHMDparam( &hmd_param[0], &hmd_param[1] );
                    arFittingMode = arFittingModeBak;

                    if( gCalibPostFunc != NULL ) {
                        (*gCalibPostFunc)( &hmd_param[0], &hmd_param[1] );
                    }
                    glutKeyboardFunc( gKeyFunc );
                    glutMouseFunc( gMouseFunc );
                    glutIdleFunc( gMainFunc );
                    glutDisplayFunc( gMainFunc );
                    return;
                }
            }
        }
    }

    if( button == GLUT_RIGHT_BUTTON  && state == GLUT_DOWN ) {
        (*gCalibPostFunc)( NULL, NULL );
        arFittingMode = arFittingMode;
        glutKeyboardFunc( gKeyFunc );
        glutMouseFunc( gMouseFunc );
        glutIdleFunc( gMainFunc );
        glutDisplayFunc( gMainFunc );
        return;
    }
}

static void argCalibMainFunc(void)
{
    ARUint8         *dataPtr;
    ARMarkerInfo    *marker_info;
    int             marker_num;
    int             i, j;
    double          cfmax;
    double          err;

    /* grab a vide frame */
    if( (dataPtr = (ARUint8 *)arVideoGetImage()) == NULL ) {
        arUtilSleep(2);
        return;
    }
    target_visible = 0;

    /* detect the markers in the video frame */
    if( arDetectMarker(dataPtr, thresh,
                       &marker_info, &marker_num) < 0 ) {
        (*gCalibPostFunc)( NULL, NULL );
        arFittingMode = arFittingModeBak;
        glutKeyboardFunc( gKeyFunc );
        glutMouseFunc( gMouseFunc );
        glutIdleFunc( gMainFunc );
        glutDisplayFunc( gMainFunc );
        return;
    }
    arVideoCapNext();

    glClearColor( 0.0, 0.0, 0.0, 0.0 );
    glClear(GL_COLOR_BUFFER_BIT);

    /* if the debug mode is on draw squares
       around the detected squares in the video image */
    if( arDebug && gMiniXnum >= 2 && gMiniYnum >= 1 ) {
        argDispImage( dataPtr, 1, 1 );
        if( arImageProcMode == AR_IMAGE_PROC_IN_HALF )
            argDispHalfImage( arImage, 2, 1 );
        else
            argDispImage( arImage, 2, 1);

        glColor3f( 1.0, 0.0, 0.0 );
        glLineWidth( 3.0 );
        for( i = 0; i < marker_num; i++ ) {
            if( marker_info[i].id < 0 ) continue;
            argDrawSquare( marker_info[i].vertex, 2, 1 );
        }
        glLineWidth( 1.0 );
    }

    if( left_right == 0 ) argDraw2dLeft();
     else                 argDraw2dRight();
    glLineWidth( 3.0 );
    glColor3f( 1.0, 1.0, 1.0 );
    argLineSegHMD( 0, calib_pos[co1][1], AR_HMD_XSIZE, calib_pos[co1][1] );
    argLineSegHMD( calib_pos[co1][0], 0, calib_pos[co1][0], AR_HMD_YSIZE );
    glLineWidth( 1.0 );
    argDrawMode2D();

    cfmax = 0.0;
    j = -1;
    for( i = 0; i < marker_num; i++ ) {
        if( marker_info[i].id != target_id ) continue;

        if( marker_info[i].cf > cfmax ) {
            cfmax = marker_info[i].cf;
            j = i;
        }
    }
    if( j < 0 ) {
        argSwapBuffers();
        return;
    }
    err = arGetTransMat(&marker_info[j], target_center, target_width, target_trans);
    if( err >= 0.0 ) {
        target_visible = 1;

        if( left_right == 0 ) argDraw2dLeft();
         else                 argDraw2dRight();
        argDrawAttention( calib_pos[co1], co2 );
        argDrawMode2D();

        if( arDebug && gMiniXnum >= 2 && gMiniYnum >= 1 ) {
            glColor3f( 0.0, 1.0, 0.0 );
            glLineWidth( 3.0 );
            argDrawSquare( marker_info[j].vertex, 1, 1 );
            glLineWidth( 1.0 );
        }
    }

    argSwapBuffers();
}

static int  argDrawAttention( double pos[2], int color )
{
    switch( color%7 ) {
      case 0: glColor3f( 1.0, 0.0, 0.0 ); break;
      case 1: glColor3f( 0.0, 1.0, 0.0 ); break;
      case 2: glColor3f( 0.0, 0.0, 1.0 ); break;
      case 3: glColor3f( 1.0, 1.0, 0.0 ); break;
      case 4: glColor3f( 1.0, 0.0, 1.0 ); break;
      case 5: glColor3f( 0.0, 1.0, 1.0 ); break;
      case 6: glColor3f( 1.0, 1.0, 1.0 ); break;
    }

    glLineWidth( 5.0 );
    argLineSegHMD( pos[0]-20, pos[1]-20, pos[0]+20, pos[1]-20 );
    argLineSegHMD( pos[0]-20, pos[1]+20, pos[0]+20, pos[1]+20 );
    argLineSegHMD( pos[0]-20, pos[1]-20, pos[0]-20, pos[1]+20 );
    argLineSegHMD( pos[0]+20, pos[1]-20, pos[0]+20, pos[1]+20 );
    glLineWidth( 1.0 );

    return(0);
}

///gsubUtil.c
