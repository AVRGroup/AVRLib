/*
  Name:        avrVision.h
  Version      1.0.1
  Author:      Douglas Oliveira, Danilo Machado, Rodrigo L. S. Silva
  Date:        10/10/2012
  Last Update: 09/10/2014
  Description: collection of the computacional vision functions present in ARToolKit and the correspondents avr functions
               * DetectMarker collection
               * GetTransMat collection
*/

#ifndef AVR_VISION_H
#define AVR_VISION_H

#include <avrPatternInfo.h>
#include <avrMatrix3x4.h>
#include <avrUtil.h>

//ar.h

/** \fn int arDetectMarker( ARUint8 *dataPtr, int thresh, ARMarkerInfo **marker_info, int *marker_num )
* \brief main function to detect the square markers in the video input frame.
*
* This function proceeds to thresholding, labeling, contour extraction and line corner estimation
* (and maintains an history).
* It's one of the main function of the detection routine with arGetTransMat and avrGetTransMat.
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
int arDetectMarker( ARUint8 *dataPtr, int thresh, ARMarkerInfo **marker_info, int *marker_num );
/** \fn int avrDetectMarker( ARUint8 *dataPtr, int thresh, avrPatternInfo **marker_info, int *marker_num )
*  \brief correspondent function to arDetectMarker, but it uses the avr-classes
*  \return 0 when the function completes normally, -1 otherwise
*/
int avrDetectMarker( ARUint8 *dataPtr, int thresh, avrPatternInfo **marker_info, int *marker_num );
/** \fn int arDetectMarkerLite( ARUint8 *dataPtr, int thresh, ARMarkerInfo **marker_info, int *marker_num )
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
int arDetectMarkerLite( ARUint8 *dataPtr, int thresh, ARMarkerInfo **marker_info, int *marker_num );

/** \fn double arGetTransMat( ARMarkerInfo *marker_info, double center[2], double width, double conv[3][4] )
* \brief compute camera position in function of detected markers.
*
* calculate the transformation between a detected marker and the real camera,
* i.e. the position and orientation of the camera relative to the tracking mark.
* \param marker_info the structure containing the parameters for the marker for
*                    which the camera position and orientation is to be found relative to.
*                    This structure is found using arDetectMarker and avrDetectMarker.
* \param center the physical center of the marker. arGetTransMat and avrGetTransMat assumes that the marker
*              is in x-y plane, and z axis is pointing downwards from marker plane.
*              So vertex positions can be represented in 2D coordinates by ignoring the
*              z axis information. The marker vertices are specified in order of clockwise.
* \param width the size of the marker (in mm).
* \param conv the transformation matrix from the marker coordinates to camera coordinate frame,
*             that is the relative position of real camera to the real marker
* \return always 0.
*/
double   arGetTransMat( ARMarkerInfo *marker_info, double center[2], double width, double conv[3][4] );
/** \fn double avrGetTransMat( avrPatternInfo *marker_info, double center[2], double width, avrMatrix3x4& conv )
*  \brief correspondent function to arGetTransMat, but it uses the avr-classes
*  \return always 0.
*/
double   avrGetTransMat( avrPatternInfo *marker_info, double center[2], double width, avrMatrix3x4& conv );
/** \fn double arGetTransMatCont( ARMarkerInfo *marker_info, double prev_conv[3][4], double center[2], double width, double conv[3][4])
* \brief compute camera position in function of detected marker with an history function.
*
* calculate the transformation between a detected marker and the real camera,
* i.e. the position and orientation of the camera relative to the tracking mark. Since
* this routine operate on previous values, the result are more stable (less jittering).
*
* \param marker_info the structure containing the parameters for the marker for
*                    which the camera position and orientation is to be found relative to.
*                    This structure is found using arDetectMarker and avrDetectMarker.
* \param prev_conv the previous transformation matrix obtain.
* \param center the physical center of the marker. arGetTransMat and avrGetTransMat assumes that the marker
*              is in x-y plane, and z axis is pointing downwards from marker plane.
*              So vertex positions can be represented in 2D coordinates by ignoring the
*              z axis information. The marker vertices are specified in order of clockwise.
* \param width the size of the marker (in mm).
* \param conv the transformation matrix from the marker coordinates to camera coordinate frame,
*             that is the relative position of real camera to the real marker
* \return always 0.
*/
double   arGetTransMatCont( ARMarkerInfo *marker_info, double prev_conv[3][4], double center[2], double width, double conv[3][4] );
/** \fn double avrGetTransMatCont( avrPatternInfo *marker_info, avrMatrix3x4& prev_conv, double center[2], double width, avrMatrix3x4& conv)
*  \brief correspondent function to arGetTransMatCont, but it uses the avr-classes
*  \return always 0.
*/
double   avrGetTransMatCont( avrPatternInfo *marker_info, avrMatrix3x4& prev_conv, double center[2], double width, avrMatrix3x4& conv);

//ar.h

//arMulti.h

/** \fn double arMultiGetTransMat(ARMarkerInfo *marker_info, int marker_num, ARMultiMarkerInfoT *config)
* \brief compute camera position in function of the multi-marker patterns (based on detected markers)
*
* calculate the transformation between the multi-marker patterns and the real camera. Based on
* confident values of detected markers in the multi-markers patterns, a global position is return.
*
* \param marker_info list of detected markers (from arDetectMarker and avrDetectMarker)
* \param marker_num number of detected markers
* \param config
* \return the estimation projection error
*/
double   arMultiGetTransMat(ARMarkerInfo *marker_info, int marker_num, ARMultiMarkerInfoT *config);
/** \fn double avrMultiGetTransMat(avrPatternInfo *marker_info, int marker_num, ARMultiMarkerInfoT *config)
*  \brief correspondent function to arMultiGetTransMat, but it uses the avr-classes
*  \return the estimation projection error
*/
double   avrMultiGetTransMat(avrPatternInfo *marker_info, int marker_num, ARMultiMarkerInfoT *config);

//arMulti.h

#endif // AVR_VISION
