/*
  Name:        avrMarker.h
  Version      0.1
  Author:      Felipe Caetano, Luiz Maurílio, Rodrigo L. S. Silva
  Date:        10/10/2012
  Last Update: 10/10/2012
  Description: Collection of the "marker" functions of ARToolkit. Receive functions
               from arDetectMarker, arDetectMarker2 and arMarkerInfo.
*/

#ifndef AVR_MARKER_H
#define AVR_MARKER_H

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
//	Public includes.
// ============================================================================

#include <stdio.h>
//#include <AR/config.h>
//#include <AR/param.h>
//#include <AR/ar.h>

#include <avrUtil.h>
// ============================================================================
//	Public types and defines.
// ============================================================================

///ar.h
/*
   Detection
*/

/**
* \brief main function to detect the square markers in the video input frame.
*
* This function proceeds to thresholding, labeling, contour extraction and line corner estimation
* (and maintains an history).
* It's one of the main function of the detection routine with arGetTransMat.
* \param dataPtr a pointer to the color image which is to be searched for square markers.
*                The pixel format depend of your architecture. Generally ABGR, but the images
*                are treated as a gray scale, so the order of BGR components does not matter.
*                However the ordering of the alpha comp, A, is important.
* \param thresh  specifies the threshold value (between 0-255) to be used to convert
*                the input image into a binary image.
* \param marker_info a pointer to an array of ARMarkerInfo structures returned
*                    which contain all the information about the detected squares in the image
* \param marker_num the number of detected markers in the image.
* \return 0 when the function completes normally, -1 otherwise
*/
int arDetectMarker( ARUint8 *dataPtr, int thresh,
                    ARMarkerInfo **marker_info, int *marker_num );

/**
* \brief main function to detect rapidly the square markers in the video input frame.
*
* this function is a simpler version of arDetectMarker that does not have the
* same error correction functions and so runs a little faster, but is more error prone
*
* \param dataPtr a pointer to the color image which is to be searched for square markers.
*                The pixel format depend of your architecture. Generally ABGR, but the images
*                are treated as a gray scale, so the order of BGR components does not matter.
*                However the ordering of the alpha component, A, is important.
* \param thresh  specifies the threshold value (between 0-255) to be used to convert
*                the input image into a binary image.
* \param marker_info a pointer to an array of ARMarkerInfo structures returned
*                    which contain all the information about the detected squares in the image
* \param marker_num the number of detected markers in the image.
* \return 0 when the function completes normally, -1 otherwise
*/
int arDetectMarkerLite( ARUint8 *dataPtr, int thresh,
                        ARMarkerInfo **marker_info, int *marker_num );

/**
* \brief save a marker.
*
* used in mk_patt to save a bitmap of the pattern of the currently detected marker.
* The saved image is a table of the normalized viewed pattern.
* \param image a pointer to the image containing the marker pattern to be trained.
* \param marker_info a pointer to the ARMarkerInfo structure of the pattern to be trained.
* \param filename The name of the file where the bitmap image is to be saved.
* \return 0 if the bitmap image is successfully saved, -1 otherwise.
*/
int arSavePatt( ARUint8 *image,
                ARMarkerInfo *marker_info, char *filename );

/**
* \brief   XXXBK
*
*   XXXBK
* \param limage  XXXBK
* \param label_num  XXXBK
* \param label_ref  XXXBK
* \param warea  XXXBK
* \param wpos  XXXBK
* \param wclip  XXXBK
* \param area_max  XXXBK
* \param area_min  XXXBK
* \param factor  XXXBK
* \param marker_num  XXXBK
* \return XXXBK  XXXBK
*/
ARMarkerInfo2 *arDetectMarker2( ARInt16 *limage,
                                int label_num, int *label_ref,
                                int *warea, double *wpos, int *wclip,
                                int area_max, int area_min, double factor, int *marker_num );

/**
* \brief information on
*
*  XXXBK
* \param image XXXBK
* \param marker_info2 XXXBK
* \param marker_num XXXBK
* \return XXXBK
*/
ARMarkerInfo *arGetMarkerInfo( ARUint8 *image,
                               ARMarkerInfo2 *marker_info2, int *marker_num );

/** \struct arPrevInfo
* \brief structure for temporal continuity of tracking
*
* History structure for arDetectMarkerLite and arGetTransMatCont
*/
typedef struct {
    ARMarkerInfo  marker;
    int     count;
} arPrevInfo;

ARMarkerInfo *arsGetMarkerInfo   ( ARUint8 *image,
                                   ARMarkerInfo2 *marker_info2, int *marker_num, int LorR );

int           arsDetectMarker    ( ARUint8 *dataPtr, int thresh,
                                   ARMarkerInfo **marker_info, int *marker_num, int LorR );

int           arsDetectMarkerLite( ARUint8 *dataPtr, int thresh,
                                   ARMarkerInfo **marker_info, int *marker_num, int LorR );

///ar.h

///arMulti.h

/**
* \brief loading multi-markers description from a file
*
* Load a configuration file for multi-markers tracking. The configuration
* file furnishs pointer to each pattern description.
*
* \param filename name of the pattern file
* \return a pattern structure, NULL if error
*/
ARMultiMarkerInfoT *arMultiReadConfigFile( const char *filename );

#ifdef __cplusplus
}
#endif

#include <avrPatternInfo.h>

typedef struct {
    avrPatternInfo  marker;
    int     count;
} avrPrevInfo;

int avrDetectMarker( ARUint8 *dataPtr, int thresh, avrPatternInfo **marker_info, int *marker_num );
avrPatternInfo *avrGetMarkerInfo( ARUint8 *image, ARMarkerInfo2 *marker_info2, int *marker_num );

#endif
