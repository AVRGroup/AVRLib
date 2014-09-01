#include <string>

#include <avrPatt.h>
#include <avrMarker.h>
#include <avrGraphics.h>
#include <avrParameters.h>
#include <avrVideo.h>
#include <avrMath.h>
#include <avrUtil.h>

#include <avrApplication.h>

using namespace std;

static int xwinT = 1, ywinT = 1;    // x and y of the video output in the threshold mode
static int xwinM = 0, ywinM = 0;    // x and y of the main video output

//Constructor
avrApplication::avrApplication()
{
   this->reshapeFunc = NULL;
   this->visibilityFunc = NULL;
   this->specialFunc = NULL;

   this->keyEvent = NULL;
   this->motionEvent = NULL;
   this->mouseEvent = NULL;

   this->thresholdMode = false;
   this->thresh = 100;
   this->count = 0;
}

//Destructor
avrApplication::~avrApplication()
{

}

// Used to set current instance
avrApplication* g_CurrentInstance;

// Local function to be passed to glut
void localDrawCallback()
{
   g_CurrentInstance->mainLoop();
}

//Setup Camera Default
void avrApplication::setCameraFiles()
{
   this->setCameraFiles((char *) "Data/WDM_camera_AVRLib.xml", (char *) "Data/camera_para.dat");
}

//Setup Camera
void avrApplication::setCameraFiles(char *vconf, char *cparam_name, int xwin, int ywin)
{
	int xsize, ysize;
	ARParam  wparam, cparam;

	/* open the video path */
	if( arVideoOpen( vconf ) < 0 )
		exit(0);
	/* find the size of the window */
	if( arVideoInqSize(&xsize, &ysize) < 0 )
		exit(0);

	printf("Image size (x,y) = (%d,%d)\n", xsize, ysize);

	/* set the initial camera parameters */
	if( arParamLoad(cparam_name, 1, &wparam) < 0 )
	{
		printf("Camera parameter load error !!\n");
		exit(0);
	}
	arParamChangeSize( &wparam, xsize, ysize, &cparam );
	arInitCparam( &cparam );
	printf("*** Camera Parameter ***\n");
	arParamDisp( &cparam );
   /* open the graphics window */
   argInit( &cparam, 1.0, 0, xwin, ywin, 0 );
}

//Sets
void avrApplication::setThreshold(int thresh)
{
   this->thresh = thresh;
}

void avrApplication::setThreshold(){ //default
   this->setThreshold(100);
}

//Set Callbacks
void avrApplication::setReshapeCallback(void (*reshapeFunction)(int w, int h))
{
   this->reshapeFunc = reshapeFunction;
}

void avrApplication::setVisibilityCallback(void (*visibilityFunction)(int visible))
{
   this->visibilityFunc = visibilityFunction;
}

void avrApplication::setSpecialCallback(void (*specialFunction)(int key, int x, int y)){
   this->specialFunc = specialFunction;
}

void avrApplication::setKeyCallback(void (*keyEvent)(unsigned char key, int x, int y)){
   this->keyEvent = keyEvent;
}

void avrApplication::setMouseCallback(void (*mouseEvent)(int button, int state, int x, int y)){
   this->mouseEvent = mouseEvent;
}

void avrApplication::setMotionCallback(void (*motionEvent)(int x, int y)){
   this->motionEvent = motionEvent;
}

void avrApplication::setNewSystem(avrSystemMarker * newSystem){
   this->markers.push_back(newSystem);
}

//Resets
void avrApplication::resetFrameRate(){
   this->count = 0;
}

//Gets
int avrApplication::getThreshHold(){
   return this->thresh;
}

double avrApplication::getFrameRate(){
   return (double)this->count/arUtilTimer();
}

avrSystemMarker *avrApplication::getSystem(int index){
   return this->markers.at(index);
}

avrPattern avrApplication::getPattern(int index){
   int prevSize = 0, currSize = this->markers.at(0)->sizePatts();
   int i = 0;

   while(index >= currSize){
      i++;
      prevSize = currSize;
      currSize += this->markers.at(i)->sizePatts();
   }

   int idPatt = index - prevSize;
   return this->markers.at(i)->getPatt(idPatt);
}

int avrApplication::numberPatts(){
   int currSize = 0;
   for(unsigned int i = 0; i < this->markers.size(); i++)
      currSize += this->markers.at(i)->sizePatts();

   return currSize;
}

void avrApplication::setMainVideoOutput(unsigned int xwin, unsigned int ywin){
   xwinM = xwin; ywinM = ywin;
}

void avrApplication::enableModeThreshold(unsigned int xwin, unsigned int ywin)
{
   xwinT = xwin; ywinT = ywin;
   arDebug = 1 - arDebug;
   this->thresholdMode = true;
}

void avrApplication::disableModeThreshold()
{
   xwinT = ywinT = 1;
   arDebug = 1 - arDebug;
   this->thresholdMode = false;

   glClearColor( 0.0, 0.0, 0.0, 0.0 );
   glClear(GL_COLOR_BUFFER_BIT);
   argSwapBuffers();
}

bool avrApplication::isThresholdMode(){
   return this->thresholdMode;
}

void avrApplication::renderContext2D(){
   argDrawMode2D();
}

void avrApplication::renderContext3D(unsigned int xwin, unsigned int ywin){
   argDrawMode3D();
   argDraw3dCamera( xwin, ywin );
}

//Add SINGLE_MARKER
void avrApplication::addPattern(const char *filename, double patt_width, double *patt_center,void (*drawFunction)(void))
{
   avrSystemSingle *newSystem = new avrSystemSingle(filename, patt_width, patt_center, drawFunction);
   markers.push_back(newSystem);
}
void avrApplication::addPattern(const char *filename, double patt_width, double *patt_center,void (*drawFunction)(int ))
{
   avrSystemSingle *newSystem = new avrSystemSingle(filename, patt_width, patt_center, drawFunction);
   markers.push_back(newSystem);
}

// Add AUTO_MULTI_MARKER
static void initAutoMulti(avrSystemAutoMulti& sam, const char* filename);

void avrApplication::addPatterns(const char *filename, int holderMode, void (*drawFunction)(void)){
   avrSystemAutoMulti *newSystem = new avrSystemAutoMulti(holderMode, drawFunction);
   initAutoMulti(*newSystem, filename);
   this->markers.push_back(newSystem);
}
void avrApplication::addPatterns(const char *filename, int holderMode, void (*drawFunction)(int )){
   avrSystemAutoMulti *newSystem = new avrSystemAutoMulti(holderMode, drawFunction);
   initAutoMulti(*newSystem, filename);
   this->markers.push_back(newSystem);
}

static void initAutoMulti(avrSystemAutoMulti& sam, const char* filename){
   int numberPatts;
   avrPattern * patts = avrPattern::avrReadConfigFileNotRelation(filename, &numberPatts);
   if(!patts){
      cout << "The config file can not opened";
      exit(EXIT_FAILURE);
   }

   for(int i = 0; i < numberPatts; i++)
      sam.addPattern(patts[i]);
   sam.initPointers();
}

// Add MULTI_MARKER
void avrApplication::addPatterns(const char *filename, void (*drawFunction)(void)){
   avrSystemMulti * newSystem = new avrSystemMulti(filename, drawFunction);
   this->markers.push_back(newSystem);
}
void avrApplication::addPatterns(const char *filename, void (*drawFunction)(int )){
   avrSystemMulti * newSystem = new avrSystemMulti(filename, drawFunction);
   this->markers.push_back(newSystem);
}

//Init and Loop Aplication
void avrApplication::mainLoop()
{
   ARUint8         *dataPtr;
   avrPatternInfo  *marker_info;
   int             marker_num;

   /* grab a video frame */
   if( (dataPtr = (ARUint8 *)arVideoGetImage()) == NULL )
   {
      arUtilSleep(2);
      return;
   }
   if( count == 0 ) arUtilTimerReset();
   count++;

   /* detect the markers in the video frame */
   if( avrDetectMarker(dataPtr, thresh, &marker_info, &marker_num) < 0 )
   {
      this->stop();
      exit(0);
   }

   glClearColor( 0.0, 0.0, 0.0, 0.0 );
   glClear(GL_COLOR_BUFFER_BIT);
   argDrawMode2D();

   if( !thresholdMode ) {
      argDispImage( dataPtr, xwinM, ywinM ); // xwinM and ywinM defined at beginning of this file
   }
   else {
      argDispImage( dataPtr, xwinM, ywinM );
      if( arImageProcMode == AR_IMAGE_PROC_IN_HALF )
         argDispHalfImage( arImage, xwinT, ywinT );
      else
         argDispImage( arImage, xwinT, ywinT);
   }

   arVideoCapNext();

   argDrawMode3D();
   argDraw3dCamera( xwinM, ywinM );
   glClearDepth( 1.0 );
   glClear(GL_DEPTH_BUFFER_BIT);

   for(unsigned int k = 0; k < this->markers.size(); k++ ) {
      // Computes the camera traformation in the space of the marker
      bool visible = this->markers.at(k)->setCameraTransformation(marker_info, marker_num);
      if(!visible) continue;
      // Computes object position and prepare opengl to draw
      this->markers.at(k)->setObjectTransformation();
      // Call internal pointer to external draw Function
      this->markers.at(k)->drawFunction();
   }
   argSwapBuffers();
}

void avrApplication::start()
{
	// Points the global instance to the current instance
	g_CurrentInstance = this;

	// Setup glut stuff
	glutDisplayFunc(localDrawCallback);
	glutIdleFunc(localDrawCallback);
	if(keyEvent)
      glutKeyboardFunc(keyEvent);
   if(specialFunc)
      glutSpecialFunc(specialFunc);
	if(motionEvent)
      glutMotionFunc(motionEvent);
	if(mouseEvent)
      glutMouseFunc(mouseEvent);
	if(reshapeFunc)
      glutReshapeFunc(reshapeFunc);
	if(visibilityFunc)
      glutVisibilityFunc(visibilityFunc);

	arVideoCapStart();
	glutMainLoop();
}

void avrApplication::stop()
{
   arVideoCapStop();
   arVideoClose();
   argCleanup();
}

// struct that stores application information
typedef struct infoapp{
   string authors;
   string projectName;
   string info;
   string markers;
}*InfoApp;

InfoApp infoApp = NULL;

void avrApplication::setProjectInfo(string projectName, string authors, string info, string requiredMarkers){
   infoApp = new struct infoapp;

   infoApp->projectName = projectName;
   infoApp->authors = authors;
   infoApp->info = info;
   infoApp->markers = requiredMarkers;
}

void avrApplication::printProjectInfo(){
   cout << "------------------------------------------------------------------------" << endl <<
            "Project:  " << infoApp->projectName << endl <<
            "Authors:  " << infoApp->authors << endl <<
            "Info:     " << infoApp->info << endl << endl <<
            "Required Markers:  " << infoApp->markers << endl <<
            "------------------------------------------------------------------------" << endl;
}
