#ifndef AVR_SYSTEM_AUTO_MULTI_H
#define AVR_SYSTEM_AUTO_MULTI_H

#include <avrSystemMarker.h>

const short  MODE_NEAR_PROJECTION   = 0;
const short  MODE_NEAR_CAMERA       = 1;
const short  MODE_RESISTENCE        = 2;
const short  MODE_INCLINATION       = 3;
const short  MODE_ACCURACY          = 4;
const short  MODE_PRIORITY          = 5;

/** \class avrSystemAutoMulti avrSystemAutoMulti.h "avrSystemAutoMulti.h"
 * \brief manages marker patterns with relations between them (calculated in real time)
 *
 * \param holderMode definition mode of the base marker (responsible by renderization) [default is MODE_NEAR_PROJECTION]
 *    Possible modes are:\n
 *       \li MODE_NEAR_PROJECTION base marker is the marker most near of the object projected\n
 *       \li MODE_NEAR_CAMERA base marker is the marker most near of the real camera\n
 *       \li MODE_RESISTENCE base marker is the marker which has less state change\n
 *       \li MODE_INCLINATION base marker is the marker that is less inclined in relation to real camera\n
 *       \li MODE_ACCURACY base marker is the marker that has the larger accuracy\n
 *       \li MODE_PRIORITY base marker is the first visible marker (in the order of insertion, in other words, in the order shown in the config file)
 * \param display renderization callback
 */
class avrSystemAutoMulti : public avrSystemMarker{
   private:
      int             definedHolderMode;  // definition mode of the base marker
      unsigned int    mainMarker;         // index main marker, where will perform the projection indice
      unsigned int    holderMarker;       // index base marker, responsible by projection
      double          *accuracyTransf;    // stores the accuracy of the relation
      avrMatrix3x4    *prevTransf;        // stores the relations between the base marker and the other markers, one previous frame
      avrMatrix3x4    *transf;            // stores the relations between the base marker and the other markers

   public:
      //! main display callback
      void (*drawFunc)(int idHolder);
      //! secondary display callback
      void (*drawFunc2)(void);

      avrSystemAutoMulti(int holderMode = MODE_NEAR_PROJECTION, void (*displayFunc)(void) = NULL);
      avrSystemAutoMulti(int holderMode = MODE_NEAR_PROJECTION, void (*displayFunc)(int ) = NULL);
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

   private:
      int    searchBestMarker(double *qualityHolder, double *bestQuality);
      void   correctError();
      void   updateTransf();

      void initialize();
};

#endif
