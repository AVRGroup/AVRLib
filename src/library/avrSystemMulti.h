/*
  Name:        avrSystemMulti.h
  Version      1.0.1
  Author:      Douglas Oliveira, Rodrigo L. S. Silva
  Date:        10/10/2012
  Last Update: 09/10/2014
  Description: class to manage marker patterns with relations previously calculated
*/

#ifndef AVR_SYSTEM_MULTI_H
#define AVR_SYSTEM_MULTI_H

#include <avrSystemMarker.h>  // this include avrPattern

/** \class avrSystemMulti avrSystemMulti.h "avrSystemMulti.h"
 * \brief manages marker patterns with relations between them (calculated in preprocessing)
 */
class avrSystemMulti : public avrSystemMarker{
   public:
      //! main display callback
      void (*drawFunc)(int id);
      //! secondary display callback \note only one renderization callback can be active
      void (*drawFunc2)(void);

      /**  \brief initialization constructor
      *  \param filename path of the configuration file of the markers
      *  \param displayFunc main renderization callback
      */
      avrSystemMulti(const std::string& filename, void (*displayFunc)(int) = NULL);
      /**  \brief initialization constructor
      *  \param filename path of the configuration file of the markers
      *  \param displayFunc secondary renderization callback
      */
      avrSystemMulti(const std::string& filename, void (*displayFunc)(void) = NULL);

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

   private:
      //! \cond
      void  initialize(const std::string&);
      //! \endcond

};

#endif
