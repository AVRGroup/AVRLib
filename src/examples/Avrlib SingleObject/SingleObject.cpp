/**
   Example:     SingleObject
   Version      1.0
   Author:      Douglas C. B. Oliveira, Rodrigo L. S. Silva
   Last Update: 15/10/2013
   Description: The objective of this example is to present the first steps of AR application development using the AVRLib.
               Each called method of the library will be explained, presenting its sequence and requirements.
               In other examples, only methods and routines not presented here, will be described.

   Example Informations:
      Renderization:    Virtual Cube

      Keyboard Events:
         ESC   Exit Application
         t     Set a new threshold value
         +     Increases threshold value in 5 units (limit 255)
         -     Decreases threshold value in 5 units (limit 0)

      Mouse Events:     none
*/

// Header avrApplication must be included. All dependences are treated in this header
#include <avrApplication.h>
#include <iostream>
#include <cstdlib>

using namespace std;

// avrApplication class instance
avrApplication *singleObjApp;

// Renderization and Keyboard Events callbacks
static void    draw();
static void    keyEvent(unsigned char key, int x, int y);

int main(int argc, char **argv)
{
   glutInit(&argc, argv);
   // First: avrApplication class is instantiated. Is very important, any and all access to library functionalities is made by this instance
   singleObjApp = new avrApplication("AVRLib Examples - SingleObject");

   // Second: library configuration. Now you need to tell which files the camera and what the markers will be used.
   // Method setProjectInfo is optional. This method define the additional informations to be shown in terminal.
   singleObjApp->setProjectInfo("SingleObject", "Douglas C. B. Oliveira e Rodrigo L. S. Silva", "This test shows a virtual object over a fiducial marker", "AVR");

   #ifdef _WIN32
      singleObjApp->setCameraFiles("data/WDM_camera_AVRLib.xml", "data/camera_para.dat");
   #else
      // -dev=/dev/video1 -palette=RGB -width=960 -height=544
      singleObjApp->setCameraFiles("-dev=/dev/video0 -palette=RGB -width=640 -height=480", "data/camera_para.dat");
   #endif

   // Sets the configuration files and camera intrinsic parameters
   // Sets the markers and sets the display callback function.
   singleObjApp->addPattern("data/avr.patt", 50.0, NULL, draw);

   // Third: Optionals. Sets other callbacks (reshape, visibility and events) and threshold initial value.
   singleObjApp->setKeyCallback(keyEvent);
   singleObjApp->setThreshold(100);
   // Optional too. Shows the additional informations in terminal. Can be called in other place of application.
   singleObjApp->printProjectInfo();

   // Fourth: Start application. Mandatorily, last command of main
   singleObjApp->start();

   return 0;
}

// Display Callback. The graphical API used is the OpenGL
static void draw()
{
   GLfloat   mat_ambient[]     = {0.0, 0.0, 1.0, 1.0};
   GLfloat   mat_flash[]       = {0.0, 0.0, 1.0, 1.0};
   GLfloat   mat_flash_shiny[] = {50.0};
   GLfloat   light_position[]  = {100.0,-200.0,200.0,0.0};
   GLfloat   ambi[]            = {0.1, 0.1, 0.1, 0.1};
   GLfloat   lightZeroColor[]  = {0.9, 0.9, 0.9, 0.1};

   // Enable lighting for the object
   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);
   // Sets lighting parameters
   glLightfv(GL_LIGHT0, GL_POSITION, light_position);
   glLightfv(GL_LIGHT0, GL_AMBIENT, ambi);
   glLightfv(GL_LIGHT0, GL_DIFFUSE, lightZeroColor);
   // Sets materials
   glMaterialfv(GL_FRONT, GL_SPECULAR, mat_flash);
   glMaterialfv(GL_FRONT, GL_SHININESS, mat_flash_shiny);
   glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);

   glTranslatef( 0.0, 0.0, 25.0 );  // Define translation
   glutSolidCube(50.0);             // Draws cube

   // Disable lighting
   glDisable( GL_LIGHTING );
}

static void keyEvent(unsigned char key, int x, int y){
    // Quit if the ESC key is pressed
   if( key == 0x1b ) {
      cout << endl << singleObjApp->getFrameRate() << " (frame/sec)" << endl;
      // Mandatorily called at the end of application for stop video capture
      singleObjApp->stop();
      exit(0);
   }

   // Change the threshold value when 't' key pressed
   if( key == 't' )
   {
      int thresh;
      cout << "Enter new threshold value (default = 100): ";
      cin >> thresh;
      singleObjApp->setThreshold(thresh);
      cout << endl;
   }

   // Increase and decrease threshhold in five units
   static int thresh = 100;
   if( key == '+' ) {
      thresh = (thresh <= 250) ? (thresh += 5) : 255;
      singleObjApp->setThreshold(thresh);
      cout << "Thresh: " << singleObjApp->getThreshHold() << endl;
   }
   if( key == '-' ) {
      thresh = (thresh >= 5) ? (thresh -= 5) : 0;
      singleObjApp->setThreshold(thresh);
      cout << "Thresh: " << singleObjApp->getThreshHold() << endl;
   }
}
