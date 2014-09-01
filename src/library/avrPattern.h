#ifndef AVR_PATTERN_H
#define AVR_PATTERN_H

#include <avrMatrix3x4.h>  // << this include avrMatrix

/** \class avrPattern avrPattern.h "avrPattern.h"
 * \brief This class stores all information of a marker pattern which was registered in the application
 *
 * Stored Informations\n
 *    \li identifier
 *    \li visibility state
 *    \li name (opcional)
 *    \li real width of the marker (in millimeters)
 *    \li accuracy
 *    \li center coordinates
 *    \li final position matrix (internal use)
 *    \li transformation matrix
 *
 * \param filename path of the file that defines the marker pattern
 * \param patt_width real width of the marker (in millimeters)
 * \param patt_center center coordinates of the marker
 */
class avrPattern{
   private:
      int            patt_id;
      bool           patt_visible;
      std::string    patt_name;
      double         patt_width;
      double         patt_accuracy;
      double         patt_center[2];
      avrMatrix *    patt_pos3D;
      avrMatrix3x4 * patt_trans; // transformation matrix

   public:
      avrPattern();  // default constructor
      avrPattern(const char *filename, double patt_width, double *patt_center = NULL);

      // Gets
      inline avrMatrix3x4&  trans()     { return *this->patt_trans; }
      inline avrMatrix&     pos3D()     { return *this->patt_pos3D; }
      inline double         width()     { return patt_width; }
      inline double         accuracy()  { return this->patt_accuracy; }
      inline double *       center()    { return (double*) this->patt_center; }
      inline std::string    name()      { return this->patt_name; }
      inline bool           visible()   { return this->patt_visible; }
      inline int            id()        { return this->patt_id; }

      //Sets
      inline void    setID(int id)                 { this->patt_id = id; }
      inline void    setName(std::string name)     { this->patt_name = name; }
      inline void    setWidth(double width)        { this->patt_width = width; }
      inline void    setVisible(bool visible)      { this->patt_visible = visible; }
      inline void    setCenter(double center[2])   { this->patt_center[0] = center[0]; this->patt_center[1] = center[1]; }
      inline void    setAccuracy(double accuracy)  { this->patt_accuracy = accuracy; }

      //Static Members

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
