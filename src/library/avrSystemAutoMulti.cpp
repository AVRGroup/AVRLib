#include <iostream>
#include <avrSystemAutoMulti.h>	// << inclui avrPattern e avrMatrix3x4
#include <avrGraphics.h>		// << inclui avrParameters que inclui avrUtil
#include <avrMath.h>			// << inclui avrUtil
#include <avrUtil.h>

#define RATE_QUALITY    1.1

using namespace std;

static double   delta       = 0.0;  // posição do quaternio q3 utilizado no cálculo de erro (valores [0, 1])
static double   theta       = 0.0;  // arco coseno do prod. escalar entre os quaternios q1 e q2 usados no cálculo de erro
static bool     firstFrame  = true; // representa o primeiro quadro em que o marcador principal está visivel

// constructor
avrSystemAutoMulti::avrSystemAutoMulti(int holderMode, void (*displayFunc)(void)) : avrSystemMarker(){
    this->definedHolderMode = holderMode;

    this->drawFunc = NULL;
    this->drawFunc2 = displayFunc;

    this->initialize();
}

avrSystemAutoMulti::avrSystemAutoMulti(int holderMode, void (*displayFunc)(int )) : avrSystemMarker()
{
   this->definedHolderMode = holderMode;

   this->drawFunc = displayFunc;
   this->drawFunc2 = NULL;

   this->initialize();
}

void avrSystemAutoMulti::initialize()
{
   this->mainMarker = 0;
   this->holderMarker = 0;
   this->transf = NULL;
   this->prevTransf = NULL;
   this->accuracyTransf = NULL;
}

avrSystemAutoMulti::~avrSystemAutoMulti(){

}

void avrSystemAutoMulti::initPointers(){
    unsigned int i;

    this->prevTransf = new avrMatrix3x4[this->patts.size()];
    this->transf  = new avrMatrix3x4[this->patts.size()];
    for ( i = 0; i < this->patts.size(); i++){
         try{
            this->prevTransf[i].add(0.0, 0, 0);
            this->prevTransf[i].add(0.0, 1, 1);
            this->prevTransf[i].add(0.0, 2, 2);
         }
         catch(out_of_range& e){
            cerr << e.what() << endl;
         }
    }

    this->accuracyTransf = new double[this->patts.size()];
    for ( i = 0; i < this->patts.size(); i++)
        this->accuracyTransf[i] = 0.0;
}

void avrSystemAutoMulti::setMainMarker(int mainMarker){
    this->mainMarker = mainMarker;
}

bool avrSystemAutoMulti::setCameraTransformation(avrPatternInfo *marker_info, int marker_num){
    for(unsigned int i = 0; i < this->patts.size(); i++){
        int k = -1;
        for(int j = 0; j < marker_num; j++ ) {
          if( this->getPatt(i).id() == marker_info[j].id ) {

//             glColor3f( 0.0, 1.0, 0.0 );
//             argDrawSquare(marker_info[j].vertex,0,0);

             if( k == -1 ) k = j;
             else
                if( marker_info[k].cf < marker_info[j].cf )
                    k = j;
          }
        }
        if( k == -1 ){
            this->getPatt(i).setVisible(false);
            continue;
        }

        // calculate the transform for each marker
		if( !this->getPatt(i).visible()) {
            double err = avrGetTransMat(&marker_info[k], this->getPatt(i).center(), this->getPatt(i).width(),
                                       this->getPatt(i).trans());
            this->getPatt(i).setAccuracy(err);
        }
        else {
            double err = avrGetTransMatCont(&marker_info[k], this->getPatt(i).trans(),
                                           this->getPatt(i).center(), this->getPatt(i).width(),
                                           this->getPatt(i).trans());
            this->getPatt(i).setAccuracy(err);
        }

        this->getPatt(i).setVisible(true);
    }

    // the application is in the first frame and the main marker isn't visible
    if(firstFrame)
        if(!this->getPatt(this->mainMarker).visible())
            return false;

    // confers if there at least one visible marker
    for(unsigned int i =0; i < this->patts.size(); i++){
        if(this->getPatt(i).visible()) break;          // there at least one visible marker
        if(i == this->patts.size() - 1) return false;   // last marker, none is visible
    }

    return true;
}

// atualiza as relações entre o principal e os demais marcadores
void avrSystemAutoMulti::updateTransf(){

    //loadIdentityMatrix3x4(this->transf[holderMarker]);
    for(unsigned int i = 0; i < this->patts.size(); i++ ){
		if (i == holderMarker) continue;

		if (this->getPatt(i).visible()){
			if (this->definedHolderMode != MODE_NEAR_PROJECTION)
                if (this->accuracyTransf[i] >= (this->getPatt(holderMarker).accuracy() * this->getPatt(i).accuracy()))
                    continue;
         this->transf[i] = this->getPatt(i).trans().getRelationWith(*this->projection);
         this->accuracyTransf[i] = this->getPatt(holderMarker).accuracy() * this->getPatt(i).accuracy();
		}
	}
}

void avrSystemAutoMulti::setObjectTransformation(){
   int bestMarker;
   double bestQuality, qualityHolder;

   bestMarker = searchBestMarker(&qualityHolder, &bestQuality);

   if ((int)holderMarker != bestMarker){
      if (qualityHolder * RATE_QUALITY < bestQuality){
         holderMarker = bestMarker;
         for(unsigned int i = 0; i < this->patts.size(); i++ ){
            this->accuracyTransf[i] = 0.0;
         }
         this->accuracyTransf[holderMarker] = 1.0;
      }
   }
   if(firstFrame){
      holderMarker = this->mainMarker;
      firstFrame = false;
   }

   double  gl_para[16];

   argDrawMode3D();
   argDraw3dCamera( 0, 0 );
   glEnable(GL_DEPTH_TEST);
   glDepthFunc(GL_LEQUAL);

   try{
      *(this->projection) = this->getPatt(holderMarker).trans() * this->transf[holderMarker];
   }
   catch(invalid_argument& e){
      cerr << e.what() << endl;
   }
   //this->correctError();
   this->projection->getMatrixGLFormat(gl_para);

   glMatrixMode(GL_MODELVIEW);
   glLoadMatrixd( gl_para );
}

void avrSystemAutoMulti::setDisplayCallback(void (*displayFunc)(int )){
    this->drawFunc = displayFunc;
}

void avrSystemAutoMulti::setDisplayCallback(void (*displayFunc)(void)){
    this->drawFunc2 = displayFunc;
}

void avrSystemAutoMulti::drawFunction(){
   if(this->drawFunc)
      this->drawFunc(this->getPatt(holderMarker).id());
   else if(this->drawFunc2)
      this->drawFunc2();

   argDrawMode2D();
   glDisable( GL_DEPTH_TEST );

   this->updateTransf();
}

// Procura por marcadores com melhor qualidade
int avrSystemAutoMulti::searchBestMarker(double *qualityHolder, double *bestQuality){
    double  auxQuality;
    int bestMarker  = holderMarker;
    *bestQuality = *qualityHolder = 0.0;

    switch(this->definedHolderMode){
        case MODE_NEAR_PROJECTION:
           try{
               *(this->projection) = this->getPatt(holderMarker).trans() * this->transf[holderMarker];
           }
           catch(out_of_range& e){
               cerr << e.what() << endl;
           }
            for(unsigned int i = 0; i < this->patts.size(); i++ ){
                if (this->getPatt(i).visible() && this->accuracyTransf[i] > 0){
                    auxQuality = this->getPatt(i).trans().euclidianDistanceBetween(*this->projection);
                    auxQuality = 1/(auxQuality);
                    if (auxQuality > *bestQuality) {
                        bestMarker = i;
                        *bestQuality = auxQuality;
                    }
                    if (i == holderMarker) {
                        *qualityHolder = auxQuality;
                    }
                }
            }
            break;

        case MODE_NEAR_CAMERA:
            for(unsigned int i = 0; i < this->patts.size(); i++ )
            {
                double (*trans)[4] = (double(*)[4]) this->getPatt(i).trans().matrix();
                if (this->getPatt(i).visible() && this->accuracyTransf[i] > 0)
                {
                    auxQuality = sqrt( pow(trans[0][3], 2) + pow(trans[1][3], 2) + pow(trans[2][3], 2));
                    auxQuality = 1/(auxQuality);
                    if (auxQuality > *bestQuality) {
                        bestMarker = i;
                        *bestQuality = auxQuality;
                    }
                    if (i == holderMarker) {
                        *qualityHolder = auxQuality;
                    }
                }
            }
            break;

        case MODE_RESISTENCE:
            for(unsigned int i = 0; i < this->patts.size(); i++ ){
                if (this->getPatt(i).visible() && this->accuracyTransf[i] > 0){
                    auxQuality = this->getPatt(i).trans().euclidianDistanceBetween(this->prevTransf[i]);
                    this->prevTransf[i] = this->getPatt(i).trans();
                    auxQuality = 1/(auxQuality);
                    if (auxQuality > *bestQuality){
                        bestMarker = i;
                        *bestQuality = auxQuality;
                    }
                    if (i == holderMarker) {
                        *qualityHolder = auxQuality;
                    }
                } else {
                   try{
                     this->prevTransf[i].add(0.0, 0, 0);
                     this->prevTransf[i].add(0.0, 1, 1);
                     this->prevTransf[i].add(0.0, 2, 2);
                   }
                   catch(out_of_range& e){
                      cerr << e.what() << endl;
                   }
                }
            }
            break;

        case MODE_INCLINATION:
            double point1[3]; //primeiro ponto do segmento de reta a ser projetado
            double point2[3]; //segundo ponto do segmento de reta a ser projetado
            double point1Result[3]; //primeiro ponto depois da transformação
            double point2Result[3]; //segundo ponto depois da transformação

            point1[0] = 0; point1[1] = 0; point1[2] = 100;
            point2[0] = 0; point2[1] = 0; point2[2] = 0;

            for(unsigned int i = 0; i < this->patts.size(); i++ ){
                if (this->getPatt(i).visible() && this->accuracyTransf[i] > 0){
                    double (*trans)[4] = (double(*)[4])this->getPatt(i).trans().matrix();
                    multiMatrix3x4Vector3(point1Result, trans, point1);
                    multiMatrix3x4Vector3(point2Result, trans, point2);
                    auxQuality = sqrt(pow(point1Result[0] - point2Result[0],2) + pow(point1Result[1] - point2Result[1],2));
                    auxQuality = 1/(auxQuality);

                    if (auxQuality > *bestQuality) {
                        bestMarker = i;
                        *bestQuality = auxQuality;
                    }
                    if (i == holderMarker) {
                        *qualityHolder = auxQuality;
                    }
                }
            }
            break;

        case MODE_ACCURACY:
            for(unsigned int i = 0; i < this->patts.size(); i++ ){
                if (this->getPatt(i).visible() && this->accuracyTransf[i] > 0){
                    auxQuality = this->getPatt(i).accuracy();
                    if (auxQuality > *bestQuality) {
                        bestMarker = i;
                        *bestQuality = auxQuality;
                    }
                    if (i == holderMarker) {
                        *qualityHolder = auxQuality;
                    }
                }
            }
            break;

        default: // MODE_PRIORITY
            for(unsigned int i = 0; i < this->patts.size(); i++ ){
                if (this->getPatt(i).visible() && this->accuracyTransf[i] > 0){
                    holderMarker = i;
                    break;
                }
            }
    }
    return bestMarker;
}

// corrige o erro de projeção quando o marcador principal está visível
void avrSystemAutoMulti::correctError(){
    if (this->getPatt(this->mainMarker).visible()) {
		double quart1[4],quart2[4],quart3[4];
		double translation1[3],translation2[3],translation3[3];
		double scalarProd;
		double distancia;
         try{
            this->projection->extractQuatAndPos(quart1, translation1);
            this->getPatt(this->mainMarker).trans().extractQuatAndPos(quart2, translation2);
         }catch(domain_error& e ){
            cerr << e.what() << endl;
         }

        scalarProduct4(scalarProd,quart1,quart2);
        if (scalarProd < 0 ) {
            quart1[0] = -quart1[0];
            quart1[1] = -quart1[1];
            quart1[2] = -quart1[2];
            quart1[3] = quart1[3];
            scalarProduct4(scalarProd,quart1,quart2);
        }
        theta = acos(scalarProd);
        distancia = this->projection->euclidianDistanceBetween(this->getPatt(this->mainMarker).trans());

        if (theta > 0 && distancia > 0) {
            quart3[0] = quart1[0] * (sin((1-delta)*theta) / sin(theta)) + quart2[0] * (sin(delta*theta) / sin(theta));
            quart3[1] = quart1[1] * (sin((1-delta)*theta) / sin(theta)) + quart2[1] * (sin(delta*theta) / sin(theta));
            quart3[2] = quart1[2] * (sin((1-delta)*theta) / sin(theta)) + quart2[2] * (sin(delta*theta) / sin(theta));
            quart3[3] = quart1[3] * (sin((1-delta)*theta) / sin(theta)) + quart2[3] * (sin(delta*theta) / sin(theta));

            translation3[0] = translation1[0] + delta*(translation2[0] - translation1[0]);
            translation3[1] = translation1[1] + delta*(translation2[1] - translation1[1]);
            translation3[2] = translation1[2] + delta*(translation2[2] - translation1[2]);

            delta += 0.05;
            delta = (delta>1)?1:delta;
            //printf("Delta: %f",delta);

            this->projection->setMatWithQuatAndPos(quart3, translation3);
        }
	} else {
		delta = 0;
	}
}
