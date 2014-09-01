#include <iostream>
#include <cstdlib>
#include <avrSystemMulti.h>   // << inclui avrUtil, avrPattern, avrMatrix3x4
#include <avrGraphics.h>      // << inclui arvParameters que inclui avrUtil
#include <avrMarker.h>        // << inclui avrUtil
#include <avrMath.h>          // << inclui avrUtil

static ARMultiMarkerInfoT* multi = NULL;

using namespace std;

avrSystemMulti::avrSystemMulti(const char* filename, void (*displayFunc)(void)) : avrSystemMarker()
{
   this->drawFunc = NULL;
   this->drawFunc2 = displayFunc;

   this->initialize(filename);
}

avrSystemMulti::avrSystemMulti(const char* filename, void (*displayFunc)(int)) : avrSystemMarker()
{
   this->drawFunc = displayFunc;
   this->drawFunc2 = NULL;

   this->initialize(filename);
}

void avrSystemMulti::initialize(const char* filename)
{
   multi = arMultiReadConfigFile(filename);
   if(!multi){
      cout << "The config file can not be opened or read";
      exit(EXIT_FAILURE);
   }

   avrPattern *newPatt;
   for(int i = 0; i < multi->marker_num; i++){
      newPatt = new avrPattern(NULL, multi->marker[i].width, multi->marker[i].center);
      newPatt->setID(multi->marker[i].patt_id);
      this->addPattern(*newPatt);
   }
}

bool avrSystemMulti::setCameraTransformation(avrPatternInfo *marker_info, int marker_num){
   double err = avrMultiGetTransMat(marker_info, marker_num, multi);
   if(err < 0 || err > 100) return false;

   for(int k = 0; k < this->patts.size(); k++){
      for(int i = 0, j = 0; i < 4, j < 3; i++, j++){
         try{
            this->getPatt(k).pos3D().add(multi->marker[k].pos3d[i][j], i, j);
         }catch(out_of_range& e){
            cerr << e.what() << endl;
         }
      }
      this->getPatt(k).trans() = multi->marker[k].trans;
      this->getPatt(k).setVisible(multi->marker[k].visible > 0);
   }

   *(this->projection) = multi->trans;

   return true;
}

void avrSystemMulti::setObjectTransformation()
{
   double    gl_para[16];

   argDrawMode3D();
   argDraw3dCamera( 0, 0 );
   glEnable(GL_DEPTH_TEST);
   glDepthFunc(GL_LEQUAL);
   // load the camera transformation matrix
   glMatrixMode(GL_MODELVIEW);
   this->projection->getMatrixGLFormat(gl_para);
   glLoadMatrixd( gl_para );
}

void avrSystemMulti::drawFunction()
{
   if(this->drawFunc){
      double gl_para[16];
      int i = 0;
      while(true){
         // Transformes each marker that compound the pattern
         this->getPatt(i).trans().getMatrixGLFormat(gl_para);
         glMultMatrixd( gl_para );
         // Call draw function
         this->drawFunc(this->getPatt(i).id());
         // Clean depth buffer
         argDrawMode2D();
         glDisable( GL_DEPTH_TEST );

         i++;
         if(i >= (int)this->patts.size()) break;

         this->setObjectTransformation();
      }
   }

   else if(this->drawFunc2)
      this->drawFunc2();

   argDrawMode2D();
   glDisable( GL_DEPTH_TEST );
}

void avrSystemMulti::setDisplayCallback(void (*displayFunc)(void))
{
   this->drawFunc2 = displayFunc;
}

void avrSystemMulti::setDisplayCallback(void (*displayFunc)(int ))
{
   this->drawFunc = displayFunc;
}
