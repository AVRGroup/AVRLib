/**
   Example:     TextureObject
   Version      1.0
   Author:      Douglas C. B. Oliveira, Rodrigo L. S. Silva
   Last Update: 15/10/2013
   Description: This exemple presents the steps to include textures in objects easily through the glTexture class.

   Example Informations:
      Renderization:    Two virtual textured teapots

      Keyboard Events:
         ESC   Exit Application
         t     Set new threshold value
         +     Increases threshold value in 5 units (limit 255)
         -     Decreases threshold value in 5 units (limit 0)

      Mouse Events:     none
*/

#include <avrApplication.h>
#include <iostream>
#include <cstdlib>
#include "glTexture.h"

using namespace std;

const float SCALE = 40.0f;

glTexture      *texture1, *texture2;
avrApplication *textureApp;

static void draw1();
static void draw2();
static void keyEvent(unsigned char key, int x, int y);

int main(int argc, char **argv)
{
   glutInit(&argc, argv);
   textureApp = new avrApplication();

   textureApp->setProjectInfo("TexturedObject", "Douglas C. B. Oliveira e Rodrigo L. S. Silva",
                          "This test shows simple objects with texture over fiducial markers", "DCC and ICE");
   #ifdef _WIN32
      textureApp->setCameraFiles("data/WDM_camera_AVRLib.xml", "data/camera_para.dat");
   #else
      // -dev=/dev/video1 -palette=RGB -width=960 -height=544
      textureApp->setCameraFiles("-dev=/dev/video0 -palette=RGB -width=640 -height=480", "data/camera_para.dat");
   #endif
   textureApp->addPattern("data/dcc.patt", 60.0, NULL, draw1);
   textureApp->addPattern("data/ice.patt", 60.0, NULL, draw2);

   textureApp->setKeyCallback(keyEvent);
   textureApp->setThreshold(100);

   // Defines the texture of object one
   texture1 = new glTexture();
   texture1->SetNumberOfTextures(1);
   texture1->CreateTexture("data/images/metal_rust.jpg", 1);
   // Defines the texture in object two
   texture2 = new glTexture();
   texture2->SetNumberOfTextures(1);
   texture2->CreateTexture("data/images/alluminium.jpg", 0);

   textureApp->printProjectInfo();

   textureApp->start();

   return 0;
}

static void draw1()
{
   GLfloat   mat_ambient[]     = {1.0, 0.0, 0.0, 1.0};
   GLfloat   mat_flash[]       = {1.0, 0.0, 0.0, 1.0};
   GLfloat   mat_flash_shiny[] = {50.0};
   GLfloat   light_position[]  = {100.0,-200.0,200.0,0.0};
   GLfloat   ambi[]            = {0.1, 0.1, 0.1, 0.1};
   GLfloat   lightZeroColor[]  = {0.9, 0.9, 0.9, 0.1};

   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);
   glLightfv(GL_LIGHT0, GL_POSITION, light_position);
   glLightfv(GL_LIGHT0, GL_AMBIENT, ambi);
   glLightfv(GL_LIGHT0, GL_DIFFUSE, lightZeroColor);
   glMaterialfv(GL_FRONT, GL_SPECULAR, mat_flash);
   glMaterialfv(GL_FRONT, GL_SHININESS, mat_flash_shiny);
   glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
   glMatrixMode(GL_MODELVIEW);

   glTranslatef( 0.0, 0.0, SCALE/1.5 );
   glRotatef(90, 1, 0, 0);

   texture1->SelectTexture(1);   // Enable and Bind the texture to object
   glutSolidTeapot(SCALE);        // Draw
   texture1->DisableTexture();   // Disable texture

   glDisable( GL_LIGHTING );
}

static void draw2()
{
   GLfloat   mat_ambient[]     = {1.0, 0.0, 0.0, 1.0};
   GLfloat   mat_flash[]       = {1.0, 0.0, 0.0, 1.0};
   GLfloat   mat_flash_shiny[] = {50.0};
   GLfloat   light_position[]  = {100.0,-200.0,200.0,0.0};
   GLfloat   ambi[]            = {0.1, 0.1, 0.1, 0.1};
   GLfloat   lightZeroColor[]  = {0.9, 0.9, 0.9, 0.1};

   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);
   glLightfv(GL_LIGHT0, GL_POSITION, light_position);
   glLightfv(GL_LIGHT0, GL_AMBIENT, ambi);
   glLightfv(GL_LIGHT0, GL_DIFFUSE, lightZeroColor);
   glMaterialfv(GL_FRONT, GL_SPECULAR, mat_flash);
   glMaterialfv(GL_FRONT, GL_SHININESS, mat_flash_shiny);
   glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
   glMatrixMode(GL_MODELVIEW);

   glTranslatef( 0.0, 0.0, SCALE/1.5 );
   glRotatef(90, 1, 0, 0);

   texture2->SelectTexture(0);   // Enable and Bind the texture to object
   glutSolidTeapot(SCALE);        // Draw
   texture2->DisableTexture();   // Disable texture

   glDisable( GL_LIGHTING );
}

static void keyEvent(unsigned char key, int x, int y)
{
    // Quit if the ESC key is pressed
   if( key == 0x1b ) {
      cout << endl << textureApp->getFrameRate() << " (frame/sec)" << endl;
      textureApp->stop();
      exit(0);
   }

   // Change the threshold value when 't' key pressed
   if( key == 't' )
   {
      int thresh;
      cout << "Enter new threshold value (default = 100): ";
      cin >> thresh;
      textureApp->setThreshold(thresh);
      cout << endl;
   }

   // Increase and decrease threshhold in five units
   static int thresh = 100;
   if( key == '+' ) {
      thresh = (thresh <= 250) ? (thresh += 5) : 255;
      textureApp->setThreshold(thresh);
      cout << "Thresh: " << textureApp->getThreshHold() << endl;
   }
   if( key == '-' ) {
      thresh = (thresh >= 5) ? (thresh -= 5) : 0;
      textureApp->setThreshold(thresh);
      cout << "Thresh: " << textureApp->getThreshHold() << endl;
   }
}
