/**
   Example:     Collision
   Version      1.0
   Author:      Igor F. Couto, Douglas C. B. Oliveira, Rodrigo L. S. Silva
   Last Update: 15/10/2013
   Description: When we need the rendered objects have some kind of relationship,
                creating a callback for each marker is not a good option.
                This example presents another way of rendering on multiple markers (single type) simulating a "collision" between two spheres that are rendered on two different markers.

   Example Informations:
      Renderization:    Two Spheres

      Keyboard Events:
         ESC   Exit Application
         t     Set new threshold value
         +     Increases threshold value in 5 units (limit 255)
         -     Decreases threshold value in 5 units (limit 0)

      Mouse Events:     none
*/

#include <avrApplication.h>
#include <cmath>

using namespace std;

// radius = (width_marker / 2) + value
const double RADIUS_1 = 50.0;
const double RADIUS_2 = 50.0;

avrApplication    *collisionApp;

// Note: Display callback has one parameter. When passing a display function with an integer parameter, the library
//    returns through this parameter, the marker identifier that the called belongs.
static void    draw(int id);

static void    draw_object(bool collide, double id);        // Renderization function
static void    keyEvent( unsigned char key, int x, int y);  // Keyboard events callback

// auxiliary function that checks if had collision or not
static bool    checkCollisions( avrPattern object0, avrPattern object1);

int main(int argc, char **argv)
{
   glutInit(&argc, argv);
   collisionApp = new avrApplication;
   collisionApp->setProjectInfo("Collision", "Douglas C. B. Oliveira, Igor F. Couto e Rodrigo L. S. Silva",
                          "This test simulates a collision between two spheres which changes\ncolors when touches each other",
                          "DCC and ICE");
   #ifdef _WIN32
      collisionApp->setCameraFiles((char*) "Data/WDM_camera_AVRLib.xml", (char *) "Data/camera_para.dat");
   #else
      // -dev=/dev/video1 -palette=RGB -width=960 -height=544
      collisionApp->setCameraFiles("-dev=/dev/video0 -palette=RGB -width=640 -height=480", (char *) "Data/camera_para.dat");
   #endif

   // Note: The same display callback is passed for the two markers
   collisionApp->addPattern((char*)"Data/dcc.patt", 60, NULL, draw);
   collisionApp->addPattern((char*)"Data/ice.patt", 60, NULL, draw);

   collisionApp->setKeyCallback(keyEvent);
   collisionApp->setThreshold(100);

   collisionApp->printProjectInfo();

   collisionApp->start();

   return 0;
}

// It checks for collisions between the objects of the markers
static bool checkCollisions( avrPattern object0, avrPattern object1)
{
   float x1,y1,z1;
   float x2,y2,z2;
   float dist;

   // access the object 0 position
   x1 = (float)object0.trans().X();
   y1 = (float)object0.trans().Y();
   z1 = (float)object0.trans().Z();
   // access the object 1 position
   x2 = (float)object1.trans().X();
   y2 = (float)object1.trans().Y();
   z2 = (float)object1.trans().Z();

   // Calculates the distance between the objects
   dist =   sqrt( (x1-x2)*(x1-x2)  +  (y1-y2)*(y1-y2)  +  (z1-z2)*(z1-z2) );

   // checks collision
   if(dist <= (RADIUS_1 + RADIUS_2)) // 2 * radius
      return true;
   else
      return false;
}

// Display callback, but not renders indeed. It prepares the renderization that will occur in draw_object
static void draw(int id)
{
   // flag for collision state
   bool    collide = false;

   // checks for collisions between objects marker 0 and 1
   if(collisionApp->getPattern(0).visible() && collisionApp->getPattern(1).visible())
      collide = checkCollisions(collisionApp->getPattern(0),collisionApp->getPattern(1));

   glEnable(GL_LIGHTING);

   // checks what marker this called belongs and calls draw_object for this marker
   if(id == 0)   draw_object(collide, RADIUS_1 );
   else          draw_object(collide, RADIUS_2 );

   glDisable( GL_LIGHTING );
}

static void  draw_object(bool collide, double radius)
{
   GLfloat   mat_ambient[]				= {0.0, 0.0, 1.0, 1.0};
   GLfloat   mat_ambient_collide[]  = {1.0, 0.0, 0.0, 1.0};
   GLfloat   mat_flash[]				= {0.0, 0.0, 1.0, 1.0};
   GLfloat   mat_flash_collide[]    = {1.0, 0.0, 0.0, 1.0};
   GLfloat   mat_flash_shiny[] = {50.0};
   GLfloat   light_position[]  = {100.0,-200.0,200.0,0.0};
   GLfloat   ambi[]            = {0.1, 0.1, 0.1, 0.1};
   GLfloat   lightZeroColor[]  = {0.9, 0.9, 0.9, 0.1};

   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);
   glLightfv(GL_LIGHT0, GL_POSITION, light_position);
   glLightfv(GL_LIGHT0, GL_AMBIENT, ambi);
   glLightfv(GL_LIGHT0, GL_DIFFUSE, lightZeroColor);

   glMaterialfv(GL_FRONT, GL_SHININESS, mat_flash_shiny);

   // if the collision has been confirmed, their appearance changes
   if(collide){
      glMaterialfv(GL_FRONT, GL_SPECULAR, mat_flash_collide);
      glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient_collide);
      glTranslatef( 0.0, 0.0, 30.0 );
      glutSolidSphere(radius, 24, 24 );
   }
   else {
      glMaterialfv(GL_FRONT, GL_SPECULAR, mat_flash);
      glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
      glTranslatef( 0.0, 0.0, 30.0 );
      glutSolidSphere(radius, 24, 24 );
   }
}

static void   keyEvent( unsigned char key, int x, int y)
{
   // Quit if the ESC key is pressed
   if( key == 0x1b ) {
      cout << endl << collisionApp->getFrameRate() << " (frame/sec)" << endl;
      collisionApp->stop();
      exit(0);
   }

   // Change the threshold value when 't' key pressed
   if( key == 't' )
   {
      int thresh;
      cout << "Enter new threshold value (default = 100): ";
      cin >> thresh;
      collisionApp->setThreshold(thresh);
      cout << endl;
   }

   // Increase and decrease threshhold value in five units
   static int thresh = 100;
   if( key == '+' ) {
      thresh = (thresh <= 250) ? (thresh += 5) : 255;
      collisionApp->setThreshold(thresh);
      cout << "Thresh: " << collisionApp->getThreshHold() << endl;
   }
   if( key == '-' ) {
      thresh = (thresh >= 5) ? (thresh -= 5) : 0;
      collisionApp->setThreshold(thresh);
      cout << "Thresh: " << collisionApp->getThreshHold() << endl;
   }
}
