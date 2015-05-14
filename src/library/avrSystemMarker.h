/*
  Name:        avrSystemMarker.h
  Version      1.0.1
  Author:      Douglas Oliveira, Rodrigo L. S. Silva
  Date:        10/10/2012
  Last Update: 09/10/2014
  Description: interface to manage various system's marker
*/

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
      avrMatrix3x4 *           projection;   // projection matrix
      std::vector<avrPattern*> patts;        // patterns in the system

   public:
      avrSystemMarker();   // default constructor
      virtual ~avrSystemMarker();

      /** \brief adds marker patterns to the system
      * \param [in] patt new marker pattern
      */
      void           addPattern(avrPattern& patt);
      //! returns the marker patterns number which aggregate the system
      int            sizePatts() const;
      //! returns the marker pattern located in position 'index' of the patts vector
      avrPattern&    getPatt(int index) const;
      //! gets the projection matrix
      avrMatrix3x4&  getProjection() const;

      /** \brief internal use function, calculates the camera transformation relative the each marker pattern
      * \param [in] marker_info list of squares (candidate markers) that were identified in scene
      * \param [in] marker_num number of squares identified in scene
      * \return true if success, false c.c.
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
