/**
   Example:     MultiMarkerAutoCreation
   Version      1.0
   Author:      Douglas C. B. Oliveira, Rodrigo L. S. Silva
   Last Update: 15/10/2013
   Description: This application creates the config file of a new pattern of multi markers. Calculates automaticaly
               the relations between the markers registered in the input file. The config file generated can be tested in
               the example MultiMarker. The idea is create the config file for markers in random positions, dispensing
               the manual calculus of the relations.

   Keyboard Events:
      ESC   Exit Application
      t     Set a new threshold value
      +     Increases threshold value in 5 units (limit 255)
      -     Decreases threshold value in 5 units (limit 0)

   Mouse Events:
      Left:    none
      Middle:  none
      Right:   Write the config file
*/

#include <avrApplication.h>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include "glcText.h"

using namespace std;

static int        numberFiles = 0;
avrApplication*   newApp;
avrMatrix3x4*     relations;

// Name for the output file
string         nameOutFile = "AutoMulti";
// Path of the marker files registered in the input file
string         pathMarkerFile = "data/multi/";

// draw functions
static void    display(int);
static void    draw_envelopment(int idHolder);
static void    draw_axis(const avrMatrix3x4& trans);
// events
static void    keyEvent( unsigned char key, int x, int y);
static void    mouseEvent(int button, int state, int x, int y);
// utils
static void    printDescription();
static void    writeConfigFile();
static string  numberToString(double number, const string& complement, int precis = 5);

int main(int argc, char **argv)
{
   glutInit(&argc, argv);
   printDescription();

   newApp = new avrApplication();

   #ifdef _WIN32
      newApp->setCameraFiles("data/WDM_camera_AVRLib.xml", "data/camera_para.dat");
   #else
      // -dev=/dev/video1 -palette=RGB -width=960 -height=544
      newApp->setCameraFiles("-dev=/dev/video0 -palette=RGB -width=640 -height=480", "data/camera_para.dat");
   #endif
   newApp->addPatterns("data/multi/markerSM.dat", MODE_NEAR_CAMERA, display);

   newApp->setKeyCallback(keyEvent);
   newApp->setMouseCallback(mouseEvent);
   newApp->setThreshold(100);

   newApp->start();

   return 0;
}

static void display(int idHolder){
   avrMatrix3x4 trans = newApp->getPattern(idHolder).trans();

   draw_axis(trans);
   draw_envelopment(idHolder);

   glLineWidth(1.0f);
   glDisable( GL_DEPTH_TEST );

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();

   glOrtho(0.0, 100.0, 0.0, 100.0, -1.0, 1.0);
   // draw polygon
   //glTranslatef(-1.0, -0.93, 0.0);
   glColor3f(0.0, 0.0, 0.7);
   glBegin(GL_POLYGON);
      glVertex2f(0.0, 0.0);
      glVertex2f(0.0, 8.0);
      glVertex2f(20.0, 8.0);
      glVertex2f(20.0, 0.0);
   glEnd();

   glcText text;
   text.setAll((char*)numberToString(numberFiles, " Files captured", 0).c_str(), 0, 3, 3, 0, 1.0, 1.0, 1.0);
   text.render();
}

// draw the user object
static void  draw_axis(const avrMatrix3x4& trans)
{
   double   gl_para[16];

   // transformation matrix
   trans.getMatrixGLFormat(gl_para);
   glMatrixMode(GL_MODELVIEW);
   glLoadMatrixd( gl_para );

   // draw the axis
   glLineWidth(2.0f);
   glColor3d(0.0, 0.0, 1.0);
   glBegin(GL_LINES);
      glVertex2d(0.0, 240.0);
      glVertex2d(0.0, -240.0);
      glVertex2d(-320.0, 0.0);
      glVertex2d(320.0, 0.0);
   glEnd();
}

static void draw_envelopment(int idHolder){
   int            numPatts   = newApp->numberPatts();
   double         minX, minY, minZ, maxX, maxY, maxZ;
   avrMatrix3x4   invHolder;

   try{
      invHolder = newApp->getPattern(idHolder).trans().inverse();
   }catch(domain_error&){

   }

   if(!relations) relations = new avrMatrix3x4[numPatts];
   relations[idHolder].setIdentityMatrix();

   double width = newApp->getPattern(idHolder).width() / 2.0;
   minX = minY = 0.0 - width;
   maxX = maxY = 0.0 + width;
   minZ = maxZ = 0.0;

   for(int i = 0; i < numPatts; i++){
      if(i == idHolder) continue;
      relations[i] = invHolder * newApp->getPattern(i).trans();

      width = newApp->getPattern(i).width() / 2.0;
      (relations[i].X() - width < minX) ? minX = relations[i].X() - width : minX = minX;
      (relations[i].X() + width > maxX) ? maxX = relations[i].X() + width : maxX = maxX;
      (relations[i].Y() - width < minY) ? minY = relations[i].Y() - width : minY = minY;
      (relations[i].Y() + width > maxY) ? maxY = relations[i].Y() + width : maxY = maxY;
      (relations[i].Z() < minZ)         ? minZ = relations[i].Z()         : minZ = minZ;
      (relations[i].Z() > maxZ)         ? maxZ = relations[i].Z()         : maxZ = maxZ;
   }

   glLineWidth(3.0f);
   glColor3d(0.0, 1.0, 0.0);
   glBegin(GL_LINE_LOOP);
      glVertex3d(maxX, maxY, maxZ);
      glVertex3d(minX, maxY, maxZ);
      glVertex3d(minX, minY, maxZ);
      glVertex3d(maxX, minY, maxZ);
   glEnd();
   glColor3d(0.0, 0.5, 0.0);
   glBegin(GL_LINE_LOOP);
      glVertex3d(maxX, maxY, minZ);
      glVertex3d(minX, maxY, minZ);
      glVertex3d(minX, minY, minZ);
      glVertex3d(maxX, minY, minZ);
   glEnd();
   glColor3d(0.0, 0.75, 0.0);
   glBegin(GL_LINES);
      glVertex3d(maxX, maxY, minZ);
      glVertex3d(maxX, maxY, maxZ);
      glVertex3d(minX, maxY, minZ);
      glVertex3d(minX, maxY, maxZ);
      glVertex3d(minX, minY, minZ);
      glVertex3d(minX, minY, maxZ);
      glVertex3d(maxX, minY, minZ);
      glVertex3d(maxX, minY, maxZ);
   glEnd();
}

static void   keyEvent( unsigned char key, int x, int y)
{
    // Quit if the ESC key is pressed
   if( key == 0x1b ) {
      cout << endl << newApp->getFrameRate() << " (frame/sec)" << endl;
      newApp->stop();
      exit(0);
   }

   // Change the threshold value when 't' key pressed
   if( key == 't' )
   {
      int thresh;
      cout << "Enter new threshold value (default = 100): ";
      cin >> thresh;
      newApp->setThreshold(thresh);
      cout << endl;
   }

   // Increase and decrease threshhold in five units
   static int thresh = 100;
   if( key == '+' ) {
      thresh = (thresh <= 250) ? (thresh += 5) : 255;
      newApp->setThreshold(thresh);
      cout << "Thresh: " << newApp->getThreshHold() << endl;
   }
   if( key == '-' ) {
      thresh = (thresh >= 5) ? (thresh -= 5) : 0;
      newApp->setThreshold(thresh);
      cout << "Thresh: " << newApp->getThreshHold() << endl;
   }
}

static void mouseEvent(int button, int state, int x, int y)
{
   if( state == GLUT_DOWN && button == GLUT_RIGHT_BUTTON){
      numberFiles++;
      writeConfigFile();
   }
}

static void writeConfigFile(){
   //matrix3x4 *relations = ((avrSystemAutoMulti*)newApp->getSystem())->getTransf();
   string config;

   config = "#the number of patterns to be recognized\n";
   config += numberToString(newApp->numberPatts(), "\n\n", 0);

   for(int i = 0; i < newApp->numberPatts(); i++){
      config += "#pattern " + numberToString(i, "\n", 0);
      config += pathMarkerFile + newApp->getPattern(i).name() + ".patt\n";
      config += numberToString((double)newApp->getPattern(i).width(), "\n", 1);

      double * center = newApp->getPattern(i).center();
      config += numberToString((double)center[0], " ", 1) + numberToString((double)center[1], "\n", 1);

      config += numberToString(relations[i].access(0, 0), " \t") + numberToString(relations[i].access(0, 1), " \t") +
                numberToString(relations[i].access(0, 2), " \t") + numberToString(relations[i].access(0, 3), "\n");
      config += numberToString(relations[i].access(1, 0), " \t") + numberToString(relations[i].access(1, 1), " \t") +
                numberToString(relations[i].access(1, 2), " \t") + numberToString(relations[i].access(1, 3), "\n");
      config += numberToString(relations[i].access(2, 0), " \t") + numberToString(relations[i].access(2, 1), " \t") +
                numberToString(relations[i].access(2, 2), " \t") + numberToString(relations[i].access(2, 3), "\n\n");
   }

   string path = "data/" + nameOutFile + numberToString(numberFiles, ".dat", 0);
   ofstream file(path.c_str());
   file.write(config.c_str(), config.length());
   file.close();
}

static string numberToString(double number, const string& complement, int precis){
   int truncated = (int)number;
   double difference = number - truncated;
   if(difference > 0.5) truncated++;

   string result;
   ostringstream convert;

   if(precis)
      convert.flags(std::ios::showpoint | std::ios::fixed);
   convert.precision(precis);
   convert << (double)truncated;
   result = convert.str() + complement;

   return result;
}

static void printDescription(){
   cout <<  "-------------------------------------------------------------------------" << endl <<
            "Multi Marker Auto Creation is a application that creates one new pattern\nof multi markers, generating " <<
            "automatically the your config file.\n\nFor this, edit the file: 'data/model.dat' wicth the informations " <<
            "of the\nmarkers that will be part of the new pattern.\n" <<
            "-------------------------------------------------------------------------" << endl << endl;

   cout << "Press to continue..."; getchar();

   #ifdef _WIN32
      system("cls");
   #else
      system("clear");
   #endif
}
