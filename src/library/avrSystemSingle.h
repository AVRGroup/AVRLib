/*
  Name:        avrSystemSingle.h
  Version      1.0.1
  Author:      Douglas Oliveira, Rodrigo L. S. Silva
  Date:        10/10/2012
  Last Update: 09/10/2014
  Description: class to manage single marker patterns
*/

#ifndef AVR_SYSTEM_SINGLE_H
#define AVR_SYSTEM_SINGLE_H

#include <avrSystemMarker.h>  // << this include avrPattern and avrUtil

/** \class avrSystemSingle avrSystemSingle.h "avrSystemSingle.h"
 * \brief manages single marker patterns
 */
class avrSystemSingle : public avrSystemMarker{
   public:
      //! main display callback
      void (*drawFunc)(int id);
      //! secondary display callback \note only one renderization callback can be active
      void (*drawFunc2)(void);

      /** \brief initialization constructor
       * \param filename path of the file that defines the marker pattern
       * \param width real width of the marker (in millimeters)
       * \param center center coordinates of the marker
       * \param displayFunc(void) main renderization callback
       */
      avrSystemSingle(const std::string& filename, double width, double *center = NULL, void (*displayFunc)(int) = NULL);
      /** \brief initialization constructor
       * \param filename path of the file that defines the marker pattern
       * \param width real width of the marker (in millimeters)
       * \param center center coordinates of the marker
       * \param displayFunc(void) secondary renderization callback
       */
      avrSystemSingle(const std::string& filename, double width, double *center = NULL, void (*displayFunc)(void) = NULL);

      // Virtual functions inherited
      //! calculates the camera transformation relative the each marker pattern
      virtual bool setCameraTransformation(avrPatternInfo *marker_info, int marker_num);
      //! sets object transformation and prepares draw
      virtual void setObjectTransformation();
      //! sets secondary display function
      virtual void setDisplayCallback(void (*displayFunc)(void));
      //! sets main display function
      virtual void setDisplayCallback(void (*displayFunc)(int ));
      //! calls the renderization callback
      virtual void drawFunction();
};

#endif

