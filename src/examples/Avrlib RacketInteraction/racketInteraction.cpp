/**
   Example:     RacketInteraction
   Version      1.0
   Author:      Douglas C. B. Oliveira, Rodrigo L. S. Silva
   Last Update: 15/10/2013
   Description: This example works with two markers system simultaneously. The objective is to obtain a relation between these systems.
               e.g. the distances of one single marker relative to the plan of multi markers or the intersection between the projected objects
               of each system. For this we rendered a virtual environment where the user must touch the racket (projected over the single marker)
               in virtual cubes (projected in multi system). While checking the intersection between the center of the objects, the cubes are transformated.

   Example Informations:
      Renderization:       System Multi:  Virtual Plan and Cubes
                           System Single: Virtual Racket

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

using namespace std;

const short NOT_TOUCHED = -1;
const short TOUCHED     = 1;
const short TARGET_NUM  = 5;

const double RACKET_RADIUS = 41.0;

// define a target struct for the objects that are to be touched
typedef struct {
	int id;
	int state;
	float     pos[3];
} targetInfo;

targetInfo myTarget[TARGET_NUM];

avrApplication *racketIntApp;

static void   keyEvent( unsigned char key, int x, int y);
static int    checkCollision(float Pos1[],float Pos2[], float range);
static void	  findRacketPosition(const avrMatrix3x4& cardTrans, const avrMatrix3x4& baseTrans, float curRacketPos[]);
static void   drawCubes(const avrMatrix3x4& baseTrans, targetInfo myTarget);
static void   drawControl();
void          drawRacket();
int           drawGroundGrid(int divisions, float x, float y, float height);

int main(int argc, char **argv)
{
   glutInit(&argc, argv);
   racketIntApp = new avrApplication();
   racketIntApp->setProjectInfo("RacketInteraction", "Douglas C. B. Oliveira e Rodrigo L. S. Silva",
                          "This test checks the collision between the racket and virtual float cubes, changing the color",
                          "AVR and AVR Numbers");
   #ifdef _WIN32
      racketIntApp->setCameraFiles((char*) "Data/WDM_camera_AVRLib.xml", (char *) "Data/camera_para.dat");
   #else
      // -dev=/dev/video1 -palette=RGB -width=960 -height=544
      racketIntApp->setCameraFiles("-dev=/dev/video0 -palette=RGB -width=640 -height=480", (char *) "Data/camera_para.dat");
   #endif

   // initialize the targets
   myTarget[0].pos[0] = 0.0f;
   myTarget[0].pos[1] = 0.0f;
   myTarget[0].pos[2] = 20.0f;
   myTarget[0].state = NOT_TOUCHED;
   myTarget[1].pos[0] = -60.0f;
   myTarget[1].pos[1] = 66.0f;
   myTarget[1].pos[2] = 50.0f;
   myTarget[1].state = NOT_TOUCHED;
   myTarget[2].pos[0] = 60.0f;
   myTarget[2].pos[1] = -66.0f;
   myTarget[2].pos[2] = 80.0f;
   myTarget[2].state = NOT_TOUCHED;
   myTarget[3].pos[0] = 60.0f;
   myTarget[3].pos[1] = 66.0f;
   myTarget[3].pos[2] = 110.0f;
   myTarget[3].state = NOT_TOUCHED;
   myTarget[4].pos[0] = -60.0f;
   myTarget[4].pos[1] = -66.0f;
   myTarget[4].pos[2] = 140.0f;
   myTarget[4].state = NOT_TOUCHED;

   // drawRacket parameter is void, the avrSystemSingle calls the callback for its marker not informing the id
   // But it's known that the id is 0, because it's the first marker added
   racketIntApp->addPattern((char*)"Data/avr.patt", 50.0, NULL, drawRacket);
   // drawControl parameter is void, the avrSystemMulti calls the callback once per frame
   racketIntApp->addPatterns((char*)"Data/multi/markerCM.dat", drawControl);

   racketIntApp->setKeyCallback(keyEvent);
   racketIntApp->setThreshold(100);

   racketIntApp->printProjectInfo();

   racketIntApp->start();

   return 0;
}

static void drawControl()
{
   float   curRacketPos[3];

   avrMatrix3x4   racketTrans = racketIntApp->getPattern(0).trans();
   avrMatrix3x4   multiTrans = racketIntApp->getSystem(1)->getProjection();

   //draws a ground grid
   drawGroundGrid(10, 250.0f, 210.0f, 0.0f);

   // find the racket position relative to the base
   findRacketPosition(racketTrans, multiTrans, curRacketPos);

   // check for collisions with targets
   for(int i=0;i<TARGET_NUM;i++)
   {
      myTarget[i].state = NOT_TOUCHED;
      if(checkCollision(curRacketPos, myTarget[i].pos, 20.0f))
         myTarget[i].state = TOUCHED;
   }

   // draw the targets
   for(int i=0;i<TARGET_NUM;i++)
      drawCubes(multiTrans, myTarget[i]);
}

// find the position of the racket card relative to the base and set the dropped blob position to this
static void	  findRacketPosition(const avrMatrix3x4& cardTrans, const avrMatrix3x4& baseTrans, float curRacketPos[])
{
   avrMatrix3x4   relation;

   // calculates the position matrix of the plan(baseTrans) in relation to racket(cardTrans)
   relation = baseTrans.getRelationWith(cardTrans);

   // x,y,z is card position relative to base pattern
   curRacketPos[0] = relation.X();
   curRacketPos[1] = relation.Y();
   curRacketPos[2] = relation.Z();
}

// check collision between two points
static int checkCollision(float pos1[],float pos2[], float range)
{
   float xdist,ydist,zdist,dist;

   xdist = pos1[0]-pos2[0];
   ydist = pos1[1]-pos2[1];
   zdist = pos1[2]-pos2[2];

   dist = xdist*xdist+ydist*ydist+zdist*zdist;
   if(dist<(range*range))
      return 1;
   else
      return 0;
}

/// draw the targets
static void drawCubes(const avrMatrix3x4& baseTrans, targetInfo myTarget)
{
   double    gl_para[16];
   GLfloat   light_position[]  = {100.0,-200.0,200.0,0.0};
   GLfloat   ambi[]            = {0.1, 0.5, 0.0, 0.1};
   GLfloat   lightZeroColor[]  = {0.6, 0.9, 0.3, 0.1};
   GLfloat   mat_ambient2[]    = {0.1, 0.1, 0.1, 1.0};
   GLfloat   mat_ambient[]     = {0.3, 1.0, 0.0, 1.0};
   GLfloat   mat_flash2[]      = {0.0, 1.0, 0.0, 1.0};
   GLfloat   mat_flash_shiny2[]= {50.0};

   //load the camera transformation matrix
   glMatrixMode(GL_MODELVIEW);
   baseTrans.getMatrixGLFormat(gl_para);
   glLoadMatrixd( gl_para );

   // set the lighting and the materials
   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);
   glLightfv(GL_LIGHT0, GL_POSITION, light_position);
   glLightfv(GL_LIGHT0, GL_AMBIENT, ambi);
   glLightfv(GL_LIGHT0, GL_DIFFUSE, lightZeroColor);
   glMaterialfv(GL_FRONT, GL_SPECULAR, mat_flash2);
   glMaterialfv(GL_FRONT, GL_SHININESS, mat_flash_shiny2);
   if(myTarget.state == TOUCHED)
      glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
   else
      glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient2);

   glMatrixMode(GL_MODELVIEW);
   glTranslatef( myTarget.pos[0], myTarget.pos[1], myTarget.pos[2] );
   glutSolidCube(40.0);

   if(myTarget.state == TOUCHED)
   {
      glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient2);
      glColor3f(0.0,0.5,0.0);
      glLineWidth(6.0);
      glutWireCube(60.0);
      glLineWidth(1.0);
   }

   glDisable( GL_LIGHTING );
}

/// Draw the racket
void drawRacket()
{
   bool racketVisible = racketIntApp->getPattern(0).visible();
   if(!racketVisible) return;

   glColor3f(0.0, 0.08, 0.0);
   glBegin(GL_POLYGON);
      glVertex3f( -7.5, 0.0, -2 );
      glVertex3f(  7.5, 0.0, -2 );
      glVertex3f(  7.5, -105.0, -2 );
      glVertex3f( -7.5, -105.0, -2 );
   glEnd();

   glColor3f(0.0, 0.08, 0.0);
   glLineWidth(4.0);
   glBegin(GL_LINE_LOOP);
      for(int i = 0; i < 16; i++ ) {
         double  x, y;
         x = RACKET_RADIUS * cos(i*3.141592*2/16);
         y = RACKET_RADIUS * sin(i*3.141592*2/16);
         glVertex2d( x, y );
      }
   glEnd();

   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   glColor4f(0.6f, 0.8f, 0.0f, 0.9);
   glBegin(GL_POLYGON);
      for(int i = 0; i < 16; i++ ) {
         double  x, y;
         x = RACKET_RADIUS * cos(i*3.141592*2/16);
         y = RACKET_RADIUS * sin(i*3.141592*2/16);
         glVertex2d( x, y );
      }
    glEnd();

   glDisable(GL_BLEND);
}

/// Draws a ground plane
int drawGroundGrid(int divisions, float x, float y, float height)
{
   GLfloat   light_position[]  = {100.0,-200.0,200.0,0.0};
   GLfloat   ambi[]            = {0.1, 0.1, 0.1, 0.1};
   GLfloat   lightZeroColor[]  = {0.9, 0.9, 0.9, 0.1};

   int      i;
   float    x0, x1, y0, y1;
   float    deltaX, deltaY;

   //glTranslatef(x/2,-y/2,height);
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   //draw the grid
   glColor4f(0.1,0.1,0.1, 0.8);
   glLineWidth(6.0);
   glBegin(GL_POLYGON);
      glVertex3f( -x, y, height-3 );
      glVertex3f(  x, y, height-3 );
      glVertex3f(  x, -y, height-3 );
      glVertex3f( -x, -y, height-3 );
   glEnd();
   glLineWidth(2.0);

   glDisable(GL_BLEND);
   //draw a grid of lines
   x0 = -x; x1 = -x;
   y0 = -y; y1 = y;
   deltaX = (2*x)/divisions;

   glColor3f(0.0,1.0,0.0);
   for(i=0;i<divisions;i++){
      x0 = x0 + deltaX;
      glBegin(GL_LINES);
         glVertex3f(x0,y0,height);
         glVertex3f(x0,y1,height);
      glEnd();
   }

   x0 = -x; x1 = x;
   deltaY = (2*y)/divisions;

   for(i=0;i<divisions;i++){
      y0 = y0 + deltaY;
      glBegin(GL_LINES);
         glVertex3f(x0,y0,height);
         glVertex3f(x1,y0,height);
      glEnd();
   }
   glLineWidth(1.0);

   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);
   glLightfv(GL_LIGHT0, GL_POSITION, light_position);
   glLightfv(GL_LIGHT0, GL_AMBIENT, ambi);
   glLightfv(GL_LIGHT0, GL_DIFFUSE, lightZeroColor);

   glDisable( GL_LIGHTING );

   return 0;
}

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
    // Quit if the ESC key is pressed
   if( key == 0x1b ) {
      cout << endl << racketIntApp->getFrameRate() << " (frame/sec)" << endl;
      racketIntApp->stop();
      exit(0);
   }

   // Turn on and off the debug mode with right mouse
   if( key == 'd' ) {
      cout << endl << racketIntApp->getFrameRate() << " (frame/sec)" << endl ;
      if( racketIntApp->isThresholdMode() )
         racketIntApp->disableModeThreshold();
      else
         racketIntApp->enableModeThreshold();

      racketIntApp->resetFrameRate();
   }

   // Change the threshold value when 't' key pressed
   if( key == 't' )
   {
      int thresh;
      cout << "Enter new threshold value (default = 100): ";
      cin >> thresh;
      racketIntApp->setThreshold(thresh);
      cout << endl;
   }

   // Increase and decrease threshhold in five units
   static int thresh = 100;
   if( key == '+' ) {
      thresh = (thresh <= 250) ? (thresh += 5) : 255;
      racketIntApp->setThreshold(thresh);
      cout << "Thresh: " << racketIntApp->getThreshHold() << endl;
   }
   if( key == '-' ) {
      thresh = (thresh >= 5) ? (thresh -= 5) : 0;
      racketIntApp->setThreshold(thresh);
      cout << "Thresh: " << racketIntApp->getThreshHold() << endl;
   }
}
