/*
arGetTransMat
arGetTransMat2
arGetTransMat3
arGetTransMatCont
arMultiGetTransMat
mDet
mInv
mMul
mPCA
mSelfInv
mTrans
vHouse
vInnerP
vTridiag
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
//#include <AR/ar.h>
//#include <AR/matrix.h>
//#include <AR/arMulti.h>

///
#include <avrGraphics.h>   // << inclui avrParameters que inclui avrUtil
#include <avrMath.h>       // << inclui avrUtil
///

#define  debug  0

#define  THRESH_1            2.0
#define  THRESH_2           20.0
#define  THRESH_3           10.0
#define  AR_MULTI_GET_TRANS_MAT_MAX_LOOP_COUNT   2
#define  AR_MULTI_GET_TRANS_MAT_MAX_FIT_ERROR    10.0

#define CHECK_CALC 0

#define MD_PI         3.14159265358979323846

#define P_MAX       500

#define MATRIX(name,x,y,width)  ( *(name + (width) * (x) + (y)) )

#define     VZERO           1e-16
#define     EPS             1e-6
#define     MAX_ITER        100
#define     xmalloc(V,T,S)  { if( ((V) = (T *)malloc( sizeof(T) * (S) ))\
                               == NULL ) {printf("malloc error\n"); exit(1);} }

static double   pos2d[P_MAX][2];
static double   pos3d[P_MAX][3];

typedef struct {
    double   pos[4][2];
    double   thresh;
    double   err;
    int      marker;
    int      dir;
} arMultiEachMarkerInternalInfoT;

static double arGetTransMatSub( double rot[3][3], double ppos2d[][2],
                                double pos3d[][3], int num, double conv[3][4],
                                double *dist_factor, double cpara[3][4] );

double avrGetTransMat( avrPatternInfo *marker_info, double center[2], double width, avrMatrix3x4& conv )
{
    double  rot[3][3];
    double  ppos2d[4][2];
    double  ppos3d[4][2];
    int     dir;
    double  err;
    int     i;

    if( avrGetInitRot( marker_info, arParam.mat, rot ) < 0 ) return -1;

    dir = marker_info->dir;
    //printf("%d\n", dir);
    ppos2d[0][0] = marker_info->vertex[(4-dir)%4][0];
    ppos2d[0][1] = marker_info->vertex[(4-dir)%4][1];
    ppos2d[1][0] = marker_info->vertex[(5-dir)%4][0];
    ppos2d[1][1] = marker_info->vertex[(5-dir)%4][1];
    ppos2d[2][0] = marker_info->vertex[(6-dir)%4][0];
    ppos2d[2][1] = marker_info->vertex[(6-dir)%4][1];
    ppos2d[3][0] = marker_info->vertex[(7-dir)%4][0];
    ppos2d[3][1] = marker_info->vertex[(7-dir)%4][1];
    ppos3d[0][0] = center[0] - width/2.0;
    ppos3d[0][1] = center[1] + width/2.0;
    ppos3d[1][0] = center[0] + width/2.0;
    ppos3d[1][1] = center[1] + width/2.0;
    ppos3d[2][0] = center[0] + width/2.0;
    ppos3d[2][1] = center[1] - width/2.0;
    ppos3d[3][0] = center[0] - width/2.0;
    ppos3d[3][1] = center[1] - width/2.0;

    for( i = 0; i < AR_GET_TRANS_MAT_MAX_LOOP_COUNT; i++ ) {
        err = avrGetTransMat3( rot, ppos2d, ppos3d, 4, conv,
                                   arParam.dist_factor, arParam.mat );
        if( err < AR_GET_TRANS_MAT_MAX_FIT_ERROR ) break;
    }
    return err;
}

static double avrGetTransMatSub( double rot[3][3], double ppos2d[][2], double pos3d[][3], int num, avrMatrix3x4& conv,
                                double *dist_factor, double cpara[3][4] );

double avrGetTransMat3( double rot[3][3], double ppos2d[][2],
                        double ppos3d[][2], int num, avrMatrix3x4& conv,
                        double *dist_factor, double cpara[3][4] )
{
   double  off[3], pmax[3], pmin[3];
    double  ret;
    int     i;

    pmax[0]=pmax[1]=pmax[2] = -10000000000.0;
    pmin[0]=pmin[1]=pmin[2] =  10000000000.0;
    for( i = 0; i < num; i++ ) {
        if( ppos3d[i][0] > pmax[0] ) pmax[0] = ppos3d[i][0];
        if( ppos3d[i][0] < pmin[0] ) pmin[0] = ppos3d[i][0];
        if( ppos3d[i][1] > pmax[1] ) pmax[1] = ppos3d[i][1];
        if( ppos3d[i][1] < pmin[1] ) pmin[1] = ppos3d[i][1];
    }
    off[0] = -(pmax[0] + pmin[0]) / 2.0;
    off[1] = -(pmax[1] + pmin[1]) / 2.0;
    off[2] = -(pmax[2] + pmin[2]) / 2.0;
    for( i = 0; i < num; i++ ) {
        pos3d[i][0] = ppos3d[i][0] + off[0];
        pos3d[i][1] = ppos3d[i][1] + off[1];

        pos3d[i][2] = 0.0;
    }

    ret = avrGetTransMatSub( rot, ppos2d, pos3d, num, conv,
                            dist_factor, cpara );

    double value0 = conv.access(0,0)*off[0] + conv.access(0,1)*off[1] + conv.access(0,2)*off[2] + conv.access(0,3);
    double value1 = conv.access(1,0)*off[0] + conv.access(1,1)*off[1] + conv.access(1,2)*off[2] + conv.access(1,3);
    double value2 = conv.access(2,0)*off[0] + conv.access(2,1)*off[1] + conv.access(2,2)*off[2] + conv.access(2,3);

    conv.add(value0,0,3);
    conv.add(value1,1,3);
    conv.add(value2,2,3);

    return ret;
}

static double avrGetTransMatSub( double rot[3][3], double ppos2d[][2],
                                 double pos3d[][3], int num, avrMatrix3x4& conv,
                                 double *dist_factor, double cpara[3][4] )
{
    ARMat   *mat_a, *mat_b, *mat_c, *mat_d, *mat_e, *mat_f;
    double  trans[3];
    double  wx, wy, wz;
    double  ret;
    int     i, j;

    mat_a = arMatrixAlloc( num*2, 3 );
    mat_b = arMatrixAlloc( 3, num*2 );
    mat_c = arMatrixAlloc( num*2, 1 );
    mat_d = arMatrixAlloc( 3, 3 );
    mat_e = arMatrixAlloc( 3, 1 );
    mat_f = arMatrixAlloc( 3, 1 );

    if( arFittingMode == AR_FITTING_TO_INPUT ) {
        for( i = 0; i < num; i++ ) {
            arParamIdeal2Observ(dist_factor, ppos2d[i][0], ppos2d[i][1],
                                             &pos2d[i][0], &pos2d[i][1]);
        }
    }
    else {
        for( i = 0; i < num; i++ ) {
            pos2d[i][0] = ppos2d[i][0];
            pos2d[i][1] = ppos2d[i][1];
        }
    }

    for( j = 0; j < num; j++ ) {
        wx = rot[0][0] * pos3d[j][0]
           + rot[0][1] * pos3d[j][1]
           + rot[0][2] * pos3d[j][2];
        wy = rot[1][0] * pos3d[j][0]
           + rot[1][1] * pos3d[j][1]
           + rot[1][2] * pos3d[j][2];
        wz = rot[2][0] * pos3d[j][0]
           + rot[2][1] * pos3d[j][1]
           + rot[2][2] * pos3d[j][2];
        mat_a->m[j*6+0] = mat_b->m[num*0+j*2] = cpara[0][0];
        mat_a->m[j*6+1] = mat_b->m[num*2+j*2] = cpara[0][1];
        mat_a->m[j*6+2] = mat_b->m[num*4+j*2] = cpara[0][2] - pos2d[j][0];
        mat_c->m[j*2+0] = wz * pos2d[j][0]
               - cpara[0][0]*wx - cpara[0][1]*wy - cpara[0][2]*wz;
        mat_a->m[j*6+3] = mat_b->m[num*0+j*2+1] = 0.0;
        mat_a->m[j*6+4] = mat_b->m[num*2+j*2+1] = cpara[1][1];
        mat_a->m[j*6+5] = mat_b->m[num*4+j*2+1] = cpara[1][2] - pos2d[j][1];
        mat_c->m[j*2+1] = wz * pos2d[j][1]
               - cpara[1][1]*wy - cpara[1][2]*wz;
    }
    arMatrixMul( mat_d, mat_b, mat_a );
    arMatrixMul( mat_e, mat_b, mat_c );
    arMatrixSelfInv( mat_d );
    arMatrixMul( mat_f, mat_d, mat_e );
    trans[0] = mat_f->m[0];
    trans[1] = mat_f->m[1];
    trans[2] = mat_f->m[2];

    ret = arModifyMatrix( rot, trans, cpara, pos3d, pos2d, num );

    for( j = 0; j < num; j++ ) {
        wx = rot[0][0] * pos3d[j][0]
           + rot[0][1] * pos3d[j][1]
           + rot[0][2] * pos3d[j][2];
        wy = rot[1][0] * pos3d[j][0]
           + rot[1][1] * pos3d[j][1]
           + rot[1][2] * pos3d[j][2];
        wz = rot[2][0] * pos3d[j][0]
           + rot[2][1] * pos3d[j][1]
           + rot[2][2] * pos3d[j][2];
        mat_a->m[j*6+0] = mat_b->m[num*0+j*2] = cpara[0][0];
        mat_a->m[j*6+1] = mat_b->m[num*2+j*2] = cpara[0][1];
        mat_a->m[j*6+2] = mat_b->m[num*4+j*2] = cpara[0][2] - pos2d[j][0];
        mat_c->m[j*2+0] = wz * pos2d[j][0]
               - cpara[0][0]*wx - cpara[0][1]*wy - cpara[0][2]*wz;
        mat_a->m[j*6+3] = mat_b->m[num*0+j*2+1] = 0.0;
        mat_a->m[j*6+4] = mat_b->m[num*2+j*2+1] = cpara[1][1];
        mat_a->m[j*6+5] = mat_b->m[num*4+j*2+1] = cpara[1][2] - pos2d[j][1];
        mat_c->m[j*2+1] = wz * pos2d[j][1]
               - cpara[1][1]*wy - cpara[1][2]*wz;
    }
    arMatrixMul( mat_d, mat_b, mat_a );
    arMatrixMul( mat_e, mat_b, mat_c );
    arMatrixSelfInv( mat_d );
    arMatrixMul( mat_f, mat_d, mat_e );
    trans[0] = mat_f->m[0];
    trans[1] = mat_f->m[1];
    trans[2] = mat_f->m[2];

    ret = arModifyMatrix( rot, trans, cpara, pos3d, pos2d, num );

    arMatrixFree( mat_a );
    arMatrixFree( mat_b );
    arMatrixFree( mat_c );
    arMatrixFree( mat_d );
    arMatrixFree( mat_e );
    arMatrixFree( mat_f );

    for( j = 0; j < 3; j++ ) {
        for( i = 0; i < 3; i++ ) conv.add(rot[j][i],j,i);
        conv.add(trans[j],j,3);
    }

    return ret;
}


double arGetTransMat( ARMarkerInfo *marker_info,
                      double center[2], double width, double conv[3][4] )
{
    double  rot[3][3];
    double  ppos2d[4][2];
    double  ppos3d[4][2];
    int     dir;
    double  err;
    int     i;

    if( arGetInitRot( marker_info, arParam.mat, rot ) < 0 ) return -1;

    dir = marker_info->dir;
    ppos2d[0][0] = marker_info->vertex[(4-dir)%4][0];
    ppos2d[0][1] = marker_info->vertex[(4-dir)%4][1];
    ppos2d[1][0] = marker_info->vertex[(5-dir)%4][0];
    ppos2d[1][1] = marker_info->vertex[(5-dir)%4][1];
    ppos2d[2][0] = marker_info->vertex[(6-dir)%4][0];
    ppos2d[2][1] = marker_info->vertex[(6-dir)%4][1];
    ppos2d[3][0] = marker_info->vertex[(7-dir)%4][0];
    ppos2d[3][1] = marker_info->vertex[(7-dir)%4][1];
    ppos3d[0][0] = center[0] - width/2.0;
    ppos3d[0][1] = center[1] + width/2.0;
    ppos3d[1][0] = center[0] + width/2.0;
    ppos3d[1][1] = center[1] + width/2.0;
    ppos3d[2][0] = center[0] + width/2.0;
    ppos3d[2][1] = center[1] - width/2.0;
    ppos3d[3][0] = center[0] - width/2.0;
    ppos3d[3][1] = center[1] - width/2.0;

    for( i = 0; i < AR_GET_TRANS_MAT_MAX_LOOP_COUNT; i++ ) {
        err = arGetTransMat3( rot, ppos2d, ppos3d, 4, conv,
                                   arParam.dist_factor, arParam.mat );
        if( err < AR_GET_TRANS_MAT_MAX_FIT_ERROR ) break;
    }
    return err;
}

double arGetTransMat2( double rot[3][3], double ppos2d[][2],
                   double ppos3d[][2], int num, double conv[3][4] )
{
    return arGetTransMat3( rot, ppos2d, ppos3d, num, conv,
                           arParam.dist_factor, arParam.mat );
}

double arGetTransMat3( double rot[3][3], double ppos2d[][2],
                       double ppos3d[][2], int num, double conv[3][4],
                       double *dist_factor, double cpara[3][4] )
{
    double  off[3], pmax[3], pmin[3];
    double  ret;
    int     i;

    pmax[0]=pmax[1]=pmax[2] = -10000000000.0;
    pmin[0]=pmin[1]=pmin[2] =  10000000000.0;
    for( i = 0; i < num; i++ ) {
        if( ppos3d[i][0] > pmax[0] ) pmax[0] = ppos3d[i][0];
        if( ppos3d[i][0] < pmin[0] ) pmin[0] = ppos3d[i][0];
        if( ppos3d[i][1] > pmax[1] ) pmax[1] = ppos3d[i][1];
        if( ppos3d[i][1] < pmin[1] ) pmin[1] = ppos3d[i][1];
/*
        if( ppos3d[i][2] > pmax[2] ) pmax[2] = ppos3d[i][2];
        if( ppos3d[i][2] < pmin[2] ) pmin[2] = ppos3d[i][2];
*/
    }
    off[0] = -(pmax[0] + pmin[0]) / 2.0;
    off[1] = -(pmax[1] + pmin[1]) / 2.0;
    off[2] = -(pmax[2] + pmin[2]) / 2.0;
    for( i = 0; i < num; i++ ) {
        pos3d[i][0] = ppos3d[i][0] + off[0];
        pos3d[i][1] = ppos3d[i][1] + off[1];
/*
        pos3d[i][2] = ppos3d[i][2] + off[2];
*/
        pos3d[i][2] = 0.0;
    }

    ret = arGetTransMatSub( rot, ppos2d, pos3d, num, conv,
                            dist_factor, cpara );

    conv[0][3] = conv[0][0]*off[0] + conv[0][1]*off[1] + conv[0][2]*off[2] + conv[0][3];
    conv[1][3] = conv[1][0]*off[0] + conv[1][1]*off[1] + conv[1][2]*off[2] + conv[1][3];
    conv[2][3] = conv[2][0]*off[0] + conv[2][1]*off[1] + conv[2][2]*off[2] + conv[2][3];

    return ret;
}

double arGetTransMat4( double rot[3][3], double ppos2d[][2],
                       double ppos3d[][3], int num, double conv[3][4] )
{
    return arGetTransMat5( rot, ppos2d, ppos3d, num, conv,
                           arParam.dist_factor, arParam.mat );
}

double arGetTransMat5( double rot[3][3], double ppos2d[][2],
                       double ppos3d[][3], int num, double conv[3][4],
                       double *dist_factor, double cpara[3][4] )
{
    double  off[3], pmax[3], pmin[3];
    double  ret;
    int     i;

    pmax[0]=pmax[1]=pmax[2] = -10000000000.0;
    pmin[0]=pmin[1]=pmin[2] =  10000000000.0;
    for( i = 0; i < num; i++ ) {
        if( ppos3d[i][0] > pmax[0] ) pmax[0] = ppos3d[i][0];
        if( ppos3d[i][0] < pmin[0] ) pmin[0] = ppos3d[i][0];
        if( ppos3d[i][1] > pmax[1] ) pmax[1] = ppos3d[i][1];
        if( ppos3d[i][1] < pmin[1] ) pmin[1] = ppos3d[i][1];
        if( ppos3d[i][2] > pmax[2] ) pmax[2] = ppos3d[i][2];
        if( ppos3d[i][2] < pmin[2] ) pmin[2] = ppos3d[i][2];
    }
    off[0] = -(pmax[0] + pmin[0]) / 2.0;
    off[1] = -(pmax[1] + pmin[1]) / 2.0;
    off[2] = -(pmax[2] + pmin[2]) / 2.0;
    for( i = 0; i < num; i++ ) {
        pos3d[i][0] = ppos3d[i][0] + off[0];
        pos3d[i][1] = ppos3d[i][1] + off[1];
        pos3d[i][2] = ppos3d[i][2] + off[2];
    }

    ret = arGetTransMatSub( rot, ppos2d, pos3d, num, conv,
                            dist_factor, cpara );

    conv[0][3] = conv[0][0]*off[0] + conv[0][1]*off[1] + conv[0][2]*off[2] + conv[0][3];
    conv[1][3] = conv[1][0]*off[0] + conv[1][1]*off[1] + conv[1][2]*off[2] + conv[1][3];
    conv[2][3] = conv[2][0]*off[0] + conv[2][1]*off[1] + conv[2][2]*off[2] + conv[2][3];

    return ret;
}

static double arGetTransMatSub( double rot[3][3], double ppos2d[][2],
                                double pos3d[][3], int num, double conv[3][4],
                                double *dist_factor, double cpara[3][4] )
{
    ARMat   *mat_a, *mat_b, *mat_c, *mat_d, *mat_e, *mat_f;
    double  trans[3];
    double  wx, wy, wz;
    double  ret;
    int     i, j;

    mat_a = arMatrixAlloc( num*2, 3 );
    mat_b = arMatrixAlloc( 3, num*2 );
    mat_c = arMatrixAlloc( num*2, 1 );
    mat_d = arMatrixAlloc( 3, 3 );
    mat_e = arMatrixAlloc( 3, 1 );
    mat_f = arMatrixAlloc( 3, 1 );

    if( arFittingMode == AR_FITTING_TO_INPUT ) {
        for( i = 0; i < num; i++ ) {
            arParamIdeal2Observ(dist_factor, ppos2d[i][0], ppos2d[i][1],
                                             &pos2d[i][0], &pos2d[i][1]);
        }
    }
    else {
        for( i = 0; i < num; i++ ) {
            pos2d[i][0] = ppos2d[i][0];
            pos2d[i][1] = ppos2d[i][1];
        }
    }

    for( j = 0; j < num; j++ ) {
        wx = rot[0][0] * pos3d[j][0]
           + rot[0][1] * pos3d[j][1]
           + rot[0][2] * pos3d[j][2];
        wy = rot[1][0] * pos3d[j][0]
           + rot[1][1] * pos3d[j][1]
           + rot[1][2] * pos3d[j][2];
        wz = rot[2][0] * pos3d[j][0]
           + rot[2][1] * pos3d[j][1]
           + rot[2][2] * pos3d[j][2];
        mat_a->m[j*6+0] = mat_b->m[num*0+j*2] = cpara[0][0];
        mat_a->m[j*6+1] = mat_b->m[num*2+j*2] = cpara[0][1];
        mat_a->m[j*6+2] = mat_b->m[num*4+j*2] = cpara[0][2] - pos2d[j][0];
        mat_c->m[j*2+0] = wz * pos2d[j][0]
               - cpara[0][0]*wx - cpara[0][1]*wy - cpara[0][2]*wz;
        mat_a->m[j*6+3] = mat_b->m[num*0+j*2+1] = 0.0;
        mat_a->m[j*6+4] = mat_b->m[num*2+j*2+1] = cpara[1][1];
        mat_a->m[j*6+5] = mat_b->m[num*4+j*2+1] = cpara[1][2] - pos2d[j][1];
        mat_c->m[j*2+1] = wz * pos2d[j][1]
               - cpara[1][1]*wy - cpara[1][2]*wz;
    }
    arMatrixMul( mat_d, mat_b, mat_a );
    arMatrixMul( mat_e, mat_b, mat_c );
    arMatrixSelfInv( mat_d );
    arMatrixMul( mat_f, mat_d, mat_e );
    trans[0] = mat_f->m[0];
    trans[1] = mat_f->m[1];
    trans[2] = mat_f->m[2];

    ret = arModifyMatrix( rot, trans, cpara, pos3d, pos2d, num );

    for( j = 0; j < num; j++ ) {
        wx = rot[0][0] * pos3d[j][0]
           + rot[0][1] * pos3d[j][1]
           + rot[0][2] * pos3d[j][2];
        wy = rot[1][0] * pos3d[j][0]
           + rot[1][1] * pos3d[j][1]
           + rot[1][2] * pos3d[j][2];
        wz = rot[2][0] * pos3d[j][0]
           + rot[2][1] * pos3d[j][1]
           + rot[2][2] * pos3d[j][2];
        mat_a->m[j*6+0] = mat_b->m[num*0+j*2] = cpara[0][0];
        mat_a->m[j*6+1] = mat_b->m[num*2+j*2] = cpara[0][1];
        mat_a->m[j*6+2] = mat_b->m[num*4+j*2] = cpara[0][2] - pos2d[j][0];
        mat_c->m[j*2+0] = wz * pos2d[j][0]
               - cpara[0][0]*wx - cpara[0][1]*wy - cpara[0][2]*wz;
        mat_a->m[j*6+3] = mat_b->m[num*0+j*2+1] = 0.0;
        mat_a->m[j*6+4] = mat_b->m[num*2+j*2+1] = cpara[1][1];
        mat_a->m[j*6+5] = mat_b->m[num*4+j*2+1] = cpara[1][2] - pos2d[j][1];
        mat_c->m[j*2+1] = wz * pos2d[j][1]
               - cpara[1][1]*wy - cpara[1][2]*wz;
    }
    arMatrixMul( mat_d, mat_b, mat_a );
    arMatrixMul( mat_e, mat_b, mat_c );
    arMatrixSelfInv( mat_d );
    arMatrixMul( mat_f, mat_d, mat_e );
    trans[0] = mat_f->m[0];
    trans[1] = mat_f->m[1];
    trans[2] = mat_f->m[2];

    ret = arModifyMatrix( rot, trans, cpara, pos3d, pos2d, num );

    arMatrixFree( mat_a );
    arMatrixFree( mat_b );
    arMatrixFree( mat_c );
    arMatrixFree( mat_d );
    arMatrixFree( mat_e );
    arMatrixFree( mat_f );

    for( j = 0; j < 3; j++ ) {
        for( i = 0; i < 3; i++ ) conv[j][i] = rot[j][i];
        conv[j][3] = trans[j];
    }

    return ret;
}

double arModifyMatrix( double rot[3][3], double trans[3], double cpara[3][4],
                             double vertex[][3], double pos2d[][2], int num )
{
    double    factor;
    double    a, b, c;
    double    a1, b1, c1;
    double    a2, b2, c2;
    double    ma = 0.0, mb = 0.0, mc = 0.0;
    double    combo[3][4];
    double    hx, hy, h, x, y;
    double    err, minerr;
    int       t1, t2, t3;
    int       s1 = 0, s2 = 0, s3 = 0;
    int       i, j;

    arGetAngle( rot, &a, &b, &c );

    a2 = a;
    b2 = b;
    c2 = c;
    factor = 10.0*MD_PI/180.0;
    for( j = 0; j < 10; j++ ) {
        minerr = 1000000000.0;
        for(t1=-1;t1<=1;t1++) {
        for(t2=-1;t2<=1;t2++) {
        for(t3=-1;t3<=1;t3++) {
            a1 = a2 + factor*t1;
            b1 = b2 + factor*t2;
            c1 = c2 + factor*t3;
            arGetNewMatrix( a1, b1, c1, trans, NULL, cpara, combo );

            err = 0.0;
            for( i = 0; i < num; i++ ) {
                hx = combo[0][0] * vertex[i][0]
                   + combo[0][1] * vertex[i][1]
                   + combo[0][2] * vertex[i][2]
                   + combo[0][3];
                hy = combo[1][0] * vertex[i][0]
                   + combo[1][1] * vertex[i][1]
                   + combo[1][2] * vertex[i][2]
                   + combo[1][3];
                h  = combo[2][0] * vertex[i][0]
                   + combo[2][1] * vertex[i][1]
                   + combo[2][2] * vertex[i][2]
                   + combo[2][3];
                x = hx / h;
                y = hy / h;

                err += (pos2d[i][0] - x) * (pos2d[i][0] - x) + (pos2d[i][1] - y) * (pos2d[i][1] - y);
            }

            if( err < minerr ) {
                minerr = err;
                ma = a1;
                mb = b1;
                mc = c1;
                s1 = t1; s2 = t2; s3 = t3;
            }
        }
        }
        }

        if( s1 == 0 && s2 == 0 && s3 == 0 ) factor *= 0.5;
        a2 = ma;
        b2 = mb;
        c2 = mc;
    }

    arGetRot( ma, mb, mc, rot );

/*  printf("factor = %10.5f\n", factor*180.0/MD_PI); */

    return minerr/num;
}

double arsModifyMatrix( double rot[3][3], double trans[3], ARSParam *arsParam,
                        double pos3dL[][3], double pos2dL[][2], int numL,
                        double pos3dR[][3], double pos2dR[][2], int numR )
{
    double    factor;
    double    a, b, c;
    double    a1, b1, c1;
    double    a2, b2, c2;
    double    ma = 0.0, mb = 0.0, mc = 0.0;
    double    combo[3][4];
    double    hx, hy, h, x, y;
    double    err, minerr;
    int       t1, t2, t3;
    int       s1 = 0, s2 = 0, s3 = 0;
    int       i, j;

    arGetAngle( rot, &a, &b, &c );

    a2 = a;
    b2 = b;
    c2 = c;
    factor = 10.0*MD_PI/180.0;
    for( j = 0; j < 10; j++ ) {
        minerr = 1000000000.0;
        for(t1=-1;t1<=1;t1++) {
        for(t2=-1;t2<=1;t2++) {
        for(t3=-1;t3<=1;t3++) {
            a1 = a2 + factor*t1;
            b1 = b2 + factor*t2;
            c1 = c2 + factor*t3;
            err = 0.0;

            arGetNewMatrix( a1, b1, c1, trans, NULL, arsParam->matL, combo );
            for( i = 0; i < numL; i++ ) {
                hx = combo[0][0] * pos3dL[i][0]
                   + combo[0][1] * pos3dL[i][1]
                   + combo[0][2] * pos3dL[i][2]
                   + combo[0][3];
                hy = combo[1][0] * pos3dL[i][0]
                   + combo[1][1] * pos3dL[i][1]
                   + combo[1][2] * pos3dL[i][2]
                   + combo[1][3];
                h  = combo[2][0] * pos3dL[i][0]
                   + combo[2][1] * pos3dL[i][1]
                   + combo[2][2] * pos3dL[i][2]
                   + combo[2][3];
                x = hx / h;
                y = hy / h;
                err += (pos2dL[i][0] - x) * (pos2dL[i][0] - x)
                     + (pos2dL[i][1] - y) * (pos2dL[i][1] - y);
            }

            arGetNewMatrix( a1, b1, c1, trans, arsParam->matL2R, arsParam->matR, combo );
            for( i = 0; i < numR; i++ ) {
                hx = combo[0][0] * pos3dR[i][0]
                   + combo[0][1] * pos3dR[i][1]
                   + combo[0][2] * pos3dR[i][2]
                   + combo[0][3];
                hy = combo[1][0] * pos3dR[i][0]
                   + combo[1][1] * pos3dR[i][1]
                   + combo[1][2] * pos3dR[i][2]
                   + combo[1][3];
                h  = combo[2][0] * pos3dR[i][0]
                   + combo[2][1] * pos3dR[i][1]
                   + combo[2][2] * pos3dR[i][2]
                   + combo[2][3];
                x = hx / h;
                y = hy / h;

                err += (pos2dR[i][0] - x) * (pos2dR[i][0] - x)
                     + (pos2dR[i][1] - y) * (pos2dR[i][1] - y);
            }

            if( err < minerr ) {
                minerr = err;
                ma = a1;
                mb = b1;
                mc = c1;
                s1 = t1; s2 = t2; s3 = t3;
            }
        }
        }
        }

        if( s1 == 0 && s2 == 0 && s3 == 0 ) factor *= 0.5;
        a2 = ma;
        b2 = mb;
        c2 = mc;
    }

    arGetRot( ma, mb, mc, rot );

    return minerr / (numL+numR);
}

static int  check_rotation( double rot[2][3] );
static int  check_dir( double dir[3], double st[2], double ed[2],
                       double cpara[3][4] );

int arGetAngle( double rot[3][3], double *wa, double *wb, double *wc )
{
    double      a, b, c;
    double      sina, cosa, sinb, cosb, sinc, cosc;
#if CHECK_CALC
double   w[3];
int      i;
for(i=0;i<3;i++) w[i] = rot[i][0];
for(i=0;i<3;i++) rot[i][0] = rot[i][1];
for(i=0;i<3;i++) rot[i][1] = rot[i][2];
for(i=0;i<3;i++) rot[i][2] = w[i];
#endif

    if( rot[2][2] > 1.0 ) {
        /* printf("cos(beta) = %f\n", rot[2][2]); */
        rot[2][2] = 1.0;
    }
    else if( rot[2][2] < -1.0 ) {
        /* printf("cos(beta) = %f\n", rot[2][2]); */
        rot[2][2] = -1.0;
    }
    cosb = rot[2][2];
    b = acos( cosb );
    sinb = sin( b );
    if( b >= 0.000001 || b <= -0.000001) {
        cosa = rot[0][2] / sinb;
        sina = rot[1][2] / sinb;
        if( cosa > 1.0 ) {
            /* printf("cos(alph) = %f\n", cosa); */
            cosa = 1.0;
            sina = 0.0;
        }
        if( cosa < -1.0 ) {
            /* printf("cos(alph) = %f\n", cosa); */
            cosa = -1.0;
            sina =  0.0;
        }
        if( sina > 1.0 ) {
            /* printf("sin(alph) = %f\n", sina); */
            sina = 1.0;
            cosa = 0.0;
        }
        if( sina < -1.0 ) {
            /* printf("sin(alph) = %f\n", sina); */
            sina = -1.0;
            cosa =  0.0;
        }
        a = acos( cosa );
        if( sina < 0 ) a = -a;

        sinc =  (rot[2][1]*rot[0][2]-rot[2][0]*rot[1][2])
              / (rot[0][2]*rot[0][2]+rot[1][2]*rot[1][2]);
        cosc =  -(rot[0][2]*rot[2][0]+rot[1][2]*rot[2][1])
               / (rot[0][2]*rot[0][2]+rot[1][2]*rot[1][2]);
        if( cosc > 1.0 ) {
            /* printf("cos(r) = %f\n", cosc); */
            cosc = 1.0;
            sinc = 0.0;
        }
        if( cosc < -1.0 ) {
            /* printf("cos(r) = %f\n", cosc); */
            cosc = -1.0;
            sinc =  0.0;
        }
        if( sinc > 1.0 ) {
            /* printf("sin(r) = %f\n", sinc); */
            sinc = 1.0;
            cosc = 0.0;
        }
        if( sinc < -1.0 ) {
            /* printf("sin(r) = %f\n", sinc); */
            sinc = -1.0;
            cosc =  0.0;
        }
        c = acos( cosc );
        if( sinc < 0 ) c = -c;
    }
    else {
        a = b = 0.0;
        cosa = cosb = 1.0;
        sina = sinb = 0.0;
        cosc = rot[0][0];
        sinc = rot[1][0];
        if( cosc > 1.0 ) {
            /* printf("cos(r) = %f\n", cosc); */
            cosc = 1.0;
            sinc = 0.0;
        }
        if( cosc < -1.0 ) {
            /* printf("cos(r) = %f\n", cosc); */
            cosc = -1.0;
            sinc =  0.0;
        }
        if( sinc > 1.0 ) {
            /* printf("sin(r) = %f\n", sinc); */
            sinc = 1.0;
            cosc = 0.0;
        }
        if( sinc < -1.0 ) {
            /* printf("sin(r) = %f\n", sinc); */
            sinc = -1.0;
            cosc =  0.0;
        }
        c = acos( cosc );
        if( sinc < 0 ) c = -c;
    }

    *wa = a;
    *wb = b;
    *wc = c;

    return 0;
}

int arGetRot( double a, double b, double c, double rot[3][3] )
{
    double   sina, sinb, sinc;
    double   cosa, cosb, cosc;
#if CHECK_CALC
    double   w[3];
    int      i;
#endif

    sina = sin(a); cosa = cos(a);
    sinb = sin(b); cosb = cos(b);
    sinc = sin(c); cosc = cos(c);
    rot[0][0] = cosa*cosa*cosb*cosc+sina*sina*cosc+sina*cosa*cosb*sinc-sina*cosa*sinc;
    rot[0][1] = -cosa*cosa*cosb*sinc-sina*sina*sinc+sina*cosa*cosb*cosc-sina*cosa*cosc;
    rot[0][2] = cosa*sinb;
    rot[1][0] = sina*cosa*cosb*cosc-sina*cosa*cosc+sina*sina*cosb*sinc+cosa*cosa*sinc;
    rot[1][1] = -sina*cosa*cosb*sinc+sina*cosa*sinc+sina*sina*cosb*cosc+cosa*cosa*cosc;
    rot[1][2] = sina*sinb;
    rot[2][0] = -cosa*sinb*cosc-sina*sinb*sinc;
    rot[2][1] = cosa*sinb*sinc-sina*sinb*cosc;
    rot[2][2] = cosb;

#if CHECK_CALC
    for(i=0;i<3;i++) w[i] = rot[i][2];
    for(i=0;i<3;i++) rot[i][2] = rot[i][1];
    for(i=0;i<3;i++) rot[i][1] = rot[i][0];
    for(i=0;i<3;i++) rot[i][0] = w[i];
#endif

    return 0;
}

int arGetNewMatrix( double a, double b, double c,
                    double trans[3], double trans2[3][4],
                    double cpara[3][4], double ret[3][4] )
{
    double   cpara2[3][4];
    double   rot[3][3];
    int      i, j;

    arGetRot( a, b, c, rot );

    if( trans2 != NULL ) {
        for( j = 0; j < 3; j++ ) {
            for( i = 0; i < 4; i++ ) {
                cpara2[j][i] = cpara[j][0] * trans2[0][i]
                             + cpara[j][1] * trans2[1][i]
                             + cpara[j][2] * trans2[2][i];
            }
        }
    }
    else {
        for( j = 0; j < 3; j++ ) {
            for( i = 0; i < 4; i++ ) {
                cpara2[j][i] = cpara[j][i];
            }
        }
    }

    for( j = 0; j < 3; j++ ) {
        for( i = 0; i < 3; i++ ) {
            ret[j][i] = cpara2[j][0] * rot[0][i]
                      + cpara2[j][1] * rot[1][i]
                      + cpara2[j][2] * rot[2][i];
        }
        ret[j][3] = cpara2[j][0] * trans[0]
                  + cpara2[j][1] * trans[1]
                  + cpara2[j][2] * trans[2]
                  + cpara2[j][3];
    }

    return(0);
}

int avrGetInitRot( avrPatternInfo *marker_info, double cpara[3][4], double rot[3][3] )
{
    double  wdir[3][3];
    double  w, w1, w2, w3;
    int     dir;
    int     j;

    dir = marker_info->dir;

    for( j = 0; j < 2; j++ ) {
        w1 = marker_info->line[(4-dir+j)%4][0] * marker_info->line[(6-dir+j)%4][1]
           - marker_info->line[(6-dir+j)%4][0] * marker_info->line[(4-dir+j)%4][1];
        w2 = marker_info->line[(4-dir+j)%4][1] * marker_info->line[(6-dir+j)%4][2]
           - marker_info->line[(6-dir+j)%4][1] * marker_info->line[(4-dir+j)%4][2];
        w3 = marker_info->line[(4-dir+j)%4][2] * marker_info->line[(6-dir+j)%4][0]
           - marker_info->line[(6-dir+j)%4][2] * marker_info->line[(4-dir+j)%4][0];

        wdir[j][0] =  w1*(cpara[0][1]*cpara[1][2]-cpara[0][2]*cpara[1][1])
                   +  w2*cpara[1][1]
                   -  w3*cpara[0][1];
        wdir[j][1] = -w1*cpara[0][0]*cpara[1][2]
                   +  w3*cpara[0][0];
        wdir[j][2] =  w1*cpara[0][0]*cpara[1][1];
        w = sqrt( wdir[j][0]*wdir[j][0]
                + wdir[j][1]*wdir[j][1]
                + wdir[j][2]*wdir[j][2] );
        wdir[j][0] /= w;
        wdir[j][1] /= w;
        wdir[j][2] /= w;
    }

    if( check_dir(wdir[0], marker_info->vertex[(4-dir)%4],
                  marker_info->vertex[(5-dir)%4], cpara) < 0 ) return -1;
    if( check_dir(wdir[1], marker_info->vertex[(7-dir)%4],
                  marker_info->vertex[(4-dir)%4], cpara) < 0 ) return -1;
    if( check_rotation(wdir) < 0 ) return -1;

    wdir[2][0] = wdir[0][1]*wdir[1][2] - wdir[0][2]*wdir[1][1];
    wdir[2][1] = wdir[0][2]*wdir[1][0] - wdir[0][0]*wdir[1][2];
    wdir[2][2] = wdir[0][0]*wdir[1][1] - wdir[0][1]*wdir[1][0];
    w = sqrt( wdir[2][0]*wdir[2][0]
            + wdir[2][1]*wdir[2][1]
            + wdir[2][2]*wdir[2][2] );
    wdir[2][0] /= w;
    wdir[2][1] /= w;
    wdir[2][2] /= w;
/*
    if( wdir[2][2] < 0 ) {
        wdir[2][0] /= -w;
        wdir[2][1] /= -w;
        wdir[2][2] /= -w;
    }
    else {
        wdir[2][0] /= w;
        wdir[2][1] /= w;
        wdir[2][2] /= w;
    }
*/

    rot[0][0] = wdir[0][0];
    rot[1][0] = wdir[0][1];
    rot[2][0] = wdir[0][2];
    rot[0][1] = wdir[1][0];
    rot[1][1] = wdir[1][1];
    rot[2][1] = wdir[1][2];
    rot[0][2] = wdir[2][0];
    rot[1][2] = wdir[2][1];
    rot[2][2] = wdir[2][2];

    return 0;
}

int arGetInitRot( ARMarkerInfo *marker_info, double cpara[3][4], double rot[3][3] )
{
    double  wdir[3][3];
    double  w, w1, w2, w3;
    int     dir;
    int     j;

    dir = marker_info->dir;

    for( j = 0; j < 2; j++ ) {
        w1 = marker_info->line[(4-dir+j)%4][0] * marker_info->line[(6-dir+j)%4][1]
           - marker_info->line[(6-dir+j)%4][0] * marker_info->line[(4-dir+j)%4][1];
        w2 = marker_info->line[(4-dir+j)%4][1] * marker_info->line[(6-dir+j)%4][2]
           - marker_info->line[(6-dir+j)%4][1] * marker_info->line[(4-dir+j)%4][2];
        w3 = marker_info->line[(4-dir+j)%4][2] * marker_info->line[(6-dir+j)%4][0]
           - marker_info->line[(6-dir+j)%4][2] * marker_info->line[(4-dir+j)%4][0];

        wdir[j][0] =  w1*(cpara[0][1]*cpara[1][2]-cpara[0][2]*cpara[1][1])
                   +  w2*cpara[1][1]
                   -  w3*cpara[0][1];
        wdir[j][1] = -w1*cpara[0][0]*cpara[1][2]
                   +  w3*cpara[0][0];
        wdir[j][2] =  w1*cpara[0][0]*cpara[1][1];
        w = sqrt( wdir[j][0]*wdir[j][0]
                + wdir[j][1]*wdir[j][1]
                + wdir[j][2]*wdir[j][2] );
        wdir[j][0] /= w;
        wdir[j][1] /= w;
        wdir[j][2] /= w;
    }

    if( check_dir(wdir[0], marker_info->vertex[(4-dir)%4],
                  marker_info->vertex[(5-dir)%4], cpara) < 0 ) return -1;
    if( check_dir(wdir[1], marker_info->vertex[(7-dir)%4],
                  marker_info->vertex[(4-dir)%4], cpara) < 0 ) return -1;
    if( check_rotation(wdir) < 0 ) return -1;

    wdir[2][0] = wdir[0][1]*wdir[1][2] - wdir[0][2]*wdir[1][1];
    wdir[2][1] = wdir[0][2]*wdir[1][0] - wdir[0][0]*wdir[1][2];
    wdir[2][2] = wdir[0][0]*wdir[1][1] - wdir[0][1]*wdir[1][0];
    w = sqrt( wdir[2][0]*wdir[2][0]
            + wdir[2][1]*wdir[2][1]
            + wdir[2][2]*wdir[2][2] );
    wdir[2][0] /= w;
    wdir[2][1] /= w;
    wdir[2][2] /= w;
/*
    if( wdir[2][2] < 0 ) {
        wdir[2][0] /= -w;
        wdir[2][1] /= -w;
        wdir[2][2] /= -w;
    }
    else {
        wdir[2][0] /= w;
        wdir[2][1] /= w;
        wdir[2][2] /= w;
    }
*/

    rot[0][0] = wdir[0][0];
    rot[1][0] = wdir[0][1];
    rot[2][0] = wdir[0][2];
    rot[0][1] = wdir[1][0];
    rot[1][1] = wdir[1][1];
    rot[2][1] = wdir[1][2];
    rot[0][2] = wdir[2][0];
    rot[1][2] = wdir[2][1];
    rot[2][2] = wdir[2][2];

    return 0;
}

static int check_dir( double dir[3], double st[2], double ed[2],
                      double cpara[3][4] )
{
    ARMat     *mat_a;
    double    world[2][3];
    double    camera[2][2];
    double    v[2][2];
    double    h;
    int       i, j;

    mat_a = arMatrixAlloc( 3, 3 );
    for(j=0;j<3;j++) for(i=0;i<3;i++) mat_a->m[j*3+i] = cpara[j][i];
    arMatrixSelfInv( mat_a );
    world[0][0] = mat_a->m[0]*st[0]*10.0
                + mat_a->m[1]*st[1]*10.0
                + mat_a->m[2]*10.0;
    world[0][1] = mat_a->m[3]*st[0]*10.0
                + mat_a->m[4]*st[1]*10.0
                + mat_a->m[5]*10.0;
    world[0][2] = mat_a->m[6]*st[0]*10.0
                + mat_a->m[7]*st[1]*10.0
                + mat_a->m[8]*10.0;
    arMatrixFree( mat_a );
    world[1][0] = world[0][0] + dir[0];
    world[1][1] = world[0][1] + dir[1];
    world[1][2] = world[0][2] + dir[2];

    for( i = 0; i < 2; i++ ) {
        h = cpara[2][0] * world[i][0]
          + cpara[2][1] * world[i][1]
          + cpara[2][2] * world[i][2];
        if( h == 0.0 ) return -1;
        camera[i][0] = (cpara[0][0] * world[i][0]
                      + cpara[0][1] * world[i][1]
                      + cpara[0][2] * world[i][2]) / h;
        camera[i][1] = (cpara[1][0] * world[i][0]
                      + cpara[1][1] * world[i][1]
                      + cpara[1][2] * world[i][2]) / h;
    }

    v[0][0] = ed[0] - st[0];
    v[0][1] = ed[1] - st[1];
    v[1][0] = camera[1][0] - camera[0][0];
    v[1][1] = camera[1][1] - camera[0][1];

    if( v[0][0]*v[1][0] + v[0][1]*v[1][1] < 0 ) {
        dir[0] = -dir[0];
        dir[1] = -dir[1];
        dir[2] = -dir[2];
    }

    return 0;
}

static int check_rotation( double rot[2][3] )
{
    double  v1[3], v2[3], v3[3];
    double  ca, cb, k1, k2, k3, k4;
    double  a, b, c, d;
    double  p1, q1, r1;
    double  p2, q2, r2;
    double  p3, q3, r3;
    double  p4, q4, r4;
    double  w;
    double  e1, e2, e3, e4;
    int     f;

    v1[0] = rot[0][0];
    v1[1] = rot[0][1];
    v1[2] = rot[0][2];
    v2[0] = rot[1][0];
    v2[1] = rot[1][1];
    v2[2] = rot[1][2];
    v3[0] = v1[1]*v2[2] - v1[2]*v2[1];
    v3[1] = v1[2]*v2[0] - v1[0]*v2[2];
    v3[2] = v1[0]*v2[1] - v1[1]*v2[0];
    w = sqrt( v3[0]*v3[0]+v3[1]*v3[1]+v3[2]*v3[2] );
    if( w == 0.0 ) return -1;
    v3[0] /= w;
    v3[1] /= w;
    v3[2] /= w;

    cb = v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
    if( cb < 0 ) cb *= -1.0;
    ca = (sqrt(cb+1.0) + sqrt(1.0-cb)) * 0.5;

    if( v3[1]*v1[0] - v1[1]*v3[0] != 0.0 ) {
        f = 0;
    }
    else {
        if( v3[2]*v1[0] - v1[2]*v3[0] != 0.0 ) {
            w = v1[1]; v1[1] = v1[2]; v1[2] = w;
            w = v3[1]; v3[1] = v3[2]; v3[2] = w;
            f = 1;
        }
        else {
            w = v1[0]; v1[0] = v1[2]; v1[2] = w;
            w = v3[0]; v3[0] = v3[2]; v3[2] = w;
            f = 2;
        }
    }
    if( v3[1]*v1[0] - v1[1]*v3[0] == 0.0 ) return -1;
    k1 = (v1[1]*v3[2] - v3[1]*v1[2]) / (v3[1]*v1[0] - v1[1]*v3[0]);
    k2 = (v3[1] * ca) / (v3[1]*v1[0] - v1[1]*v3[0]);
    k3 = (v1[0]*v3[2] - v3[0]*v1[2]) / (v3[0]*v1[1] - v1[0]*v3[1]);
    k4 = (v3[0] * ca) / (v3[0]*v1[1] - v1[0]*v3[1]);

    a = k1*k1 + k3*k3 + 1;
    b = k1*k2 + k3*k4;
    c = k2*k2 + k4*k4 - 1;

    d = b*b - a*c;
    if( d < 0 ) return -1;
    r1 = (-b + sqrt(d))/a;
    p1 = k1*r1 + k2;
    q1 = k3*r1 + k4;
    r2 = (-b - sqrt(d))/a;
    p2 = k1*r2 + k2;
    q2 = k3*r2 + k4;
    if( f == 1 ) {
        w = q1; q1 = r1; r1 = w;
        w = q2; q2 = r2; r2 = w;
        w = v1[1]; v1[1] = v1[2]; v1[2] = w;
        w = v3[1]; v3[1] = v3[2]; v3[2] = w;
        f = 0;
    }
    if( f == 2 ) {
        w = p1; p1 = r1; r1 = w;
        w = p2; p2 = r2; r2 = w;
        w = v1[0]; v1[0] = v1[2]; v1[2] = w;
        w = v3[0]; v3[0] = v3[2]; v3[2] = w;
        f = 0;
    }

    if( v3[1]*v2[0] - v2[1]*v3[0] != 0.0 ) {
        f = 0;
    }
    else {
        if( v3[2]*v2[0] - v2[2]*v3[0] != 0.0 ) {
            w = v2[1]; v2[1] = v2[2]; v2[2] = w;
            w = v3[1]; v3[1] = v3[2]; v3[2] = w;
            f = 1;
        }
        else {
            w = v2[0]; v2[0] = v2[2]; v2[2] = w;
            w = v3[0]; v3[0] = v3[2]; v3[2] = w;
            f = 2;
        }
    }
    if( v3[1]*v2[0] - v2[1]*v3[0] == 0.0 ) return -1;
    k1 = (v2[1]*v3[2] - v3[1]*v2[2]) / (v3[1]*v2[0] - v2[1]*v3[0]);
    k2 = (v3[1] * ca) / (v3[1]*v2[0] - v2[1]*v3[0]);
    k3 = (v2[0]*v3[2] - v3[0]*v2[2]) / (v3[0]*v2[1] - v2[0]*v3[1]);
    k4 = (v3[0] * ca) / (v3[0]*v2[1] - v2[0]*v3[1]);

    a = k1*k1 + k3*k3 + 1;
    b = k1*k2 + k3*k4;
    c = k2*k2 + k4*k4 - 1;

    d = b*b - a*c;
    if( d < 0 ) return -1;
    r3 = (-b + sqrt(d))/a;
    p3 = k1*r3 + k2;
    q3 = k3*r3 + k4;
    r4 = (-b - sqrt(d))/a;
    p4 = k1*r4 + k2;
    q4 = k3*r4 + k4;
    if( f == 1 ) {
        w = q3; q3 = r3; r3 = w;
        w = q4; q4 = r4; r4 = w;
        w = v2[1]; v2[1] = v2[2]; v2[2] = w;
        w = v3[1]; v3[1] = v3[2]; v3[2] = w;
        f = 0;
    }
    if( f == 2 ) {
        w = p3; p3 = r3; r3 = w;
        w = p4; p4 = r4; r4 = w;
        w = v2[0]; v2[0] = v2[2]; v2[2] = w;
        w = v3[0]; v3[0] = v3[2]; v3[2] = w;
        f = 0;
    }

    e1 = p1*p3+q1*q3+r1*r3; if( e1 < 0 ) e1 = -e1;
    e2 = p1*p4+q1*q4+r1*r4; if( e2 < 0 ) e2 = -e2;
    e3 = p2*p3+q2*q3+r2*r3; if( e3 < 0 ) e3 = -e3;
    e4 = p2*p4+q2*q4+r2*r4; if( e4 < 0 ) e4 = -e4;
    if( e1 < e2 ) {
        if( e1 < e3 ) {
            if( e1 < e4 ) {
                rot[0][0] = p1;
                rot[0][1] = q1;
                rot[0][2] = r1;
                rot[1][0] = p3;
                rot[1][1] = q3;
                rot[1][2] = r3;
            }
            else {
                rot[0][0] = p2;
                rot[0][1] = q2;
                rot[0][2] = r2;
                rot[1][0] = p4;
                rot[1][1] = q4;
                rot[1][2] = r4;
            }
        }
        else {
            if( e3 < e4 ) {
                rot[0][0] = p2;
                rot[0][1] = q2;
                rot[0][2] = r2;
                rot[1][0] = p3;
                rot[1][1] = q3;
                rot[1][2] = r3;
            }
            else {
                rot[0][0] = p2;
                rot[0][1] = q2;
                rot[0][2] = r2;
                rot[1][0] = p4;
                rot[1][1] = q4;
                rot[1][2] = r4;
            }
        }
    }
    else {
        if( e2 < e3 ) {
            if( e2 < e4 ) {
                rot[0][0] = p1;
                rot[0][1] = q1;
                rot[0][2] = r1;
                rot[1][0] = p4;
                rot[1][1] = q4;
                rot[1][2] = r4;
            }
            else {
                rot[0][0] = p2;
                rot[0][1] = q2;
                rot[0][2] = r2;
                rot[1][0] = p4;
                rot[1][1] = q4;
                rot[1][2] = r4;
            }
        }
        else {
            if( e3 < e4 ) {
                rot[0][0] = p2;
                rot[0][1] = q2;
                rot[0][2] = r2;
                rot[1][0] = p3;
                rot[1][1] = q3;
                rot[1][2] = r3;
            }
            else {
                rot[0][0] = p2;
                rot[0][1] = q2;
                rot[0][2] = r2;
                rot[1][0] = p4;
                rot[1][1] = q4;
                rot[1][2] = r4;
            }
        }
    }

    return 0;
}

static double avrGetTransMatContSub( avrPatternInfo *marker_info, avrMatrix3x4& prev_conv,
                                    double center[2], double width, avrMatrix3x4& conv);

double avrGetTransMatCont( avrPatternInfo *marker_info, avrMatrix3x4& prev_conv,
                          double center[2], double width, avrMatrix3x4& conv)
{
    double        err1, err2;
    avrMatrix3x4  wtrans;
    int     i, j;

    err1 = avrGetTransMatContSub(marker_info, prev_conv, center, width, conv);
    if( err1 > AR_GET_TRANS_CONT_MAT_MAX_FIT_ERROR ) {
        err2 = avrGetTransMat(marker_info, center, width, wtrans);
        if( err2 < err1 ) {
            for( i = 0; i < 3; i++ ) {
                for( j = 0; j < 4; j++ ) conv.add(wtrans.access(i, j), i, j);
            }
            err1 = err2;
        }
    }

    return err1;
}

static double avrGetTransMatContSub( avrPatternInfo *marker_info, avrMatrix3x4& prev_conv,
                                    double center[2], double width, avrMatrix3x4& conv)
{
    double  rot[3][3];
    double  ppos2d[4][2];
    double  ppos3d[4][2];
    int     dir;
    double  err;
    int     i, j;

    for( i = 0; i < 3; i++ ) {
        for( j = 0; j < 3; j++ ) {
            rot[i][j] = prev_conv.access(i,j);
        }
    }

    dir = marker_info->dir;
    //printf("%d - ", dir);
    ppos2d[0][0] = marker_info->vertex[(4-dir)%4][0];
    ppos2d[0][1] = marker_info->vertex[(4-dir)%4][1];
    ppos2d[1][0] = marker_info->vertex[(5-dir)%4][0];
    ppos2d[1][1] = marker_info->vertex[(5-dir)%4][1];
    ppos2d[2][0] = marker_info->vertex[(6-dir)%4][0];
    ppos2d[2][1] = marker_info->vertex[(6-dir)%4][1];
    ppos2d[3][0] = marker_info->vertex[(7-dir)%4][0];
    ppos2d[3][1] = marker_info->vertex[(7-dir)%4][1];
    ppos3d[0][0] = center[0] - width/2.0;
    ppos3d[0][1] = center[1] + width/2.0;
    ppos3d[1][0] = center[0] + width/2.0;
    ppos3d[1][1] = center[1] + width/2.0;
    ppos3d[2][0] = center[0] + width/2.0;
    ppos3d[2][1] = center[1] - width/2.0;
    ppos3d[3][0] = center[0] - width/2.0;
    ppos3d[3][1] = center[1] - width/2.0;

    for( i = 0; i < AR_GET_TRANS_MAT_MAX_LOOP_COUNT; i++ ) {
        err = avrGetTransMat3( rot, ppos2d, ppos3d, 4, conv,
                                   arParam.dist_factor, arParam.mat );
        if( err < AR_GET_TRANS_MAT_MAX_FIT_ERROR ) break;
    }
    return err;
}

static double arGetTransMatContSub( ARMarkerInfo *marker_info, double prev_conv[3][4],
                                    double center[2], double width, double conv[3][4] );

double arGetTransMatCont( ARMarkerInfo *marker_info, double prev_conv[3][4],
                          double center[2], double width, double conv[3][4] )
{
    double  err1, err2;
    double  wtrans[3][4];
    int     i, j;

    err1 = arGetTransMatContSub(marker_info, prev_conv, center, width, conv);
    if( err1 > AR_GET_TRANS_CONT_MAT_MAX_FIT_ERROR ) {
        err2 = arGetTransMat(marker_info, center, width, wtrans);
        if( err2 < err1 ) {
            for( j = 0; j < 3; j++ ) {
                for( i = 0; i < 4; i++ ) conv[j][i] = wtrans[j][i];
            }
            err1 = err2;
        }
    }

    return err1;
}


static double arGetTransMatContSub( ARMarkerInfo *marker_info, double prev_conv[3][4],
                                    double center[2], double width, double conv[3][4] )
{
    double  rot[3][3];
    double  ppos2d[4][2];
    double  ppos3d[4][2];
    int     dir;
    double  err;
    int     i, j;

    for( i = 0; i < 3; i++ ) {
        for( j = 0; j < 3; j++ ) {
            rot[i][j] = prev_conv[i][j];
        }
    }

    dir = marker_info->dir;
    ppos2d[0][0] = marker_info->vertex[(4-dir)%4][0];
    ppos2d[0][1] = marker_info->vertex[(4-dir)%4][1];
    ppos2d[1][0] = marker_info->vertex[(5-dir)%4][0];
    ppos2d[1][1] = marker_info->vertex[(5-dir)%4][1];
    ppos2d[2][0] = marker_info->vertex[(6-dir)%4][0];
    ppos2d[2][1] = marker_info->vertex[(6-dir)%4][1];
    ppos2d[3][0] = marker_info->vertex[(7-dir)%4][0];
    ppos2d[3][1] = marker_info->vertex[(7-dir)%4][1];
    ppos3d[0][0] = center[0] - width/2.0;
    ppos3d[0][1] = center[1] + width/2.0;
    ppos3d[1][0] = center[0] + width/2.0;
    ppos3d[1][1] = center[1] + width/2.0;
    ppos3d[2][0] = center[0] + width/2.0;
    ppos3d[2][1] = center[1] - width/2.0;
    ppos3d[3][0] = center[0] - width/2.0;
    ppos3d[3][1] = center[1] - width/2.0;

    for( i = 0; i < AR_GET_TRANS_MAT_MAX_LOOP_COUNT; i++ ) {
        err = arGetTransMat3( rot, ppos2d, ppos3d, 4, conv,
                                   arParam.dist_factor, arParam.mat );
        if( err < AR_GET_TRANS_MAT_MAX_FIT_ERROR ) break;
    }
    return err;
}

static int avrVerify_markers(avrPatternInfo *marker_info, int marker_num, ARMultiMarkerInfoT *config);

double avrMultiGetTransMat(avrPatternInfo *marker_info, int marker_num, ARMultiMarkerInfoT *config)
{
    double                *pos2d, *pos3d;
    double                rot[3][3], trans2[3][4];
    avrMatrix3x4          trans1;
    double                err, err2;
    int                   max, max_area, max_marker, vnum;
    int                   dir;
    int                   i, j, k;

    if( config->prevF ) {
        avrVerify_markers( marker_info, marker_num, config );
    }

    max = -1;
    vnum = 0;
    for( i = 0; i < config->marker_num; i++ ) {
        k = -1;
        for( j = 0; j < marker_num; j++ ) {
            if( marker_info[j].id != config->marker[i].patt_id ) continue;
            if( marker_info[j].cf < 0.70 ) continue;

//            glColor3f( 0.0, 1.0, 0.0 );
//            argDrawSquare(marker_info[j].vertex,0,0);

            if( k == -1 ) k = j;
            else if( marker_info[k].cf < marker_info[j].cf ) k = j;
        }
        if( (config->marker[i].visible=k) == -1) continue;

        err = avrGetTransMat(&marker_info[k], config->marker[i].center,
                            config->marker[i].width, trans1);
#if debug
printf("##err = %10.5f %d %10.5f %10.5f\n", err, marker_info[k].dir, marker_info[k].pos[0], marker_info[k].pos[1]);
#endif
        if( err > THRESH_1 ) {
            config->marker[i].visible = -1;
            continue;
        }

        vnum++;
        if( max == -1 || marker_info[k].area > max_area ) {
            max = i;
            max_marker = k;
            max_area   = marker_info[k].area;
            for( j = 0; j < 3; j++ ) {
                for( k = 0; k < 4; k++ ) {
                    trans2[j][k] = trans1.access(j, k);
                }
            }
        }
    }
    if( max == -1 ) {
        config->prevF = 0;
        return -1;
    }

    arMalloc(pos2d, double, vnum*4*2);
    arMalloc(pos3d, double, vnum*4*3);

    j = 0;
    for( i = 0; i < config->marker_num; i++ ) {
        if( (k=config->marker[i].visible) < 0 ) continue;

        dir = marker_info[k].dir;
        pos2d[j*8+0] = marker_info[k].vertex[(4-dir)%4][0];
        pos2d[j*8+1] = marker_info[k].vertex[(4-dir)%4][1];
        pos2d[j*8+2] = marker_info[k].vertex[(5-dir)%4][0];
        pos2d[j*8+3] = marker_info[k].vertex[(5-dir)%4][1];
        pos2d[j*8+4] = marker_info[k].vertex[(6-dir)%4][0];
        pos2d[j*8+5] = marker_info[k].vertex[(6-dir)%4][1];
        pos2d[j*8+6] = marker_info[k].vertex[(7-dir)%4][0];
        pos2d[j*8+7] = marker_info[k].vertex[(7-dir)%4][1];
        pos3d[j*12+0] = config->marker[i].pos3d[0][0];
        pos3d[j*12+1] = config->marker[i].pos3d[0][1];
        pos3d[j*12+2] = config->marker[i].pos3d[0][2];
        pos3d[j*12+3] = config->marker[i].pos3d[1][0];
        pos3d[j*12+4] = config->marker[i].pos3d[1][1];
        pos3d[j*12+5] = config->marker[i].pos3d[1][2];
        pos3d[j*12+6] = config->marker[i].pos3d[2][0];
        pos3d[j*12+7] = config->marker[i].pos3d[2][1];
        pos3d[j*12+8] = config->marker[i].pos3d[2][2];
        pos3d[j*12+9] = config->marker[i].pos3d[3][0];
        pos3d[j*12+10] = config->marker[i].pos3d[3][1];
        pos3d[j*12+11] = config->marker[i].pos3d[3][2];
        j++;
    }

    if( config->prevF ) {
        for( j = 0; j < 3; j++ ) {
            for( i = 0; i < 3; i++ ) {
                rot[j][i] = config->trans[j][i];
            }
        }
        for( i = 0; i < AR_MULTI_GET_TRANS_MAT_MAX_LOOP_COUNT; i++ ) {
            err = arGetTransMat4( rot, (double (*)[2])pos2d,
                                       (double (*)[3])pos3d,
                                        vnum*4, config->trans );
            if( err < AR_MULTI_GET_TRANS_MAT_MAX_FIT_ERROR ) break;
        }

        if( err < THRESH_2 ) {
            config->prevF = 1;
            free(pos3d);
            free(pos2d);
            return err;
        }
    }

    double  mult[3][4];
    arUtilMatMul( trans2, config->marker[max].itrans, mult);
    trans1 = mult;

    for( j = 0; j < 3; j++ ) {
        for( i = 0; i < 3; i++ ) {
            rot[j][i] = trans1.access(j, i);
        }
    }

    for( i = 0; i < AR_MULTI_GET_TRANS_MAT_MAX_LOOP_COUNT; i++ ) {
        err2 = arGetTransMat4( rot, (double (*)[2])pos2d, (double (*)[3])pos3d,
                              vnum*4, trans2 );
        if( err2 < AR_MULTI_GET_TRANS_MAT_MAX_FIT_ERROR ) break;
    }

    if( config->prevF == 0 || err2 < err ) {
        for( j = 0; j < 3; j++ ) {
            for( i = 0; i < 4; i++ ) {
                config->trans[j][i] = trans2[j][i];
            }
        }
        err = err2;
    }

    if( err < THRESH_3 ) {
        config->prevF = 1;
    }
    else {
        config->prevF = 0;
    }

    free(pos3d);
    free(pos2d);
    return err;
}

static int avrVerify_markers(avrPatternInfo *marker_info, int marker_num,
                          ARMultiMarkerInfoT *config)
{
    arMultiEachMarkerInternalInfoT *winfo;
    double                         wtrans[3][4];
    double                         pos3d[4][2];
    double                         wx, wy, wz, hx, hy, h;
    int                            dir1, dir2, marker2;
    double                         err, err1, err2;
    double                         x1, x2, y1, y2;
    int                            w1, w2;
    int                            i, j, k;

    arMalloc(winfo,arMultiEachMarkerInternalInfoT,config->marker_num);

    for( i = 0; i < config->marker_num; i++ ) {
        arUtilMatMul(config->trans, config->marker[i].trans, wtrans);
        pos3d[0][0] = config->marker[i].center[0] - config->marker[i].width/2.0;
        pos3d[0][1] = config->marker[i].center[1] + config->marker[i].width/2.0;
        pos3d[1][0] = config->marker[i].center[0] + config->marker[i].width/2.0;
        pos3d[1][1] = config->marker[i].center[1] + config->marker[i].width/2.0;
        pos3d[2][0] = config->marker[i].center[0] + config->marker[i].width/2.0;
        pos3d[2][1] = config->marker[i].center[1] - config->marker[i].width/2.0;
        pos3d[3][0] = config->marker[i].center[0] - config->marker[i].width/2.0;
        pos3d[3][1] = config->marker[i].center[1] - config->marker[i].width/2.0;
        for( j = 0; j < 4; j++ ) {
            wx = wtrans[0][0] * pos3d[j][0]
               + wtrans[0][1] * pos3d[j][1]
               + wtrans[0][3];
            wy = wtrans[1][0] * pos3d[j][0]
               + wtrans[1][1] * pos3d[j][1]
               + wtrans[1][3];
            wz = wtrans[2][0] * pos3d[j][0]
               + wtrans[2][1] * pos3d[j][1]
               + wtrans[2][3];
            hx = arParam.mat[0][0] * wx
               + arParam.mat[0][1] * wy
               + arParam.mat[0][2] * wz
               + arParam.mat[0][3];
            hy = arParam.mat[1][0] * wx
               + arParam.mat[1][1] * wy
               + arParam.mat[1][2] * wz
               + arParam.mat[1][3];
            h  = arParam.mat[2][0] * wx
               + arParam.mat[2][1] * wy
               + arParam.mat[2][2] * wz
               + arParam.mat[2][3];
            winfo[i].pos[j][0] = hx / h;
            winfo[i].pos[j][1] = hy / h;

            if(j ==0) {x1=x2=winfo[i].pos[j][0]; y1=y2=winfo[i].pos[j][1];}
            else {
                if( winfo[i].pos[j][0] < x1 ) x1 = winfo[i].pos[j][0];
                if( winfo[i].pos[j][0] > x2 ) x2 = winfo[i].pos[j][0];
                if( winfo[i].pos[j][1] < y1 ) y1 = winfo[i].pos[j][1];
                if( winfo[i].pos[j][1] > y2 ) y2 = winfo[i].pos[j][1];
            }
        }
        winfo[i].thresh = (x2 - x1 + 1)*(y2 - y1 + 1) / 2;
    }

#if debug
printf("\n");
printf("================================================================\n");
for( i = 0; i < config->marker_num; i++) {
printf("%3d: ", i+1);
for( j = 0; j < 4; j++ ) {
    printf("(%5.1f %5.1f) ", winfo[i].pos[j][0], winfo[i].pos[j][1]);
}
printf("\n");
}
printf("--------\n");
for( i = 0; i < marker_num; i++) {
printf("%3d: ", i+1);
for( j = 0; j < 4; j++ ) {
    printf("(%5.1f %5.1f) ", marker_info[i].vertex[j][0], marker_info[i].vertex[j][1]);
}
printf("\n");
}
#endif

    w1 = w2 = 0;
    for( i = 0; i < config->marker_num; i++ ) {
        marker2 = -1;
        err2 = winfo[i].thresh;
        for( j = 0; j < marker_num; j++ ) {
            if( marker_info[j].id != -1
             && marker_info[j].id != config->marker[i].patt_id
             && marker_info[j].cf > 0.7 ) continue;

            dir1 = -1;
            for( k = 0; k < 4; k++ ) {
                err = (winfo[i].pos[0][0] - marker_info[j].vertex[(k+0)%4][0])
                    * (winfo[i].pos[0][0] - marker_info[j].vertex[(k+0)%4][0])
                    + (winfo[i].pos[0][1] - marker_info[j].vertex[(k+0)%4][1])
                    * (winfo[i].pos[0][1] - marker_info[j].vertex[(k+0)%4][1])
                    + (winfo[i].pos[1][0] - marker_info[j].vertex[(k+1)%4][0])
                    * (winfo[i].pos[1][0] - marker_info[j].vertex[(k+1)%4][0])
                    + (winfo[i].pos[1][1] - marker_info[j].vertex[(k+1)%4][1])
                    * (winfo[i].pos[1][1] - marker_info[j].vertex[(k+1)%4][1])
                    + (winfo[i].pos[2][0] - marker_info[j].vertex[(k+2)%4][0])
                    * (winfo[i].pos[2][0] - marker_info[j].vertex[(k+2)%4][0])
                    + (winfo[i].pos[2][1] - marker_info[j].vertex[(k+2)%4][1])
                    * (winfo[i].pos[2][1] - marker_info[j].vertex[(k+2)%4][1])
                    + (winfo[i].pos[3][0] - marker_info[j].vertex[(k+3)%4][0])
                    * (winfo[i].pos[3][0] - marker_info[j].vertex[(k+3)%4][0])
                    + (winfo[i].pos[3][1] - marker_info[j].vertex[(k+3)%4][1])
                    * (winfo[i].pos[3][1] - marker_info[j].vertex[(k+3)%4][1]);
                if( dir1 == -1 || err < err1 ) {
                    err1 = err;
                    dir1 = k;
                }
            }
#if debug
printf("%f\n", err1);
#endif
            if( err1 < err2 ) {
                err2 = err1;
                dir2 = dir1;
                marker2 = j;
            }
        }

#if debug
printf("%3d<=>%3d, err = %f(%f)\n", i+1, marker2+1, err2, winfo[i].thresh);
#endif
        if( marker2 != -1 ) {
            winfo[i].marker = marker2;
            winfo[i].dir    = dir2;
            winfo[i].err    = err2;

            if( marker_info[marker2].id == config->marker[i].patt_id ) w1++;
            else if( marker_info[marker2].id != -1 )                   w2++;
        }
        else {
            winfo[i].marker = -1;
        }
    }
#if debug
printf("w1,w2 = %d,%d\n", w1, w2);
#endif
    if( w2 >= w1 ) {
        free(winfo);
        return -1;
    }

    for( i = 0; i < config->marker_num; i++ ) {
        for( j = 0; j < marker_num; j++ ) {
                if( marker_info[j].id == config->marker[i].patt_id ) marker_info[j].id = -1;
        }
        if( winfo[i].marker != -1 ) {
            marker_info[winfo[i].marker].id  = config->marker[i].patt_id;
            marker_info[winfo[i].marker].dir = (4-winfo[i].dir)%4;
            marker_info[winfo[i].marker].cf  = 1.0;
        }
    }

    free(winfo);

    return 0;
}


static int verify_markers(ARMarkerInfo *marker_info, int marker_num, ARMultiMarkerInfoT *config);

double arMultiGetTransMat(ARMarkerInfo *marker_info, int marker_num, ARMultiMarkerInfoT *config)
{
    double                *pos2d, *pos3d;
    double                rot[3][3], trans1[3][4], trans2[3][4];
    double                err, err2;
    int                   max, max_area, max_marker, vnum;
    int                   dir;
    int                   i, j, k;

    if( config->prevF ) {
        verify_markers( marker_info, marker_num, config );
    }

    max = -1;
    vnum = 0;
    for( i = 0; i < config->marker_num; i++ ) {
        k = -1;
        for( j = 0; j < marker_num; j++ ) {
            if( marker_info[j].id != config->marker[i].patt_id ) continue;
            if( marker_info[j].cf < 0.70 ) continue;

//            glColor3f( 0.0, 1.0, 0.0 );
//            argDrawSquare(marker_info[j].vertex,0,0);

            if( k == -1 ) k = j;
            else if( marker_info[k].cf < marker_info[j].cf ) k = j;
        }
        if( (config->marker[i].visible=k) == -1) continue;

        err = arGetTransMat(&marker_info[k], config->marker[i].center,
                            config->marker[i].width, trans1);
#if debug
printf("##err = %10.5f %d %10.5f %10.5f\n", err, marker_info[k].dir, marker_info[k].pos[0], marker_info[k].pos[1]);
#endif
        if( err > THRESH_1 ) {
            config->marker[i].visible = -1;
            continue;
        }

        vnum++;
        if( max == -1 || marker_info[k].area > max_area ) {
            max = i;
            max_marker = k;
            max_area   = marker_info[k].area;
            for( j = 0; j < 3; j++ ) {
                for( k = 0; k < 4; k++ ) {
                    trans2[j][k] = trans1[j][k];
                }
            }
        }
    }
    if( max == -1 ) {
        config->prevF = 0;
        return -1;
    }

    arMalloc(pos2d, double, vnum*4*2);
    arMalloc(pos3d, double, vnum*4*3);

    j = 0;
    for( i = 0; i < config->marker_num; i++ ) {
        if( (k=config->marker[i].visible) < 0 ) continue;

        dir = marker_info[k].dir;
        pos2d[j*8+0] = marker_info[k].vertex[(4-dir)%4][0];
        pos2d[j*8+1] = marker_info[k].vertex[(4-dir)%4][1];
        pos2d[j*8+2] = marker_info[k].vertex[(5-dir)%4][0];
        pos2d[j*8+3] = marker_info[k].vertex[(5-dir)%4][1];
        pos2d[j*8+4] = marker_info[k].vertex[(6-dir)%4][0];
        pos2d[j*8+5] = marker_info[k].vertex[(6-dir)%4][1];
        pos2d[j*8+6] = marker_info[k].vertex[(7-dir)%4][0];
        pos2d[j*8+7] = marker_info[k].vertex[(7-dir)%4][1];
        pos3d[j*12+0] = config->marker[i].pos3d[0][0];
        pos3d[j*12+1] = config->marker[i].pos3d[0][1];
        pos3d[j*12+2] = config->marker[i].pos3d[0][2];
        pos3d[j*12+3] = config->marker[i].pos3d[1][0];
        pos3d[j*12+4] = config->marker[i].pos3d[1][1];
        pos3d[j*12+5] = config->marker[i].pos3d[1][2];
        pos3d[j*12+6] = config->marker[i].pos3d[2][0];
        pos3d[j*12+7] = config->marker[i].pos3d[2][1];
        pos3d[j*12+8] = config->marker[i].pos3d[2][2];
        pos3d[j*12+9] = config->marker[i].pos3d[3][0];
        pos3d[j*12+10] = config->marker[i].pos3d[3][1];
        pos3d[j*12+11] = config->marker[i].pos3d[3][2];
        j++;
    }

    if( config->prevF ) {
        for( j = 0; j < 3; j++ ) {
            for( i = 0; i < 3; i++ ) {
                rot[j][i] = config->trans[j][i];
            }
        }
        for( i = 0; i < AR_MULTI_GET_TRANS_MAT_MAX_LOOP_COUNT; i++ ) {
            err = arGetTransMat4( rot, (double (*)[2])pos2d,
                                       (double (*)[3])pos3d,
                                        vnum*4, config->trans );
            if( err < AR_MULTI_GET_TRANS_MAT_MAX_FIT_ERROR ) break;
        }

        if( err < THRESH_2 ) {
            config->prevF = 1;
            free(pos3d);
            free(pos2d);
            return err;
        }
    }

    arUtilMatMul( trans2, config->marker[max].itrans, trans1 );
    for( j = 0; j < 3; j++ ) {
        for( i = 0; i < 3; i++ ) {
            rot[j][i] = trans1[j][i];
        }
    }

    for( i = 0; i < AR_MULTI_GET_TRANS_MAT_MAX_LOOP_COUNT; i++ ) {
        err2 = arGetTransMat4( rot, (double (*)[2])pos2d, (double (*)[3])pos3d,
                              vnum*4, trans2 );
        if( err2 < AR_MULTI_GET_TRANS_MAT_MAX_FIT_ERROR ) break;
    }

    if( config->prevF == 0 || err2 < err ) {
        for( j = 0; j < 3; j++ ) {
            for( i = 0; i < 4; i++ ) {
                config->trans[j][i] = trans2[j][i];
            }
        }
        err = err2;
    }

    if( err < THRESH_3 ) {
        config->prevF = 1;
    }
    else {
        config->prevF = 0;
    }

    free(pos3d);
    free(pos2d);
    return err;
}

static int verify_markers(ARMarkerInfo *marker_info, int marker_num,
                          ARMultiMarkerInfoT *config)
{
    arMultiEachMarkerInternalInfoT *winfo;
    double                         wtrans[3][4];
    double                         pos3d[4][2];
    double                         wx, wy, wz, hx, hy, h;
    int                            dir1, dir2, marker2;
    double                         err, err1, err2;
    double                         x1, x2, y1, y2;
    int                            w1, w2;
    int                            i, j, k;

    arMalloc(winfo,arMultiEachMarkerInternalInfoT,config->marker_num);

    for( i = 0; i < config->marker_num; i++ ) {
        arUtilMatMul(config->trans, config->marker[i].trans, wtrans);
        pos3d[0][0] = config->marker[i].center[0] - config->marker[i].width/2.0;
        pos3d[0][1] = config->marker[i].center[1] + config->marker[i].width/2.0;
        pos3d[1][0] = config->marker[i].center[0] + config->marker[i].width/2.0;
        pos3d[1][1] = config->marker[i].center[1] + config->marker[i].width/2.0;
        pos3d[2][0] = config->marker[i].center[0] + config->marker[i].width/2.0;
        pos3d[2][1] = config->marker[i].center[1] - config->marker[i].width/2.0;
        pos3d[3][0] = config->marker[i].center[0] - config->marker[i].width/2.0;
        pos3d[3][1] = config->marker[i].center[1] - config->marker[i].width/2.0;
        for( j = 0; j < 4; j++ ) {
            wx = wtrans[0][0] * pos3d[j][0]
               + wtrans[0][1] * pos3d[j][1]
               + wtrans[0][3];
            wy = wtrans[1][0] * pos3d[j][0]
               + wtrans[1][1] * pos3d[j][1]
               + wtrans[1][3];
            wz = wtrans[2][0] * pos3d[j][0]
               + wtrans[2][1] * pos3d[j][1]
               + wtrans[2][3];
            hx = arParam.mat[0][0] * wx
               + arParam.mat[0][1] * wy
               + arParam.mat[0][2] * wz
               + arParam.mat[0][3];
            hy = arParam.mat[1][0] * wx
               + arParam.mat[1][1] * wy
               + arParam.mat[1][2] * wz
               + arParam.mat[1][3];
            h  = arParam.mat[2][0] * wx
               + arParam.mat[2][1] * wy
               + arParam.mat[2][2] * wz
               + arParam.mat[2][3];
            winfo[i].pos[j][0] = hx / h;
            winfo[i].pos[j][1] = hy / h;

            if(j ==0) {x1=x2=winfo[i].pos[j][0]; y1=y2=winfo[i].pos[j][1];}
            else {
                if( winfo[i].pos[j][0] < x1 ) x1 = winfo[i].pos[j][0];
                if( winfo[i].pos[j][0] > x2 ) x2 = winfo[i].pos[j][0];
                if( winfo[i].pos[j][1] < y1 ) y1 = winfo[i].pos[j][1];
                if( winfo[i].pos[j][1] > y2 ) y2 = winfo[i].pos[j][1];
            }
        }
        winfo[i].thresh = (x2 - x1 + 1)*(y2 - y1 + 1) / 2;
    }

#if debug
printf("\n");
printf("================================================================\n");
for( i = 0; i < config->marker_num; i++) {
printf("%3d: ", i+1);
for( j = 0; j < 4; j++ ) {
    printf("(%5.1f %5.1f) ", winfo[i].pos[j][0], winfo[i].pos[j][1]);
}
printf("\n");
}
printf("--------\n");
for( i = 0; i < marker_num; i++) {
printf("%3d: ", i+1);
for( j = 0; j < 4; j++ ) {
    printf("(%5.1f %5.1f) ", marker_info[i].vertex[j][0], marker_info[i].vertex[j][1]);
}
printf("\n");
}
#endif

    w1 = w2 = 0;
    for( i = 0; i < config->marker_num; i++ ) {
        marker2 = -1;
        err2 = winfo[i].thresh;
        for( j = 0; j < marker_num; j++ ) {
            if( marker_info[j].id != -1
             && marker_info[j].id != config->marker[i].patt_id
             && marker_info[j].cf > 0.7 ) continue;

            dir1 = -1;
            for( k = 0; k < 4; k++ ) {
                err = (winfo[i].pos[0][0] - marker_info[j].vertex[(k+0)%4][0])
                    * (winfo[i].pos[0][0] - marker_info[j].vertex[(k+0)%4][0])
                    + (winfo[i].pos[0][1] - marker_info[j].vertex[(k+0)%4][1])
                    * (winfo[i].pos[0][1] - marker_info[j].vertex[(k+0)%4][1])
                    + (winfo[i].pos[1][0] - marker_info[j].vertex[(k+1)%4][0])
                    * (winfo[i].pos[1][0] - marker_info[j].vertex[(k+1)%4][0])
                    + (winfo[i].pos[1][1] - marker_info[j].vertex[(k+1)%4][1])
                    * (winfo[i].pos[1][1] - marker_info[j].vertex[(k+1)%4][1])
                    + (winfo[i].pos[2][0] - marker_info[j].vertex[(k+2)%4][0])
                    * (winfo[i].pos[2][0] - marker_info[j].vertex[(k+2)%4][0])
                    + (winfo[i].pos[2][1] - marker_info[j].vertex[(k+2)%4][1])
                    * (winfo[i].pos[2][1] - marker_info[j].vertex[(k+2)%4][1])
                    + (winfo[i].pos[3][0] - marker_info[j].vertex[(k+3)%4][0])
                    * (winfo[i].pos[3][0] - marker_info[j].vertex[(k+3)%4][0])
                    + (winfo[i].pos[3][1] - marker_info[j].vertex[(k+3)%4][1])
                    * (winfo[i].pos[3][1] - marker_info[j].vertex[(k+3)%4][1]);
                if( dir1 == -1 || err < err1 ) {
                    err1 = err;
                    dir1 = k;
                }
            }
#if debug
printf("%f\n", err1);
#endif
            if( err1 < err2 ) {
                err2 = err1;
                dir2 = dir1;
                marker2 = j;
            }
        }

#if debug
printf("%3d<=>%3d, err = %f(%f)\n", i+1, marker2+1, err2, winfo[i].thresh);
#endif
        if( marker2 != -1 ) {
            winfo[i].marker = marker2;
            winfo[i].dir    = dir2;
            winfo[i].err    = err2;

            if( marker_info[marker2].id == config->marker[i].patt_id ) w1++;
            else if( marker_info[marker2].id != -1 )                   w2++;
        }
        else {
            winfo[i].marker = -1;
        }
    }
#if debug
printf("w1,w2 = %d,%d\n", w1, w2);
#endif
    if( w2 >= w1 ) {
        free(winfo);
        return -1;
    }

    for( i = 0; i < config->marker_num; i++ ) {
        for( j = 0; j < marker_num; j++ ) {
                if( marker_info[j].id == config->marker[i].patt_id ) marker_info[j].id = -1;
        }
        if( winfo[i].marker != -1 ) {
            marker_info[winfo[i].marker].id  = config->marker[i].patt_id;
            marker_info[winfo[i].marker].dir = (4-winfo[i].dir)%4;
            marker_info[winfo[i].marker].cf  = 1.0;
        }
    }

    free(winfo);

    return 0;
}

static double mdet( double *ap, int dimen, int rowa );

double arMatrixDet(ARMat *m)
{

	if(m->row != m->clm) return 0.0;

	return mdet(m->m, m->row, m->row);
}



static double mdet(double *ap, int dimen, int rowa)
/*  double  *ap;          input matrix */
/*  int     dimen;        Dimension of linre and row, those must be equal,
                          that is square matrix.       */
/*  int     rowa;         ROW Dimension of matrix A    */
{
    double det = 1.0;
    double work;
    int    is = 0;
    int    mmax;
    int    i, j, k;

    for(k = 0; k < dimen - 1; k++) {
        mmax = k;
        for(i = k + 1; i < dimen; i++)
            if (fabs(MATRIX(ap, i, k, rowa)) > fabs(MATRIX(ap, mmax, k, rowa)))
                mmax = i;
        if(mmax != k) {
            for (j = k; j < dimen; j++) {
                work = MATRIX(ap, k, j, rowa);
                MATRIX(ap, k, j, rowa) = MATRIX(ap, mmax, j, rowa);
                MATRIX(ap, mmax, j, rowa) = work;
            }
            is++;
        }
        for(i = k + 1; i < dimen; i++) {
            work = MATRIX(ap, i, k, rowa) / MATRIX(ap, k, k, rowa);
            for (j = k + 1; j < dimen; j++)
                MATRIX(ap, i, j, rowa) -= work * MATRIX(ap, k, j, rowa);
        }
    }
    for(i = 0; i < dimen; i++)
        det *= MATRIX(ap, i, i, rowa);
    for(i = 0; i < is; i++)
        det *= -1.0;
    return(det);
}

int arMatrixInv(ARMat *dest, ARMat *source)
{
	if(arMatrixDup(dest, source) < 0) return -1;

	return arMatrixSelfInv(dest);
}

int arMatrixMul(ARMat *dest, ARMat *a, ARMat *b)
{
	int r, c, i;

	if(a->clm != b->row || dest->row != a->row || dest->clm != b->clm) return -1;

	for(r = 0; r < dest->row; r++) {
		for(c = 0; c < dest->clm; c++) {
			ARELEM0(dest, r, c) = 0.0;
			for(i = 0; i < a->clm; i++) {
				ARELEM0(dest, r, c) += ARELEM0(a, r, i) * ARELEM0(b, i, c);
			}
		}
	}

	return 0;
}

ARMat *arMatrixAllocInv(ARMat *source)
{
	ARMat *dest;

	dest = arMatrixAlloc(source->row, source->row);
	if( dest == NULL ) return NULL;

	if( arMatrixInv(dest, source) < 0 ) {
		arMatrixFree( dest );
		return NULL;
	}

	return dest;
}

ARMat *arMatrixAllocMul(ARMat *a, ARMat *b)
{
	ARMat *dest;

	dest = arMatrixAlloc(a->row, b->clm);
	if( dest == NULL ) return NULL;

	if( arMatrixMul(dest, a, b) < 0 ) {
		arMatrixFree(dest);
		return NULL;
	}

	return dest;
}

ARMat *arMatrixAllocTrans(ARMat *source)
{
	ARMat *dest;

	dest = arMatrixAlloc(source->clm, source->row);
	if( dest == NULL ) return NULL;

	if( arMatrixTrans(dest, source) < 0 ) {
		arMatrixFree(dest);
		return NULL;
	}

	return dest;
}

ARMat *arMatrixAllocUnit(int dim)
{
	ARMat *m;

	m = arMatrixAlloc(dim, dim);
	if( m == NULL ) return NULL;

	if( arMatrixUnit(m) < 0 ) {
		arMatrixFree(m);
		return NULL;
	}

	return m;
}

static int EX( ARMat *input, ARVec *mean );
static int CENTER( ARMat *inout, ARVec *mean );
static int PCA( ARMat *input, ARMat *output, ARVec *ev );
static int x_by_xt( ARMat *input, ARMat *output );
static int xt_by_x( ARMat *input, ARMat *output );
static int EV_create( ARMat *input, ARMat *u, ARMat *output, ARVec *ev );
static int QRM( ARMat *u, ARVec *ev );


/* === matrix definition ===

Input:
  <---- clm (Data dimention)--->
  [ 10  20  30 ] ^
  [ 20  10  15 ] |
  [ 12  23  13 ] row
  [ 20  10  15 ] |(Sample number)
  [ 13  14  15 ] v

Evec:
  <---- clm (Eigen vector)--->
  [ 10  20  30 ] ^
  [ 20  10  15 ] |
  [ 12  23  13 ] row
  [ 20  10  15 ] |(Number of egen vec)
  [ 13  14  15 ] v

Ev:
  <---- clm (Number of eigen vector)--->
  [ 10  20  30 ] eigen value

Mean:
  <---- clm (Data dimention)--->
  [ 10  20  30 ] mean value

=========================== */


int arMatrixPCA( ARMat *input, ARMat *evec, ARVec *ev, ARVec *mean )
{
    ARMat     *work;
    double  srow, sum;
    int     row, clm;
    int     check, rval;
    int     i;

    row = input->row;
    clm = input->clm;
    check = (row < clm)? row: clm;
    if( row < 2 || clm < 2 ) return(-1);
    if( evec->clm != input->clm || evec->row != check ) return(-1);
    if( ev->clm   != check )      return(-1);
    if( mean->clm != input->clm ) return(-1);

    work = arMatrixAllocDup( input );
    if( work == NULL ) return -1;

    srow = sqrt((double)row);
    if( EX( work, mean ) < 0 ) {
        arMatrixFree( work );
        return(-1);
    }
    if( CENTER( work, mean ) < 0 ) {
        arMatrixFree( work );
        return(-1);
    }
    for(i=0; i<row*clm; i++) work->m[i] /= srow;

    rval = PCA( work, evec, ev );
    arMatrixFree( work );

    sum = 0.0;
    for( i = 0; i < ev->clm; i++ ) sum += ev->v[i];
    for( i = 0; i < ev->clm; i++ ) ev->v[i] /= sum;

    return( rval );
}

int arMatrixPCA2( ARMat *input, ARMat *evec, ARVec *ev )
{
    ARMat   *work;
    // double  srow; // unreferenced
    double  sum;
    int     row, clm;
    int     check, rval;
    int     i;

    row = input->row;
    clm = input->clm;
    check = (row < clm)? row: clm;
    if( row < 2 || clm < 2 ) return(-1);
    if( evec->clm != input->clm || evec->row != check ) return(-1);
    if( ev->clm   != check )      return(-1);

    work = arMatrixAllocDup( input );
    if( work == NULL ) return -1;

/*
    srow = sqrt((double)row);
    for(i=0; i<row*clm; i++) work->m[i] /= srow;
*/

    rval = PCA( work, evec, ev );
    arMatrixFree( work );

    sum = 0.0;
    for( i = 0; i < ev->clm; i++ ) sum += ev->v[i];
    for( i = 0; i < ev->clm; i++ ) ev->v[i] /= sum;

    return( rval );
}

static int PCA( ARMat *input, ARMat *output, ARVec *ev )
{
    ARMat     *u;
    double  *m1, *m2;
    int     row, clm, min;
    int     i, j;

    row = input->row;
    clm = input->clm;
    min = (clm < row)? clm: row;
    if( row < 2 || clm < 2 )        return(-1);
    if( output->clm != input->clm ) return(-1);
    if( output->row != min )        return(-1);
    if( ev->clm != min )            return(-1);

    u = arMatrixAlloc( min, min );
    if( u->row != min || u->clm != min ) return(-1);
    if( row < clm ) {
        if( x_by_xt( input, u ) < 0 ) { arMatrixFree(u); return(-1); }
    }
    else {
        if( xt_by_x( input, u ) < 0 ) { arMatrixFree(u); return(-1); }
    }

    if( QRM( u, ev ) < 0 ) { arMatrixFree(u); return(-1); }

    if( row < clm ) {
        if( EV_create( input, u, output, ev ) < 0 ) {
            arMatrixFree(u);
            return(-1);
        }
    }
    else{
        m1 = u->m;
        m2 = output->m;
	for( i = 0; i < min; i++){
	    if( ev->v[i] < VZERO ) break;
            for( j = 0; j < min; j++ ) *(m2++) = *(m1++);
        }
	for( ; i < min; i++){
            ev->v[i] = 0.0;
            for( j = 0; j < min; j++ ) *(m2++) = 0.0;
        }
    }

    arMatrixFree(u);

    return( 0 );
}

static int EX( ARMat *input, ARVec *mean )
{
    double  *m, *v;
    int     row, clm;
    int     i, j;

    row = input->row;
    clm = input->clm;
    if( row <= 0 || clm <= 0 )  return(-1);
    if( mean->clm != clm )      return(-1);

    for( i = 0; i < clm; i++ ) mean->v[i] = 0.0;

    m = input->m;
    for( i = 0; i < row; i++ ) {
        v = mean->v;
        for( j = 0; j < clm; j++ ) *(v++) += *(m++);
    }

    for( i = 0; i < clm; i++ ) mean->v[i] /= row;

    return(0);
}

static int CENTER( ARMat *inout, ARVec *mean )
{
    double  *m, *v;
    int     row, clm;
    int     i, j;

    row = inout->row;
    clm = inout->clm;
    if( mean->clm != clm ) return(-1);

    m = inout->m;
    for( i = 0; i < row; i++ ) {
        v = mean->v;
        for( j = 0; j < clm; j++ ) *(m++) -= *(v++);
    }

    return(0);
}

static int x_by_xt( ARMat *input, ARMat *output )
{
    double  *in1, *in2, *out;
    int     row, clm;
    int     i, j, k;

    row = input->row;
    clm = input->clm;
    if( output->row != row || output->clm != row ) return(-1);

    out = output->m;
    for( i = 0; i < row; i++ ) {
        for( j = 0; j < row; j++ ) {
            if( j < i ) {
                *out = output->m[j*row+i];
            }
            else {
                in1 = &(input->m[clm*i]);
                in2 = &(input->m[clm*j]);
                *out = 0.0;
                for( k = 0; k < clm; k++ ) {
                    *out += *(in1++) * *(in2++);
                }
            }
            out++;
        }
    }

    return(0);
}

static int xt_by_x( ARMat *input, ARMat *output )
{
    double  *in1, *in2, *out;
    int     row, clm;
    int     i, j, k;

    row = input->row;
    clm = input->clm;
    if( output->row != clm || output->clm != clm ) return(-1);

    out = output->m;
    for( i = 0; i < clm; i++ ) {
        for( j = 0; j < clm; j++ ) {
            if( j < i ) {
                *out = output->m[j*clm+i];
            }
            else {
                in1 = &(input->m[i]);
                in2 = &(input->m[j]);
                *out = 0.0;
                for( k = 0; k < row; k++ ) {
                    *out += *in1 * *in2;
                    in1 += clm;
                    in2 += clm;
                }
            }
            out++;
        }
    }

    return(0);
}

static int EV_create( ARMat *input, ARMat *u, ARMat *output, ARVec *ev )
{
    double  *m, *m1, *m2;
    double  sum, work;
    int     row, clm;
    int     i, j, k;

    row = input->row;
    clm = input->clm;
    if( row <= 0 || clm <= 0 ) return(-1);
    if( u->row != row || u->clm != row ) return(-1);
    if( output->row != row || output->clm != clm ) return(-1);
    if( ev->clm != row ) return(-1);

    m = output->m;
    for( i = 0; i < row; i++ ) {
        if( ev->v[i] < VZERO ) break;
        work = 1 / sqrt(fabs(ev->v[i]));
        for( j = 0; j < clm; j++ ) {
            sum = 0.0;
            m1 = &(u->m[i*row]);
            m2 = &(input->m[j]);
            for( k = 0; k < row; k++ ) {
                sum += *m1 * *m2;
                m1++;
                m2 += clm;
            }
            *(m++) = sum * work;
        }
    }
    for( ; i < row; i++ ) {
        ev->v[i] = 0.0;
        for( j = 0; j < clm; j++ ) *(m++) = 0.0;
    }

    return(0);
}

static int QRM( ARMat *a, ARVec *dv )
{
    ARVec     *ev, ev1;
    double  w, t, s, x, y, c;
    double  *v1, *v2;
    int     dim, iter;
    int     i, j, k, h;

    dim = a->row;
    if( dim != a->clm || dim < 2 ) return(-1);
    if( dv->clm != dim ) return(-1);

    ev = arVecAlloc( dim );
    if( ev == NULL ) return(-1);

    ev1.clm = dim-1;
    ev1.v = &(ev->v[1]);
    if( arVecTridiagonalize( a, dv, &ev1 ) < 0 ) {
        arVecFree( ev );
        return(-1);
    }

    ev->v[0] = 0.0;
    for( h = dim-1; h > 0; h-- ) {
        j = h;
        while(j>0 && fabs(ev->v[j]) > EPS*(fabs(dv->v[j-1])+fabs(dv->v[j]))) j--;
        if( j == h ) continue;

        iter = 0;
        do{
            iter++;
            if( iter > MAX_ITER ) break;

            w = (dv->v[h-1] - dv->v[h]) / 2;
            t = ev->v[h] * ev->v[h];
            s = sqrt(w*w+t);
            if( w < 0 ) s = -s;
            x = dv->v[j] - dv->v[h] + t/(w+s);
            y = ev->v[j+1];
            for( k = j; k < h; k++ ) {
                if( fabs(x) >= fabs(y) ) {
		    if( fabs(x) > VZERO ) {
			t = -y / x;
			c = 1 / sqrt(t*t+1);
			s = t * c;
		    }
		    else{
			c = 1.0;
			s = 0.0;
		    }
                }
                else{
		    t = -x / y;
		    s = 1.0 / sqrt(t*t+1);
		    c = t * s;
                }
                w = dv->v[k] - dv->v[k+1];
                t = (w * s + 2 * c * ev->v[k+1]) * s;
                dv->v[k]   -= t;
                dv->v[k+1] += t;
                if( k > j) ev->v[k] = c * ev->v[k] - s * y;
                ev->v[k+1] += s * (c * w - 2 * s * ev->v[k+1]);

                for( i = 0; i < dim; i++ ) {
                    x = a->m[k*dim+i];
                    y = a->m[(k+1)*dim+i];
                    a->m[k*dim+i]     = c * x - s * y;
                    a->m[(k+1)*dim+i] = s * x + c * y;
                }
                if( k < h-1 ) {
                    x = ev->v[k+1];
                    y = -s * ev->v[k+2];
                    ev->v[k+2] *= c;
                }
            }
        } while(fabs(ev->v[h]) > EPS*(fabs(dv->v[h-1])+fabs(dv->v[h])));
    }

    for( k = 0; k < dim-1; k++ ) {
        h = k;
        t = dv->v[h];
        for( i = k+1; i < dim; i++ ) {
            if( dv->v[i] > t ) {
                h = i;
                t = dv->v[h];
            }
        }
        dv->v[h] = dv->v[k];
        dv->v[k] = t;
        v1 = &(a->m[h*dim]);
        v2 = &(a->m[k*dim]);
        for( i = 0; i < dim; i++ ) {
            w = *v1;
            *v1 = *v2;
            *v2 = w;
            v1++;
            v2++;
        }
    }

    arVecFree( ev );
    return(0);
}

static double *minv( double *ap, int dimen, int rowa );

int arMatrixSelfInv(ARMat *m)
{
	if(minv(m->m, m->row, m->row) == NULL) return -1;

	return 0;
}


/********************************/
/*                              */
/*    MATRIX inverse function   */
/*                              */
/********************************/
static double *minv( double *ap, int dimen, int rowa )
{
        double *wap, *wcp, *wbp;/* work pointer                 */
        int i,j,n,ip,nwork;
        int nos[50];
        double epsl;
        double p,pbuf,work;
        //double  fabs();

        epsl = 1.0e-10;         /* Threshold value      */

        switch (dimen) {
                case (0): return(NULL);                 /* check size */
                case (1): *ap = 1.0 / (*ap);
                          return(ap);                   /* 1 dimension */
        }

        for(n = 0; n < dimen ; n++)
                nos[n] = n;

        for(n = 0; n < dimen ; n++) {
                wcp = ap + n * rowa;

                for(i = n, wap = wcp, p = 0.0; i < dimen ; i++, wap += rowa)
                        if( p < ( pbuf = fabs(*wap)) ) {
                                p = pbuf;
                                ip = i;
                        }
                if (p <= epsl)
                        return(NULL);

                nwork = nos[ip];
                nos[ip] = nos[n];
                nos[n] = nwork;

                for(j = 0, wap = ap + ip * rowa, wbp = wcp; j < dimen ; j++) {
                        work = *wap;
                        *wap++ = *wbp;
                        *wbp++ = work;
                }

                for(j = 1, wap = wcp, work = *wcp; j < dimen ; j++, wap++)
                        *wap = *(wap + 1) / work;
                *wap = 1.0 / work;

                for(i = 0; i < dimen ; i++) {
                        if(i != n) {
                                wap = ap + i * rowa;
                                for(j = 1, wbp = wcp, work = *wap;
                                                j < dimen ; j++, wap++, wbp++)
                                        *wap = *(wap + 1) - work * (*wbp);
                                *wap = -work * (*wbp);
                        }
                }
        }

        for(n = 0; n < dimen ; n++) {
                for(j = n; j < dimen ; j++)
                        if( nos[j] == n) break;
                nos[j] = nos[n];
                for(i = 0, wap = ap + j, wbp = ap + n; i < dimen ;
                                        i++, wap += rowa, wbp += rowa) {
                        work = *wap;
                        *wap = *wbp;
                        *wbp = work;
                }
        }
        return(ap);
}

int arMatrixTrans(ARMat *dest, ARMat *source)
{
	int r, c;

	if(dest->row != source->clm || dest->clm != source->row) return -1;

	for(r = 0; r < dest->row; r++) {
		for(c = 0; c < dest->clm; c++) {
			ARELEM0(dest, r, c) = ARELEM0(source, c, r);
		}
	}

	return 0;
}

int arMatrixUnit(ARMat *unit)
{
	int r, c;

	if(unit->row != unit->clm) return -1;

	for(r = 0; r < unit->row; r++) {
		for(c = 0; c < unit->clm; c++) {
			if(r == c) {
				ARELEM0(unit, r, c) = 1.0;
			}
			else {
				ARELEM0(unit, r, c) = 0.0;
			}
		}
	}

	return 0;
}

double arVecHousehold( ARVec *x )
{
    double s, t;
    int    i;

    s = sqrt( arVecInnerproduct(x,x) );

    if( s != 0.0 ) {
        if(x->v[0] < 0) s = -s;
        x->v[0] += s;
        t = 1 / sqrt(x->v[0] * s);
        for( i = 0; i < x->clm; i++ ) {
            x->v[i] *= t;
        }
    }

    return(-s);
}

double arVecInnerproduct( ARVec *x, ARVec *y )
{
    double   result = 0.0;
    int      i;

    if( x->clm != y->clm ) exit(0);

    for( i = 0; i < x->clm; i++ ) {
        result += x->v[i] * y->v[i];
    }

    return( result );
}

int arVecTridiagonalize( ARMat *a, ARVec *d, ARVec *e )
{
    ARVec     wv1, wv2;
    double  *v;
    double  s, t, p, q;
    int     dim;
    int     i, j, k;

    if( a->clm != a->row )   return(-1);
    if( a->clm != d->clm )   return(-1);
    if( a->clm != e->clm+1 ) return(-1);
    dim = a->clm;

    for( k = 0; k < dim-2; k++ ) {
        v = &(a->m[k*dim]);
        d->v[k] = v[k];

        wv1.clm = dim-k-1;
        wv1.v = &(v[k+1]);
        e->v[k] = arVecHousehold(&wv1);
        if( e->v[k] == 0.0 ) continue;

        for( i = k+1; i < dim; i++ ) {
            s = 0.0;
            for( j = k+1; j < i; j++ ) {
                s += a->m[j*dim+i] * v[j];
            }
            for( j = i; j < dim; j++ ) {
                s += a->m[i*dim+j] * v[j];
            }
            d->v[i] = s;
        }

        wv1.clm = wv2.clm = dim-k-1;
        wv1.v = &(v[k+1]);
        wv2.v = &(d->v[k+1]);
        t = arVecInnerproduct( &wv1, &wv2 ) / 2;
        for( i = dim-1; i > k; i-- ) {
            p = v[i];
            q = d->v[i] -= t*p;
            for( j = i; j < dim; j++ ) {
                a->m[i*dim+j] -= p*(d->v[j]) + q*v[j];
            }
        }
    }

    if( dim >= 2) {
        d->v[dim-2] = a->m[(dim-2)*dim+(dim-2)];
        e->v[dim-2] = a->m[(dim-2)*dim+(dim-1)];
    }

    if( dim >= 1 ) d->v[dim-1] = a->m[(dim-1)*dim+(dim-1)];

    for( k = dim-1; k >= 0; k-- ) {
        v = &(a->m[k*dim]);
        if( k < dim-2 ) {
            for( i = k+1; i < dim; i++ ) {
                wv1.clm = wv2.clm = dim-k-1;
                wv1.v = &(v[k+1]);
                wv2.v = &(a->m[i*dim+k+1]);
                t = arVecInnerproduct( &wv1, &wv2 );
                for( j = k+1; j < dim; j++ ) a->m[i*dim+j] -= t * v[j];
            }
        }
        for( i = 0; i < dim; i++ ) v[i] = 0.0;
        v[k] = 1;
    }

    return(0);
}
