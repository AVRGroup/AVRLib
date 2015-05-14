/*
  Name:        avrMath.h
  Version      1.0.1
  Author:      Felipe Caetano, Luiz Maurílio, Rodrigo L. S. Silva
  Date:        10/10/2012
  Last Update: 09/10/2014
  Description: Collection of the mathematical functions of ARToolkit. Receive functions
               from mDet, mInv, mMul, mPCA, mSelfInv, mTrans, vHouse,
               vInnerP and vTridiag.
*/

#ifndef AVR_MATH_H
#define AVR_MATH_H

/** \def ARELEM0(mat,r,c)
* \brief macro function that give direct access to an element (0 origin)
*/
/* 0 origin */
#define ARELEM0(mat,r,c) ((mat)->m[(r)*((mat)->clm)+(c)])

/** \def ARELEM1(mat,row,clm)
* \brief macro function that give direct access to an element (1 origin)
*/
/* 1 origin */
#define ARELEM1(mat,row,clm) ARELEM0(mat,row-1,clm-1)

/* matrix and vector utilities */
/** \def multiMatrix3x4Vector3(dest,m,v)
*  \brief macro function that multiplies a 3x4 matrix by a 3-dimension vector
*/
#define multiMatrix3x4Vector3(dest,m,v)\
    dest[0] = m[0][0]*v[0] + m[0][1]*v[1] + m[0][2]*v[2] + m[0][3];\
	dest[1] = m[1][0]*v[0] + m[1][1]*v[1] + m[1][2]*v[2] + m[1][3];\
	dest[2] = m[2][0]*v[0] + m[2][1]*v[1] + m[2][2]*v[2] + m[2][3];

/** \def euclidianDistanceMatrix3x4(result,src1,src2)
*  \brief macro function that calculates the euclidian distance between two 3x4 tranformations matrices
*/
#define euclidianDistanceMatrix3x4(result,src1,src2)\
    result = 0;\
	result += pow(src1[0][3]-src2[0][3],2);\
    result += pow(src1[1][3]-src2[1][3],2);\
    result += pow(src1[2][3]-src2[2][3],2);\
	result = sqrt(result);

/** \def euclidianDistanceVector3(result,src1,src2)
*  \brief macro function that calculates the euclidian distance between two points (3-dimension vectors)
*/
#define euclidianDistanceVector3(result,src1,src2)\
    result = 0;\
	result += pow(src1[0]-src2[0],2); \
	result += pow(src1[1]-src2[1],2); \
	result += pow(src1[2]-src2[2],2); \
    result = sqrt(result);

/** \def scalarProduct4(result,src1,src2)
*  \brief macro function that calculates the scalar product of two 4-dimension vectors
*/
#define scalarProduct4(result,src1,src2)\
    result = 0;\
	result += src1[0]*src2[0];\
	result += src1[1]*src2[1];\
	result += src1[2]*src2[2];\
	result += src1[3]*src2[3];

/** \def copyMatrix3x4(dest,src)
*  \brief macro function that copy a source 3x4 matrix in destination matrix
*/
#define copyMatrix3x4(dest,src)\
    dest[0][0] = src[0][0]; dest[0][1] = src[0][1]; dest[0][2] = src[0][2]; dest[0][3] = src[0][3];\
    dest[1][0] = src[1][0]; dest[1][1] = src[1][1]; dest[1][2] = src[1][2]; dest[1][3] = src[1][3];\
    dest[2][0] = src[2][0]; dest[2][1] = src[2][1]; dest[2][2] = src[2][2]; dest[2][3] = src[2][3];

#ifdef __cplusplus
extern "C" {
#endif

//matrix.h
/** \struct ARMat
* \brief ARToolKit struct, matrix structure.
*
* Defined the structure of the matrix type based on a dynamic allocation.
* The matrix format is :<br>
*  <---- clm --->		   <br>
*  [ 10  20  30 ] ^		<br>
*  [ 20  10  15 ] |		<br>
*  [ 12  23  13 ] row	<br>
*  [ 20  10  15 ] |		<br>
*  [ 13  14  15 ] v		<br>
*
* \param m content of matrix
* \param row number of lines in matrix
* \param clm number of column in matrix
*/
typedef struct {
	double *m;
	int row;
	int clm;
} ARMat;

/** \struct ARVec
* \brief ARToolKit struct, vector structure.
*
* The vector format is :<br>
*  <---- clm ---><br>
*  [ 10  20  30 ]<br>
* Defined the structure of the vector type based on a dynamic allocation.
* \param m content of vector
* \param clm number of column in matrix
*/
typedef struct {
   double *v;
   int    clm;
} ARVec;

// ARMat
/** \fn ARMat *arMatrixAlloc(int row, int clm)
* \brief creates a new matrix.
*
* Allocate and initialize a new matrix structure.
* \param row number of line
* \param clm number of column
* \return the matrix structure, NULL if allocation is impossible
*/
ARMat*   arMatrixAlloc(int row, int clm);
/** \fn int arMatrixFree(ARMat *m)
* \brief deletes a matrix.
*
* Delete a matrix structure (deallocate used memory).
* \param m matrix to delete
* \return always 0
*/
int      arMatrixFree(ARMat *m);

/** \fn int arMatrixDup(ARMat *dest, ARMat *source)
* \brief copy a matrix
*
* copy one matrix to another. The two ARMat must
* be allocated.
* \param dest the destination matrix of the copy
* \param source the original matrix source
* \return 0 if success, -1 if error (matrix with different size)
*/
int      arMatrixDup(ARMat *dest, ARMat *source);
/** \fn ARMat *arMatrixAllocDup(ARMat *source)
* \brief dumps a new matrix
*
* Allocates and recopy the original source matrix.
* \param source the source matrix to copy
* \return the matrix if success, NULL if error
*/
ARMat*   arMatrixAllocDup(ARMat *source);

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
int      arMatrixMul(ARMat *dest, ARMat *a, ARMat *b);
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
ARMat*   arMatrixAllocMul(ARMat *a, ARMat *b);

/** \fn int arMatrixUnit(ARMat *unit)
* \brief Creates a unit matrix.
*
* Transforms the source parameter matrix to
* a unit matrix (all values are modified).
* the unit matrix needs to be allocated.
* \param unit the matrix to transform
* \return 0 if success, -1 if error
*/
int      arMatrixUnit(ARMat *unit);
/** \fn int arMatrixAllocUnit(int dim)
* \brief Creates a unit matrix.
*
* Allocates and initializes a matrix to a
* an identity matrix.
* \param dim dimensions of the unit matrix (square)
* \return the matrix allocated if success, NULL if error
*/
ARMat*   arMatrixAllocUnit(int dim);

/** \fn int  arMatrixTrans(ARMat *dest, ARMat *source)
* \brief transposes a matrix.
*
* Transposes a matrix. The destination matrix
* must be allocated (the source matrix is unmodified).
* \param dest the destination matrix of the copy
* \param source the source matrix
* \return 0 if success, -1 if error (source and destination matrix have different size)
*/
int      arMatrixTrans(ARMat *dest, ARMat *source);
/** \fn ARMat *arMatrixAllocTrans(ARMat *source)
* \brief transposes a matrix with allocation.
*
* transposes a matrix and copy the result in a new
* allocate matrix (the source matrix is unmodified).
* \param source the matrix to transpose
* \return the allocated matrix if success, NULL if error (creation or transposition impossible)
*/
ARMat*   arMatrixAllocTrans(ARMat *source);

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
int      arMatrixInv(ARMat *dest, ARMat *source);
/** \fn int arMatrixSelfInv(ARMat *m)
* \brief inverses a matrix.
*
* Inverses a matrix and copy the result in
* the same structure.
* \param m the matrix to inverse
* \return 0 if success, -1 if error
*/
int      arMatrixSelfInv(ARMat *m);
/** \fn int arMatrixAllocInv(ARMat *source)
* \brief inverses a matrix.
*
* Inverses a matrix and copy the result in
* in a new allocated structure.
* \param source the matrix to inverse
* \return the inversed matrix if success, NULL if error
*/
ARMat*   arMatrixAllocInv(ARMat *source);

/** \fn int arMatrixDet(ARMat *m)
* \brief compute determinant of a matrix.
*
* Compute the determinant of a matrix.
* \param m matrix source
* \return the computed determinant
*/
double   arMatrixDet(ARMat *m);
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
int      arMatrixPCA( ARMat *input, ARMat *evec, ARVec *ev, ARVec *mean );
/** \fn int arMatrixPCA2( ARMat *input, ARMat *evec, ARVec *ev )
* \brief compute the PCA of a matrix.
*
* Compute the Principal Component Analysis (PCA) of a matrix.
* \param input source matrix
* \param evec result matrix
* \param ev egein value computed
* \return 0 if success to compute, -1 otherwise
*/
int      arMatrixPCA2( ARMat *input, ARMat *evec, ARVec *ev );
/** \fn int arMatrixDisp(ARMat *m)
* \brief display content of a matrix.
*
* Display in current console, the content of
* the matrix. The display is done line by line.
* \param m
* \return always 0
*/
int      arMatrixDisp(ARMat *m);

// ARVec
/** \fn ARVec *arVecAlloc( int clm )
* \brief creates a new vector.
*
* Allocates and initializes new vector structure.
* \param clm dimension of vector
* \return the allocated vector, NULL if error (impossible allocation)
*/
ARVec*   arVecAlloc( int clm );
/** \fn int arVecFree( ARVec *v )
* \brief delete a vector.
*
* Delete a vector structure (deallocate used memory).
* \param v the vector to delete
* \return always 0
*/
int      arVecFree( ARVec *v );
/** \fn int arVecDisp( ARVec *v )
* \brief display a vector.
*
* Display element of a vector.
* \param v the vector to display
* \return always 0
*/
int      arVecDisp( ARVec *v );

/** \fn double arVecHousehold( ARVec *x )
* \brief compute the Householder Transformation (QR decomposition)
*/
double   arVecHousehold( ARVec *x );
/** \fn double arVecInnerproduct( ARVec *x, ARVec *y )
* \brief Computes the inner product of 2 vectors.
*
* computes the inner product of the two argument vectors.
* the operation done is  a=x.y (and a is return)
* \param x first vector source
* \param y second vector source
* \return the computed innerproduct
*/
double   arVecInnerproduct( ARVec *x, ARVec *y );
/** \fn int arVecTridiagonalize( ARMat *a, ARVec *d, ARVec *e )
* \brief tridiagonalize the vectors
*/
int      arVecTridiagonalize( ARMat *a, ARVec *d, ARVec *e );

//matrix.h

//ar.h

/** \fn int arGetAngle( double rot[3][3], double *wa, double *wb, double *wc )
* \brief extract euler angle from a rotation matrix.
*
* Based on a matrix rotation representation, furnish the cprresponding euler angles.
* \param rot the initial rotation matrix
* \return always 0
*/
int      arGetAngle( double rot[3][3], double *wa, double *wb, double *wc );
/** \fn int arGetRot( double a, double b, double c, double rot[3][3] )
* \brief create a rotation matrix with euler angle.
*
* Based on a euler description, furnish a rotation matrix.
* \param rot the resulted rotation matrix
* \return always 0
*/
int      arGetRot( double a, double b, double c, double rot[3][3] );

/** \fn int arGetNewMatrix( double a, double b, double c, double trans[3], double trans2[3][4], double cpara[3][4], double ret[3][4] )
* \brief contructs a new transformation matrix
*  \param ret the result matrix
*/
int      arGetNewMatrix( double a, double b, double c, double trans[3], double trans2[3][4], double cpara[3][4], double ret[3][4] );
/**
*  \fn double arModifyMatrix( double rot[3][3], double trans[3], double cpara[3][4], double vertex[][3], double pos2d[][2], int num );
*/
double   arModifyMatrix( double rot[3][3], double trans[3], double cpara[3][4], double vertex[][3], double pos2d[][2], int num );
//ar.h
#ifdef __cplusplus
}
#endif

#endif
