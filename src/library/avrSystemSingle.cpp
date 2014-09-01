#include <avrSystemSingle.h>  // << inclui avrUtil, avrPattern, avrMatrix3x4
#include <avrGraphics.h>      // << inclui arvParameters que inclui avrUtil
#include <avrMath.h>          // << inclui avrUtil

avrSystemSingle::avrSystemSingle(const char *filename, double width, double *center, void (*displayFunc)(void)) : avrSystemMarker()
{
    avrPattern *newPatt = new avrPattern(filename, width, center);
    this->addPattern(*newPatt);
    this->drawFunc = NULL;
    this->drawFunc2 = displayFunc;
}

avrSystemSingle::avrSystemSingle(const char *filename, double width, double *center, void (*displayFunc)(int)) : avrSystemMarker()
{
    avrPattern *newPatt = new avrPattern(filename, width, center);
    this->addPattern(*newPatt);
    this->drawFunc = displayFunc;
    this->drawFunc2 = NULL;
}

bool avrSystemSingle::setCameraTransformation(avrPatternInfo *marker_info, int marker_num){
    int k = -1;
    for(int j = 0; j < marker_num; j++ ) {
      if( this->getPatt(0).id() == marker_info[j].id ) {

//         glColor3f( 0.0, 1.0, 0.0 );
//         argDrawSquare(marker_info[j].vertex,0,0);

         if( k == -1 ) k = j;
         else
            if( marker_info[k].cf < marker_info[j].cf )
                k = j;
      }
    }
    if( k == -1 ){
        this->getPatt(0).setVisible(false);
        return false;
    }
    else{
        this->getPatt(0).setVisible(true);
        avrGetTransMat(&marker_info[k], this->getPatt(0).center(), this->getPatt(0).width(), this->getPatt(0).trans());
    }
    return true;
}

void avrSystemSingle::setObjectTransformation()
{
   double    gl_para[16];

   argDrawMode3D();
   argDraw3dCamera( 0, 0 );
   glEnable(GL_DEPTH_TEST);
   glDepthFunc(GL_LEQUAL);

   // load the camera transformation matrix
   this->getPatt(0).trans().getMatrixGLFormat(gl_para);
   glMatrixMode(GL_MODELVIEW);
   glLoadMatrixd( gl_para );
}

void avrSystemSingle::drawFunction()
{
   if(this->drawFunc)
      this->drawFunc(this->getPatt(0).id());
   else if(this->drawFunc2)
      this->drawFunc2();
   else;
      // nop

   argDrawMode2D();
   glDisable( GL_DEPTH_TEST );
}

void avrSystemSingle::setDisplayCallback(void (*displayFunc)(void))
{
   this->drawFunc2 = displayFunc;
}

void avrSystemSingle::setDisplayCallback(void (*displayFunc)(int ))
{
   this->drawFunc = displayFunc;
}
