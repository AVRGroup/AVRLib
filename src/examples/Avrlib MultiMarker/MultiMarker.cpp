/**
   Example:     MultiMarker
   Version      1.0
   Author:      Douglas C. B. Oliveira, Rodrigo L. S. Silva
   Last Update: 15/10/2013
   Description: This example is the first using the avrSystemMulti system markers that handle the cases of multiple markers with a relation
               between themselves that are previously calculated. The objective of this example is to show how the system works.

   Example Informations:
      Renderization:    Virtual Cubes

      Keyboard Events:
         ESC   Exit Application
         t     Set new threshold value
         +     Increases threshold value in 5 units (limit 255)
         -     Decreases threshold value in 5 units (limit 0)
         d     Enable/Disable visualization mode of threshold

      Mouse Events:     none
*/

#include <avrApplication.h>

using namespace std;

avrApplication *multiApp;
static void    draw(int id);
static void    keyEvent( unsigned char key, int x, int y);

int main(int argc, char **argv)
{
   glutInit(&argc, argv);
   multiApp = new avrApplication();
   multiApp->setProjectInfo("MultiMarker", "Douglas C. B. Oliveira e Rodrigo L. S. Silva",
                          "This test shows virtual objects over a multi fiducial marker", "AVR Numbers");
   #ifdef _WIN32
      multiApp->setCameraFiles((char*) "Data/WDM_camera_AVRLib.xml", (char *) "Data/camera_para.dat");
   #else
      // -dev=/dev/video1 -palette=RGB -width=960 -height=544
      multiApp->setCameraFiles("-dev=/dev/video0 -palette=RGB -width=640 -height=480", (char *) "Data/camera_para.dat");
   #endif
   // Note: addPatternS. The file passed here contains all markers that composes the system
   // The draw function contains one interger parameter, in this case, avrSystemMulti calls the callback for each marker.
   // if the parameter is void avrSystemMulti calls the callback for the entire system.
   multiApp->addPatterns((char *) "Data/multi/markerCM.dat", draw);

   multiApp->setKeyCallback(keyEvent);
   multiApp->setThreshold(100);

   multiApp->printProjectInfo();

   multiApp->start();

   return 0;
}

// draw cube
static void draw(int id)
{
   GLfloat   mat_ambient[]     = {0.0, 0.0, 1.0, 1.0};
   GLfloat   mat_ambient1[]    = {1.0, 0.0, 0.0, 1.0};
   GLfloat   mat_flash[]       = {0.0, 0.0, 1.0, 1.0};
   GLfloat   mat_flash1[]      = {1.0, 0.0, 0.0, 1.0};
   GLfloat   mat_flash_shiny[] = {50.0};
   GLfloat   mat_flash_shiny1[]= {50.0};
   GLfloat   light_position[]  = {100.0,-200.0,200.0,0.0};
   GLfloat   ambi[]            = {0.1, 0.1, 0.1, 0.1};
   GLfloat   lightZeroColor[]  = {0.9, 0.9, 0.9, 0.1};

   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);
   glLightfv(GL_LIGHT0, GL_POSITION, light_position);
   glLightfv(GL_LIGHT0, GL_AMBIENT, ambi);
   glLightfv(GL_LIGHT0, GL_DIFFUSE, lightZeroColor);

   bool visible = multiApp->getPattern(id).visible();

   // marker is visible, draws blue cube
   if( visible ) {
      glMaterialfv(GL_FRONT, GL_SPECULAR, mat_flash);
      glMaterialfv(GL_FRONT, GL_SHININESS, mat_flash_shiny);
      glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
   }
   // marker not is visible, draws red cube
   else {
      glMaterialfv(GL_FRONT, GL_SPECULAR, mat_flash1);
      glMaterialfv(GL_FRONT, GL_SHININESS, mat_flash_shiny1);
      glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient1);
   }

    glMatrixMode(GL_MODELVIEW);
    glTranslatef( 0.0, 0.0, 25.0 );
    glLineWidth(3.0);

    glutSolidCube(50.0);

    glDisable( GL_LIGHTING );
}

static void keyEvent( unsigned char key, int x, int y)
{
   // quit if the ESC key is pressed
   if( key == 0x1b ) {
      cout << endl << multiApp->getFrameRate() << " (frame/sec)" << endl;
      multiApp->stop();
      exit(0);
   }

   // turn on and off the debug mode with right mouse
   if( key == 'd' )
   {
      cout << endl << multiApp->getFrameRate() << " (frame/sec)" << endl ;
      if( multiApp->isThresholdMode() )
         multiApp->disableModeThreshold();
      else
         multiApp->enableModeThreshold();

      multiApp->resetFrameRate();
   }

   //change the threshold value when 't' key pressed
   if( key == 't' )
   {
      int thresh;
      cout << "Enter new threshold value (default = 100): ";
      cin >> thresh;
      multiApp->setThreshold(thresh);
      cout << endl;
   }

   // Increase and decrease threshhold in five units
   static int thresh = 100;
   if( key == '+' ) {
      thresh = (thresh <= 250) ? (thresh += 5) : 255;
      multiApp->setThreshold(thresh);
      cout << "Thresh: " << multiApp->getThreshHold() << endl;
   }
   if( key == '-' ) {
      thresh = (thresh >= 5) ? (thresh -= 5) : 0;
      multiApp->setThreshold(thresh);
      cout << "Thresh: " << multiApp->getThreshHold() << endl;
   }
}
