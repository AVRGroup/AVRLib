/**
   Example:     MarkerCameraDistance
   Version      1.0
   Author:      Douglas C. B. Oliveira, Rodrigo L. S. Silva
   Last Update: 15/10/2013
   Description: This example performs the calculation of the distance between the marker and the camera.
                This information is rendered on the video without using the glcText class but instead, directly using the OpenGL routines.

   Example Informations:
      Renderization:    Virtual Cube and Distance of marker in the camera relation

      Keyboard Events:
         ESC   Exit Application
         t     Set new threshold value
         +     Increases threshold value in 5 units (limit 255)
         -     Decreases threshold value in 5 units (limit 0)

      Mouse Events:     none
*/

#include <avrApplication.h>
#include <sstream>
#include <string.h>
#include <cstdio>
#include <cmath>

const double MARKER_SIZE = 60.0;

using namespace std;

avrApplication  *camDistApp;

static void   draw();
static void   showString( char *string );
static void   keyEvent( unsigned char key, int x, int y);
static string numberToString(double number, string complement = "");

int main(int argc, char **argv)
{
   glutInit(&argc, argv);
   camDistApp = new avrApplication();

   camDistApp->setProjectInfo("MarkerCameraDistance", "Igor F. Couto, Douglas C. B. Oliveira e Rodrigo L. S. Silva",
                              "This test shows the distance between the camera and the marker\n\nOBS: For correct messure, the expected size of the marker is " +
                              numberToString(MARKER_SIZE/10, "cm"), "AVR");

   #ifdef _WIN32
      camDistApp->setCameraFiles((char*) "Data/WDM_camera_AVRLib.xml", (char *) "Data/camera_para.dat");
   #else
      // -dev=/dev/video1 -palette=RGB -width=960 -height=544
      camDistApp->setCameraFiles("-dev=/dev/video0 -palette=RGB -width=640 -height=480", (char *) "Data/camera_para.dat");
   #endif
   camDistApp->addPattern((char*) "Data/avr.patt", MARKER_SIZE, NULL, draw);

   camDistApp->setThreshold(100);
   camDistApp->setKeyCallback(keyEvent);

   camDistApp->printProjectInfo();

   camDistApp->start();

   return 0;
}

static void draw()
{
   float     Xpos, Ypos, Zpos;
   float     dist;
   char      str[256];

   avrMatrix3x4 trans = camDistApp->getPattern().trans();

   Xpos = trans.X();
   Ypos = trans.Y();
   Zpos = trans.Z();

   GLfloat   mat_ambient[]     = {0.0, 1.0, 0.0, 1.0};
   GLfloat   mat_flash[]       = {0.0, 1.0, 0.1, 1.0};
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

   // draws object
   glRotatef( 90, 1.0, 0.0, 0.0 );
   glTranslatef( 0.0, (MARKER_SIZE * 0.9)/2.0, 0.0 );
   glutSolidCube(MARKER_SIZE * 0.9);

   glDisable( GL_LIGHTING );

   // calculates the distance of marker in the camera relation
   dist = sqrt( pow(Xpos,2) + pow(Ypos,2) + pow(Zpos,2) ); //mm
   dist = dist / 10; //cm
   // formats the output string
   sprintf(str, "Coordinates: (%3.1f, %3.1f, %3.1f) | Distance: %3.1fcm", Xpos, Ypos, Zpos, dist);
   // shows the string
   showString(str);
}

static void showString( char * str )
{
   int i;
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();

   // draws white rectangle
   glTranslatef(-0.80, -0.80, 0.0);
   glColor3f(1.0, 1.0, 1.0);
   glBegin(GL_POLYGON);
      glVertex2f(-1.0, 0.10);
      glVertex2f(2.0, 0.10);
      glVertex2f(2.0, -0.15);
      glVertex2f(-1.0, -0.15);
   glEnd();

   // draw text over white rectangle
   glColor3f(0.0, 0.6, 0.0);
   glRasterPos2f(-0.1, -0.05);
   for (i = 0; i < (int)strlen(str); i++)
   {
      if(str[i] != '\n' )
         glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, str[i]);
      else
      {
         glTranslatef(0.0, -0.09, 0.0);
         glRasterPos2i(0.0, 0.0);
      }
   }
}

static void   keyEvent( unsigned char key, int x, int y)
{
    // Quit if the ESC key is pressed
   if( key == 0x1b ) {
      cout << endl << camDistApp->getFrameRate() << " (frame/sec)" << endl;
      camDistApp->stop();
      exit(0);
   }

   // Change the threshold value when 't' key pressed
   if( key == 't' )
   {
      int thresh;
      cout << "Enter new threshold value (default = 100): ";
      cin >> thresh;
      camDistApp->setThreshold(thresh);
      cout << endl;
   }

   // Increase and decrease threshhold in five units
   static int thresh = 100;
   if( key == '+' ) {
      thresh = (thresh <= 250) ? (thresh += 5) : 255;
      camDistApp->setThreshold(thresh);
      cout << "Thresh: " << camDistApp->getThreshHold() << endl;
   }
   if( key == '-' ) {
      thresh = (thresh >= 5) ? (thresh -= 5) : 0;
      camDistApp->setThreshold(thresh);
      cout << "Thresh: " << camDistApp->getThreshHold() << endl;
   }
}

// Auxiliary function. Convert one number to string
static string numberToString(double number, string complement){
    string result;
    ostringstream convert;
    convert << number;
    result = convert.str() + complement;

    return result;
}
