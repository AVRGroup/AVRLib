/**
   Example:     MultiObject
   Version      1.0
   Author:      Igor F. Couto, Douglas C. B. Oliveira, Rodrigo L. S. Silva
   Last Update: 15/10/2013
   Description: This exemple is an extension of SingleObject. It aims to present the correct way to rendering multiple
                objects by defining multiple markers.

   Example Informations:
      Renderization:    3 different objects over 3 markers

      Keyboard Events:
         ESC   Exit Application
         t     Set new threshold value
         +     Increases threshold value in 5 units (limit 255)
         -     Decreases threshold value in 5 units (limit 0)

      Mouse Events:     none
*/

#include <avrApplication.h>

// Definitions of the marker files
#define  OBJ1_PATT_NAME    "Data/avr.patt"
#define  OBJ2_PATT_NAME    "Data/dcc.patt"
#define  OBJ3_PATT_NAME    "Data/ice.patt"
// Definitions of width each marker
#define  OBJ1_SIZE         60.0
#define  OBJ2_SIZE         60.0
#define  OBJ3_SIZE         60.0

using namespace std;

avrApplication  *multiObjApp;

// Renderization and Keyboard Events callbacks
// Note: there various display callbacks
static void   draw1( void );
static void   draw2( void );
static void   draw3( void );
static void   keyEvent( unsigned char key, int x, int y);

int main(int argc, char *argv[])
{
   glutInit(&argc, argv);
   multiObjApp = new avrApplication();
   multiObjApp->setProjectInfo("MultiObject", "Douglas C. B. Oliveira, Igor F. Couto e Rodrigo L. S. Silva",
                          "This test shows several different virtual objects over different fiducial markers",
                          "AVR, DCC and ICE");
   #ifdef _WIN32
      multiObjApp->setCameraFiles((char*) "Data/WDM_camera_AVRLib.xml", (char *) "Data/camera_para.dat");
   #else
      // -dev=/dev/video1 -palette=RGB -width=960 -height=544
      multiObjApp->setCameraFiles("-dev=/dev/video0 -palette=RGB -width=640 -height=480", (char *) "Data/camera_para.dat");
   #endif
   // Note: For various markers of single type, various calls for the addPattern method are made. One for each marker.
   multiObjApp->addPattern((char*) OBJ1_PATT_NAME, OBJ1_SIZE, NULL, draw1);
   multiObjApp->addPattern((char*) OBJ2_PATT_NAME, OBJ2_SIZE, NULL, draw2);
   multiObjApp->addPattern((char*) OBJ3_PATT_NAME, OBJ3_SIZE, NULL, draw3);

   multiObjApp->setKeyCallback(keyEvent);
   multiObjApp->setThreshold(100);

   multiObjApp->printProjectInfo();

   multiObjApp->start();

   return 0;
}

// Display Callback One: Rendering over OBJ1_PATT_NAME marker one virtual cube
static void draw1( void )
{
   GLfloat   mat_ambient[]     = {0.0, 0.6, 0.0, 1.0};
   GLfloat   mat_flash[]       = {0.0, 0.6, 0.0, 1.0};
   GLfloat   mat_flash_shiny[] = {50.0};
   GLfloat   light_position[]  = {100.0,-200.0,200.0,0.0};
   GLfloat   ambi[]            = {0.0, 0.1, 0.0, 0.1};
   GLfloat   lightZeroColor[]  = {0.0, 0.9, 0.0, 0.1};

   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);
   glLightfv(GL_LIGHT0, GL_POSITION, light_position);
   glLightfv(GL_LIGHT0, GL_AMBIENT, ambi);
   glLightfv(GL_LIGHT0, GL_DIFFUSE, lightZeroColor);
   glMaterialfv(GL_FRONT, GL_SPECULAR, mat_flash);
   glMaterialfv(GL_FRONT, GL_SHININESS, mat_flash_shiny);
   glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
   glMatrixMode(GL_MODELVIEW);

   glTranslatef( 0.0, 0.0, 25.0 );
   glutSolidCube(50.0);

   glDisable( GL_LIGHTING );
   glDisable( GL_DEPTH_TEST );
}

// Display Callback Two: Rendering over OBJ2_PATT_NAME marker one virtual cone
static void draw2( void )
{
   GLfloat   mat_ambient[]     = {0.6, 0.6, 0.0, 1.0};
   GLfloat   mat_flash[]       = {0.6, 0.6, 0.0, 1.0};
   GLfloat   mat_flash_shiny[] = {50.0};
   GLfloat   light_position[]  = {100.0,-200.0,200.0,0.0};
   GLfloat   ambi[]            = {0.1, 0.1, 0.0, 0.1};
   GLfloat   lightZeroColor[]  = {0.9, 0.9, 0.0, 0.1};

   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);
   glLightfv(GL_LIGHT0, GL_POSITION, light_position);
   glLightfv(GL_LIGHT0, GL_AMBIENT, ambi);
   glLightfv(GL_LIGHT0, GL_DIFFUSE, lightZeroColor);
   glMaterialfv(GL_FRONT, GL_SPECULAR, mat_flash);
   glMaterialfv(GL_FRONT, GL_SHININESS, mat_flash_shiny);
   glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
   glMatrixMode(GL_MODELVIEW);

   glutSolidCone(25.0, 100.0, 20, 24);

   glDisable( GL_LIGHTING );
   glDisable( GL_DEPTH_TEST );
}

// Display Callback Three: Rendering over OBJ3_PATT_NAME marker one virtual sphere
static void draw3( void )
{
   GLfloat   mat_ambient[]     = {0.0, 0.0, 0.6, 1.0};
   GLfloat   mat_flash[]       = {0.0, 0.0, 0.6, 1.0};
   GLfloat   mat_flash_shiny[] = {50.0};
   GLfloat   light_position[]  = {100.0,-200.0,200.0,0.0};
   GLfloat   ambi[]            = {0.0, 0.0, 0.1, 0.1};
   GLfloat   lightZeroColor[]  = {0.0, 0.0, 0.9, 0.1};

   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);
   glLightfv(GL_LIGHT0, GL_POSITION, light_position);
   glLightfv(GL_LIGHT0, GL_AMBIENT, ambi);
   glLightfv(GL_LIGHT0, GL_DIFFUSE, lightZeroColor);
   glMaterialfv(GL_FRONT, GL_SPECULAR, mat_flash);
   glMaterialfv(GL_FRONT, GL_SHININESS, mat_flash_shiny);
   glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
   glMatrixMode(GL_MODELVIEW);

   glTranslatef( 0.0, 0.0, 30.0 );
   glutSolidSphere(30.0, 24, 24);

   glDisable( GL_LIGHTING );
   glDisable( GL_DEPTH_TEST );
}

static void   keyEvent( unsigned char key, int x, int y)
{
    // Quit if the ESC key is pressed
   if( key == 0x1b ) {
      cout << endl << multiObjApp->getFrameRate() << " (frame/sec)" << endl;
      multiObjApp->stop();
      exit(0);
   }

   // Change the threshold value when 't' key pressed
   if( key == 't' )
   {
      int thresh;
      cout << "Enter new threshold value (default = 100): ";
      cin >> thresh;
      multiObjApp->setThreshold(thresh);
      cout << endl;
   }

   // Increase and decrease threshhold in five units
   static int thresh = 100;
   if( key == '+' ) {
      thresh = (thresh <= 250) ? (thresh += 5) : 255;
      multiObjApp->setThreshold(thresh);
      cout << "Thresh: " << multiObjApp->getThreshHold() << endl;
   }
   if( key == '-' ) {
      thresh = (thresh >= 5) ? (thresh -= 5) : 0;
      multiObjApp->setThreshold(thresh);
      cout << "Thresh: " << multiObjApp->getThreshHold() << endl;
   }
}
