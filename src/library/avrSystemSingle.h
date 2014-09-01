#ifndef AVR_SYSTEM_SINGLE_H
#define AVR_SYSTEM_SINGLE_H

#include <avrSystemMarker.h>  // << this include avrPattern and avrUtil

/** \class avrSystemSingle avrSystemSingle.h "avrSystemSingle.h"
 * \brief manages single marker patterns
 *
 * \param filename path of the file that defines the marker pattern
 * \param width real width of the marker (in millimeters)
 * \param center center coordinates of the marker
 * \param display renderization callback
 */
class avrSystemSingle : public avrSystemMarker{
   public:
      //! main display callback
      void (*drawFunc)(int id);
      //! secondary display callback
      void (*drawFunc2)(void);

      avrSystemSingle(const char *filename, double width, double *center = NULL,void (*displayFunc)(void) = NULL);
      avrSystemSingle(const char *filename, double width, double *center = NULL,void (*displayFunc)(int) = NULL);

      // Virtual functions inherited
      //! calculates the camera transfomation relative the each marker pattern
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

