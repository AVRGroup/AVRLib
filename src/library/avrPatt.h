/*
  Name:        avrPatt.h
  Version      0.1
  Author:      Felipe Caetano, Luiz Maurílio, Rodrigo L. S. Silva
  Date:        10/10/2012
  Last Update: 10/10/2012
  Description: Collection of the "pattern" functions of ARToolkit. Receive functions
               from arGetCode and arMultiActive
*/

#ifndef AVR_PATT_H
#define AVR_PATT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <avrUtil.h>

///arMulti.h

/**
* \brief activate a multi-marker pattern on the recognition procedure.
*
* Activate a multi-marker for be checking during the template matching
* operation.
* \param config pointer to the multi-marker
* \return 0 if success, -1 if error
*/
int arMultiActivate( ARMultiMarkerInfoT *config );

/**
* \brief Desactivate a multi-marker pattern on the recognition procedure.
*
* Desactivate a multi-marker for not be checking during the template matching
* operation.
* \param config pointer to the multi-marker
* \return 0 if success, -1 if error
*/
int arMultiDeactivate( ARMultiMarkerInfoT *config );

/**
* \brief remove a multi-marker pattern from memory.
*
* desactivate a pattern and remove it from memory. Post-condition
* of this function is unavailability of the multi-marker pattern.
* \param config pointer to the multi-marker
* \return 0 if success, -1 if error
*/
int arMultiFreeConfig( ARMultiMarkerInfoT *config );

///arMulti.h

///ar.h
/**
* \brief load markers description from a file
*
* load the bitmap pattern specified in the file filename into the pattern
* matching array for later use by the marker detection routines.
* \param filename name of the file containing the pattern bitmap to be loaded
* \return the identity number of the pattern loaded or –1 if the pattern load failed.
*/
int arLoadPatt( const char *filename );

/**
* \brief remove a pattern from memory.
*
* desactivate a pattern and remove from memory. post-condition
* of this function is unavailability of the pattern.
* \param patt_no number of pattern to free
* \return return 1 in success, -1 if error
*/
int arFreePatt( int patt_no );

/**
* \brief activate a pattern on the recognition procedure.
*
* Activate a pattern to be check during the template matching
* operation.
* \param patt_no number of pattern to activate
* \return return 1 in success, -1 if error
*/
int arActivatePatt( int pat_no );

/**
* \brief desactivate a pattern on the recognition procedure.
*
* Desactivate a pattern for not be check during the template matching
* operation.
* \param patt_no number of pattern to desactivate
* \return return 1 in success, -1 if error
*/
int arDeactivatePatt( int pat_no );

/**
* \brief  XXXBK
*
*  XXXBK
* \param image XXXBK
* \param x_coord XXXBK
* \param y_coord XXXBK
* \param vertex XXXBK
* \param code XXXBK
* \param dir XXXBK
* \param cf XXXBK
* \return XXXBK
*/
int arGetCode( ARUint8 *image, int *x_coord, int *y_coord, int *vertex,
               int *code, int *dir, double *cf );

/**
* \brief Get a normalized pattern from a video image.
*
* This function returns a normalized pattern from a video image. The
* format is a table with AR_PATT_SIZE_X by AR_PATT_SIZE_Y
* \param image video input image
* \param x_coord XXXBK
* \param y_coord XXXBK
* \param vertex XXXBK
* \param ext_pat detected pattern.
* \return  XXXBK
*/
int arGetPatt( ARUint8 *image, int *x_coord, int *y_coord, int *vertex,
               ARUint8 ext_pat[AR_PATT_SIZE_Y][AR_PATT_SIZE_X][3] );

/**
* \brief  XXXBK
*
*  XXXBK
* \param limage XXXBK
* \param label_ref XXXBK
* \param label XXXBK
* \param clip XXXBK
* \param marker_info2 XXXBK
* \return  XXXBK
*/
int arGetContour( ARInt16 *limage, int *label_ref,
                  int label, int clip[4], ARMarkerInfo2 *marker_info2 );

///ar.h

#ifdef __cplusplus
}
#endif

#endif
