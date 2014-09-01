/*
  Name:        avrMath.h
  Version      0.1
  Author:      Felipe Caetano, Luiz Maurílio, Rodrigo L. S. Silva
  Date:        10/10/2012
  Last Update: 10/10/2012
  Description: Collection of the mathematical functions of ARToolkit. Receive functions
               from arGetTransMat, arGetTransMat2, arGetTransMat3, arGetTransMatCont,
               arMultiGetTransMat, mDet, mInv, mMul, mPCA, mSelfInv, mTrans, vHouse,
               vInnerP and vTridiag.
*/

#ifndef AVR_MATH_H
#define AVR_MATH_H

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
//	Public includes.
// ============================================================================

#include <math.h>
#include <avrUtil.h>

///*------------------------------------*/
//double arsMultiGetTransMat(ARMarkerInfo *marker_infoL, int marker_numL,
//                           ARMarkerInfo *marker_infoR, int marker_numR,
//                           ARMultiMarkerInfoT *config);

/**
* \brief compute camera position in function of the multi-marker patterns (based on detected markers)
*
* calculate the transformation between the multi-marker patterns and the real camera. Based on
* confident values of detected markers in the multi-markers patterns, a global position is return.
*
* \param marker_info list of detected markers (from arDetectMarker)
* \param marker_num number of detected markers
* \param config
* \return
*/

double  arMultiGetTransMat(ARMarkerInfo *marker_info, int marker_num, ARMultiMarkerInfoT *config);

///arMulti.h

///matrix.h

// ============================================================================
//	Public types and defines.
// ============================================================================

// ============================================================================
//	Public globals.
// ============================================================================

// ============================================================================
//	Public functions.
// ============================================================================


/** \fn int arMatrixMul(ARMat *dest, ARMat *a, ARMat *b)
* \brief Multiply two matrix
*
* Multiply two matrix and copy the result in another
* the product is this one : dest = a * b. The destination
* matrix must be allocated. Matrix a and b need to have
* the same size (the source matrix is unmodified).
* \param dest final matrix product
* \param a first matrix
* \param b second matrix
* \return 0 if success, -1 if error (multiplication impossible, or destination matrix have not comptabile size)
*/
int    arMatrixMul(ARMat *dest, ARMat *a, ARMat *b);

/** \fn ARMat *arMatrixAllocMul(ARMat *a, ARMat *b)
* \brief Multiply two matrix with memory allocation.
*
* multiply two matrix and copy the result in a new
* allocate matrix (the source matrix is unmodified).
* the product is this one : dest = a * b
*
* \param a first matrix
* \param b second matrix
* \return the allocated matrix if success, NULL if error
*/
ARMat  *arMatrixAllocMul(ARMat *a, ARMat *b);

/** \fn int arMatrixUnit(ARMat *unit)
* \brief Creates a unit matrix.
*
* Transforms the source parameter matrix to
* a unit matrix (all values are modified).
* the unit matrix needs to be allocated.
* \param unit the matrix to transform
* \return 0 if success, -1 if error
*/
int    arMatrixUnit(ARMat *unit);

/** \fn int arMatrixAllocUnit(int dim)
* \brief Creates a unit matrix.
*
* Allocates and initializes a matrix to a
* an identity matrix.
* \param dim dimensions of the unit matrix (square)
* \return the matrix allocated if success, NULL if error
*/
ARMat  *arMatrixAllocUnit(int dim);

/** \fn int  arMatrixTrans(ARMat *dest, ARMat *source)
* \brief transposes a matrix.
*
* Transposes a matrix. The destination matrix
* must be allocated (the source matrix is unmodified).
* \param dest the destination matrix of the copy
* \param source the source matrix
* \return 0 if success, -1 if error (source and destination matrix have different size)
*/
int    arMatrixTrans(ARMat *dest, ARMat *source);

/** \fn ARMat *arMatrixAllocTrans(ARMat *source)
* \brief transposes a matrix with allocation.
*
* transposes a matrix and copy the result in a new
* allocate matrix (the source matrix is unmodified).
* \param source the matrix to transpose
* \return the allocated matrix if success, NULL if error (creation or transposition impossible)
*/
ARMat  *arMatrixAllocTrans(ARMat *source);

/** \fn int arMatrixInv(ARMat *dest, ARMat *source)
* \brief inverse a matrix.
*
* inverse a matrix and copy the result in a new
* one (the source matrix is unmodified). the destination
* matrix must be allocated. the source matrix need to be a
* square matrix.
* \param dest result matrix of the inverse operation
* \param source source matrix
* \return 0 if success, -1 if error (not square matrix)
*/
int    arMatrixInv(ARMat *dest, ARMat *source);

/** \fn int arMatrixSelfInv(ARMat *m)
* \brief inverses a matrix.
*
* Inverses a matrix and copy the result in
* the same structure.
* \param m the matrix to inverse
* \return 0 if success, -1 if error
*/
int    arMatrixSelfInv(ARMat *m);

/** \fn int arMatrixAllocInv(ARMat *source)
* \brief inverses a matrix.
*
* Inverses a matrix and copy the result in
* in a new allocated structure.
* \param source the matrix to inverse
* \return the inversed matrix if success, NULL if error
*/
ARMat  *arMatrixAllocInv(ARMat *source);

/** \fn int arMatrixDet(ARMat *m)
* \brief compute determinant of a matrix.
*
* Compute the determinant of a matrix.
* \param m matrix source
* \return the computed determinant
*/
double arMatrixDet(ARMat *m);

/** \fn int arMatrixPCA( ARMat *input, ARMat *evec, ARVec *ev, ARVec *mean )
* \brief compute the PCA of a matrix.
*
* Compute the Principal Component Analysis (PCA) of a matrix.
* \param input source matrix
* \param evec eigen vector computed
* \param ev eigen value computed
* \param mean mean computed
* \return 0 if success to compute, -1 otherwise
*/
int    arMatrixPCA( ARMat *input, ARMat *evec, ARVec *ev, ARVec *mean );

/** \fn int arMatrixPCA2( ARMat *input, ARMat *evec, ARVec *ev )
* \brief compute the PCA of a matrix.
*
* Compute the Principal Component Analysis (PCA) of a matrix.
* \param input source matrix
* \param evec result matrix
* \param ev egein value computed
* \return 0 if success to compute, -1 otherwise
*/
int    arMatrixPCA2( ARMat *input, ARMat *evec, ARVec *ev );

/** \fn int arMatrixDisp(ARMat *m)
* \brief display content of a matrix.
*
* Display in current console, the content of
* the matrix. The display is done line by line.
* \param m
* \return 0
*/
int    arMatrixDisp(ARMat *m);

/** \fn double arVecHousehold( ARVec *x )
* \brief XXXBK
*
* XXXBK: for QR decomposition ?? (can't success to find french translation of this term)
* \param x XXXBK
* \return XXXBK
*/
double arVecHousehold( ARVec *x );

/** \fn double arVecInnerproduct( ARVec *x, ARVec *y )
* \brief Computes the inner product of 2 vectors.
*
* computes the inner product of the two argument vectors.
* the operation done is  a=x.y (and a is return)
* \param x first vector source
* \param y second vector source
* \return the computed innerproduct
*/
double arVecInnerproduct( ARVec *x, ARVec *y );

/** \fn int arVecTridiagonalize( ARMat *a, ARVec *d, ARVec *e )
* \brief XXXBK
*
* XXXBK
* \param a XXXBK
* \param d XXXBK
* \param e XXXBK
* \return XXXBK
*/
int    arVecTridiagonalize( ARMat *a, ARVec *d, ARVec *e );

///matrix.h

///ar.h
/**
* \brief compute camera position in function of detected markers.
*
* calculate the transformation between a detected marker and the real camera,
* i.e. the position and orientation of the camera relative to the tracking mark.
* \param marker_info the structure containing the parameters for the marker for
*                    which the camera position and orientation is to be found relative to.
*                    This structure is found using arDetectMarker.
* \param center the physical center of the marker. arGetTransMat assumes that the marker
*              is in x-y plane, and z axis is pointing downwards from marker plane.
*              So vertex positions can be represented in 2D coordinates by ignoring the
*              z axis information. The marker vertices are specified in order of clockwise.
* \param width the size of the marker (in mm).
* \param conv the transformation matrix from the marker coordinates to camera coordinate frame,
*             that is the relative position of real camera to the real marker
* \return always 0.
*/
double arGetTransMat( ARMarkerInfo *marker_info,
                      double center[2], double width, double conv[3][4] );

/**
* \brief compute camera position in function of detected marker with an history function.
*
* calculate the transformation between a detected marker and the real camera,
* i.e. the position and orientation of the camera relative to the tracking mark. Since
* this routine operate on previous values, the result are more stable (less jittering).
*
* \param marker_info the structure containing the parameters for the marker for
*                    which the camera position and orientation is to be found relative to.
*                    This structure is found using arDetectMarker.
* \param prev_conv the previous transformation matrix obtain.
* \param center the physical center of the marker. arGetTransMat assumes that the marker
*              is in x-y plane, and z axis is pointing downwards from marker plane.
*              So vertex positions can be represented in 2D coordinates by ignoring the
*              z axis information. The marker vertices are specified in order of clockwise.
* \param width the size of the marker (in mm).
* \param conv the transformation matrix from the marker coordinates to camera coordinate frame,
*             that is the relative position of real camera to the real marker
* \return always 0.
*/
double arGetTransMatCont( ARMarkerInfo *marker_info, double prev_conv[3][4],
                          double center[2], double width, double conv[3][4] );

double arGetTransMat2( double rot[3][3], double pos2d[][2],
                       double pos3d[][2], int num, double conv[3][4] );
double arGetTransMat3( double rot[3][3], double ppos2d[][2],
                     double ppos3d[][2], int num, double conv[3][4],
                     double *dist_factor, double cpara[3][4] );
double arGetTransMat4( double rot[3][3], double ppos2d[][2],
                       double ppos3d[][3], int num, double conv[3][4] );
double arGetTransMat5( double rot[3][3], double ppos2d[][2],
                       double ppos3d[][3], int num, double conv[3][4],
                       double *dist_factor, double cpara[3][4] );

/**
* \brief  XXXBK
*
*  XXXBK
* \param rot XXXBK
* \param trans XXXBK
* \param cpara XXXBK
* \param vertex XXXBK
* \param pos2d XXXBK
* \param num XXXBK
* \return XXXBK
*/
double arModifyMatrix( double rot[3][3], double trans[3], double cpara[3][4],
                             double vertex[][3], double pos2d[][2], int num );

/**
* \brief extract euler angle from a rotation matrix.
*
* Based on a matrix rotation representation, furnish the cprresponding euler angles.
* \param rot the initial rotation matrix
* \param wa XXXBK:which element ?
* \param wb XXXBK:which element ?
* \param wc XXXBK:which element ?
* \return XXXBK
*/
int arGetAngle( double rot[3][3], double *wa, double *wb, double *wc );

/**
* \brief create a rotation matrix with euler angle.
*
* Based on a euler description, furnish a rotation matrix.
* \param a XXXBK:which element ?
* \param b XXXBK:which element ?
* \param c XXXBK:which element ?
* \param rot the resulted rotation matrix
* \return XXXBK
*/
int arGetRot( double a, double b, double c, double rot[3][3] );

/**
* \brief XXXBK
*
* XXXBK
* \param a XXXBK
* \param b XXXBK
* \param c XXXBK
* \param trans XXXBK
* \param trans2 XXXBK
* \param cpara XXXBK
* \param ret XXXBK
* \return XXXBK
*/
int arGetNewMatrix( double a, double b, double c,
                    double trans[3], double trans2[3][4],
                    double cpara[3][4], double ret[3][4] );

/**
* \brief XXXBK
*
* XXXBK:initial of what ?
* \param marker_info XXXBK
* \param cpara XXXBK
* \param rot XXXBK
* \return XXXBK
*/
int arGetInitRot( ARMarkerInfo *marker_info, double cpara[3][4], double rot[3][3] );

double arsModifyMatrix( double rot[3][3], double trans[3], ARSParam *arsParam,
                        double pos3dL[][3], double pos2dL[][2], int numL,
                        double pos3dR[][3], double pos2dR[][2], int numR );


///ar.h

#ifdef __cplusplus
}
#endif

#include <avrPatternInfo.h>
#include <avrMatrix3x4.h>

int avrGetInitRot( avrPatternInfo *marker_info, double cpara[3][4], double rot[3][3] );
double avrGetTransMat( avrPatternInfo *marker_info, double center[2], double width, avrMatrix3x4& conv );
double avrGetTransMat3( double rot[3][3], double ppos2d[][2], double ppos3d[][2], int num, avrMatrix3x4& conv, double *dist_factor, double cpara[3][4] );
double avrGetTransMatCont( avrPatternInfo *marker_info, avrMatrix3x4& prev_conv, double center[2], double width, avrMatrix3x4& conv);
double avrMultiGetTransMat(avrPatternInfo *marker_info, int marker_num, ARMultiMarkerInfoT *config);

#endif
