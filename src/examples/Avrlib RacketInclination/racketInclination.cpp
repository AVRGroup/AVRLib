/**
   Example:     Racket Inclination
   Version      1.0
   Author:      Douglas C. B. Oliveira, Rodrigo L. S. Silva
   Last Update: 15/10/2013
   Description: This example is similar to the last one, it works with two system markers and obtain relation between these systems.
                This time we get the single marker inclination in relation to the multi markers plan. The renderization simulates
                the objects of an tennis table. The user must incline the racket in order to little ball that is on it falls over table. It's possible to catch
                another little ball by approaching the racket pretty close to the chosen ball.

   Example Informations:
      Renderization:    System Multi:  Table and Small Balls
                        System Single: Racket

      Keyboard Events:
         ESC   Exit Application
         t     Set new threshold value
         +     Increases threshold value in 5 units (limit 255)
         -     Decreases threshold value in 5 units (limit 0)
         d     Enable/Disable visualization mode of threshold

      Mouse Events:     none
*/

#include <avrApplication.h>
#include <cmath>
#include "command_sub.h"

using namespace std;

const double RACKET_RADIUS = 41.0;

avrApplication *racketIncApp;
RacketItemInfo myRacketItem;
ItemList       myListItem;

GLfloat   light_position[]  = {100.0,-200.0,200.0,0.0};
GLfloat   ambi[]            = {0.1, 0.1, 0.1, 0.1};
GLfloat   lightZeroColor[]  = {0.9, 0.9, 0.9, 0.1};

static void    drawRacket();
static void    drawControl();
static void    drawItems(ItemList* list);
static int     drawTable(float x, float y, float height);
static void	   findRacketPosition(const avrMatrix3x4& cardTrans, const avrMatrix3x4& baseTrans, float curRacketPos[]);
static void    keyEvent( unsigned char key, int x, int y);

int main(int argc, char **argv)
{
   glutInit(&argc, argv);
   racketIncApp = new avrApplication();
   racketIncApp->setProjectInfo("Racket Inclination", "Douglas C. B. Oliveira e Rodrigo L. S. Silva",
                          "This test checks the angle and distance between the racket and the surface", "AVR and AVR Numbers");
   #ifdef _WIN32
      racketIncApp->setCameraFiles((char*) "Data/WDM_camera_AVRLib.xml", (char *) "Data/camera_para.dat");
   #else
      // -dev=/dev/video1 -palette=RGB -width=960 -height=544
      racketIncApp->setCameraFiles("-dev=/dev/video0 -palette=RGB -width=640 -height=480", (char *) "Data/camera_para.dat");
   #endif

   // Initialize itens
   myListItem.itemnum=4;
   myListItem.item[0].pos[0]=0.;
   myListItem.item[0].pos[1]=-66.;
   myListItem.item[0].onracket=0;
   myListItem.item[1].pos[0]=-60.;
   myListItem.item[1].pos[1]=66.;
   myListItem.item[1].onracket=0;
   myListItem.item[2].pos[0]=60.;
   myListItem.item[2].pos[1]=0.;
   myListItem.item[2].onracket=0;
   myListItem.item[3].pos[0]=0.;
   myListItem.item[3].pos[1]=0.;
   myListItem.item[3].onracket=1;

   // Set the initial racket item configurations
   myRacketItem.item = 3;
   myRacketItem.angle = 0.0;
   myRacketItem.x = 0.0;
   myRacketItem.y = 0.0;

   racketIncApp->addPattern((char*)"Data/avr.patt", 50.0, NULL, drawRacket);
   racketIncApp->addPatterns((char*)"Data/multi/markerCM.dat", drawControl);

   racketIncApp->setKeyCallback(keyEvent);
   racketIncApp->setThreshold(100);

   racketIncApp->printProjectInfo();

   racketIncApp->start();

   return 0;
}

static void drawControl()
{
   float    curRacketPos[3];
   int      i;
   double	angle;

   bool         racketVisible = racketIncApp->getPattern(0).visible();
   avrMatrix3x4 racketTrans   = racketIncApp->getPattern(0).trans();
   avrMatrix3x4 multiTrans    = racketIncApp->getSystem(1)->getProjection();

	//draw a gray ground grid
	drawTable(274.0, 152.5, 0.0); //

	// find the racket position relative to the base
	if (racketVisible)
		findRacketPosition(racketTrans, multiTrans, curRacketPos);

	// checking for racket gesture
	if(racketVisible)
   {
      int findItem=-1;
      if (myRacketItem.item!=-1)
      {
         if( check_incline(racketTrans, multiTrans, &angle) ) {
            myRacketItem.x += 2.0 * cos(angle);
            myRacketItem.y += 2.0 * sin(angle);
            if( myRacketItem.x*myRacketItem.x + myRacketItem.y*myRacketItem.y > 900.0 ) {
               myRacketItem.x -= 2.0 * cos(angle);
               myRacketItem.y -= 2.0 * sin(angle);
               myListItem.item[myRacketItem.item].onracket=0;
               myListItem.item[myRacketItem.item].pos[0]=curRacketPos[0];
               myListItem.item[myRacketItem.item].pos[1]=curRacketPos[1];
               myRacketItem.item = -1;
            }
         }
      }
      else if ((findItem=check_pickup(racketTrans,multiTrans,&myListItem, &angle))!=-1)
      {
         myRacketItem.item=findItem;
         myRacketItem.x =0.0;
         myRacketItem.y =0.0;
         myRacketItem.angle = 0.0;
         myListItem.item[myRacketItem.item].onracket=1;
      }
    }

	// draw the item
	drawItems(&myListItem);
}

/// find the position of the racket card relative to the base and set the dropped blob position to this
static void	  findRacketPosition(const avrMatrix3x4& cardTrans, const avrMatrix3x4& baseTrans, float curRacketPos[])
{
   avrMatrix3x4   relation;

   relation = baseTrans.getRelationWith(cardTrans);

   // x,y,z is card position relative to base pattern
   curRacketPos[0] = relation.X();
   curRacketPos[1] = relation.Y();
   curRacketPos[2] = relation.Z();
}


/// draw the racket
void  drawRacket()
{
   glColor3f(0.4, 0.2, 0.0);
   glBegin(GL_POLYGON);
      glVertex3f( -7.5, 0.0, -2 );
      glVertex3f(  7.5, 0.0, -2 );
      glVertex3f(  7.5, -105.0, -2 );
      glVertex3f( -7.5, -105.0, -2 );
   glEnd();

   glColor3f(0.4, 0.2, 0.0);
   glLineWidth(4.0);
   glBegin(GL_LINE_LOOP);
      for(int i = 0; i < 16; i++ ) {
         double  x, y;
         x = RACKET_RADIUS * cos(i*3.141592*2/16);
         y = RACKET_RADIUS * sin(i*3.141592*2/16);
         glVertex2d( x, y );
      }
   glEnd();

   glColor3f(0.4, 0.0, 0.0);
   glBegin(GL_POLYGON);
      for(int i = 0; i < 16; i++ ) {
         double  x, y;
         x = RACKET_RADIUS * cos(i*3.141592*2/16);
         y = RACKET_RADIUS * sin(i*3.141592*2/16);
         glVertex2d( x, y );
      }
   glEnd();

   // draw any objects on the racket
   if(myRacketItem.item != -1) {
      glPushMatrix();
      glTranslatef( myRacketItem.x, myRacketItem.y, 10.0 );
      glRotatef( myRacketItem.angle * 180.0/3.141592, 0.0, 0.0, 1.0 );
      glColor3f(0.9,0.6,0.0);
      glutSolidSphere(10,24,20);
      glPopMatrix();
   }
}

/// draw the items on the ground
void drawItems(ItemList* itlist)
{
   int i;
   //double   gl_para[16];
   GLfloat   light_position[]  = {100.0,-200.0,200.0,0.0};
   GLfloat   ambi[]            = {0.7, 0.5, 0.0, 0.1};
   GLfloat   lightZeroColor[]  = {0.9, 0.9, 0.9, 0.1};
   GLfloat   mat_ambient[]    = {0.9, 0.6, 0.0, 1.0};
   GLfloat   mat_flash2[]      = {0.8, 0.4, 0.0, 1.0};
   GLfloat   mat_flash_shiny2[]= {50.0};

   glEnable(GL_DEPTH_TEST);
   glDepthFunc(GL_LEQUAL);

   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);

   glLightfv(GL_LIGHT0, GL_POSITION, light_position);
   glLightfv(GL_LIGHT0, GL_AMBIENT, ambi);
   glLightfv(GL_LIGHT0, GL_DIFFUSE, lightZeroColor);

   glMaterialfv(GL_FRONT, GL_SPECULAR, mat_flash2);
   glMaterialfv(GL_FRONT, GL_SHININESS, mat_flash_shiny2);
   glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);

   for(i = 0; i < itlist->itemnum; i ++ )
   {
      if (!itlist->item[i].onracket)
      {
         glPushMatrix();
         glTranslatef(itlist->item[i].pos[0],itlist->item[i].pos[1], 10.0 );
         glColor3f(0.7,0.5,0.0);
         glutSolidSphere(10,24,20);
         glPopMatrix();
      }
   }
   glDisable( GL_LIGHTING );
   glDisable( GL_DEPTH_TEST );
}

/// Draws a plane table
int drawTable(float x, float y, float height)
{


   //draw the table
   glColor3f(0.1,0.2,0.4);
   glBegin(GL_POLYGON);
      glVertex3f( -x, y, height-5 );
      glVertex3f(  x, y, height-5 );
      glVertex3f(  x, -y, height-5 );
      glVertex3f( -x, -y, height-5 );
   glEnd();

   // contour
   glColor3f(1.0,1.0,1.0);
   glLineWidth(3.0);
   glBegin(GL_LINE_LOOP);
      glVertex3f( -x, y, height-5 );
      glVertex3f(  x, y, height-5 );
      glVertex3f(  x, -y, height-5 );
      glVertex3f( -x, -y, height-5 );
   glEnd();

   // central horizon line
   glBegin(GL_LINES);
      glVertex3f(-x, 0, -4);
      glVertex3f(x, 0, -4);
   glEnd();

   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);
   glLightfv(GL_LIGHT0, GL_POSITION, light_position);
   glLightfv(GL_LIGHT0, GL_AMBIENT, ambi);
   glLightfv(GL_LIGHT0, GL_DIFFUSE, lightZeroColor);

   glDisable( GL_LIGHTING );

   return 0;
}

/* keyboard events */
/** \brief Get a key and realize an action
 *
 * \param key unsigned char   Code corresponding to keyboard key
 * \param x int   X axis mouse coordinate
 * \param y int   Y axis mouse coordinate
 * \return void
 *
 */
static void   keyEvent( unsigned char key, int x, int y)
{
    /* quit if the ESC key is pressed */
   if( key == 0x1b ) {
      cout << endl << racketIncApp->getFrameRate()<< " (frame/sec)" << endl;
      racketIncApp->stop();
      exit(0);
   }

   /* turn on and off the debug mode with right mouse */
   if( key == 'd' ) {
      cout << endl << racketIncApp->getFrameRate() << " (frame/sec)" << endl ;

      cout << endl << endl << racketIncApp->isThresholdMode() << endl;
      if( racketIncApp->isThresholdMode() )
         racketIncApp->disableModeThreshold();
      else
         racketIncApp->enableModeThreshold();

      racketIncApp->resetFrameRate();
   }

   //change the threshold value when 't' key pressed //
   if( key == 't' )
   {
      int thresh;
      cout << "Enter new threshold value (default = 100): ";
      cin >> thresh;
      racketIncApp->setThreshold(thresh);
      cout << endl;
   }

   // Increase and decrease threshhold value in five units
   static int thresh = 100;
   if( key == '+' ) {
      thresh = (thresh <= 250) ? (thresh += 5) : 255;
      racketIncApp->setThreshold(thresh);
      cout << "Thresh: " << racketIncApp->getThreshHold() << endl;
   }
   if( key == '-' ) {
      thresh = (thresh >= 5) ? (thresh -= 5) : 0;
      racketIncApp->setThreshold(thresh);
      cout << "Thresh: " << racketIncApp->getThreshHold() << endl;
   }
}
