/*
arDetectMarker
arDetectMarker2
arMarkerInfo
*/

#include <cstdio>
#include <cstdlib>
//#include <AR/ar.h>

#include <avrPatt.h>       // << inclui avrUtil
#include <avrMarker.h>     // << inclui avrParameters que inclui avrUtil
#include <avrGraphics.h>   // << inclui avrParameters que inclui avrUtil

static avrPatternInfo         *avrmarker_info;
static ARMarkerInfo           *wmarker_info;
static ARMarkerInfo2          *marker_info2;
static int                    wmarker_num = 0;

static avrPrevInfo            avrprev_info[AR_SQUARE_MAX];
static arPrevInfo             prev_info[AR_SQUARE_MAX];
static int                    prev_num = 0;

static arPrevInfo             sprev_info[2][AR_SQUARE_MAX];
static int                    sprev_num[2] = {0,0};

static char *get_buff( char *buf, int n, FILE *fp );
#include <iostream>
using namespace std;
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

static char *get_buff( char *buf, int n, FILE *fp )
{
    char *ret;

    for(;;) {
        ret = fgets( buf, n, fp );
        if( ret == NULL ) return(NULL);
        if( buf[0] != '\n' && buf[0] != '\r' && buf[0] != '#' ) return(ret);
    }
}

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

int arsDetectMarker( ARUint8 *dataPtr, int thresh,
                     ARMarkerInfo **marker_info, int *marker_num, int LorR )
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

    limage = arsLabeling( dataPtr, thresh,
                          &label_num, &area, &pos, &clip, &label_ref, LorR );
    if( limage == 0 )    return -1;

    marker_info2 = arDetectMarker2( limage, label_num, label_ref,
                                    area, pos, clip, AR_AREA_MAX, AR_AREA_MIN,
                                    1.0, &wmarker_num);
    if( marker_info2 == 0 ) return -1;

    wmarker_info = arsGetMarkerInfo( dataPtr, marker_info2, &wmarker_num, LorR );
    if( wmarker_info == 0 ) return -1;

    for( i = 0; i < sprev_num[LorR]; i++ ) {
        rlenmin = 10.0;
        cid = -1;
        for( j = 0; j < wmarker_num; j++ ) {
            rarea = (double)sprev_info[LorR][i].marker.area / (double)wmarker_info[j].area;
            if( rarea < 0.7 || rarea > 1.43 ) continue;
            rlen = ( (wmarker_info[j].pos[0] - sprev_info[LorR][i].marker.pos[0])
                   * (wmarker_info[j].pos[0] - sprev_info[LorR][i].marker.pos[0])
                   + (wmarker_info[j].pos[1] - sprev_info[LorR][i].marker.pos[1])
                   * (wmarker_info[j].pos[1] - sprev_info[LorR][i].marker.pos[1]) ) / wmarker_info[j].area;
            if( rlen < 0.5 && rlen < rlenmin ) {
                rlenmin = rlen;
                cid = j;
            }
        }
        if( cid >= 0 && wmarker_info[cid].cf < sprev_info[LorR][i].marker.cf ) {
            wmarker_info[cid].cf = sprev_info[LorR][i].marker.cf;
            wmarker_info[cid].id = sprev_info[LorR][i].marker.id;
            diffmin = 10000.0 * 10000.0;
            cdir = -1;
            for( j = 0; j < 4; j++ ) {
                diff = 0;
                for( k = 0; k < 4; k++ ) {
                    diff += (sprev_info[LorR][i].marker.vertex[k][0] - wmarker_info[cid].vertex[(j+k)%4][0])
                          * (sprev_info[LorR][i].marker.vertex[k][0] - wmarker_info[cid].vertex[(j+k)%4][0])
                          + (sprev_info[LorR][i].marker.vertex[k][1] - wmarker_info[cid].vertex[(j+k)%4][1])
                          * (sprev_info[LorR][i].marker.vertex[k][1] - wmarker_info[cid].vertex[(j+k)%4][1]);
                }
                if( diff < diffmin ) {
                    diffmin = diff;
                    cdir = (sprev_info[LorR][i].marker.dir - j + 4) % 4;
                }
            }
            wmarker_info[cid].dir = cdir;
        }
    }

    for( i = 0; i < wmarker_num; i++ ) {
        if( wmarker_info[i].cf < 0.5 ) wmarker_info[i].id = -1;
    }

    j = 0;
    for( i = 0; i < wmarker_num; i++ ) {
        if( wmarker_info[i].id < 0 ) continue;
        sprev_info[LorR][j].marker = wmarker_info[i];
        sprev_info[LorR][j].count  = 1;
        j++;
    }
    sprev_num[LorR] = j;

    *marker_num  = wmarker_num;
    *marker_info = wmarker_info;

    return 0;
}

int arsDetectMarkerLite( ARUint8 *dataPtr, int thresh,
                         ARMarkerInfo **marker_info, int *marker_num, int LorR )
{
    ARInt16                *limage;
    int                    label_num;
    int                    *area, *clip, *label_ref;
    double                 *pos;
    int                    i;

    *marker_num = 0;

    limage = arsLabeling( dataPtr, thresh,
                          &label_num, &area, &pos, &clip, &label_ref, LorR );
    if( limage == 0 )    return -1;

    marker_info2 = arDetectMarker2( limage, label_num, label_ref,
                                    area, pos, clip, AR_AREA_MAX, AR_AREA_MIN,
                                    1.0, &wmarker_num);
    if( marker_info2 == 0 ) return -1;

    wmarker_info = arsGetMarkerInfo( dataPtr, marker_info2, &wmarker_num, LorR );
    if( wmarker_info == 0 ) return -1;

    for( i = 0; i < wmarker_num; i++ ) {
        if( wmarker_info[i].cf < 0.5 ) wmarker_info[i].id = -1;
    }


    *marker_num  = wmarker_num;
    *marker_info = wmarker_info;

    return 0;
}

static int check_square( int area, ARMarkerInfo2 *marker_info2, double factor );

static int get_vertex( int x_coord[], int y_coord[], int st, int ed,
                       double thresh, int vertex[], int *vnum );

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

int arGetContour( ARInt16 *limage, int *label_ref,
                  int label, int clip[4], ARMarkerInfo2 *marker_info2 )
{
    static int      xdir[8] = { 0, 1, 1, 1, 0,-1,-1,-1};
    static int      ydir[8] = {-1,-1, 0, 1, 1, 1, 0,-1};
    static int      wx[AR_CHAIN_MAX];
    static int      wy[AR_CHAIN_MAX];
    ARInt16         *p1;
    int             xsize, ysize;
    int             sx, sy, dir;
    int             dmax, d, v1;
    int             i, j;

    if( arImageProcMode == AR_IMAGE_PROC_IN_HALF ) {
        xsize = arImXsize / 2;
        ysize = arImYsize / 2;
    }
    else {
        xsize = arImXsize;
        ysize = arImYsize;
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

static int check_square( int area, ARMarkerInfo2 *marker_info2, double factor )
{
    int             sx, sy;
    int             dmax, d, v1;
    int             vertex[10], vnum;
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
    vnum = 1;
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

static avrPatternInfo  avrmarker_infoL[AR_SQUARE_MAX];
static avrPatternInfo  avrmarker_infoR[AR_SQUARE_MAX];
static ARMarkerInfo    marker_infoL[AR_SQUARE_MAX];
static ARMarkerInfo    marker_infoR[AR_SQUARE_MAX];

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

ARMarkerInfo *arsGetMarkerInfo( ARUint8 *image,
                                ARMarkerInfo2 *marker_info2, int *marker_num, int LorR )
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
