/**
   Example:     MarkersDistance
   Version      1.0
   Author:      Douglas C. B. Oliveira, Rodrigo L. S. Silva
   Last Update: 15/10/2013
   Description: This example shows a simple technique to get the distance
                between markers and presents the glcText class, that makes the text rendering an easy task

   Example Informations:
      Renderization:    Lines and distance value between markers

      Keyboard Events:
         ESC   Exit Application
         t     Set new threshold value
         +     Increases threshold value in 5 units (limit 255)
         -     Decreases threshold value in 5 units (limit 0)
         ↑     Increases shine of line
         ↓     Decreases shine of line

      Mouse Events:     none
*/

#include <avrApplication.h>
#include <sstream>
#include <cmath>

#include "glcText.h"

using namespace std;

avrApplication *markersDistApp;
static double  shine = 0.5;

static void    draw(int idT);
static void    keyEvent( unsigned char key, int x, int y);
static void    specialKeys(int key, int x, int y);
static string  numberToString(double number, string complement, int precis = 2);

int main(int argc, char *argv[])
{
   glutInit(&argc, argv);
   markersDistApp = new avrApplication();
   markersDistApp->setProjectInfo("MarkersDistance", "Douglas C. B. Oliveira e Rodrigo L. S. Silva",
                          "This test shows the distance between markers of the scene",
                          "AVR, DCC and ICE");
   #ifdef _WIN32
      markersDistApp->setCameraFiles((char*) "Data/WDM_camera_AVRLib.xml", (char *) "Data/camera_para.dat");
   #else
      // -dev=/dev/video1 -palette=RGB -width=960 -height=544
      markersDistApp->setCameraFiles("-dev=/dev/video0 -palette=RGB -width=640 -height=480", (char *) "Data/camera_para.dat");
   #endif
   markersDistApp->addPattern((char*) "Data/avr.patt",60.0, NULL, draw);
   markersDistApp->addPattern((char*) "Data/dcc.patt", 60.0, NULL, draw);
   markersDistApp->addPattern((char*) "Data/ice.patt", 60.0, NULL, draw);

   markersDistApp->setKeyCallback(keyEvent);
   // Special keyboard callback(non-printable keys)
   markersDistApp->setSpecialCallback(specialKeys);
   markersDistApp->setThreshold(100);

   markersDistApp->printProjectInfo();

   markersDistApp->start();

   return 0;
}

static void draw(int idT){
   int idD;
   // Obs: The line is rendered from 0 to 1 and from 1 to 2 and from 2 to 0
   (idT == 2) ? idD = 0 : idD = idT + 1;

   avrMatrix3x4   trans1   = markersDistApp->getPattern(idT).trans();   // transformation matrix of current marker
   avrMatrix3x4   trans2   = markersDistApp->getPattern(idD).trans();   // transformation matrix of destination marker
   bool           visibleD = markersDistApp->getPattern(idD).visible(); // visibility of destination marker

   // Current marker is visible(because the process came here by it). Checks visibility of destination marker
   if(!visibleD)
      return;

   avrMatrix3x4   trans1to2;
   double   dist;

   // calculates the relation between the markers
   trans1to2  = trans1.getRelationWith(trans2);

   // draws the line
   glLineWidth(4.0f);
   glColor3d(shine, 0.0, 0.0);
   glBegin(GL_LINES);
      glVertex3d(0.0, 0.0, 0.0);
      glVertex3d(trans1to2.X(), trans1to2.Y(), trans1to2.Z());
   glEnd();

   // calculates the distance between the markers (in millimeters)
   dist = sqrt( pow(trans1.X() - trans2.X(), 2) + pow(trans1.Y() - trans2.Y(), 2) + pow(trans1.Z() - trans2.Z(), 2));
   dist = dist / 10.0; // convert in centimeters

   // defines the position of text in scene
   float xDest = trans1to2.X();
   float yDest = trans1to2.Y();
   float zDest = trans1to2.Z();
   float x = (xDest / 2) + 5.0;
   float y = (yDest / 2) + 5.0;
   float z = (zDest / 2) + 5.0;

   // draws the text
   // Step One: Create the glcText object
   glcText text;
   // Step Two: Sets the parameters (string, text font, position and color)
   text.setAll((char*)numberToString(dist, "cm").c_str(), 4, x, y, z, shine, shine, shine);
   // Step Three: Draws the text
   text.render();
}

static void   keyEvent( unsigned char key, int x, int y)
{
    // Quit if the ESC key is pressed
   if( key == 0x1b ){
      cout << endl << markersDistApp->getFrameRate() << " (frame/sec)" << endl;
      markersDistApp->stop();
      exit(0);
   }

   // Change the threshold value when 't' key pressed
   if( key == 't' )
   {
      int thresh;
      cout << "Enter new threshold value (default = 100): ";
      cin >> thresh;
      markersDistApp->setThreshold(thresh);
      cout << endl;
   }

   // Increase and decrease threshhold value in five units
   static int thresh = 100;
   if( key == '+' ) {
      thresh = (thresh <= 250) ? (thresh += 5) : 255;
      markersDistApp->setThreshold(thresh);
      cout << "Thresh: " << markersDistApp->getThreshHold() << endl;
   }
   if( key == '-' ) {
      thresh = (thresh >= 5) ? (thresh -= 5) : 0;
      markersDistApp->setThreshold(thresh);
      cout << "Thresh: " << markersDistApp->getThreshHold() << endl;
   }
}

static void specialKeys(int key, int x, int y){
   // Increses or Decreases shine of line
   if(key == GLUT_KEY_UP)
      (shine < 0.8) ? shine += 0.1 : shine = shine;
   else if(key == GLUT_KEY_DOWN)
      (shine > 0.2) ? shine -= 0.1 : shine = shine;
}

// Auxiliary function. Convert one number to string
static string numberToString(double number, string complement, int precis){
    string result;
    ostringstream convert;

    if(precis)
        convert.flags(std::ios::showpoint | std::ios::fixed);
    convert.precision(precis);
    convert << number;
    result = convert.str() + complement;

    return result;
}
