/*
  Name:        avrPattern.h
  Version      1.0.1
  Author:      Douglas Oliveira, Rodrigo L. S. Silva
  Date:        10/10/2012
  Last Update: 09/10/2014
  Description: class to store the pattern's data
*/

#ifndef AVR_PATTERN_H
#define AVR_PATTERN_H

#include <avrMatrix3x4.h>  // << this include avrMatrix

/** \class avrPattern avrPattern.h "avrPattern.h"
 * \brief This class stores all information of a marker pattern which was registered in the application
 *
 * Stored Informations\n
 *    \li identifier
 *    \li visibility state
 *    \li name (optional)
 *    \li real width of the marker (in millimeters)
 *    \li accuracy
 *    \li center coordinates
 *    \li final position matrix (internal use)
 *    \li transformation matrix
 */
class avrPattern{
   //! \cond
   private:
      int            patt_id;
      bool           patt_visible;
      std::string    patt_name;
      double         patt_width;
      double         patt_accuracy;
      double         patt_center[2];
      avrMatrix *    patt_pos3D;
      avrMatrix3x4 * patt_trans; // transformation matrix
   //! \endcond
   public:
      avrPattern();  // default constructor
      /** \brief initialization constructor
       * \param filename path of the file that defines the marker pattern
       * \param patt_width real width of the marker (in millimeters)
       * \param patt_center center coordinates of the marker (if NULL, center is defined as [0, 0] )
       */
      avrPattern(const std::string& filename, double patt_width, double *patt_center = NULL);

      // Public Gets
      inline avrMatrix3x4&  trans()     { return *this->patt_trans; }
      inline double         width()     { return patt_width; }
      inline double         accuracy()  { return this->patt_accuracy; }
      inline std::string    name()      { return this->patt_name; }
      inline bool           visible()   { return this->patt_visible; }
      inline int            id()        { return this->patt_id; }

      // Gets of internal use
      inline avrMatrix&     pos3D()     { return *this->patt_pos3D; }
      inline double *       center()    { return (double*) this->patt_center; }

      // Public Sets
      inline void    setName(std::string name)     { this->patt_name = name; }

      // Sets of internal use
      inline void    setID(int id)                 { this->patt_id = id; }
      inline void    setWidth(double width)        { this->patt_width = width; }
      inline void    setVisible(bool visible)      { this->patt_visible = visible; }
      inline void    setCenter(double center[2])   { this->patt_center[0] = center[0]; this->patt_center[1] = center[1]; }
      inline void    setAccuracy(double accuracy)  { this->patt_accuracy = accuracy; }

      // Static Members
      /** \brief auxiliary function, reads the config file of the markers WITHOUT relation between them pre-calculated
       *
       * \param [in] filename config file path
       * \param [out] numberPatts number of markers registered
       * \return avrPattern* array with the avrPatterns registered
       */
      static avrPattern *avrReadConfigFileNotRelation(const char *filename, int *numberPatts);
      /** \brief auxiliary function, reads the config file of the markers WITH relation between them pre-calculated
       *
       * \param [in] filename config file path
       * \param [out] numberPatts number of markers registered
       * \return avrPattern* array with the avrPatterns registered
       */
      static avrPattern *avrReadConfigFileWithRelation(const char *filename, int *numberPatts);
};

#endif
