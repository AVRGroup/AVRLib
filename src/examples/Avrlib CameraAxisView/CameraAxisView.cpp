/**
   Example:     CameraAxisView
   Version      1.0
   Author:      Igor F. Couto, Douglas C. B. Oliveira, Rodrigo L. S. Silva
   Last Update: 15/10/2013
   Description: This application is slightly more complex than the previous.
                The purpose of this example is to simulate the position and pose of the real camera in a relation with the scene marker.
                For this a virtual environment is rendered on a secondary video output.
                In this environment has an object to represent the real camera and a cube, which represents the marker.
                When you move the real camera, the object that represents it virtually moves in the same direction.
                The virtual camera position and pose are estimated by the inverse transformation matrix of the marker.

   Example Informations:
      Renderization:    Coordinated Axes

      Keyboard Events:
         ESC   Exit Application
         t     Set new threshold value
         +     Increases threshold value in 5 units (limit 255)
         -     Decreases threshold value in 5 units (limit 0)
         d     Enable/Disable visualization of threshold image
         o     Enable/Disable visualization of quaternions informations
         c     Swap main video output for by secondary video output

      Mouse Events:
         Left     scroll virtual plan
         Right    zoom
         Middle   Swap main video output for by secondary video output
*/

#include <avrApplication.h>
#include <cstdio>
#include <string.h>
#include <cmath>

using namespace std;

int      outputMode = 0;      // flag for visibility quaternion informations
int      disp_mode = 1;       // flag for main video output
int      mouse_ox;            // current X coordinate of mouse cursor
int      mouse_oy;            // current Y coordinate of mouse cursor
int      mouse_st = 0;        // mouse state
double   a =  0.0;            // angle rotation around Oy
double   b =  -45.0;          // angle rotation around Ox
double   r =  1000.0;         // range

avrApplication *axisViewApp;

// Controler draw
static void drawControl();
static void getResultRaw();
static void getResultQuat();

// Events callbacks
static void keyEvent(unsigned char key, int x, int y);
static void mouseEvent(int button, int state, int x, int y);
static void motionEvent(int x, int y);

// Draw functions
void        showString(string str);
int         drawObject(const avrMatrix3x4& trans, unsigned int xwin, unsigned int ywin);
int         drawExview(const avrMatrix3x4& trans, double a, double b, double r, unsigned int xwin, unsigned int ywin);
static void drawCamera(const avrMatrix3x4& trans);
static void drawAxis();

// light configuration
static void setupLight();
// calculates the new transformation matrix by rotation plan
static void getTrans(double a, double b, double r, avrMatrix3x4& trans);

int main(int argc, char **argv)
{
   glutInit(&argc, argv);
   axisViewApp = new avrApplication();

   axisViewApp->setProjectInfo("CameraAxisView", "Igor F. Couto, Douglas C. B. Oliveira e Rodrigo L. S. Silva",
                          "This test shows a relation between the camera and the virtual object", "AVR");
   #ifdef _WIN32
      axisViewApp->setCameraFiles((char*) "Data/WDM_camera_AVRLib.xml", (char *) "Data/camera_para.dat", 2, 1);
   #else
      // -dev=/dev/video1 -palette=RGB -width=960 -height=544
      axisViewApp->setCameraFiles("-dev=/dev/video0 -palette=RGB -width=640 -height=480", (char *) "Data/camera_para.dat", 2, 1);
   #endif
   axisViewApp->addPattern((char*) "Data/avr.patt", 60.0, NULL, drawControl);

   axisViewApp->setKeyCallback(keyEvent);
   axisViewApp->setMouseCallback(mouseEvent);
   // Callback for motion mouse cursor
   axisViewApp->setMotionCallback(motionEvent);
   axisViewApp->setThreshold(100);

   axisViewApp->printProjectInfo();

   axisViewApp->start();

   return 0;
}

static void drawControl()
{
   //
   if( !disp_mode )
      axisViewApp->setMainVideoOutput(1, 1);
   else
      axisViewApp->setMainVideoOutput(0, 0);

   // checks visibility of marker
   if( axisViewApp->getPattern().visible() )
   {
      switch( outputMode )
      {
         case 0:
            getResultRaw();   // not renders informations of quaternions
            break;
         case 1:
            getResultQuat();  // renders informations of quaternions
            break;
      }
   }
}

static void getResultRaw()
{
   avrMatrix3x4   target_trans = axisViewApp->getPattern().trans();
   avrMatrix3x4   cam_trans;
   char           str[256];

   try{
      // pose and position of camera is estimated by inverse transformation matrix of the marker
      cam_trans = target_trans.inverse();
   }
   catch(domain_error&){
      return;
   }

   // writes the string which will be rendered in the video.
   sprintf(str, " RAW: Cam Pos x: %3.1f  y: %3.1f  z: %3.1f", cam_trans.X(), cam_trans.Y(), cam_trans.Z());

   // disp_mode enabled, virtual ambient is rendered in secondary video output
   if( disp_mode )
   {
      drawObject(target_trans, 0, 0 );
      drawExview(target_trans, a, b, r, 1, 1 );
   }
   // disp_mode disabled, virtual ambient is rendered in main video output
   else
   {
      drawObject(target_trans, 1, 1 );
      drawExview(target_trans, a, b, r, 0, 0 );
   }
   // draws the string
   showString( str );
}

static void getResultQuat( )
{
   avrMatrix3x4  target_trans = axisViewApp->getPattern().trans();
   avrMatrix3x4  cam_trans;

   double   quat[4], pos[3];
   char     string1[256];
   char     string2[256];

   try{
      cam_trans = target_trans.inverse();
      // extracts the vectors of quaternion and position
      cam_trans.extractQuatAndPos(quat, pos);
   }catch(domain_error&){
      return;
   }

   sprintf(string1, " QUAT: Pos x: %3.1f  y: %3.1f  z: %3.1f\n", pos[0], pos[1], pos[2]);
   sprintf(string2, " \tQuat qx: %3.2f qy: %3.2f qz: %3.2f qw: %3.2f ", quat[0], quat[1], quat[2], quat[3]);
   strcat( string1, string2 );

   if( disp_mode )
   {
     drawObject(target_trans, 0, 0 );
     drawExview(target_trans, a, b, r, 1, 1 );
   }
   else
   {
     drawObject(target_trans, 1, 1 );
     drawExview(target_trans, a, b, r, 0, 0 );
   }
   showString( string1 );
}

void showString(string str)
{
   int   i;

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();

   glTranslatef(-0.95, -0.20, 0.0);

   // draws white rectangle
   glColor3f(1.0, 1.0, 1.0);
   glBegin(GL_POLYGON);
      glVertex2f(1.50, 0.10);
      glVertex2f(1.50, -0.15);
      glVertex2f(0.001, -0.15);
      glVertex2f(0.001, 0.10);
   glEnd();

   // draws text
   glColor3f(0.75, 0.0, 0.0);
   glRasterPos2i(0.0, 0.0);
   for (i = 0; i < (int)str.length(); i++)
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

// draws virtual ambient of the camera
int drawExview(const avrMatrix3x4& trans, double a, double b, double r, unsigned int xwin, unsigned int ywin)
{
   avrMatrix3x4   vtrans;
   double   gl_para[16];
   int      i, j;

   // changes the renderization context for 3D
   axisViewApp->renderContext3D(xwin, ywin);

   glDepthFunc(GL_LEQUAL);
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_NORMALIZE);

   // calculates and sets projection matrix
   getTrans( a, b, r, vtrans );
   vtrans.getMatrixGLFormat(gl_para);
   glMatrixMode(GL_PROJECTION);
   glMultMatrixd( gl_para );
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();

   setupLight();
   // draw the plan with axis and cube
   glPushMatrix();
      glEnable(GL_LIGHTING);
      glEnable(GL_LIGHT0);

      for( j = -300; j <= 200; j+= 100 )
      {
         for( i = -300; i <= 200; i+= 100 )
         {
            glBegin(GL_QUADS);
               glNormal3f( 0.0, 0.0, 1.0 );
               if( (j/100+i/100)%2 )
                  glColor4f( 0.6, 0.6, 0.6, 1.0 );
               else
                  glColor4f( 0.0, 0.3, 0.0, 1.0 );
               glVertex3f( i,     j,     0.0 );
               glVertex3f( i,     j+100, 0.0 );
               glVertex3f( i+100, j+100, 0.0 );
               glVertex3f( i+100, j,     0.0 );
            glEnd();
         }
      }
      drawAxis();

      glColor4f( 0.0, 0.0, 0.5, 1.0 );
      glTranslatef( 0.0, 0.0, 25.0 );
      glutSolidCube(50.0);

      glDisable( GL_LIGHTING );
   glPopMatrix();

   // draw the virtual camera
   drawCamera(trans);

   glDisable(GL_NORMALIZE);
   glDisable( GL_DEPTH_TEST );

   // back with the renderization context for 2D
   axisViewApp->renderContext2D();

   return 0;
}

// draws the virtual camera
static void drawCamera( const avrMatrix3x4& trans )
{
   avrMatrix3x4   btrans = trans.inverse();
   double         quat[4], pos[3], angle;

   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
      btrans.extractQuatAndPos(quat, pos);
      angle = -acos(quat[3])*360.0/M_PI;

      glTranslatef( pos[0], pos[1], pos[2] );
      glRotated( angle, quat[0], quat[1], quat[2] );

      glEnable(GL_LIGHTING);
      glEnable(GL_LIGHT0);

      glPushMatrix();
         glColor4f( 0.9, 0.9, 0.9, 1.0 );
         glTranslatef( 0.0, 0.0, -10.0 );
         glScalef( 10.0, 10.0, 20.0 );
         glRotatef(180.0, 1.0, 0.0, 0.0);
         glutSolidCone(1.0, 1.0, 10, 10);
      glPopMatrix();

      glColor4f( 0.7, 0.7, 0.7, 1.0 );
      glLineWidth(3);
      glPushMatrix();
         glTranslatef( 0.0, 0.0, -40.0 );
         glScalef( 30.0, 30.0, 40.0 );
         glutWireCube(1.0);
      glPopMatrix();
      glLineWidth(1);

      glColor4f( 0.3, 0.3, 0.3, 1.0 );
      glPushMatrix();
         glTranslatef( 0.0, 0.0, -40.0 );
         glScalef( 30.0, 30.0, 40.0 );
         glutSolidCube(1.0);
      glPopMatrix();

      glDisable( GL_LIGHTING );
   glPopMatrix();
}

int  drawObject(const avrMatrix3x4& trans, unsigned int xwin, unsigned int ywin )
{
   double gl_para[16];

   axisViewApp->renderContext3D(xwin, ywin);

   glDepthFunc(GL_LEQUAL);
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_NORMALIZE);

   // sets a transformation matrix
   trans.getMatrixGLFormat(gl_para);
   glMatrixMode(GL_PROJECTION);
   glMultMatrixd( gl_para );
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();

   setupLight();
   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);

   // draws axis
   drawAxis();

   glDisable( GL_LIGHTING );
   glDisable( GL_NORMALIZE );
   glDisable( GL_DEPTH_TEST );

   return 0;
}

// Configure lighting
static void setupLight()
{
   static int  mat_f = 1;
   GLfloat     mat_amb_diff[]  = {0.9, 0.9, 0.0, 1.0};
   GLfloat     mat_specular[]  = {0.5, 0.5, 0.5, 1.0};
   GLfloat     mat_shininess[] = {10.0};
   GLfloat     light_ambient[] = { 0.01, 0.01, 0.01, 1.0 };
   GLfloat     light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
   GLfloat     light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
   GLfloat     light_position[] = { 100.0, 300.0, 700.0, 1.0 };

   if( mat_f )
   {
      glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat_amb_diff);
      glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
      glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
      glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
      glEnable(GL_COLOR_MATERIAL);
      mat_f = 0;
   }

   glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
   glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
   glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
   glLightfv(GL_LIGHT0, GL_POSITION, light_position);
}

// Calculates new transformation matrix
static void getTrans(double a, double b, double r, avrMatrix3x4& newTrans)
{
   avrMatrix   mat(4, 4);
   double  sa, ca, sb, cb;
   double  x, y, z;

   sa = sin(a*3.141592/180.0);
   ca = cos(a*3.141592/180.0);
   sb = sin(b*3.141592/180.0);
   cb = cos(b*3.141592/180.0);

   x = 0.0;
   y = -r * cb;
   z = -r * sb;

   mat.add(ca, 0 , 0);
   mat.add(sa*sb, 0, 1);
   mat.add(sa*cb, 0, 2);
   mat.add(-sa, 1, 0);
   mat.add(ca*sb, 1, 1);
   mat.add(ca*cb, 1, 2);
   mat.add(0, 2, 0);
   mat.add(-cb, 2, 1);
   mat.add(sb, 2, 2);
   mat.add(x*ca + y*sa, 0, 3);
   mat.add(-x*sa + y*ca, 1, 3);
   mat.add(z, 2, 3);
   // last line [0, 0, 0, 1]

   try{
      newTrans = mat.inverse();
   }catch(domain_error&){
      //nop
   }
}

// Draws the coordinated axes
static void drawAxis()
{
   glPushMatrix();
      glRotatef( 90.0, 0.0, 1.0, 0.0 );
      glColor4f( 1.0, 0.0, 0.0, 1.0 );
      glutSolidCone(5.0, 100.0, 20, 24);
   glPopMatrix();

   glPushMatrix();
      glRotatef( -90.0, 1.0, 0.0, 0.0 );
      glColor4f( 0.0, 1.0, 0.0, 1.0 );
      glutSolidCone(5.0, 100.0, 20, 24);
   glPopMatrix();

   glPushMatrix();
      glRotatef( 00.0, 0.0, 0.0, 1.0 );
      glColor4f( 0.0, 0.0, 1.0, 1.0 );
      glutSolidCone(5.0, 100.0, 20, 24);
   glPopMatrix();
}

static void   keyEvent( unsigned char key, int x, int y)
{
   // Quit if the ESC key is pressed
   if( key == 0x1b )
   {
      cout << endl <<
      axisViewApp->getFrameRate() << " (frame/sec)" << endl;
      axisViewApp->stop();
      exit(0);
   }

   // Change the threshold value when 't' key pressed
   if( key == 't' )
   {
      int thresh;
      cout << "Enter new threshold value (default = 100): ";
      cin >> thresh;
      axisViewApp->setThreshold(thresh);
      cout << endl;
   }

   // Increase and decrease threshhold in five units
   static int thresh = 100;
   if( key == '+' ) {
      thresh = (thresh <= 250) ? (thresh += 5) : 255;
      axisViewApp->setThreshold(thresh);
      cout << "Thresh: " << axisViewApp->getThreshHold() << endl;
   }
   if( key == '-' ) {
      thresh = (thresh >= 5) ? (thresh -= 5) : 0;
      axisViewApp->setThreshold(thresh);
      cout << "Thresh: " << axisViewApp->getThreshHold() << endl;
   }

   // Enable/Disable threshold mode
   if( key == 'd' )
   {
      if( axisViewApp->isThresholdMode() )
         axisViewApp->disableModeThreshold();
      else
         axisViewApp->enableModeThreshold(2, 1);
   }

   // Enable/Disable visualization quaternion informations
   if(key == 'o')
      outputMode = (outputMode + 1) % 2;

   // Swap main video output
   if(key == 'c' )
      disp_mode = 1 - disp_mode;
}

static void mouseEvent(int button, int state, int x, int y)
{
   // disable mouse motion
   if( state == GLUT_UP )
      mouse_st = 0;

   // enable mouse motion
   else if( state == GLUT_DOWN )
   {
      // mouse state rotation of the plan
      if( button == GLUT_LEFT_BUTTON )
      {
         mouse_st = 1;
         mouse_ox = x;
         mouse_oy = y;
      }
      // mouse state zoom
      else if( button == GLUT_RIGHT_BUTTON )
      {
         mouse_st = 2;
         mouse_ox = x;
         mouse_oy = y;
      }
      // swap main video output
      else if( button == GLUT_MIDDLE_BUTTON )
         disp_mode = 1 - disp_mode;
   }
}

static void motionEvent( int x, int y )
{
   // rotation of plan
   if( mouse_st == 1 )
   {
      a += ((double)x - mouse_ox) / 2.0;
      b -= ((double)y - mouse_oy) / 2.0;

      if( a <   0.0 ) a += 360.0;
      if( a > 360.0 ) a -= 360.0;
      if( b < -90.0 ) b =  -90.0;
      if( b >   0.0 ) b =    0.0;
   }
   // zoom
   else if( mouse_st == 2 )
   {
      r *= (1.0 + ((double)y - mouse_oy)*0.01);
      if( r < 10.0 ) r = 10.0;
   }

   // stores current mouse position
   mouse_ox = x;
   mouse_oy = y;
}
