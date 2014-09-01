#ifndef AVR_SYSTEM_MULTI_H
#define AVR_SYSTEM_MULTI_H

#include <avrSystemMarker.h>  // this include avrPattern and avrUtil

/** \class avrSystemMulti avrSystemMulti.h "avrSystemMulti.h"
 * \brief manages marker patterns with relations between them (calculated in preprocessing)
 *
 * \param filename path of the config file of the markers
 * \param display renderization callback
 */
class avrSystemMulti : public avrSystemMarker{
   public:
      //! main display callback
      void (*drawFunc)(int id);
      //! secondary display callback
      void (*drawFunc2)(void);

      avrSystemMulti(const char* filename, void (*displayFunc)(void) = NULL);
      avrSystemMulti(const char* filename, void (*displayFunc)(int) = NULL);

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

   private:
      void  initialize(const char*);

};

#endif
