/*
  Name:        avrMatrix.h
  Version      1.0.1
  Author:      Douglas Oliveira, Rodrigo L. S. Silva
  Date:        10/10/2012
  Last Update: 09/10/2014
  Description: class to represents a n x m matrix and its operations
*/

#ifndef AVRMATRIX_H
#define AVRMATRIX_H

#include <stdexcept>

/** \class avrMatrix avrMatrix.h "avrMatrix.h"
 * \brief represents a nxm dimension matrix
 *
 * This class simplifies the operations between nxm dimension matrices
 */
class avrMatrix
{
   //! \cond
   private:
      unsigned int	rows; // vertical dimension
      unsigned int	clms; // horizontal dimension
   //! \endcond
   protected:
      //! array which store elements (position = i * clms + j)
      double*			data;

   public:
      avrMatrix();  // default constructor
      /** \brief initialization constructor
      * \param row vertical dimension
      * \param clm horizontal dimension
      */
      avrMatrix(int row, int clm);
      //! destructor
      virtual ~avrMatrix();

      // Operations
      //! copy constructor
      avrMatrix&	operator= (const avrMatrix&);

      /** \brief sum two matrices (this object + parameter matrix)
       * \pre dimension of the matrices must be equal
       *
       * \param [in] avrMatrix&
       * \return result matrix
       * \exception invalid_argument& if pre-condition doesn't satisfied
       */
      avrMatrix&	operator+ (const avrMatrix&) throw(std::invalid_argument&);
      /** \brief subtracts two matrices (this object - parameter matrix)
       * \pre dimension of the matrices must be equal
       *
       * \param [in] avrMatrix&
       * \return result matrix
       * \exception invalid_argument& if pre-condition doesn't satisfied
       */
      avrMatrix&	operator- (const avrMatrix&) throw(std::invalid_argument&);
      /** \brief multiplies two matrices (this object x parameter matrix)
       * \pre column dimension of this object must be equal the row dimension of parameter matrix
       *
       * \param [in] avrMatrix&
       * \return result matrix
       * \exception invalid_argument& if pre-condition doesn't satisfied
       */
      avrMatrix&	operator* (const avrMatrix&) throw(std::invalid_argument&);
      /** \brief multiplies the matrix by a scalar number (this object x scalar number)
       *
       * \param [in] scalar scalar value
       * \return result matrix
       */
      avrMatrix&	operator* (double scalar);
      /** \brief returns a reference to the row of the matrix
       *
       * \param [in] index row index
       * \return reference to the row of the matrix
       */
      double*		operator[] (unsigned int index) throw(std::out_of_range&);

      /** \brief calculates the inverse matrix
       *
       * \return inverse matrix
       * \exception domain_error& not exists the inverse
       */
      avrMatrix&	inverse() const throw(std::domain_error&);
      /** \brief calculates the transposed matrix
       *
       * \return transposed matrix
       */
      avrMatrix&	transposed() const;
      /** \brief calculates the determinant of the matrix
       * \note if the non-square matrix, determinant equals to zero
       *
       * \return determinant value
       */
      double		determinant() const;

      // Auxiliary
      /** \brief adds a new element in matrix
       *
       * \param [in] element new element to add
       * \param [in] row index of row
       * \param [in] clm index of column
       * \exception out_of_range& if invalid indexes
       */
      virtual void	add(double element, unsigned int row, unsigned int clm) throw(std::out_of_range&);
      /** \brief access an element in matrix
       *
       * \param [in] row index of row
       * \param [in] clm index of column
       * \return element of the position [row][clm]
       * \exception out_of_range& if invalid indexes
       */
      virtual double	access(unsigned int row, unsigned int clm) const throw(std::out_of_range&);
      /** \brief shows a matrix
       *
       * \param [in] name define a name for matrix (optional)
       * \param [in] decimals decimals number of the elements (default is 5)
       */
      virtual void	print(std::string name = "", int decimals = 5) const;

      // Gets
      //! Get row dimension
      virtual unsigned int	row()    const;
      //! Get column dimension
      virtual unsigned int	column() const;
      /** \brief Get matrix in c format
       *
       * The reference is for a new matrix, not for the attribute 'data'
       * \return matrix in c format (mat[n][m])
       */
      virtual double**	matrix() const;

      // Sets
      //! initialize matrix with identity matrix
      void					setIdentityMatrix();
};

#endif // AVRMATRIX_H
