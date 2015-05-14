#ifndef AVR_CALIBRATION_H
#define AVR_CALIBRATION_H

#include "../avrParameters.h"

///gsub_lite.h
///gsubUtil.h

#ifdef __cplusplus
extern "C" {
#endif

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
void argUtilCalibHMD( int targetId, int thresh2, void (*postFunc)(ARParam *lpara, ARParam *rpara) );

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
void argInqSetting( int *hmdMode, int *gMiniXnum2, int *gMiniYnum2, void (**mouseFunc)(int button, int state, int x, int y),
                    void (**keyFunc)(unsigned char key, int x, int y), void (**mainFunc)(void) );

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

#ifdef __cplusplus
}
#endif

#endif // AVR_CALIBRATION_H
