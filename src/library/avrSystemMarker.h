#ifndef AVR_SYSTEM_MARKER_H
#define AVR_SYSTEM_MARKER_H

#include <vector>
#include <avrPattern.h>    // this include avrMatrix3x4
#include <avrPatternInfo.h>

/** \class avrSystemMarker avrSystemMarker.h "avrSystemMarker.h"
 * \brief Abstract class for manages the systems marker of the application
 *
 * This class has a vector with the marker patterns registered in the application and the system transformation matrix
 */
class avrSystemMarker{
   protected:
      std::vector<avrPattern*> patts;        // patterns in the system
      avrMatrix3x4 *           projection;   // projection matrix

   public:
      avrSystemMarker();   // default constructor
      virtual ~avrSystemMarker();

      /** \brief adds marker patterns to the system
       *
       * \param [in] patt new marker pattern
       * \return void
       */
      void           addPattern(avrPattern& patt);
      //! returns the marker patterns number which aggregate the system
      int            sizePatts() const;
      //! returns the marker pattern located in position 'index' of the patts vector
      avrPattern&    getPatt(int index) const;
      //! Get the projection matrix
      avrMatrix3x4&  getProjection() const;

      /** \brief internal use function, calculates the camera transfomation relative the each marker pattern
       *
       * \param [in] marker_info possible marker patterns that were identified by the ARToolKit
       * \param [in] marker_num number of marker patterns identified
       * \return bool
       */
      virtual bool setCameraTransformation(avrPatternInfo *marker_info, int marker_num) = 0;
      //! internal use function, sets object transformation and prepares draw
      virtual void setObjectTransformation() = 0;
      //! sets secondary display function
      virtual void setDisplayCallback(void (*displayFunc)(void)) = 0;
      //! sets main display function
      virtual void setDisplayCallback(void (*displayFunc)(int )) = 0;
      //! internal use function, calls the renderization callback
      virtual void drawFunction() = 0;
};

#endif
