/*
paramChangeSize
paramDecomp
paramDisp
paramDistortion
paramFile
paramGet
*/

// AR/param.h
// AR/matrix.h

#include <cstdio>
#include <cmath>
#include <stdarg.h>

///
#include <avrParameters.h>
#include <avrMath.h>
#include <avrUtil.h>
///

#define  PD_LOOP   3

#define  AR_PARAM_CDMIN      12

#define   AR_PARAM_NMIN         6
#define   AR_PARAM_NMAX      1000
#define   AR_PARAM_C34        100.0

ARParam    arParam;
ARSParam   arsParam;

static int arGetLine2(int x_coord[], int y_coord[], int coord_num,
                      int vertex[], double line[4][3], double v[4][2], double *dist_factor);
static double norm( double a, double b, double c );
static double dot( double a1, double a2, double a3, double b1, double b2, double b3 );

static int  arParamDecomp( ARParam *source, ARParam *icpara, double trans[3][4] );


int arInitCparam( ARParam *param )
{
    arImXsize = param->xsize;
    arImYsize = param->ysize;
    arParam = *param;

    return(0);
}

int arGetLine(int x_coord[], int y_coord[], int coord_num,
              int vertex[], double line[4][3], double v[4][2])
{
    return arGetLine2( x_coord, y_coord, coord_num, vertex, line, v, arParam.dist_factor );
}

int arsGetLine(int x_coord[], int y_coord[], int coord_num,
               int vertex[], double line[4][3], double v[4][2], int LorR)
{
    if( LorR )
        return arGetLine2( x_coord, y_coord, coord_num, vertex, line, v, arsParam.dist_factorL );
    else
        return arGetLine2( x_coord, y_coord, coord_num, vertex, line, v, arsParam.dist_factorR );
}

static int arGetLine2(int x_coord[], int y_coord[], int coord_num,
                      int vertex[], double line[4][3], double v[4][2], double *dist_factor)
{
    ARMat    *input, *evec;
    ARVec    *ev, *mean;
    double   w1;
    int      st, ed, n;
    int      i, j;

    ev     = arVecAlloc( 2 );
    mean   = arVecAlloc( 2 );
    evec   = arMatrixAlloc( 2, 2 );
    for( i = 0; i < 4; i++ ) {
        w1 = (double)(vertex[i+1]-vertex[i]+1) * 0.05 + 0.5;
        st = (int)(vertex[i]   + w1);
        ed = (int)(vertex[i+1] - w1);
        n = ed - st + 1;
        input  = arMatrixAlloc( n, 2 );
        for( j = 0; j < n; j++ ) {
            arParamObserv2Ideal( dist_factor, x_coord[st+j], y_coord[st+j],
                                 &(input->m[j*2+0]), &(input->m[j*2+1]) );
        }
        if( arMatrixPCA(input, evec, ev, mean) < 0 ) {
            arMatrixFree( input );
            arMatrixFree( evec );
            arVecFree( mean );
            arVecFree( ev );
            return(-1);
        }
        line[i][0] =  evec->m[1];
        line[i][1] = -evec->m[0];
        line[i][2] = -(line[i][0]*mean->v[0] + line[i][1]*mean->v[1]);
        arMatrixFree( input );
    }
    arMatrixFree( evec );
    arVecFree( mean );
    arVecFree( ev );

    for( i = 0; i < 4; i++ ) {
        w1 = line[(i+3)%4][0] * line[i][1] - line[i][0] * line[(i+3)%4][1];
        if( w1 == 0.0 ) return(-1);
        v[i][0] = (  line[(i+3)%4][1] * line[i][2]
                   - line[i][1] * line[(i+3)%4][2] ) / w1;
        v[i][1] = (  line[i][0] * line[(i+3)%4][2]
                   - line[(i+3)%4][0] * line[i][2] ) / w1;
    }

    return(0);
}

int arParamChangeSize( ARParam *source, int xsize, int ysize, ARParam *newparam )
{
    double  scale;
    int     i;

    newparam->xsize = xsize;
    newparam->ysize = ysize;

    scale = (double)xsize / (double)(source->xsize);
    for( i = 0; i < 4; i++ ) {
        newparam->mat[0][i] = source->mat[0][i] * scale;
        newparam->mat[1][i] = source->mat[1][i] * scale;
        newparam->mat[2][i] = source->mat[2][i];
    }

    newparam->dist_factor[0] = source->dist_factor[0] * scale;
    newparam->dist_factor[1] = source->dist_factor[1] * scale;
    newparam->dist_factor[2] = source->dist_factor[2] / (scale*scale);
    newparam->dist_factor[3] = source->dist_factor[3];

    return 0;
}

int  arParamDecomp( ARParam *source, ARParam *icpara, double trans[3][4] )
{
    icpara->xsize          = source->xsize;
    icpara->ysize          = source->ysize;
    icpara->dist_factor[0] = source->dist_factor[0];
    icpara->dist_factor[1] = source->dist_factor[1];
    icpara->dist_factor[2] = source->dist_factor[2];
    icpara->dist_factor[3] = source->dist_factor[3];

    return arParamDecompMat( source->mat, icpara->mat, trans );
}

int  arParamDecompMat( double source[3][4], double cpara[3][4], double trans[3][4] )
{
    int       r, c;
    double    Cpara[3][4];
    double    rem1, rem2, rem3;

    if( source[2][3] >= 0 ) {
        for( r = 0; r < 3; r++ ){
	    for( c = 0; c < 4; c++ ){
                Cpara[r][c] = source[r][c];
            }
        }
    }
    else {
        for( r = 0; r < 3; r++ ){
	    for( c = 0; c < 4; c++ ){
                Cpara[r][c] = -(source[r][c]);
            }
        }
    }

    for( r = 0; r < 3; r++ ){
	for( c = 0; c < 4; c++ ){
            cpara[r][c] = 0.0;
	}
    }
    cpara[2][2] = norm( Cpara[2][0], Cpara[2][1], Cpara[2][2] );
    trans[2][0] = Cpara[2][0] / cpara[2][2];
    trans[2][1] = Cpara[2][1] / cpara[2][2];
    trans[2][2] = Cpara[2][2] / cpara[2][2];
    trans[2][3] = Cpara[2][3] / cpara[2][2];

    cpara[1][2] = dot( trans[2][0], trans[2][1], trans[2][2],
                       Cpara[1][0], Cpara[1][1], Cpara[1][2] );
    rem1 = Cpara[1][0] - cpara[1][2] * trans[2][0];
    rem2 = Cpara[1][1] - cpara[1][2] * trans[2][1];
    rem3 = Cpara[1][2] - cpara[1][2] * trans[2][2];
    cpara[1][1] = norm( rem1, rem2, rem3 );
    trans[1][0] = rem1 / cpara[1][1];
    trans[1][1] = rem2 / cpara[1][1];
    trans[1][2] = rem3 / cpara[1][1];

    cpara[0][2] = dot( trans[2][0], trans[2][1], trans[2][2],
                       Cpara[0][0], Cpara[0][1], Cpara[0][2] );
    cpara[0][1] = dot( trans[1][0], trans[1][1], trans[1][2],
                       Cpara[0][0], Cpara[0][1], Cpara[0][2] );
    rem1 = Cpara[0][0] - cpara[0][1]*trans[1][0] - cpara[0][2]*trans[2][0];
    rem2 = Cpara[0][1] - cpara[0][1]*trans[1][1] - cpara[0][2]*trans[2][1];
    rem3 = Cpara[0][2] - cpara[0][1]*trans[1][2] - cpara[0][2]*trans[2][2];
    cpara[0][0] = norm( rem1, rem2, rem3 );
    trans[0][0] = rem1 / cpara[0][0];
    trans[0][1] = rem2 / cpara[0][0];
    trans[0][2] = rem3 / cpara[0][0];

    trans[1][3] = (Cpara[1][3] - cpara[1][2]*trans[2][3]) / cpara[1][1];
    trans[0][3] = (Cpara[0][3] - cpara[0][1]*trans[1][3]
                               - cpara[0][2]*trans[2][3]) / cpara[0][0];

    for( r = 0; r < 3; r++ ){
	for( c = 0; c < 3; c++ ){
            cpara[r][c] /= cpara[2][2];
	}
    }

    return 0;
}

static double norm( double a, double b, double c )
{
    return( sqrt( a*a + b*b + c*c ) );
}

static double dot( double a1, double a2, double a3, double b1, double b2, double b3 )
{
    return( a1 * b1 + a2 * b2 + a3 * b3 );
}

int arParamDisp( ARParam *param )
{
    int     i, j;

    printf("--------------------------------------\n");
    printf("SIZE = %d, %d\n", param->xsize, param->ysize);
    printf("Distortion factor = %f %f %f %f\n", param->dist_factor[0],
            param->dist_factor[1], param->dist_factor[2], param->dist_factor[3] );
    for( j = 0; j < 3; j++ ) {
        for( i = 0; i < 4; i++ ) printf("%7.5f ", param->mat[j][i]);
        printf("\n");
    }
    printf("--------------------------------------\n");

    return 0;
}

int arParamObserv2Ideal( const double dist_factor[4], const double ox, const double oy,
                         double *ix, double *iy )
{
    double  z02, z0, p, q, z, px, py;
    int     i;

    px = ox - dist_factor[0];
    py = oy - dist_factor[1];
    p = dist_factor[2]/100000000.0;
    z02 = px*px+ py*py;
    q = z0 = sqrt(px*px+ py*py);

    for( i = 1; ; i++ ) {
        if( z0 != 0.0 ) {
            z = z0 - ((1.0 - p*z02)*z0 - q) / (1.0 - 3.0*p*z02);
            px = px * z / z0;
            py = py * z / z0;
        }
        else {
            px = 0.0;
            py = 0.0;
            break;
        }
        if( i == PD_LOOP ) break;

        z02 = px*px+ py*py;
        z0 = sqrt(px*px+ py*py);
    }

    *ix = px / dist_factor[3] + dist_factor[0];
    *iy = py / dist_factor[3] + dist_factor[1];

    return(0);
}

int arParamIdeal2Observ( const double dist_factor[4], const double ix, const double iy,
                         double *ox, double *oy )
{
    double    x, y, d;

    x = (ix - dist_factor[0]) * dist_factor[3];
    y = (iy - dist_factor[1]) * dist_factor[3];
    if( x == 0.0 && y == 0.0 ) {
        *ox = dist_factor[0];
        *oy = dist_factor[1];
    }
    else {
        d = 1.0 - dist_factor[2]/100000000.0 * (x*x+y*y);
        *ox = x * d + dist_factor[0];
        *oy = y * d + dist_factor[1];
    }

    return(0);
}

int  arParamGet( double global[][3], double screen[][2], int num, double mat[3][4] )
{
    ARMat     *mat_a, *mat_at, *mat_r, mat_cpara;
    ARMat     *mat_wm1, *mat_wm2;
    double    *pa1, *pa2, *pr;                        /* working pointer */
    int       i;                                      /* working variables */

    if(num < AR_PARAM_NMIN) return( -1 );
    if(num > AR_PARAM_NMAX) return( -1 );

    mat_a = arMatrixAlloc( 2*num, AR_PARAM_CDMIN-1 );
    if( mat_a == NULL ) {
        return -1;
    }
    mat_at = arMatrixAlloc( AR_PARAM_CDMIN-1, 2*num );
    if( mat_at == NULL ) {
        arMatrixFree( mat_a );
        return -1;
    }
    mat_r = arMatrixAlloc( 2*num, 1 );
    if( mat_r == NULL ) {
        arMatrixFree( mat_a );
        arMatrixFree( mat_at );
        return -1;
    }
    mat_wm1 = arMatrixAlloc( AR_PARAM_CDMIN-1, AR_PARAM_CDMIN-1 );
    if( mat_wm1 == NULL ) {
        arMatrixFree( mat_a );
        arMatrixFree( mat_at );
        arMatrixFree( mat_r );
        return -1;
    }
    mat_wm2 = arMatrixAlloc( AR_PARAM_CDMIN-1, 2*num );
    if( mat_wm2 == NULL ) {
        arMatrixFree( mat_a );
        arMatrixFree( mat_at );
        arMatrixFree( mat_r );
        arMatrixFree( mat_wm1 );
        return -1;
    }

    /* Initializing array */
    pa1 = mat_a->m;
    for(i = 0; i < 2 * num * (AR_PARAM_CDMIN-1); i++) *pa1++ = 0.0;

    /* Calculate A,R matrix */
    for(i = 0, pr = mat_r->m; i < num; i++) {
        pa1 = &(mat_a->m[ (2*i)   * (AR_PARAM_CDMIN-1)    ]);
        pa2 = &(mat_a->m[ (2*i+1) * (AR_PARAM_CDMIN-1) + 4]);
        *pa1++ = global[i][0]; *pa1++ = global[i][1];
        *pa1++ = global[i][2]; *pa1++  = 1.0;
        *pa2++ = global[i][0]; *pa2++ = global[i][1];
        *pa2++ = global[i][2]; *pa2++ = 1.0;
        pa1 += 4;
        *pa1++ = -global[i][0] * screen[i][0];
        *pa1++ = -global[i][1] * screen[i][0];
        *pa1   = -global[i][2] * screen[i][0];
        *pa2++ = -global[i][0] * screen[i][1];
        *pa2++ = -global[i][1] * screen[i][1];
        *pa2   = -global[i][2] * screen[i][1];

        *pr++  = screen[i][0] * AR_PARAM_C34;
        *pr++  = screen[i][1] * AR_PARAM_C34;
    }

    if( arMatrixTrans( mat_at, mat_a ) < 0 ) {
        arMatrixFree( mat_a );
        arMatrixFree( mat_at );
        arMatrixFree( mat_r );
        arMatrixFree( mat_wm1 );
        arMatrixFree( mat_wm2 );
        return -1;
    }
    if( arMatrixMul( mat_wm1, mat_at, mat_a ) < 0 ) {
        arMatrixFree( mat_a );
        arMatrixFree( mat_at );
        arMatrixFree( mat_r );
        arMatrixFree( mat_wm1 );
        arMatrixFree( mat_wm2 );
        return -1;
    }
    if( arMatrixSelfInv( mat_wm1 ) < 0 ) {
        arMatrixFree( mat_a );
        arMatrixFree( mat_at );
        arMatrixFree( mat_r );
        arMatrixFree( mat_wm1 );
        arMatrixFree( mat_wm2 );
        return -1;
    }
    if( arMatrixMul( mat_wm2, mat_wm1, mat_at ) < 0 ) {
        arMatrixFree( mat_a );
        arMatrixFree( mat_at );
        arMatrixFree( mat_r );
        arMatrixFree( mat_wm1 );
        arMatrixFree( mat_wm2 );
        return -1;
    }
    mat_cpara.row = AR_PARAM_CDMIN-1;
    mat_cpara.clm = 1;
    mat_cpara.m = &(mat[0][0]);
    if( arMatrixMul( &mat_cpara, mat_wm2, mat_r ) < 0 ) {
        arMatrixFree( mat_a );
        arMatrixFree( mat_at );
        arMatrixFree( mat_r );
        arMatrixFree( mat_wm1 );
        arMatrixFree( mat_wm2 );
        return -1;
    }
    mat[2][3] = AR_PARAM_C34;

    arMatrixFree( mat_a );
    arMatrixFree( mat_at );
    arMatrixFree( mat_r );
    arMatrixFree( mat_wm1 );
    arMatrixFree( mat_wm2 );
    return 0;
}

#ifdef AR_LITTLE_ENDIAN
typedef union {
	int  x;
	unsigned char y[4];
} SwapIntT;

typedef union {
	double  x;
	unsigned char y[8];
} SwapDoubleT;

static void byteSwapInt( int *from, int *to )
{
    SwapIntT   *w1, *w2;
    int        i;

    w1 = (SwapIntT *)from;
    w2 = (SwapIntT *)to;
    for( i = 0; i < 4; i++ ) {
        w2->y[i] = w1->y[3-i];
    }

    return;
}

static void byteSwapDouble( double *from, double *to )
{
    SwapDoubleT   *w1, *w2;
    int           i;

    w1 = (SwapDoubleT *)from;
    w2 = (SwapDoubleT *)to;
    for( i = 0; i < 8; i++ ) {
        w2->y[i] = w1->y[7-i];
    }

    return;
}

static void byteswap( ARParam *param )
{
    ARParam  wparam;
    int      i, j;

    byteSwapInt( &(param->xsize), &(wparam.xsize) );
    byteSwapInt( &(param->ysize), &(wparam.ysize) );

    for( j = 0; j < 3; j++ ) {
        for( i = 0; i < 4; i++ ) {
            byteSwapDouble( &(param->mat[j][i]), &(wparam.mat[j][i]) );
        }
    }

    for( i = 0; i < 4; i++ ) {
        byteSwapDouble( &(param->dist_factor[i]), &(wparam.dist_factor[i]) );
    }

    *param = wparam;
}

static void byteswap2( ARSParam *sparam )
{
    ARSParam wsparam;
    int      i, j;

    byteSwapInt( &(sparam->xsize), &(wsparam.xsize) );
    byteSwapInt( &(sparam->ysize), &(wsparam.ysize) );

    for( j = 0; j < 3; j++ ) {
        for( i = 0; i < 4; i++ ) {
            byteSwapDouble( &(sparam->matL[j][i]),   &(wsparam.matL[j][i]) );
            byteSwapDouble( &(sparam->matR[j][i]),   &(wsparam.matR[j][i]) );
            byteSwapDouble( &(sparam->matL2R[j][i]), &(wsparam.matL2R[j][i]) );
        }
    }
    for( i = 0; i < 4; i++ ) {
        byteSwapDouble( &(sparam->dist_factorL[i]), &(wsparam.dist_factorL[i]) );
        byteSwapDouble( &(sparam->dist_factorR[i]), &(wsparam.dist_factorR[i]) );
    }

    *sparam = wsparam;
}
#endif


int    arParamSave( char *filename, int num, ARParam *param, ...)
{
    FILE        *fp;
    va_list     ap;
    ARParam     *param1;
    int         i;

    if( num < 1 ) return -1;

    fp = fopen( filename, "wb" );
    if( fp == NULL ) return -1;

#ifdef AR_LITTLE_ENDIAN
    byteswap( param );
#endif
    if( fwrite( param, sizeof(ARParam), 1, fp ) != 1 ) {
        fclose(fp);
#ifdef AR_LITTLE_ENDIAN
        byteswap( param );
#endif
        return -1;
    }
#ifdef AR_LITTLE_ENDIAN
    byteswap( param );
#endif

    va_start(ap, param);
    for( i = 1; i < num; i++ ) {
        param1 = va_arg(ap, ARParam *);
#ifdef AR_LITTLE_ENDIAN
        byteswap( param1 );
#endif
        if( fwrite( param1, sizeof(ARParam), 1, fp ) != 1 ) {
            fclose(fp);
#ifdef AR_LITTLE_ENDIAN
            byteswap( param1 );
#endif
            return -1;
        }
#ifdef AR_LITTLE_ENDIAN
        byteswap( param1 );
#endif
    }

    fclose(fp);

    return 0;
}

int    arParamLoad( const char *filename, int num, ARParam *param, ...)
{
    FILE        *fp;
    va_list     ap;
    ARParam     *param1;
    int         i;

    if( num < 1 ) return -1;

    fp = fopen( filename, "rb" );
    if( fp == NULL ) return -1;

    if( fread( param, sizeof(ARParam), 1, fp ) != 1 ) {
        fclose(fp);
        return -1;
    }
#ifdef AR_LITTLE_ENDIAN
    byteswap( param );
#endif

    va_start(ap, param);
    for( i = 1; i < num; i++ ) {
        param1 = va_arg(ap, ARParam *);
        if( fread( param1, sizeof(ARParam), 1, fp ) != 1 ) {
            fclose(fp);
            return -1;
        }
#ifdef AR_LITTLE_ENDIAN
        byteswap( param1 );
#endif
    }

    fclose(fp);

    return 0;
}

