/*
arDetectMaker
arGetTransMat
arGetTransMat2
arGetTransMat3
arGetTransMat4
arGetTransMat5
arGetTransMatCont
arMultiGetTransMat
*/

#define P_MAX       500

#define  THRESH_1           2.0
#define  THRESH_2           20.0
#define  THRESH_3           10.0

#define  AR_MULTI_GET_TRANS_MAT_MAX_LOOP_COUNT   2
#define  AR_MULTI_GET_TRANS_MAT_MAX_FIT_ERROR    10.0

#define   AR_GET_TRANS_MAT_MAX_LOOP_COUNT         5
#define   AR_GET_TRANS_MAT_MAX_FIT_ERROR          1.0
#define   AR_GET_TRANS_CONT_MAT_MAX_FIT_ERROR     1.0

#define   AR_AREA_MAX      100000
#define   AR_AREA_MIN          70

#define   AR_SQUARE_MAX        30

#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cmath>

#include <avrVision.h>
#include <avrMath.h>
#include <avrParameters.h>

using namespace std;

typedef struct {
    double   pos[4][2];
    double   thresh;
    double   err;
    int      marker;
    int      dir;
} arMultiEachMarkerInternalInfoT;

/** \struct arPrevInfo
* \brief structure for temporal continuity of tracking
*
* History structure for arDetectMarkerLite and arGetTransMatCont
*/
typedef struct {
    ARMarkerInfo  marker;
    int     count;
} arPrevInfo;

/** \struct  avrPrevInfo
* \brief structure for temporal continuity of tracking
*
* History structure for avrGetTransMatCont
*/
typedef struct {
    avrPatternInfo  marker;
    int     count;
} avrPrevInfo;

static avrPatternInfo  avrmarker_infoL[AR_SQUARE_MAX];

static ARMarkerInfo    marker_infoL[AR_SQUARE_MAX];
static ARMarkerInfo    marker_infoR[AR_SQUARE_MAX];

static int get_vertex( int x_coord[], int y_coord[], int st, int ed,
                       double thresh, int vertex[], int *vnum );

static avrPatternInfo         *avrmarker_info;
static ARMarkerInfo           *wmarker_info;

static ARMarkerInfo2          *marker_info2;
static int                    wmarker_num = 0;

static avrPrevInfo            avrprev_info[AR_SQUARE_MAX];
static arPrevInfo             prev_info[AR_SQUARE_MAX];
static int                    prev_num = 0;

static int check_square( int area, ARMarkerInfo2 *marker_info2, double factor );

static ARMarkerInfo *arGetMarkerInfo( ARUint8 *image, ARMarkerInfo2 *marker_info2, int *marker_num );
static ARMarkerInfo *arsGetMarkerInfo   ( ARUint8 *image, ARMarkerInfo2 *marker_info2, int *marker_num, int LorR );

static ARMarkerInfo2 *arDetectMarker2( ARInt16 *limage, int label_num, int *label_ref, int *warea, double *wpos,
                                      int *wclip, int area_max, int area_min, double factor, int *marker_num );

static int arGetInitRot( ARMarkerInfo *marker_info, double cpara[3][4], double rot[3][3] );

static double arGetTransMat2( double rot[3][3], double pos2d[][2], double pos3d[][2], int num, double conv[3][4] );
static double arGetTransMat3( double rot[3][3], double ppos2d[][2], double ppos3d[][2], int num, double conv[3][4],
                            double *dist_factor, double cpara[3][4] );
static double arGetTransMat4( double rot[3][3], double ppos2d[][2], double ppos3d[][3], int num, double conv[3][4] );
static double arGetTransMat5( double rot[3][3], double ppos2d[][2], double ppos3d[][3],
                            int num, double conv[3][4], double *dist_factor, double cpara[3][4] );

static avrPatternInfo* avrGetMarkerInfo( ARUint8 *image, ARMarkerInfo2 *marker_info2, int *marker_num );

avrPatternInfo *avrGetMarkerInfo( ARUint8 *image,
                               ARMarkerInfo2 *marker_info2, int *marker_num )
{
    int            id, dir;
    double         cf;
    int            i, j;

    for (i = j = 0; i < *marker_num; i++) {
        avrmarker_infoL[j].area   = marker_info2[i].area;
        avrmarker_infoL[j].pos[0] = marker_info2[i].pos[0];
        avrmarker_infoL[j].pos[1] = marker_info2[i].pos[1];

        if (arGetLine(marker_info2[i].x_coord, marker_info2[i].y_coord,
                      marker_info2[i].coord_num, marker_info2[i].vertex,
                      avrmarker_infoL[j].line, avrmarker_infoL[j].vertex) < 0 ) continue;

        arGetCode(image,
                  marker_info2[i].x_coord, marker_info2[i].y_coord,
                  marker_info2[i].vertex, &id, &dir, &cf );

        avrmarker_infoL[j].id  = id;
        avrmarker_infoL[j].dir = dir;
        avrmarker_infoL[j].cf  = cf;

        j++;
    }
    *marker_num = j;

    return (avrmarker_infoL);
}

int avrDetectMarker( ARUint8 *dataPtr, int thresh,
                    avrPatternInfo **marker_info, int *marker_num )
{
    ARInt16                *limage;
    int                    label_num;
    int                    *area, *clip, *label_ref;
    double                 *pos;
    double                 rarea, rlen, rlenmin;
    double                 diff, diffmin;
    int                    cid, cdir;
    int                    i, j, k;

    *marker_num = 0;

    limage = arLabeling( dataPtr, thresh,
                         &label_num, &area, &pos, &clip, &label_ref );
    if( limage == 0 )    return -1;

    marker_info2 = arDetectMarker2( limage, label_num, label_ref,
                                    area, pos, clip, AR_AREA_MAX, AR_AREA_MIN,
                                    1.0, &wmarker_num);
    if( marker_info2 == 0 ) return -1;

    avrmarker_info = avrGetMarkerInfo( dataPtr, marker_info2, &wmarker_num );
    if( avrmarker_info == 0 ) return -1;

    for( i = 0; i < prev_num; i++ ) {
        rlenmin = 10.0;
        cid = -1;
        for( j = 0; j < wmarker_num; j++ ) {
            rarea = (double)avrprev_info[i].marker.area / (double)avrmarker_info[j].area;
            if( rarea < 0.7 || rarea > 1.43 ) continue;
            rlen = ( (avrmarker_info[j].pos[0] - avrprev_info[i].marker.pos[0])
                   * (avrmarker_info[j].pos[0] - avrprev_info[i].marker.pos[0])
                   + (avrmarker_info[j].pos[1] - avrprev_info[i].marker.pos[1])
                   * (avrmarker_info[j].pos[1] - avrprev_info[i].marker.pos[1]) ) / avrmarker_info[j].area;
            if( rlen < 0.5 && rlen < rlenmin ) {
                rlenmin = rlen;
                cid = j;
            }
        }
        if( cid >= 0 && avrmarker_info[cid].cf < avrprev_info[i].marker.cf ) {
            avrmarker_info[cid].cf = avrprev_info[i].marker.cf;
            avrmarker_info[cid].id = avrprev_info[i].marker.id;
            diffmin = 10000.0 * 10000.0;
            cdir = -1;
            for( j = 0; j < 4; j++ ) {
                diff = 0;
                for( k = 0; k < 4; k++ ) {
                    diff += (avrprev_info[i].marker.vertex[k][0] - avrmarker_info[cid].vertex[(j+k)%4][0])
                          * (avrprev_info[i].marker.vertex[k][0] - avrmarker_info[cid].vertex[(j+k)%4][0])
                          + (avrprev_info[i].marker.vertex[k][1] - avrmarker_info[cid].vertex[(j+k)%4][1])
                          * (avrprev_info[i].marker.vertex[k][1] - avrmarker_info[cid].vertex[(j+k)%4][1]);
                }
                if( diff < diffmin ) {
                    diffmin = diff;
                    cdir = (avrprev_info[i].marker.dir - j + 4) % 4;
                }
            }
            avrmarker_info[cid].dir = cdir;
        }
    }

    for( i = 0; i < wmarker_num; i++ ) {
/*
	printf("cf = %g\n", avrmarker_info[i].cf);
*/
        if( avrmarker_info[i].cf < 0.5 ) avrmarker_info[i].id = -1;
   }


/*------------------------------------------------------------*/

    for( i = j = 0; i < prev_num; i++ ) {
        avrprev_info[i].count++;
        if( avrprev_info[i].count < 4 ) {
            avrprev_info[j] = avrprev_info[i];
            j++;
        }
    }
    prev_num = j;

    for( i = 0; i < wmarker_num; i++ ) {
        if( avrmarker_info[i].id < 0 ) continue;

        for( j = 0; j < prev_num; j++ ) {
            if( avrprev_info[j].marker.id == avrmarker_info[i].id ) break;
        }
        avrprev_info[j].marker = avrmarker_info[i];
        avrprev_info[j].count  = 1;
        if( j == prev_num ) prev_num++;
    }

    for( i = 0; i < prev_num; i++ ) {
        for( j = 0; j < wmarker_num; j++ ) {
            rarea = (double)avrprev_info[i].marker.area / (double)avrmarker_info[j].area;
            if( rarea < 0.7 || rarea > 1.43 ) continue;
            rlen = ( (avrmarker_info[j].pos[0] - avrprev_info[i].marker.pos[0])
                   * (avrmarker_info[j].pos[0] - avrprev_info[i].marker.pos[0])
                   + (avrmarker_info[j].pos[1] - avrprev_info[i].marker.pos[1])
                   * (avrmarker_info[j].pos[1] - avrprev_info[i].marker.pos[1]) ) / avrmarker_info[j].area;
            if( rlen < 0.5 ) break;
        }
        if( j == wmarker_num ) {
            avrmarker_info[wmarker_num] = avrprev_info[i].marker;
            wmarker_num++;
        }
    }


    *marker_num  = wmarker_num;
    *marker_info = avrmarker_info;

    return 0;
}

ARMarkerInfo *arGetMarkerInfo( ARUint8 *image,
                               ARMarkerInfo2 *marker_info2, int *marker_num )
{
    int            id, dir;
    double         cf;
    int            i, j;

    for (i = j = 0; i < *marker_num; i++) {
        marker_infoL[j].area   = marker_info2[i].area;
        marker_infoL[j].pos[0] = marker_info2[i].pos[0];
        marker_infoL[j].pos[1] = marker_info2[i].pos[1];

        if (arGetLine(marker_info2[i].x_coord, marker_info2[i].y_coord,
                      marker_info2[i].coord_num, marker_info2[i].vertex,
                      marker_infoL[j].line, marker_infoL[j].vertex) < 0 ) continue;

        arGetCode(image,
                  marker_info2[i].x_coord, marker_info2[i].y_coord,
                  marker_info2[i].vertex, &id, &dir, &cf );

        marker_infoL[j].id  = id;
        marker_infoL[j].dir = dir;
        marker_infoL[j].cf  = cf;

        j++;
    }
    *marker_num = j;

    return (marker_infoL);
}

ARMarkerInfo *arsGetMarkerInfo( ARUint8 *image, ARMarkerInfo2 *marker_info2, int *marker_num, int LorR )
{
    ARMarkerInfo   *info;
    int            id, dir;
    double         cf;
    int            i, j;

    if (LorR) info = &marker_infoL[0];
	else      info = &marker_infoR[0];

    for (i = j = 0; i < *marker_num; i++) {
        info[j].area   = marker_info2[i].area;
        info[j].pos[0] = marker_info2[i].pos[0];
        info[j].pos[1] = marker_info2[i].pos[1];

        if (arsGetLine(marker_info2[i].x_coord, marker_info2[i].y_coord,
                       marker_info2[i].coord_num, marker_info2[i].vertex,
                       info[j].line, info[j].vertex, LorR) < 0 ) continue;

        arGetCode(image,
                  marker_info2[i].x_coord, marker_info2[i].y_coord,
                  marker_info2[i].vertex, &id, &dir, &cf );

        info[j].id  = id;
        info[j].dir = dir;
        info[j].cf  = cf;

        j++;
    }
    *marker_num = j;

    return (info);
}

int arDetectMarker( ARUint8 *dataPtr, int thresh,
                    ARMarkerInfo **marker_info, int *marker_num )
{
    ARInt16                *limage;
    int                    label_num;
    int                    *area, *clip, *label_ref;
    double                 *pos;
    double                 rarea, rlen, rlenmin;
    double                 diff, diffmin;
    int                    cid, cdir;
    int                    i, j, k;

    *marker_num = 0;

    limage = arLabeling( dataPtr, thresh,
                         &label_num, &area, &pos, &clip, &label_ref );
    if( limage == 0 )    return -1;

    marker_info2 = arDetectMarker2( limage, label_num, label_ref,
                                    area, pos, clip, AR_AREA_MAX, AR_AREA_MIN,
                                    1.0, &wmarker_num);
    if( marker_info2 == 0 ) return -1;

    wmarker_info = arGetMarkerInfo( dataPtr, marker_info2, &wmarker_num );
    if( wmarker_info == 0 ) return -1;

    for( i = 0; i < prev_num; i++ ) {
        rlenmin = 10.0;
        cid = -1;
        for( j = 0; j < wmarker_num; j++ ) {
            rarea = (double)prev_info[i].marker.area / (double)wmarker_info[j].area;
            if( rarea < 0.7 || rarea > 1.43 ) continue;
            rlen = ( (wmarker_info[j].pos[0] - prev_info[i].marker.pos[0])
                   * (wmarker_info[j].pos[0] - prev_info[i].marker.pos[0])
                   + (wmarker_info[j].pos[1] - prev_info[i].marker.pos[1])
                   * (wmarker_info[j].pos[1] - prev_info[i].marker.pos[1]) ) / wmarker_info[j].area;
            if( rlen < 0.5 && rlen < rlenmin ) {
                rlenmin = rlen;
                cid = j;
            }
        }
        if( cid >= 0 && wmarker_info[cid].cf < prev_info[i].marker.cf ) {
            wmarker_info[cid].cf = prev_info[i].marker.cf;
            wmarker_info[cid].id = prev_info[i].marker.id;
            diffmin = 10000.0 * 10000.0;
            cdir = -1;
            for( j = 0; j < 4; j++ ) {
                diff = 0;
                for( k = 0; k < 4; k++ ) {
                    diff += (prev_info[i].marker.vertex[k][0] - wmarker_info[cid].vertex[(j+k)%4][0])
                          * (prev_info[i].marker.vertex[k][0] - wmarker_info[cid].vertex[(j+k)%4][0])
                          + (prev_info[i].marker.vertex[k][1] - wmarker_info[cid].vertex[(j+k)%4][1])
                          * (prev_info[i].marker.vertex[k][1] - wmarker_info[cid].vertex[(j+k)%4][1]);
                }
                if( diff < diffmin ) {
                    diffmin = diff;
                    cdir = (prev_info[i].marker.dir - j + 4) % 4;
                }
            }
            wmarker_info[cid].dir = cdir;
        }
    }

    for( i = 0; i < wmarker_num; i++ ) {
/*
	printf("cf = %g\n", wmarker_info[i].cf);
*/
        if( wmarker_info[i].cf < 0.5 ) wmarker_info[i].id = -1;
   }


/*------------------------------------------------------------*/

    for( i = j = 0; i < prev_num; i++ ) {
        prev_info[i].count++;
        if( prev_info[i].count < 4 ) {
            prev_info[j] = prev_info[i];
            j++;
        }
    }
    prev_num = j;

    for( i = 0; i < wmarker_num; i++ ) {
        if( wmarker_info[i].id < 0 ) continue;

        for( j = 0; j < prev_num; j++ ) {
            if( prev_info[j].marker.id == wmarker_info[i].id ) break;
        }
        prev_info[j].marker = wmarker_info[i];
        prev_info[j].count  = 1;
        if( j == prev_num ) prev_num++;
    }

    for( i = 0; i < prev_num; i++ ) {
        for( j = 0; j < wmarker_num; j++ ) {
            rarea = (double)prev_info[i].marker.area / (double)wmarker_info[j].area;
            if( rarea < 0.7 || rarea > 1.43 ) continue;
            rlen = ( (wmarker_info[j].pos[0] - prev_info[i].marker.pos[0])
                   * (wmarker_info[j].pos[0] - prev_info[i].marker.pos[0])
                   + (wmarker_info[j].pos[1] - prev_info[i].marker.pos[1])
                   * (wmarker_info[j].pos[1] - prev_info[i].marker.pos[1]) ) / wmarker_info[j].area;
            if( rlen < 0.5 ) break;
        }
        if( j == wmarker_num ) {
            wmarker_info[wmarker_num] = prev_info[i].marker;
            wmarker_num++;
        }
    }


    *marker_num  = wmarker_num;
    *marker_info = wmarker_info;

    return 0;
}

static ARMarkerInfo2    marker_info3[AR_SQUARE_MAX];

ARMarkerInfo2 *arDetectMarker2( ARInt16 *limage, int label_num, int *label_ref,
                                int *warea, double *wpos, int *wclip,
                                int area_max, int area_min, double factor, int *marker_num )
{
    ARMarkerInfo2     *pm;
    int               xsize, ysize;
    int               marker_num2;
    int               i, j, ret;
    double            d;

    if( arImageProcMode == AR_IMAGE_PROC_IN_HALF ) {
        area_min /= 4;
        area_max /= 4;
        xsize = arImXsize / 2;
        ysize = arImYsize / 2;
    }
    else {
        xsize = arImXsize;
        ysize = arImYsize;
    }
    marker_num2 = 0;
    for(i=0; i<label_num; i++ ) {
        if( warea[i] < area_min || warea[i] > area_max ) continue;
        if( wclip[i*4+0] == 1 || wclip[i*4+1] == xsize-2 ) continue;
        if( wclip[i*4+2] == 1 || wclip[i*4+3] == ysize-2 ) continue;

        ret = arGetContour( limage, label_ref, i+1,
                            &(wclip[i*4]), &(marker_info3[marker_num2]));
        if( ret < 0 ) continue;

        ret = check_square( warea[i], &(marker_info3[marker_num2]), factor );
        if( ret < 0 ) continue;

        marker_info3[marker_num2].area   = warea[i];
        marker_info3[marker_num2].pos[0] = wpos[i*2+0];
        marker_info3[marker_num2].pos[1] = wpos[i*2+1];
        marker_num2++;
        if( marker_num2 == AR_SQUARE_MAX ) break;
    }

    for( i=0; i < marker_num2; i++ ) {
        for( j=i+1; j < marker_num2; j++ ) {
            d = (marker_info3[i].pos[0] - marker_info3[j].pos[0])
              * (marker_info3[i].pos[0] - marker_info3[j].pos[0])
              + (marker_info3[i].pos[1] - marker_info3[j].pos[1])
              * (marker_info3[i].pos[1] - marker_info3[j].pos[1]);
            if( marker_info3[i].area > marker_info3[j].area ) {
                if( d < marker_info3[i].area / 4 ) {
                    marker_info3[j].area = 0;
                }
            }
            else {
                if( d < marker_info3[j].area / 4 ) {
                    marker_info3[i].area = 0;
                }
            }
        }
    }
    for( i=0; i < marker_num2; i++ ) {
        if( marker_info3[i].area == 0.0 ) {
            for( j=i+1; j < marker_num2; j++ ) {
                marker_info3[j-1] = marker_info3[j];
            }
            marker_num2--;
        }
    }

    if( arImageProcMode == AR_IMAGE_PROC_IN_HALF ) {
        pm = &(marker_info3[0]);
        for( i = 0; i < marker_num2; i++ ) {
            pm->area *= 4;
            pm->pos[0] *= 2.0;
            pm->pos[1] *= 2.0;
            for( j = 0; j< pm->coord_num; j++ ) {
                pm->x_coord[j] *= 2;
                pm->y_coord[j] *= 2;
            }
            pm++;
        }
    }

    *marker_num = marker_num2;
    return( &(marker_info3[0]) );
}

int arDetectMarkerLite( ARUint8 *dataPtr, int thresh,
                        ARMarkerInfo **marker_info, int *marker_num )
{
    ARInt16                *limage;
    int                    label_num;
    int                    *area, *clip, *label_ref;
    double                 *pos;
    int                    i;

    *marker_num = 0;

    limage = arLabeling( dataPtr, thresh,
                         &label_num, &area, &pos, &clip, &label_ref );
    if( limage == 0 )    return -1;

    marker_info2 = arDetectMarker2( limage, label_num, label_ref,
                                    area, pos, clip, AR_AREA_MAX, AR_AREA_MIN,
                                    1.0, &wmarker_num);
    if( marker_info2 == 0 ) return -1;

    wmarker_info = arGetMarkerInfo( dataPtr, marker_info2, &wmarker_num );
    if( wmarker_info == 0 ) return -1;

    for( i = 0; i < wmarker_num; i++ ) {
        if( wmarker_info[i].cf < 0.5 ) wmarker_info[i].id = -1;
    }


    *marker_num  = wmarker_num;
    *marker_info = wmarker_info;

    return 0;
}

static int get_vertex( int x_coord[], int y_coord[], int st,  int ed,
                       double thresh, int vertex[], int *vnum)
{
    double   d, dmax;
    double   a, b, c;
    int      i, v1;

    a = y_coord[ed] - y_coord[st];
    b = x_coord[st] - x_coord[ed];
    c = x_coord[ed]*y_coord[st] - y_coord[ed]*x_coord[st];
    dmax = 0;
    for(i=st+1;i<ed;i++) {
        d = a*x_coord[i] + b*y_coord[i] + c;
        if( d*d > dmax ) {
            dmax = d*d;
            v1 = i;
        }
    }
    if( dmax/(a*a+b*b) > thresh ) {
        if( get_vertex(x_coord, y_coord, st,  v1, thresh, vertex, vnum) < 0 )
            return(-1);

        if( (*vnum) > 5 ) return(-1);
        vertex[(*vnum)] = v1;
        (*vnum)++;

        if( get_vertex(x_coord, y_coord, v1,  ed, thresh, vertex, vnum) < 0 )
            return(-1);
    }

    return(0);
}

static int check_square( int area, ARMarkerInfo2 *marker_info2, double factor )
{
    int             sx, sy;
    int             dmax, d, v1;
    int             vertex[10];
    int             wv1[10], wvnum1, wv2[10], wvnum2, v2;
    double          thresh;
    int             i;


    dmax = 0;
    v1 = 0;
    sx = marker_info2->x_coord[0];
    sy = marker_info2->y_coord[0];
    for(i=1;i<marker_info2->coord_num-1;i++) {
        d = (marker_info2->x_coord[i]-sx)*(marker_info2->x_coord[i]-sx)
          + (marker_info2->y_coord[i]-sy)*(marker_info2->y_coord[i]-sy);
        if( d > dmax ) {
            dmax = d;
            v1 = i;
        }
    }

    thresh = (area/0.75) * 0.01 * factor;
    vertex[0] = 0;
    wvnum1 = 0;
    wvnum2 = 0;
    if( get_vertex(marker_info2->x_coord, marker_info2->y_coord, 0,  v1,
                   thresh, wv1, &wvnum1) < 0 ) {
        return(-1);
    }
    if( get_vertex(marker_info2->x_coord, marker_info2->y_coord,
                   v1,  marker_info2->coord_num-1, thresh, wv2, &wvnum2) < 0 ) {
        return(-1);
    }

    if( wvnum1 == 1 && wvnum2 == 1 ) {
        vertex[1] = wv1[0];
        vertex[2] = v1;
        vertex[3] = wv2[0];
    }
    else if( wvnum1 > 1 && wvnum2 == 0 ) {
        v2 = v1 / 2;
        wvnum1 = wvnum2 = 0;
        if( get_vertex(marker_info2->x_coord, marker_info2->y_coord,
                       0,  v2, thresh, wv1, &wvnum1) < 0 ) {
            return(-1);
        }
        if( get_vertex(marker_info2->x_coord, marker_info2->y_coord,
                       v2,  v1, thresh, wv2, &wvnum2) < 0 ) {
            return(-1);
        }
        if( wvnum1 == 1 && wvnum2 == 1 ) {
            vertex[1] = wv1[0];
            vertex[2] = wv2[0];
            vertex[3] = v1;
        }
        else {
            return(-1);
        }
    }
    else if( wvnum1 == 0 && wvnum2 > 1 ) {
        v2 = (v1 + marker_info2->coord_num-1) / 2;
        wvnum1 = wvnum2 = 0;
        if( get_vertex(marker_info2->x_coord, marker_info2->y_coord,
                   v1, v2, thresh, wv1, &wvnum1) < 0 ) {
            return(-1);
        }
        if( get_vertex(marker_info2->x_coord, marker_info2->y_coord,
                   v2, marker_info2->coord_num-1, thresh, wv2, &wvnum2) < 0 ) {
            return(-1);
        }
        if( wvnum1 == 1 && wvnum2 == 1 ) {
            vertex[1] = v1;
            vertex[2] = wv1[0];
            vertex[3] = wv2[0];
        }
        else {
            return(-1);
        }
    }
    else {
        return(-1);
    }

    marker_info2->vertex[0] = vertex[0];
    marker_info2->vertex[1] = vertex[1];
    marker_info2->vertex[2] = vertex[2];
    marker_info2->vertex[3] = vertex[3];
    marker_info2->vertex[4] = marker_info2->coord_num-1;

    return(0);
}

static double   pos2d[P_MAX][2];
static double   pos3d[P_MAX][3];

static int  check_dir( double dir[3], double st[2], double ed[2],
                       double cpara[3][4] );

static double avrGetTransMatContSub( avrPatternInfo *marker_info, avrMatrix3x4& prev_conv,
                                    double center[2], double width, avrMatrix3x4& conv);

static double avrGetTransMatSub( double rot[3][3], double ppos2d[][2], double pos3d[][3], int num, avrMatrix3x4& conv,
                                double *dist_factor, double cpara[3][4] );

static int avrVerify_markers(avrPatternInfo *marker_info, int marker_num, ARMultiMarkerInfoT *config);

static int  check_rotation( double rot[2][3] );

static double arGetTransMatContSub( ARMarkerInfo *marker_info, double prev_conv[3][4],
                                    double center[2], double width, double conv[3][4] );

static int verify_markers(ARMarkerInfo *marker_info, int marker_num, ARMultiMarkerInfoT *config);

static double arGetTransMatSub( double rot[3][3], double ppos2d[][2],
                                double pos3d[][3], int num, double conv[3][4],
                                double *dist_factor, double cpara[3][4] );

int avrGetInitRot( avrPatternInfo *marker_info, double cpara[3][4], double rot[3][3] );
double avrGetTransMat3( double rot[3][3], double ppos2d[][2], double ppos3d[][2], int num, avrMatrix3x4& conv, double *dist_factor, double cpara[3][4] );

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
        err = avrGetTransMat3( rot, ppos2d, ppos3d, 4, conv, arParam.dist_factor, arParam.mat );
        if( err < AR_GET_TRANS_MAT_MAX_FIT_ERROR ) break;
    }
    return err;
}

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

        err = avrGetTransMat(&marker_info[k], config->marker[i].center, config->marker[i].width, trans1);
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
            max_area   = marker_info[max_marker].area;
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
            max_area   = marker_info[max_marker].area;
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
