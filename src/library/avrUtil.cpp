/*
arUtil
arMultiReadConfigFile
mAlloc
mAllocDup
mAllocInv
mAllocMul
mAllocTrans
mAllocUnit
mDisp
mDup
mFree
vAlloc
vDisp
vFree
arGetCode
arMultiActive
*/

#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#ifdef _WIN32
#include <sys/timeb.h>
#include <windows.h>
#else
#include <sys/time.h>
#endif

#ifndef __APPLE__
#include <malloc.h>
#else
#include <stdlib.h>
#endif

///
#include <avrMath.h>
#include <avrUtil.h>
///

#define WORK_SIZE   1024*32

#define  AR_TEMPLATE_MATCHING_COLOR   0
#define  AR_TEMPLATE_MATCHING_BW      1
#define  AR_MATCHING_WITHOUT_PCA      0
#define  AR_MATCHING_WITH_PCA         1
#define  DEFAULT_TEMPLATE_MATCHING_MODE     AR_TEMPLATE_MATCHING_COLOR
#define  DEFAULT_MATCHING_PCA_MODE          AR_MATCHING_WITHOUT_PCA

#define   AR_PATT_NUM_MAX      50
#define   AR_PATT_SAMPLE_NUM   64

///Labeling
#define HARDCODED_BUFFER_WIDTH  1024
#define HARDCODED_BUFFER_HEIGHT 1024
#define USE_OPTIMIZATIONS

#define   DEBUG        0
#define   EVEC_MAX     10

int        arDebug                 = 0;
int        arFittingMode           = DEFAULT_FITTING_MODE;
int        arImageProcMode         = DEFAULT_IMAGE_PROC_MODE;
int        arImXsize, arImYsize;
int        arTemplateMatchingMode  = DEFAULT_TEMPLATE_MATCHING_MODE;
int        arMatchingPCAMode       = DEFAULT_MATCHING_PCA_MODE;

ARUint8*          arImage          = NULL;
static ARUint8*   arImageL         = NULL;
static ARUint8*   arImageR         = NULL;

///Labeling

static ARInt16      l_imageL[HARDCODED_BUFFER_WIDTH*HARDCODED_BUFFER_HEIGHT];
static ARInt16      l_imageR[HARDCODED_BUFFER_WIDTH*HARDCODED_BUFFER_HEIGHT];

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

/// Globals (Patt)
static int    pattern_num = -1;
static int    patf[AR_PATT_NUM_MAX] = { 0 };
static int    pat[AR_PATT_NUM_MAX][4][AR_PATT_SIZE_Y*AR_PATT_SIZE_X*3];
static double patpow[AR_PATT_NUM_MAX][4];
static int    patBW[AR_PATT_NUM_MAX][4][AR_PATT_SIZE_Y*AR_PATT_SIZE_X*3];
static double patpowBW[AR_PATT_NUM_MAX][4];

static double evec[EVEC_MAX][AR_PATT_SIZE_Y*AR_PATT_SIZE_X*3];
static double epat[AR_PATT_NUM_MAX][4][EVEC_MAX];
static int    evec_dim;
static int    evecf = 0;
static int    evecBWf = 0;
/// Static Functions (Patt)
static void   get_cpara( double world[4][2], double vertex[4][2], double para[3][3] );
static int    pattern_match( ARUint8 *data, int *code, int *dir, double *cf );
static void   put_zero( ARUint8 *p, int size );
static void   gen_evec(void);

static char *get_buff( char *buf, int n, FILE *fp );

int arUtilMatMul( double s1[3][4], double s2[3][4], double d[3][4] )
{
    int     i, j;

    for( j = 0; j < 3; j++ ) {
        for( i = 0; i < 4; i++) {
            d[j][i] = s1[j][0] * s2[0][i]
                    + s1[j][1] * s2[1][i]
                    + s1[j][2] * s2[2][i];
        }
        d[j][3] += s1[j][3];
    }

    return 0;
}

int arUtilMatInv( double s[3][4], double d[3][4] )
{
    ARMat       *mat;
    int         i, j;

    mat = arMatrixAlloc( 4, 4 );
    for( j = 0; j < 3; j++ ) {
        for( i = 0; i < 4; i++ ) {
            mat->m[j*4+i] = s[j][i];
        }
    }
    mat->m[3*4+0] = 0; mat->m[3*4+1] = 0;
    mat->m[3*4+2] = 0; mat->m[3*4+3] = 1;
    arMatrixSelfInv( mat );
    for( j = 0; j < 3; j++ ) {
        for( i = 0; i < 4; i++ ) {
            d[j][i] = mat->m[j*4+i];
        }
    }
    arMatrixFree( mat );

    return 0;
}

int arUtilMat2QuatPos( double m[3][4], double q[4], double p[3] )
{
    double   w;

    w = m[0][0] + m[1][1] + m[2][2] + 1;
    if( w < 0.0 ) return -1;

    w = sqrt( w );
    q[0] = (m[1][2] - m[2][1]) / (w*2.0);
    q[1] = (m[2][0] - m[0][2]) / (w*2.0);
    q[2] = (m[0][1] - m[1][0]) / (w*2.0);
    q[3] = w / 2.0;

    p[0] = m[0][3];
    p[1] = m[1][3];
    p[2] = m[2][3];

    return 0;
}

int arUtilQuatPos2Mat( double q[4], double p[3], double m[3][4] )
{
    double    x2, y2, z2;
    double    xx, xy, xz;
    double    yy, yz, zz;
    double    wx, wy, wz;

    x2 = q[0] * 2.0;
    y2 = q[1] * 2.0;
    z2 = q[2] * 2.0;
    xx = q[0] * x2;
    xy = q[0] * y2;
    xz = q[0] * z2;
    yy = q[1] * y2;
    yz = q[1] * z2;
    zz = q[2] * z2;
    wx = q[3] * x2;
    wy = q[3] * y2;
    wz = q[3] * z2;

    m[0][0] = 1.0 - (yy + zz);
    m[1][1] = 1.0 - (xx + zz);
    m[2][2] = 1.0 - (xx + yy);
    m[1][0] = xy - wz;
    m[0][1] = xy + wz;
    m[2][0] = xz + wy;
    m[0][2] = xz - wy;
    m[2][1] = yz - wx;
    m[1][2] = yz + wx;

    m[0][3] = p[0];
    m[1][3] = p[1];
    m[2][3] = p[2];

    return 0;
}

static int      ss, sms;

double arUtilTimer(void)
{
#ifdef _WIN32
    struct _timeb sys_time;
    double             tt;
    int                s1, s2;

    _ftime(&sys_time);
    s1 = sys_time.time  - ss;
    s2 = sys_time.millitm - sms;
#else
    struct timeval     time;
    double             tt;
    int                s1, s2;

#if defined(__linux) || defined(__APPLE__)
    gettimeofday( &time, NULL );
#else
    gettimeofday( &time );
#endif
    s1 = time.tv_sec  - ss;
    s2 = time.tv_usec/1000 - sms;
#endif

    tt = (double)s1 + (double)s2 / 1000.0;

    return( tt );
}

void arUtilTimerReset(void)
{
#ifdef _WIN32
    struct _timeb sys_time;

    _ftime(&sys_time);
    ss  = sys_time.time;
    sms = sys_time.millitm;
#else
    struct timeval     time;

#if defined(__linux) || defined(__APPLE__)
    gettimeofday( &time, NULL );
#else
    gettimeofday( &time );
#endif
    ss  = time.tv_sec;
    sms = time.tv_usec / 1000;
#endif
}

void arUtilSleep( int msec )
{
#ifndef _WIN32
    struct timespec  req;

    req.tv_sec = 0;
    req.tv_nsec = msec* 1000;
    nanosleep( &req, NULL );
#else
	Sleep(msec);
#endif
    return;
}

/// patt
int arLoadPatt( const char *filename )
{
    FILE    *fp;
    int     patno;
    int     h, i, j, l, m;
    int     i1, i2, i3;

    if(pattern_num == -1 ) {
        for( i = 0; i < AR_PATT_NUM_MAX; i++ ) patf[i] = 0;
        pattern_num = 0;
    }

    for( i = 0; i < AR_PATT_NUM_MAX; i++ ) {
        if(patf[i] == 0) break;
    }
    if( i == AR_PATT_NUM_MAX ) return -1;
    patno = i;

    if( (fp=fopen(filename, "r")) == NULL ) {
        printf("\"%s\" not found!!\n", filename);
        return(-1);
    }

    for( h=0; h<4; h++ ) {
        l = 0;
        for( i3 = 0; i3 < 3; i3++ ) {
            for( i2 = 0; i2 < AR_PATT_SIZE_Y; i2++ ) {
                for( i1 = 0; i1 < AR_PATT_SIZE_X; i1++ ) {
                    if( fscanf(fp, "%d", &j) != 1 ) {
                        printf("Pattern Data read error!!\n");
                        return -1;
                    }
                    j = 255-j;
                    pat[patno][h][(i2*AR_PATT_SIZE_X+i1)*3+i3] = j;
                    if( i3 == 0 ) patBW[patno][h][i2*AR_PATT_SIZE_X+i1]  = j;
                    else          patBW[patno][h][i2*AR_PATT_SIZE_X+i1] += j;
                    if( i3 == 2 ) patBW[patno][h][i2*AR_PATT_SIZE_X+i1] /= 3;
                    l += j;
                }
            }
        }
        l /= (AR_PATT_SIZE_Y*AR_PATT_SIZE_X*3);

        m = 0;
        for( i = 0; i < AR_PATT_SIZE_Y*AR_PATT_SIZE_X*3; i++ ) {
            pat[patno][h][i] -= l;
            m += (pat[patno][h][i]*pat[patno][h][i]);
        }
        patpow[patno][h] = sqrt((double)m);
        if( patpow[patno][h] == 0.0 ) patpow[patno][h] = 0.0000001;

        m = 0;
        for( i = 0; i < AR_PATT_SIZE_Y*AR_PATT_SIZE_X; i++ ) {
            patBW[patno][h][i] -= l;
            m += (patBW[patno][h][i]*patBW[patno][h][i]);
        }
        patpowBW[patno][h] = sqrt((double)m);
        if( patpowBW[patno][h] == 0.0 ) patpowBW[patno][h] = 0.0000001;
    }
    fclose(fp);

    patf[patno] = 1;
    pattern_num++;

/*
    gen_evec();
*/

    return( patno );
}

int arFreePatt( int patno )
{
    if( patf[patno] == 0 ) return -1;

    patf[patno] = 0;
    pattern_num--;

    gen_evec();

    return 1;
}

int arActivatePatt( int patno )
{
    if( patf[patno] == 0 ) return -1;
    patf[patno] = 1;
    return 1;
}

int arDeactivatePatt( int patno )
{
    if( patf[patno] == 0 ) return -1;
    patf[patno] = 2;
    return 1;
}

int arGetCode( ARUint8 *image, int *x_coord, int *y_coord, int *vertex, int *code, int *dir, double *cf )
{
#if DEBUG
static int count = 0;
static double a1 = 0.0;
static double a2 = 0.0;
double b1, b2, b3;
#endif
    ARUint8 ext_pat[AR_PATT_SIZE_Y][AR_PATT_SIZE_X][3];

#if DEBUG
b1 = arUtilTimer();
#endif
    arGetPatt(image, x_coord, y_coord, vertex, ext_pat);
#if DEBUG
b2 = arUtilTimer();
#endif

    pattern_match((ARUint8 *)ext_pat, code, dir, cf);
#if DEBUG
b3 = arUtilTimer();
#endif

#if DEBUG
a1 += (b2 - b1);
a2 += (b3 - b2);
count++;
if( count == 60 ) {
    printf("%10.5f[msec], %10.5f[msec]\n", a1*1000.0/60.0, a2*1000.0/60.0);
    count = 0;
    a1 = a2 = 0.0;
}
#endif

    return(0);
}

#if 1
int arGetPatt( ARUint8 *image, int *x_coord, int *y_coord, int *vertex,
               ARUint8 ext_pat[AR_PATT_SIZE_Y][AR_PATT_SIZE_X][3] )
{
    ARUint32  ext_pat2[AR_PATT_SIZE_Y][AR_PATT_SIZE_X][3];
    double    world[4][2];
    double    local[4][2];
    double    para[3][3];
    double    d, xw, yw;
    int       xc, yc;
    int       xdiv, ydiv;
    int       xdiv2, ydiv2;
    int       lx1, lx2, ly1, ly2;
    int       i, j;
    // int       k1, k2, k3; // unreferenced
	double    xdiv2_reciprocal; // [tp]
	double    ydiv2_reciprocal; // [tp]
	int       ext_pat2_x_index;
	int       ext_pat2_y_index;
	int       image_index;

    world[0][0] = 100.0;
    world[0][1] = 100.0;
    world[1][0] = 100.0 + 10.0;
    world[1][1] = 100.0;
    world[2][0] = 100.0 + 10.0;
    world[2][1] = 100.0 + 10.0;
    world[3][0] = 100.0;
    world[3][1] = 100.0 + 10.0;
    for( i = 0; i < 4; i++ ) {
        local[i][0] = x_coord[vertex[i]];
        local[i][1] = y_coord[vertex[i]];
    }
    get_cpara( world, local, para );

    lx1 = (int)((local[0][0] - local[1][0])*(local[0][0] - local[1][0])
        + (local[0][1] - local[1][1])*(local[0][1] - local[1][1]));
    lx2 = (int)((local[2][0] - local[3][0])*(local[2][0] - local[3][0])
        + (local[2][1] - local[3][1])*(local[2][1] - local[3][1]));
    ly1 = (int)((local[1][0] - local[2][0])*(local[1][0] - local[2][0])
        + (local[1][1] - local[2][1])*(local[1][1] - local[2][1]));
    ly2 = (int)((local[3][0] - local[0][0])*(local[3][0] - local[0][0])
        + (local[3][1] - local[0][1])*(local[3][1] - local[0][1]));
    if( lx2 > lx1 ) lx1 = lx2;
    if( ly2 > ly1 ) ly1 = ly2;
    xdiv2 = AR_PATT_SIZE_X;
    ydiv2 = AR_PATT_SIZE_Y;
    if( arImageProcMode == AR_IMAGE_PROC_IN_FULL ) {
        while( xdiv2*xdiv2 < lx1/4 ) xdiv2*=2;
        while( ydiv2*ydiv2 < ly1/4 ) ydiv2*=2;
    }
    else {
        while( xdiv2*xdiv2*4 < lx1/4 ) xdiv2*=2;
        while( ydiv2*ydiv2*4 < ly1/4 ) ydiv2*=2;
    }
    if( xdiv2 > AR_PATT_SAMPLE_NUM ) xdiv2 = AR_PATT_SAMPLE_NUM;
    if( ydiv2 > AR_PATT_SAMPLE_NUM ) ydiv2 = AR_PATT_SAMPLE_NUM;

    xdiv = xdiv2/AR_PATT_SIZE_X;
    ydiv = ydiv2/AR_PATT_SIZE_Y;
/*
printf("%3d(%f), %3d(%f)\n", xdiv2, sqrt(lx1), ydiv2, sqrt(ly1));
*/

	xdiv2_reciprocal = 1.0 / xdiv2;
	ydiv2_reciprocal = 1.0 / ydiv2;

    put_zero( (ARUint8 *)ext_pat2, AR_PATT_SIZE_Y*AR_PATT_SIZE_X*3*sizeof(ARUint32) );
    for( j = 0; j < ydiv2; j++ ) {
        yw = 102.5 + 5.0 * (j+0.5) * ydiv2_reciprocal;
        for( i = 0; i < xdiv2; i++ ) {
            xw = 102.5 + 5.0 * (i+0.5) * xdiv2_reciprocal;
            d = para[2][0]*xw + para[2][1]*yw + para[2][2];
            if( d == 0 ) return(-1);
            xc = (int)((para[0][0]*xw + para[0][1]*yw + para[0][2])/d);
            yc = (int)((para[1][0]*xw + para[1][1]*yw + para[1][2])/d);
            if( arImageProcMode == AR_IMAGE_PROC_IN_HALF ) {
                xc = ((xc+1)/2)*2;
                yc = ((yc+1)/2)*2;
            }
            if( xc >= 0 && xc < arImXsize && yc >= 0 && yc < arImYsize ) {
				ext_pat2_y_index = j/ydiv;
				ext_pat2_x_index = i/xdiv;
				image_index = (yc*arImXsize+xc)*AR_PIX_SIZE_DEFAULT;
#if (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_ARGB)
                ext_pat2[ext_pat2_y_index][ext_pat2_x_index][0] += image[image_index+3];
                ext_pat2[ext_pat2_y_index][ext_pat2_x_index][1] += image[image_index+2];
                ext_pat2[ext_pat2_y_index][ext_pat2_x_index][2] += image[image_index+1];
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_ABGR)
                ext_pat2[ext_pat2_y_index][ext_pat2_x_index][0] += image[image_index+1];
                ext_pat2[ext_pat2_y_index][ext_pat2_x_index][1] += image[image_index+2];
                ext_pat2[ext_pat2_y_index][ext_pat2_x_index][2] += image[image_index+3];
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_BGRA)
                ext_pat2[ext_pat2_y_index][ext_pat2_x_index][0] += image[image_index+0];
                ext_pat2[ext_pat2_y_index][ext_pat2_x_index][1] += image[image_index+1];
                ext_pat2[ext_pat2_y_index][ext_pat2_x_index][2] += image[image_index+2];
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_BGR)
                ext_pat2[ext_pat2_y_index][ext_pat2_x_index][0] += image[image_index+0];
                ext_pat2[ext_pat2_y_index][ext_pat2_x_index][1] += image[image_index+1];
                ext_pat2[ext_pat2_y_index][ext_pat2_x_index][2] += image[image_index+2];
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_RGBA)
                ext_pat2[ext_pat2_y_index][ext_pat2_x_index][0] += image[image_index+2];
                ext_pat2[ext_pat2_y_index][ext_pat2_x_index][1] += image[image_index+1];
                ext_pat2[ext_pat2_y_index][ext_pat2_x_index][2] += image[image_index+0];
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_RGB)
                ext_pat2[ext_pat2_y_index][ext_pat2_x_index][0] += image[image_index+2];
                ext_pat2[ext_pat2_y_index][ext_pat2_x_index][1] += image[image_index+1];
                ext_pat2[ext_pat2_y_index][ext_pat2_x_index][2] += image[image_index+0];
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_MONO)
                ext_pat2[ext_pat2_y_index][ext_pat2_x_index][0] += image[image_index];
                ext_pat2[ext_pat2_y_index][ext_pat2_x_index][1] += image[image_index];
                ext_pat2[ext_pat2_y_index][ext_pat2_x_index][2] += image[image_index];
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_2vuy)
                ext_pat2[ext_pat2_y_index][ext_pat2_x_index][0] += image[image_index+1];
                ext_pat2[ext_pat2_y_index][ext_pat2_x_index][1] += image[image_index+1];
                ext_pat2[ext_pat2_y_index][ext_pat2_x_index][2] += image[image_index+1];
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_yuvs)
                ext_pat2[ext_pat2_y_index][ext_pat2_x_index][0] += image[image_index+0];
                ext_pat2[ext_pat2_y_index][ext_pat2_x_index][1] += image[image_index+0];
                ext_pat2[ext_pat2_y_index][ext_pat2_x_index][2] += image[image_index+0];
#else
#  error Unknown default pixel format defined in config.h
#endif
            }
        }
    }

    for( j = 0; j < AR_PATT_SIZE_Y; j++ ) {
        for( i = 0; i < AR_PATT_SIZE_X; i++ ) {				// PRL 2006-06-08.
            ext_pat[j][i][0] = ext_pat2[j][i][0] / (xdiv*ydiv);
            ext_pat[j][i][1] = ext_pat2[j][i][1] / (xdiv*ydiv);
            ext_pat[j][i][2] = ext_pat2[j][i][2] / (xdiv*ydiv);
        }
    }

    return(0);
}
#else
int arGetPatt( ARUint8 *image, int *x_coord, int *y_coord, int *vertex,
               ARUint8 ext_pat[AR_PATT_SIZE_Y][AR_PATT_SIZE_X][3] )
{
    double  world[4][2];
    double  local[4][2];
    double  para[3][3];
    double  d, xw, yw;
    int     xc, yc;
    int     i, j;
    int     k1, k2, k3;

    world[0][0] = 100.0;
    world[0][1] = 100.0;
    world[1][0] = 100.0 + 10.0;
    world[1][1] = 100.0;
    world[2][0] = 100.0 + 10.0;
    world[2][1] = 100.0 + 10.0;
    world[3][0] = 100.0;
    world[3][1] = 100.0 + 10.0;
    for( i = 0; i < 4; i++ ) {
        local[i][0] = x_coord[vertex[i]];
        local[i][1] = y_coord[vertex[i]];
    }
    get_cpara( world, local, para );

    put_zero( (ARUint8 *)ext_pat, AR_PATT_SIZE_Y*AR_PATT_SIZE_X*3 );
    for( j = 0; j < AR_PATT_SAMPLE_NUM; j++ ) {
        yw = 102.5 + 5.0 * (j+0.5) / (double)AR_PATT_SAMPLE_NUM;
        for( i = 0; i < AR_PATT_SAMPLE_NUM; i++ ) {
            xw = 102.5 + 5.0 * (i+0.5) / (double)AR_PATT_SAMPLE_NUM;
            d = para[2][0]*xw + para[2][1]*yw + para[2][2];
            if( d == 0 ) return(-1);
            xc = (int)((para[0][0]*xw + para[0][1]*yw + para[0][2])/d);
            yc = (int)((para[1][0]*xw + para[1][1]*yw + para[1][2])/d);
            if( arImageProcMode == AR_IMAGE_PROC_IN_HALF ) {
                xc = ((xc+1)/2)*2;
                yc = ((yc+1)/2)*2;
            }
            if( xc >= 0 && xc < arImXsize && yc >= 0 && yc < arImYsize ) {
#if (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_ARGB)
                k1 = image[(yc*arImXsize+xc)*AR_PIX_SIZE_DEFAULT+3];
                k1 = ext_pat[j*AR_PATT_SIZE_Y/AR_PATT_SAMPLE_NUM][i*AR_PATT_SIZE_X/AR_PATT_SAMPLE_NUM][0]
					+ k1*(AR_PATT_SIZE_Y*AR_PATT_SIZE_X)/(AR_PATT_SAMPLE_NUM*AR_PATT_SAMPLE_NUM);
                ext_pat[j*AR_PATT_SIZE_Y/AR_PATT_SAMPLE_NUM][i*AR_PATT_SIZE_X/AR_PATT_SAMPLE_NUM][0] = (k1 > 255)? 255: k1;
                k2 = image[(yc*arImXsize+xc)*AR_PIX_SIZE_DEFAULT+2];
                k2 = ext_pat[j*AR_PATT_SIZE_Y/AR_PATT_SAMPLE_NUM][i*AR_PATT_SIZE_X/AR_PATT_SAMPLE_NUM][1]
					+ k2*(AR_PATT_SIZE_Y*AR_PATT_SIZE_X)/(AR_PATT_SAMPLE_NUM*AR_PATT_SAMPLE_NUM);
                ext_pat[j*AR_PATT_SIZE_Y/AR_PATT_SAMPLE_NUM][i*AR_PATT_SIZE_X/AR_PATT_SAMPLE_NUM][1] = (k2 > 255)? 255: k2;
                k3 = image[(yc*arImXsize+xc)*AR_PIX_SIZE_DEFAULT+1];
                k3 = ext_pat[j*AR_PATT_SIZE_Y/AR_PATT_SAMPLE_NUM][i*AR_PATT_SIZE_X/AR_PATT_SAMPLE_NUM][2]
					+ k3*(AR_PATT_SIZE_Y*AR_PATT_SIZE_X)/(AR_PATT_SAMPLE_NUM*AR_PATT_SAMPLE_NUM);
                ext_pat[j*AR_PATT_SIZE_Y/AR_PATT_SAMPLE_NUM][i*AR_PATT_SIZE_X/AR_PATT_SAMPLE_NUM][2] = (k3 > 255)? 255: k3;
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_ABGR)
                k1 = image[(yc*arImXsize+xc)*AR_PIX_SIZE_DEFAULT+1];
                k1 = ext_pat[j*AR_PATT_SIZE_Y/AR_PATT_SAMPLE_NUM][i*AR_PATT_SIZE_X/AR_PATT_SAMPLE_NUM][0]
                   + k1*(AR_PATT_SIZE_Y*AR_PATT_SIZE_X)/(AR_PATT_SAMPLE_NUM*AR_PATT_SAMPLE_NUM);
                ext_pat[j*AR_PATT_SIZE_Y/AR_PATT_SAMPLE_NUM][i*AR_PATT_SIZE_X/AR_PATT_SAMPLE_NUM][0] = (k1 > 255)? 255: k1;
                k2 = image[(yc*arImXsize+xc)*AR_PIX_SIZE_DEFAULT+2];
                k2 = ext_pat[j*AR_PATT_SIZE_Y/AR_PATT_SAMPLE_NUM][i*AR_PATT_SIZE_X/AR_PATT_SAMPLE_NUM][1]
                   + k2*(AR_PATT_SIZE_Y*AR_PATT_SIZE_X)/(AR_PATT_SAMPLE_NUM*AR_PATT_SAMPLE_NUM);
                ext_pat[j*AR_PATT_SIZE_Y/AR_PATT_SAMPLE_NUM][i*AR_PATT_SIZE_X/AR_PATT_SAMPLE_NUM][1] = (k2 > 255)? 255: k2;
                k3 = image[(yc*arImXsize+xc)*AR_PIX_SIZE_DEFAULT+3];
                k3 = ext_pat[j*AR_PATT_SIZE_Y/AR_PATT_SAMPLE_NUM][i*AR_PATT_SIZE_X/AR_PATT_SAMPLE_NUM][2]
                   + k3*(AR_PATT_SIZE_Y*AR_PATT_SIZE_X)/(AR_PATT_SAMPLE_NUM*AR_PATT_SAMPLE_NUM);
                ext_pat[j*AR_PATT_SIZE_Y/AR_PATT_SAMPLE_NUM][i*AR_PATT_SIZE_X/AR_PATT_SAMPLE_NUM][2] = (k3 > 255)? 255: k3;
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_BGRA)
                k1 = image[(yc*arImXsize+xc)*AR_PIX_SIZE_DEFAULT+0];
                k1 = ext_pat[j*AR_PATT_SIZE_Y/AR_PATT_SAMPLE_NUM][i*AR_PATT_SIZE_X/AR_PATT_SAMPLE_NUM][0]
                   + k1*(AR_PATT_SIZE_Y*AR_PATT_SIZE_X)/(AR_PATT_SAMPLE_NUM*AR_PATT_SAMPLE_NUM);
                ext_pat[j*AR_PATT_SIZE_Y/AR_PATT_SAMPLE_NUM][i*AR_PATT_SIZE_X/AR_PATT_SAMPLE_NUM][0] = (k1 > 255)? 255: k1;
                k2 = image[(yc*arImXsize+xc)*AR_PIX_SIZE_DEFAULT+1];
                k2 = ext_pat[j*AR_PATT_SIZE_Y/AR_PATT_SAMPLE_NUM][i*AR_PATT_SIZE_X/AR_PATT_SAMPLE_NUM][1]
                   + k2*(AR_PATT_SIZE_Y*AR_PATT_SIZE_X)/(AR_PATT_SAMPLE_NUM*AR_PATT_SAMPLE_NUM);
                ext_pat[j*AR_PATT_SIZE_Y/AR_PATT_SAMPLE_NUM][i*AR_PATT_SIZE_X/AR_PATT_SAMPLE_NUM][1] = (k2 > 255)? 255: k2;
                k3 = image[(yc*arImXsize+xc)*AR_PIX_SIZE_DEFAULT+2];
                k3 = ext_pat[j*AR_PATT_SIZE_Y/AR_PATT_SAMPLE_NUM][i*AR_PATT_SIZE_X/AR_PATT_SAMPLE_NUM][2]
                   + k3*(AR_PATT_SIZE_Y*AR_PATT_SIZE_X)/(AR_PATT_SAMPLE_NUM*AR_PATT_SAMPLE_NUM);
                ext_pat[j*AR_PATT_SIZE_Y/AR_PATT_SAMPLE_NUM][i*AR_PATT_SIZE_X/AR_PATT_SAMPLE_NUM][2] = (k3 > 255)? 255: k3;
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_BGR)
                k1 = image[(yc*arImXsize+xc)*AR_PIX_SIZE_DEFAULT+0];
                k1 = ext_pat[j*AR_PATT_SIZE_Y/AR_PATT_SAMPLE_NUM][i*AR_PATT_SIZE_X/AR_PATT_SAMPLE_NUM][0]
                   + k1*(AR_PATT_SIZE_Y*AR_PATT_SIZE_X)/(AR_PATT_SAMPLE_NUM*AR_PATT_SAMPLE_NUM);
                ext_pat[j*AR_PATT_SIZE_Y/AR_PATT_SAMPLE_NUM][i*AR_PATT_SIZE_X/AR_PATT_SAMPLE_NUM][0] = (k1 > 255)? 255: k1;
                k2 = image[(yc*arImXsize+xc)*AR_PIX_SIZE_DEFAULT+1];
                k2 = ext_pat[j*AR_PATT_SIZE_Y/AR_PATT_SAMPLE_NUM][i*AR_PATT_SIZE_X/AR_PATT_SAMPLE_NUM][1]
                   + k2*(AR_PATT_SIZE_Y*AR_PATT_SIZE_X)/(AR_PATT_SAMPLE_NUM*AR_PATT_SAMPLE_NUM);
                ext_pat[j*AR_PATT_SIZE_Y/AR_PATT_SAMPLE_NUM][i*AR_PATT_SIZE_X/AR_PATT_SAMPLE_NUM][1] = (k2 > 255)? 255: k2;
                k3 = image[(yc*arImXsize+xc)*AR_PIX_SIZE_DEFAULT+2];
                k3 = ext_pat[j*AR_PATT_SIZE_Y/AR_PATT_SAMPLE_NUM][i*AR_PATT_SIZE_X/AR_PATT_SAMPLE_NUM][2]
                   + k3*(AR_PATT_SIZE_Y*AR_PATT_SIZE_X)/(AR_PATT_SAMPLE_NUM*AR_PATT_SAMPLE_NUM);
                ext_pat[j*AR_PATT_SIZE_Y/AR_PATT_SAMPLE_NUM][i*AR_PATT_SIZE_X/AR_PATT_SAMPLE_NUM][2] = (k3 > 255)? 255: k3;
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_RGBA)
                k1 = image[(yc*arImXsize+xc)*AR_PIX_SIZE_DEFAULT+2];
                k1 = ext_pat[j*AR_PATT_SIZE_Y/AR_PATT_SAMPLE_NUM][i*AR_PATT_SIZE_X/AR_PATT_SAMPLE_NUM][0]
                   + k1*(AR_PATT_SIZE_Y*AR_PATT_SIZE_X)/(AR_PATT_SAMPLE_NUM*AR_PATT_SAMPLE_NUM);
                ext_pat[j*AR_PATT_SIZE_Y/AR_PATT_SAMPLE_NUM][i*AR_PATT_SIZE_X/AR_PATT_SAMPLE_NUM][0] = (k1 > 255)? 255: k1;
                k2 = image[(yc*arImXsize+xc)*AR_PIX_SIZE_DEFAULT+1];
                k2 = ext_pat[j*AR_PATT_SIZE_Y/AR_PATT_SAMPLE_NUM][i*AR_PATT_SIZE_X/AR_PATT_SAMPLE_NUM][1]
                   + k2*(AR_PATT_SIZE_Y*AR_PATT_SIZE_X)/(AR_PATT_SAMPLE_NUM*AR_PATT_SAMPLE_NUM);
                ext_pat[j*AR_PATT_SIZE_Y/AR_PATT_SAMPLE_NUM][i*AR_PATT_SIZE_X/AR_PATT_SAMPLE_NUM][1] = (k2 > 255)? 255: k2;
                k3 = image[(yc*arImXsize+xc)*AR_PIX_SIZE_DEFAULT+0];
                k3 = ext_pat[j*AR_PATT_SIZE_Y/AR_PATT_SAMPLE_NUM][i*AR_PATT_SIZE_X/AR_PATT_SAMPLE_NUM][2]
                   + k3*(AR_PATT_SIZE_Y*AR_PATT_SIZE_X)/(AR_PATT_SAMPLE_NUM*AR_PATT_SAMPLE_NUM);
                ext_pat[j*AR_PATT_SIZE_Y/AR_PATT_SAMPLE_NUM][i*AR_PATT_SIZE_X/AR_PATT_SAMPLE_NUM][2] = (k3 > 255)? 255: k3;
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_RGB)
                k1 = image[(yc*arImXsize+xc)*AR_PIX_SIZE_DEFAULT+2];
                k1 = ext_pat[j*AR_PATT_SIZE_Y/AR_PATT_SAMPLE_NUM][i*AR_PATT_SIZE_X/AR_PATT_SAMPLE_NUM][0]
                   + k1*(AR_PATT_SIZE_Y*AR_PATT_SIZE_X)/(AR_PATT_SAMPLE_NUM*AR_PATT_SAMPLE_NUM);
                ext_pat[j*AR_PATT_SIZE_Y/AR_PATT_SAMPLE_NUM][i*AR_PATT_SIZE_X/AR_PATT_SAMPLE_NUM][0] = (k1 > 255)? 255: k1;
                k2 = image[(yc*arImXsize+xc)*AR_PIX_SIZE_DEFAULT+1];
                k2 = ext_pat[j*AR_PATT_SIZE_Y/AR_PATT_SAMPLE_NUM][i*AR_PATT_SIZE_X/AR_PATT_SAMPLE_NUM][1]
                   + k2*(AR_PATT_SIZE_Y*AR_PATT_SIZE_X)/(AR_PATT_SAMPLE_NUM*AR_PATT_SAMPLE_NUM);
                ext_pat[j*AR_PATT_SIZE_Y/AR_PATT_SAMPLE_NUM][i*AR_PATT_SIZE_X/AR_PATT_SAMPLE_NUM][1] = (k2 > 255)? 255: k2;
                k3 = image[(yc*arImXsize+xc)*AR_PIX_SIZE_DEFAULT+0];
                k3 = ext_pat[j*AR_PATT_SIZE_Y/AR_PATT_SAMPLE_NUM][i*AR_PATT_SIZE_X/AR_PATT_SAMPLE_NUM][2]
                   + k3*(AR_PATT_SIZE_Y*AR_PATT_SIZE_X)/(AR_PATT_SAMPLE_NUM*AR_PATT_SAMPLE_NUM);
                ext_pat[j*AR_PATT_SIZE_Y/AR_PATT_SAMPLE_NUM][i*AR_PATT_SIZE_X/AR_PATT_SAMPLE_NUM][2] = (k3 > 255)? 255: k3;
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_MONO)
                k1 = image[(yc*arImXsize+xc)*AR_PIX_SIZE_DEFAULT];
                k1 = ext_pat[j*AR_PATT_SIZE_Y/AR_PATT_SAMPLE_NUM][i*AR_PATT_SIZE_X/AR_PATT_SAMPLE_NUM][0]
					+ k1*(AR_PATT_SIZE_Y*AR_PATT_SIZE_X)/(AR_PATT_SAMPLE_NUM*AR_PATT_SAMPLE_NUM);
                ext_pat[j*AR_PATT_SIZE_Y/AR_PATT_SAMPLE_NUM][i*AR_PATT_SIZE_X/AR_PATT_SAMPLE_NUM][0] = (k1 > 255)? 255: k1;
                k2 = image[(yc*arImXsize+xc)*AR_PIX_SIZE_DEFAULT];
                k2 = ext_pat[j*AR_PATT_SIZE_Y/AR_PATT_SAMPLE_NUM][i*AR_PATT_SIZE_X/AR_PATT_SAMPLE_NUM][1]
					+ k2*(AR_PATT_SIZE_Y*AR_PATT_SIZE_X)/(AR_PATT_SAMPLE_NUM*AR_PATT_SAMPLE_NUM);
                ext_pat[j*AR_PATT_SIZE_Y/AR_PATT_SAMPLE_NUM][i*AR_PATT_SIZE_X/AR_PATT_SAMPLE_NUM][1] = (k2 > 255)? 255: k2;
                k3 = image[(yc*arImXsize+xc)*AR_PIX_SIZE_DEFAULT];
                k3 = ext_pat[j*AR_PATT_SIZE_Y/AR_PATT_SAMPLE_NUM][i*AR_PATT_SIZE_X/AR_PATT_SAMPLE_NUM][2]
					+ k3*(AR_PATT_SIZE_Y*AR_PATT_SIZE_X)/(AR_PATT_SAMPLE_NUM*AR_PATT_SAMPLE_NUM);
                ext_pat[j*AR_PATT_SIZE_Y/AR_PATT_SAMPLE_NUM][i*AR_PATT_SIZE_X/AR_PATT_SAMPLE_NUM][2] = (k3 > 255)? 255: k3;
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_2vuy)
                k1 = image[(yc*arImXsize+xc)*AR_PIX_SIZE_DEFAULT+1];
                k1 = ext_pat[j*AR_PATT_SIZE_Y/AR_PATT_SAMPLE_NUM][i*AR_PATT_SIZE_X/AR_PATT_SAMPLE_NUM][0]
					+ k1*(AR_PATT_SIZE_Y*AR_PATT_SIZE_X)/(AR_PATT_SAMPLE_NUM*AR_PATT_SAMPLE_NUM);
                ext_pat[j*AR_PATT_SIZE_Y/AR_PATT_SAMPLE_NUM][i*AR_PATT_SIZE_X/AR_PATT_SAMPLE_NUM][0] = (k1 > 255)? 255: k1;
                k2 = image[(yc*arImXsize+xc)*AR_PIX_SIZE_DEFAULT+1];
                k2 = ext_pat[j*AR_PATT_SIZE_Y/AR_PATT_SAMPLE_NUM][i*AR_PATT_SIZE_X/AR_PATT_SAMPLE_NUM][1]
					+ k2*(AR_PATT_SIZE_Y*AR_PATT_SIZE_X)/(AR_PATT_SAMPLE_NUM*AR_PATT_SAMPLE_NUM);
                ext_pat[j*AR_PATT_SIZE_Y/AR_PATT_SAMPLE_NUM][i*AR_PATT_SIZE_X/AR_PATT_SAMPLE_NUM][1] = (k2 > 255)? 255: k2;
                k3 = image[(yc*arImXsize+xc)*AR_PIX_SIZE_DEFAULT+1];
                k3 = ext_pat[j*AR_PATT_SIZE_Y/AR_PATT_SAMPLE_NUM][i*AR_PATT_SIZE_X/AR_PATT_SAMPLE_NUM][2]
					+ k3*(AR_PATT_SIZE_Y*AR_PATT_SIZE_X)/(AR_PATT_SAMPLE_NUM*AR_PATT_SAMPLE_NUM);
                ext_pat[j*AR_PATT_SIZE_Y/AR_PATT_SAMPLE_NUM][i*AR_PATT_SIZE_X/AR_PATT_SAMPLE_NUM][2] = (k3 > 255)? 255: k3;
#elif (AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_yuvs)
                k1 = image[(yc*arImXsize+xc)*AR_PIX_SIZE_DEFAULT+0];
                k1 = ext_pat[j*AR_PATT_SIZE_Y/AR_PATT_SAMPLE_NUM][i*AR_PATT_SIZE_X/AR_PATT_SAMPLE_NUM][0]
					+ k1*(AR_PATT_SIZE_Y*AR_PATT_SIZE_X)/(AR_PATT_SAMPLE_NUM*AR_PATT_SAMPLE_NUM);
                ext_pat[j*AR_PATT_SIZE_Y/AR_PATT_SAMPLE_NUM][i*AR_PATT_SIZE_X/AR_PATT_SAMPLE_NUM][0] = (k1 > 255)? 255: k1;
                k2 = image[(yc*arImXsize+xc)*AR_PIX_SIZE_DEFAULT+0];
                k2 = ext_pat[j*AR_PATT_SIZE_Y/AR_PATT_SAMPLE_NUM][i*AR_PATT_SIZE_X/AR_PATT_SAMPLE_NUM][1]
					+ k2*(AR_PATT_SIZE_Y*AR_PATT_SIZE_X)/(AR_PATT_SAMPLE_NUM*AR_PATT_SAMPLE_NUM);
                ext_pat[j*AR_PATT_SIZE_Y/AR_PATT_SAMPLE_NUM][i*AR_PATT_SIZE_X/AR_PATT_SAMPLE_NUM][1] = (k2 > 255)? 255: k2;
                k3 = image[(yc*arImXsize+xc)*AR_PIX_SIZE_DEFAULT+0];
                k3 = ext_pat[j*AR_PATT_SIZE_Y/AR_PATT_SAMPLE_NUM][i*AR_PATT_SIZE_X/AR_PATT_SAMPLE_NUM][2]
					+ k3*(AR_PATT_SIZE_Y*AR_PATT_SIZE_X)/(AR_PATT_SAMPLE_NUM*AR_PATT_SAMPLE_NUM);
                ext_pat[j*AR_PATT_SIZE_Y/AR_PATT_SAMPLE_NUM][i*AR_PATT_SIZE_X/AR_PATT_SAMPLE_NUM][2] = (k3 > 255)? 255: k3;
#else
#  error Unknown default pixel format defined in config.h
#endif
            }
        }
    }

    return(0);
}
#endif

int arGetContour( ARInt16 *limage, int *label_ref, int label, int clip[4], ARMarkerInfo2 *marker_info2 )
{
    static int      xdir[8] = { 0, 1, 1, 1, 0,-1,-1,-1};
    static int      ydir[8] = {-1,-1, 0, 1, 1, 1, 0,-1};
    static int      wx[AR_CHAIN_MAX];
    static int      wy[AR_CHAIN_MAX];
    ARInt16         *p1;
    int             xsize;
    int             sx, sy, dir;
    int             dmax, d, v1;
    int             i, j;

    if( arImageProcMode == AR_IMAGE_PROC_IN_HALF ) {
        xsize = arImXsize / 2;
    }
    else {
        xsize = arImXsize;
    }
    j = clip[2];
    p1 = &(limage[j*xsize+clip[0]]);
    for( i = clip[0]; i <= clip[1]; i++, p1++ ) {
        if( *p1 > 0 && label_ref[(*p1)-1] == label ) {
            sx = i; sy = j; break;
        }
    }
    if( i > clip[1] ) {
        printf("??? 1\n"); return(-1);
    }

    marker_info2->coord_num = 1;
    marker_info2->x_coord[0] = sx;
    marker_info2->y_coord[0] = sy;
    dir = 5;
    for(;;) {
        p1 = &(limage[marker_info2->y_coord[marker_info2->coord_num-1] * xsize
                    + marker_info2->x_coord[marker_info2->coord_num-1]]);
        dir = (dir+5)%8;
        for(i=0;i<8;i++) {
            if( p1[ydir[dir]*xsize+xdir[dir]] > 0 ) break;
            dir = (dir+1)%8;
        }
        if( i == 8 ) {
            printf("??? 2\n"); return(-1);
        }
        marker_info2->x_coord[marker_info2->coord_num]
            = marker_info2->x_coord[marker_info2->coord_num-1] + xdir[dir];
        marker_info2->y_coord[marker_info2->coord_num]
            = marker_info2->y_coord[marker_info2->coord_num-1] + ydir[dir];
        if( marker_info2->x_coord[marker_info2->coord_num] == sx
         && marker_info2->y_coord[marker_info2->coord_num] == sy ) break;
        marker_info2->coord_num++;
        if( marker_info2->coord_num == AR_CHAIN_MAX-1 ) {
            printf("??? 3\n"); return(-1);
        }
    }

    dmax = 0;
    for(i=1;i<marker_info2->coord_num;i++) {
        d = (marker_info2->x_coord[i]-sx)*(marker_info2->x_coord[i]-sx)
          + (marker_info2->y_coord[i]-sy)*(marker_info2->y_coord[i]-sy);
        if( d > dmax ) {
            dmax = d;
            v1 = i;
        }
    }

    for(i=0;i<v1;i++) {
        wx[i] = marker_info2->x_coord[i];
        wy[i] = marker_info2->y_coord[i];
    }
    for(i=v1;i<marker_info2->coord_num;i++) {
        marker_info2->x_coord[i-v1] = marker_info2->x_coord[i];
        marker_info2->y_coord[i-v1] = marker_info2->y_coord[i];
    }
    for(i=0;i<v1;i++) {
        marker_info2->x_coord[i-v1+marker_info2->coord_num] = wx[i];
        marker_info2->y_coord[i-v1+marker_info2->coord_num] = wy[i];
    }
    marker_info2->x_coord[marker_info2->coord_num] = marker_info2->x_coord[0];
    marker_info2->y_coord[marker_info2->coord_num] = marker_info2->y_coord[0];
    marker_info2->coord_num++;

    return 0;
}

static void get_cpara( double world[4][2], double vertex[4][2],
                       double para[3][3] )
{
    ARMat   *a, *b, *c;
    int     i;

    a = arMatrixAlloc( 8, 8 );
    b = arMatrixAlloc( 8, 1 );
    c = arMatrixAlloc( 8, 1 );
    for( i = 0; i < 4; i++ ) {
        a->m[i*16+0]  = world[i][0];
        a->m[i*16+1]  = world[i][1];
        a->m[i*16+2]  = 1.0;
        a->m[i*16+3]  = 0.0;
        a->m[i*16+4]  = 0.0;
        a->m[i*16+5]  = 0.0;
        a->m[i*16+6]  = -world[i][0] * vertex[i][0];
        a->m[i*16+7]  = -world[i][1] * vertex[i][0];
        a->m[i*16+8]  = 0.0;
        a->m[i*16+9]  = 0.0;
        a->m[i*16+10] = 0.0;
        a->m[i*16+11] = world[i][0];
        a->m[i*16+12] = world[i][1];
        a->m[i*16+13] = 1.0;
        a->m[i*16+14] = -world[i][0] * vertex[i][1];
        a->m[i*16+15] = -world[i][1] * vertex[i][1];
        b->m[i*2+0] = vertex[i][0];
        b->m[i*2+1] = vertex[i][1];
    }
    arMatrixSelfInv( a );
    arMatrixMul( c, a, b );
    for( i = 0; i < 2; i++ ) {
        para[i][0] = c->m[i*3+0];
        para[i][1] = c->m[i*3+1];
        para[i][2] = c->m[i*3+2];
    }
    para[2][0] = c->m[2*3+0];
    para[2][1] = c->m[2*3+1];
    para[2][2] = 1.0;
    arMatrixFree( a );
    arMatrixFree( b );
    arMatrixFree( c );
}

static int pattern_match( ARUint8 *data, int *code, int *dir, double *cf )
{
    double invec[EVEC_MAX];
    int    input[AR_PATT_SIZE_Y*AR_PATT_SIZE_X*3];
    int    i, j, l;
    int    k = 0; // fix VC7 compiler warning: uninitialized variable
    int    ave, sum, res, res2;
    double datapow, sum2, min;
    double max = 0.0; // fix VC7 compiler warning: uninitialized variable

    sum = ave = 0;
    for(i=0;i<AR_PATT_SIZE_Y*AR_PATT_SIZE_X*3;i++) {
        ave += (255-data[i]);
    }
    ave /= (AR_PATT_SIZE_Y*AR_PATT_SIZE_X*3);

    if( arTemplateMatchingMode == AR_TEMPLATE_MATCHING_COLOR ) {
        for(i=0;i<AR_PATT_SIZE_Y*AR_PATT_SIZE_X*3;i++) {
            input[i] = (255-data[i]) - ave;
            sum += input[i]*input[i];
        }
    }
    else {
        for(i=0;i<AR_PATT_SIZE_Y*AR_PATT_SIZE_X;i++) {
            input[i] = ((255-data[i*3+0]) + (255-data[i*3+1]) + (255-data[i*3+02]))/3 - ave;
            sum += input[i]*input[i];
        }
    }

    datapow = sqrt( (double)sum );
    if( datapow == 0.0 ) {
        *code = 0;
        *dir  = 0;
        *cf   = -1.0;
        return -1;
    }

    res = res2 = -1;
    if( arTemplateMatchingMode == AR_TEMPLATE_MATCHING_COLOR ) {
        if( arMatchingPCAMode == AR_MATCHING_WITH_PCA && evecf ) {

            for( i = 0; i < evec_dim; i++ ) {
                invec[i] = 0.0;
                for( j = 0; j < AR_PATT_SIZE_Y*AR_PATT_SIZE_X*3; j++ ) {
                    invec[i] += evec[i][j] * input[j];
                }
                invec[i] /= datapow;
            }

            min = 10000.0;
            k = -1;
            for( l = 0; l < pattern_num; l++ ) {
                k++;
                while( patf[k] == 0 ) k++;
                if( patf[k] == 2 ) continue;
#if DEBUG
                printf("%3d: ", k);
#endif
                for( j = 0; j < 4; j++ ) {
                    sum2 = 0;
                    for(i = 0; i < evec_dim; i++ ) {
                        sum2 += (invec[i] - epat[k][j][i]) * (invec[i] - epat[k][j][i]);
                    }
#if DEBUG
                    printf("%10.7f ", sum2);
#endif
                    if( sum2 < min ) { min = sum2; res = j; res2 = k; }
                }
#if DEBUG
                printf("\n");
#endif
            }
            sum = 0;
            for(i=0;i<AR_PATT_SIZE_Y*AR_PATT_SIZE_X*3;i++) sum += input[i]*pat[res2][res][i];
            max = sum / patpow[res2][res] / datapow;
        }
        else {
            k = -1;
            max = 0.0;
            for( l = 0; l < pattern_num; l++ ) {
                k++;
                while( patf[k] == 0 ) k++;
                if( patf[k] == 2 ) continue;
                for( j = 0; j < 4; j++ ) {
                    sum = 0;
                    for(i=0;i<AR_PATT_SIZE_Y*AR_PATT_SIZE_X*3;i++) sum += input[i]*pat[k][j][i];
                    sum2 = sum / patpow[k][j] / datapow;
                    if( sum2 > max ) { max = sum2; res = j; res2 = k; }
                }
            }
        }
    }
    else {
        for( l = 0; l < pattern_num; l++ ) {
            k++;
            while( patf[k] == 0 ) k++;
            if( patf[k] == 2 ) continue;
            for( j = 0; j < 4; j++ ) {
                sum = 0;
                for(i=0;i<AR_PATT_SIZE_Y*AR_PATT_SIZE_X;i++) sum += input[i]*patBW[k][j][i];
                sum2 = sum / patpowBW[k][j] / datapow;
                if( sum2 > max ) { max = sum2; res = j; res2 = k; }
            }
        }
    }

    *code = res2;
    *dir  = res;
    *cf   = max;

#if DEBUG
    printf("%d %d %f\n", res2, res, max);
#endif

    return 0;
}

static void   put_zero( ARUint8 *p, int size )
{
    while( (size--) > 0 ) *(p++) = 0;
}

static void gen_evec(void)
{
    int    i, j, k, ii, jj;
    ARMat  *input, *wevec;
    ARVec  *wev;
    double sum, sum2;
    int    dim;

    if( pattern_num < 4 ) {
        evecf   = 0;
        evecBWf = 0;
        return;
    }

#if DEBUG
    printf("------------------------------------------\n");
#endif

    dim = (pattern_num*4 < AR_PATT_SIZE_Y*AR_PATT_SIZE_X*3)? pattern_num*4: AR_PATT_SIZE_Y*AR_PATT_SIZE_X*3;
    input  = arMatrixAlloc( pattern_num*4, AR_PATT_SIZE_Y*AR_PATT_SIZE_X*3 );
    wevec   = arMatrixAlloc( dim, AR_PATT_SIZE_Y*AR_PATT_SIZE_X*3 );
    wev     = arVecAlloc( dim );

    for( j = jj = 0; jj < AR_PATT_NUM_MAX; jj++ ) {
        if( patf[jj] == 0 ) continue;
        for( k = 0; k < 4; k++ ) {
            for( i = 0; i < AR_PATT_SIZE_Y*AR_PATT_SIZE_X*3; i++ ) {
                input->m[(j*4+k)*AR_PATT_SIZE_Y*AR_PATT_SIZE_X*3+i] = pat[j][k][i] / patpow[j][k];
            }
        }
        j++;
    }

    if( arMatrixPCA2(input, wevec, wev) < 0 ) {
        arMatrixFree( input );
        arMatrixFree( wevec );
        arVecFree( wev );
        evecf   = 0;
        evecBWf = 0;
        return;
    }

    sum = 0.0;
    for( i = 0; i < dim; i++ ) {
        sum += wev->v[i];
#if DEBUG
        printf("%2d(%10.7f): \n", i+1, sum);
#endif
        if( sum > 0.90 ) break;
        if( i == EVEC_MAX-1 ) break;
    }
    evec_dim = i+1;

    for( j = 0; j < evec_dim; j++ ) {
        for( i = 0; i < AR_PATT_SIZE_Y*AR_PATT_SIZE_X*3; i++ ) {
            evec[j][i] = wevec->m[j*AR_PATT_SIZE_Y*AR_PATT_SIZE_X*3+i];
        }
    }

    for( i = 0; i < AR_PATT_NUM_MAX; i++ ) {
        if(patf[i] == 0) continue;
        for( j = 0; j < 4; j++ ) {
#if DEBUG
            printf("%2d[%d]: ", i+1, j+1);
#endif
            sum2 = 0.0;
            for( k = 0; k < evec_dim; k++ ) {
                sum = 0.0;
                for(ii=0;ii<AR_PATT_SIZE_Y*AR_PATT_SIZE_X*3;ii++) {
                    sum += evec[k][ii] * pat[i][j][ii] / patpow[i][j];
                }
#if DEBUG
                printf("%10.7f ", sum);
#endif
                epat[i][j][k] = sum;
                sum2 += sum*sum;
            }
#if DEBUG
            printf(":: %10.7f\n", sqrt(sum2));
#endif
        }
    }

    arMatrixFree( input );
    arMatrixFree( wevec );
    arVecFree( wev );

    evecf   = 1;
    evecBWf = 0;

    return;
}

int arMultiActivate(ARMultiMarkerInfoT *config)
{
    int    i;
    config->prevF = 0;
    for(i = 0; i < config->marker_num; i++) {
        if (arActivatePatt(config->marker[i].patt_id) != 1) return (-1);
    }
    return 0;
}

int arMultiDeactivate(ARMultiMarkerInfoT *config)
{
    int    i;
    config->prevF = 0;
    for(i = 0; i < config->marker_num; i++) {
        if (arDeactivatePatt(config->marker[i].patt_id)) return (-1);
    }
    return 0;
}

int arMultiFreeConfig(ARMultiMarkerInfoT *config)
{
    int    i;

    for(i = 0; i < config->marker_num; i++) {
        if (arFreePatt(config->marker[i].patt_id) != 1) return (-1);
    }
    free(config->marker);
    free(config);
    config = NULL;

    return 0;
}

static ARMarkerInfo2          *marker_info2;
static int                    wmarker_num = 0;

int arSavePatt( ARUint8 *image, ARMarkerInfo *marker_info, char *filename )
{
    FILE      *fp;
    ARUint8   ext_pat[4][AR_PATT_SIZE_Y][AR_PATT_SIZE_X][3];
    int       vertex[4];
    int       i, j, k, x, y;

	// Match supplied info against previously recognised marker.
    for( i = 0; i < wmarker_num; i++ ) {
        if( marker_info->area   == marker_info2[i].area
         && marker_info->pos[0] == marker_info2[i].pos[0]
         && marker_info->pos[1] == marker_info2[i].pos[1] ) break;
    }
    if( i == wmarker_num ) return -1;

    for( j = 0; j < 4; j++ ) {
        for( k = 0; k < 4; k++ ) {
            vertex[k] = marker_info2[i].vertex[(k+j+2)%4];
        }
        arGetPatt( image, marker_info2[i].x_coord,
                   marker_info2[i].y_coord, vertex, ext_pat[j] );
    }

    fp = fopen( filename, "w" );
    if( fp == NULL ) return -1;

	// Write out in order AR_PATT_SIZE_X columns x AR_PATT_SIZE_Y rows x 3 colours x 4 orientations.
    for( i = 0; i < 4; i++ ) {
        for( j = 0; j < 3; j++ ) {
            for( y = 0; y < AR_PATT_SIZE_Y; y++ ) {
                for( x = 0; x < AR_PATT_SIZE_X; x++ ) {
                    fprintf( fp, "%4d", ext_pat[i][y][x][j] );
                }
                fprintf(fp, "\n");
            }
        }
        fprintf(fp, "\n");
    }

    fclose( fp );

    return 0;
}


/// labeling
static ARInt16 *labeling2( ARUint8 *image, int thresh, int *label_num, int **area,
                          double **pos, int **clip, int **label_ref, int LorR )
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

static ARInt16 *labeling3( ARUint8 *image, int thresh, int *label_num, int **area,
                          double **pos, int **clip, int **label_ref, int LorR )
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

ARInt16 *arLabeling( ARUint8 *image, int thresh, int *label_num, int **area,
                    double **pos, int **clip, int **label_ref )
{
    if( arDebug ) {
        return( labeling3(image, thresh, label_num,
                          area, pos, clip, label_ref, 1) );
    } else {
        return( labeling2(image, thresh, label_num,
                          area, pos, clip, label_ref, 1) );
    }
}

ARInt16 *arsLabeling( ARUint8 *image, int thresh, int *label_num, int **area,
                     double **pos, int **clip, int **label_ref, int LorR )
{
    if( arDebug ) {
        return( labeling3(image, thresh, label_num,
                          area, pos, clip, label_ref, LorR) );
    } else {
        return( labeling2(image, thresh, label_num,
                          area, pos, clip, label_ref, LorR) );
    }
}

static char *get_buff( char *buf, int n, FILE *fp )
{
    char *ret;

    for(;;) {
        ret = fgets( buf, n, fp );
        if( ret == NULL ) return(NULL);
        if( buf[0] != '\n' && buf[0] != '\r' && buf[0] != '#' ) return(ret);
    }
}

ARMultiMarkerInfoT *arMultiReadConfigFile( const char *filename )
{
    FILE                   *fp;
    ARMultiEachMarkerInfoT *marker;
    ARMultiMarkerInfoT     *marker_info;
    double                 wpos3d[4][2];
    char                   buf[256], buf1[256];
    int                    num;
    int                    i, j;

    if( (fp=fopen(filename,"r")) == NULL ) return NULL;

    get_buff(buf, 256, fp);
    if( sscanf(buf, "%d", &num) != 1 ) {fclose(fp); return NULL;}

    arMalloc(marker,ARMultiEachMarkerInfoT,num);
    for( i = 0; i < num; i++ ) {
        get_buff(buf, 256, fp);
        if( sscanf(buf, "%s", buf1) != 1 ) {
            fclose(fp); free(marker); return NULL;
        }

        if( (marker[i].patt_id = arLoadPatt(buf1)) < 0 ) {
            fclose(fp); free(marker); return NULL;
        }

        get_buff(buf, 256, fp);
        if( sscanf(buf, "%lf", &marker[i].width) != 1 ) {
            fclose(fp); free(marker); return NULL;
        }

        get_buff(buf, 256, fp);
        if( sscanf(buf, "%lf %lf", &marker[i].center[0], &marker[i].center[1]) != 2 ) {
            fclose(fp); free(marker); return NULL;
        }

        for( j = 0; j < 3; j++ ) {
            get_buff(buf, 256, fp);
            if( sscanf(buf, "%lf %lf %lf %lf", &marker[i].trans[j][0],
                       &marker[i].trans[j][1], &marker[i].trans[j][2],
                       &marker[i].trans[j][3]) != 4 ) {
                fclose(fp); free(marker); return NULL;
            }
        }
        arUtilMatInv( marker[i].trans, marker[i].itrans );

        wpos3d[0][0] = marker[i].center[0] - marker[i].width/2.0;
        wpos3d[0][1] = marker[i].center[1] + marker[i].width/2.0;
        wpos3d[1][0] = marker[i].center[0] + marker[i].width/2.0;
        wpos3d[1][1] = marker[i].center[1] + marker[i].width/2.0;
        wpos3d[2][0] = marker[i].center[0] + marker[i].width/2.0;
        wpos3d[2][1] = marker[i].center[1] - marker[i].width/2.0;
        wpos3d[3][0] = marker[i].center[0] - marker[i].width/2.0;
        wpos3d[3][1] = marker[i].center[1] - marker[i].width/2.0;
        for( j = 0; j < 4; j++ ) {
            marker[i].pos3d[j][0] = marker[i].trans[0][0] * wpos3d[j][0]
                                  + marker[i].trans[0][1] * wpos3d[j][1]
                                  + marker[i].trans[0][3];
            marker[i].pos3d[j][1] = marker[i].trans[1][0] * wpos3d[j][0]
                                  + marker[i].trans[1][1] * wpos3d[j][1]
                                  + marker[i].trans[1][3];
            marker[i].pos3d[j][2] = marker[i].trans[2][0] * wpos3d[j][0]
                                  + marker[i].trans[2][1] * wpos3d[j][1]
                                  + marker[i].trans[2][3];
        }
    }

    fclose(fp);

    marker_info = (ARMultiMarkerInfoT *)malloc( sizeof(ARMultiMarkerInfoT) );
    if( marker_info == NULL ) {free(marker); return NULL;}
    marker_info->marker     = marker;
    marker_info->marker_num = num;
    marker_info->prevF      = 0;

    return marker_info;
}
