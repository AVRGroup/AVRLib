#ifndef AVRMATRIX3X4_H
#define AVRMATRIX3X4_H

#include <avrMatrix.h>

/** \class avrMatrix3x4 avrMatrix3x4.h "avrMatrix3x4.h"
*   \brief represents a 3x4 dimension matrix
*
*   This class simplifies the operations between 3x4 dimension matrices and offers auxiliary functions useful in
*   renderization routines which use transformation matrices
*   Internally, the matrix has 4x4 dimension, where the fourth dimension row, corresponds to a vector identity.
*/
class avrMatrix3x4 : public avrMatrix
{
   public:
      //! default constructor
      avrMatrix3x4();                     // default constructor
      //! conversion constructor (if dimension avrMatrix different 3x4, copy all possible elements)
      avrMatrix3x4(const avrMatrix&);     // conversion constructor
      //! conversion constructor (from c matrix format)
      avrMatrix3x4(double matrix[3][4]);  // conversion constructor
      ~avrMatrix3x4();

      // Methods Overwritten
      // Auxiliary
      /** \brief adds a new element in matrix
       *
       * \param [in] element new element to add
       * \param [in] row index of row
       * \param [in] clm index of column
       * \return void
       * \exception out_of_range& if invalid indexes
       */
      virtual void      add(double element, int row, int clm) throw(std::out_of_range&);
      /** \brief access an element in matrix
       *
       * \param [in] row index of row
       * \param [in] clm index of column
       * \return double element of the position [row][clm]
       * \exception out_of_range& if invalid indexes
       */
      virtual double    access(int row, int clm) const throw(std::out_of_range&);
      /** \brief shows a matrix
       *
       * \param [in] name define a name for matrix (opcional)
       * \param [in] decimals decimals number of the elements (default is 5)
       * \return void
       */
      virtual void      print(std::string name = "", int decimals = 5) const;

      // Gets
      //! returns 3 value (row dimension)
      virtual int       row()     const;
      //! returns 4 value (column dimension)
      virtual int       column()  const;
      /** \brief Get matrix in c format
       *
       * The reference is for a new matrix, not for the attribute 'matrixx'
       * \return double** matrix in c format (mat[3][4])
       */
      virtual double**  matrix()  const;

      // Methods Specific
      // Operations
      //! copy constructor
      avrMatrix3x4&  operator= (const avrMatrix3x4&);

      /** \brief multiplies two matrices of dimension 3x4 (this object x parameter matrix)
       * \pre column dimension of this object must be equal the row dimension of parameter matrix
       *
       * \param [in] avrMatrix3x4&
       * \return avrMatrix& result matrix
       * \exception invalid_argument& if pre-condition doesn't satisfied
       */
      avrMatrix3x4&  operator* (const avrMatrix3x4&);

      /** \brief calculates the relation matrix between the parameter matrix
       *
       * The result matrix has the relation required for take the coordinate system this object to the referential of parameter matrix
       * \param [in] avrMatrix3x4&
       * \return avrMatrix3x4& relation matrix
       */
      avrMatrix3x4&  getRelationWith(const avrMatrix3x4&) const;
      /** \brief calculates the euclidian distance between the parameter matrix
       *
       * \param [in] avrMatrix3x4&
       * \return double euclidian distance value
       */
      double         euclidianDistanceBetween(const avrMatrix3x4&) const;
      /** \brief copy the elements of matrix for the matrix in OpenGL format
       *
       * \param [out] gl_para matrix in OpenGL format
       * \return void
       */
      void           getMatrixGLFormat(double gl_para[16]) const;
      /** \brief calculates the elements of the matrix based in the quaternion and position vectors
       *
       * \param [in] quat quaternion vector
       * \param [in] pos position vector
       * \return void
       */
      void           setMatWithQuatAndPos(double quat[4], double pos[3]);
      /** \brief extracts the quaternion and position vectors of matrix
       *
       * \param [out] quat quaternion vector result
       * \param [out] pos position vector result
       * \return void
       * \exception domain_error& if non-sucess extraction (quaternion not normalize)
       */
      void           extractQuatAndPos(double quat[4], double pos[3]) const throw(std::domain_error&);

      // Special Gets
      //! Get X position (same as access(0, 3))
      double   X() const;
      //! Get Y position (same as access(1, 3))
      double   Y() const;
      //! Get Z position (same as access(2, 3))
      double   Z() const;
};

#endif // AVRMATRIX3X4_H
