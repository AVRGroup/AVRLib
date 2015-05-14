/*
  Name:        avrSystemAutoMulti.h
  Version      1.0.1
  Author:      Douglas Oliveira, Rodrigo L. S. Silva
  Date:        10/10/2012
  Last Update: 09/10/2014
  Description: class to manage marker patterns with relations between them
*/

#ifndef AVR_SYSTEM_AUTO_MULTI_H
#define AVR_SYSTEM_AUTO_MULTI_H

#include <avrSystemMarker.h>

/** \enum BASE_SELECTION_MODE
*  \brief defines the base marker selection modes
*/
enum BASE_SELECTION_MODE {
   //! base marker is the marker most near of the object projected
   MODE_NEAR_PROJECTION,
   //! base marker is the marker most near of the real camera
   MODE_NEAR_CAMERA,
   //! base marker is the marker which has less state change
   MODE_RESISTENCE,
   //! base marker is the marker that is less inclined in relation to real camera
   MODE_INCLINATION,
   //! base marker is the marker that has the larger accuracy
   MODE_ACCURACY,
   //! base marker is the first visible marker (in the order of insertion, in other words, in the order shown in the configuration file)
   MODE_PRIORITY
};

/** \class avrSystemAutoMulti avrSystemAutoMulti.h "avrSystemAutoMulti.h"
 * \brief manages marker patterns with relations between them (calculated in real time)
 */
class avrSystemAutoMulti : public avrSystemMarker{
   //! \cond
   private:
      unsigned int         mainMarker;         // index main marker, where will perform the projection indice
      unsigned int         holderMarker;       // index base marker, responsible by projection
      double               *accuracyTransf;    // stores the accuracy of the relation
      avrMatrix3x4         *prevTransf;        // stores the relations between the base marker and the other markers, one previous frame
      avrMatrix3x4         *transf;            // stores the relations between the base marker and the other markers
      BASE_SELECTION_MODE  definedHolderMode;  // definition mode of the base marker
   //! \endcond

   public:
      //! main display callback
      void (*drawFunc)(int idHolder);
      //! secondary display callback
      void (*drawFunc2)(void);

      /** \brief initialization constructor
       * \param BASE_SELECTION_MODE base marker selection mode (default is MODE_NEAR_PROJECTION)
       * \param display main renderization callback
       */
      avrSystemAutoMulti(BASE_SELECTION_MODE = MODE_NEAR_PROJECTION, void (*displayFunc)(int ) = NULL);
      /** \brief initialization constructor
       * \param BASE_SELECTION_MODE base marker selection mode (default is MODE_NEAR_PROJECTION)
       * \param display secondary renderization callback
       */
      avrSystemAutoMulti(BASE_SELECTION_MODE = MODE_NEAR_PROJECTION, void (*displayFunc)(void) = NULL);
      //! destructor
      virtual ~avrSystemAutoMulti();

      //! sets marker responsible by projection
      void    setMainMarker(int mainMarker);
      //! initializes tranf, accuracyTranf and prevTranf
      void    initPointers();

      // Gets
      inline int            getMainMarker()     { return this->mainMarker; }
      inline int            getHolderMarker()   { return this->holderMarker; }
      inline avrMatrix3x4   *getPrevTransf()    { return this->prevTransf; }
      inline avrMatrix3x4   *getTransf()        { return this->transf; }

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

   //! \cond
   private:
      int   searchBestMarker(double *qualityHolder, double *bestQuality);
      void  correctError();
      void  updateTransf();
      void  initialize();
   //! \endcond
};

#endif
