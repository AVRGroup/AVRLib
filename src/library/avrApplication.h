/*
  Name:        avrApplication.h
  Version      1.0.1
  Author:      Douglas Oliveira, Felipe Caetano, Luiz Maur�lio, Rodrigo L. S. Silva
  Date:        10/10/2012
  Last Update: 09/10/2014
  Description: Main class to control the system.
*/

#ifndef AVR_APPLICATION_H
#define AVR_APPLICATION_H

#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glut.h>

#include <avrMatrix.h>
#include <avrMatrix3x4.h>
#include <avrPattern.h>
#include <avrSystemMarker.h>
#include <avrSystemSingle.h>
#include <avrSystemAutoMulti.h>
#include <avrSystemMulti.h>

/** \class avrApplication avrApplication.h "avrApplication.h"
 * \brief main class of the applications
 *
 * This class is a states machine. Controls the entire execution flow of the application. Manages all markers systems,
 * the outputs and properties video and makes calls for the callbacks.
 */
class avrApplication
{
   private:
      //! vector with the markers systems that aggregate the application
      std::vector<avrSystemMarker*>    markers;

      //! \cond

      bool  thresholdMode; // if displays the threshold image or not
      int   thresh;        // threshold value
      int   count;         // counter frames

      // Callbacks
      //! visibility callback
      void (*visibilityFunc)(int visible);
      //! special keyboard events callback
      void (*specialFunc)(int key, int x, int y);
      //! keyboard events callback
      void (*keyEvent)(unsigned char key, int x, int y);
      //! mouse events callback
      void (*mouseEvent)(int button, int state, int x, int y);
      //! motion events callback
      void (*motionEvent)( int x, int y );

      //! \endcond

   public:
      //! \param windowName a name for the glut window
      avrApplication(const std::string& windowName = "AVRLib Project");    // constructor
      ~avrApplication();   // destructor

      //! sets a new markers system
      void              setNewSystem(avrSystemMarker * newSystem);
      //! gets a markers system (default is first system)
      avrSystemMarker * getSystem(int index = 0);
      //! gets a marker pattern (default is firts pattern of the first system)
      avrPattern        getPattern(int index = 0);
      //! returns the total number marker patterns of the application
      int               numberPatts();

      // Video output options
      //! says in which mini window the main video will be displayed
      void    setMainVideoOutput(unsigned int xwin, unsigned int ywin);
      //! enable threshold image visualization in the mini window defined by parameters (by default in main video output)
      void    enableModeThreshold(unsigned int xwin = 0, unsigned int ywin = 0);
      //! disable threshold image visualization
      void    disableModeThreshold();
      //! checks if threshold image visualization is enabled or disabled
      bool    isThresholdMode();

      //! gets the horizontal size of the video image
      int     getVideoWidth();
      //! gets the vertical size of the video image
      int     getVideoHeight();
      //! gets the horizontal size of the window
      int     getWindowWidth();
      //! gets the vertical size of the window
      int     getWindowHeight();

      //! gets threshold value
      int     getThreshHold();
      //! gets frame rate value
      double  getFrameRate();
      //! restarts counting frames
      void    resetFrameRate();

      //! changes the context renderization for 2D
      void    renderContext2D();
      //! changes the context renderization for 3D and sets video output window (by default in main video output)
      void    renderContext3D(unsigned int xwin = 0, unsigned int ywin = 0);

      // Default Parameters for some Camera and Threshold
      //! sets default camera files
      void setCameraFiles();
      //! sets default threshold value (is 100)
      void setThreshold();

   // User Parameters for pattern
   public:
      //! creates a new "Single markers system" and registers a new marker pattern in this system (secondary display callback)
      void addPattern(const std::string& filename, double patt_width, double *patt_center = NULL,void (*drawFunction)(void) = NULL); //SINGLE_MARKER
      //! creates a new "Single markers system" and registers a new marker pattern in this system (main display callback)
      void addPattern(const std::string& filename, double patt_width, double *patt_center = NULL,void (*drawFunction)(int ) = NULL); //SINGLE_MARKER
      //! creates a new "AutoMulti markers system" and registers the new markers patterns in this system (secondary display callback)
      void addPatterns(const std::string& filename, BASE_SELECTION_MODE, void (*drawFunction)(void) = NULL); // AUTO_MULTI
      //! creates a new "AutoMulti markers system" and registers the new markers patterns in this system (main display callback)
      void addPatterns(const std::string& filename, BASE_SELECTION_MODE, void (*drawFunction)(int ) = NULL); // AUTO_MULTI
      //! creates a new "Multi markers system" and registers the new markers patterns in this system (secondary display callback)
      void addPatterns(const std::string& filename, void (*drawFunction)(void) = NULL); //MULTI
      //! creates a new "AutoMulti markers system" and registers the new markers patterns in this system (main display callback)
      void addPatterns(const std::string& filename, void (*drawFunction)(int ) = NULL); //MULTI

      // User Parameters threshold and camera
      //! sets a new threshold value
      void setThreshold(int thresh);
      /** \brief sets the camera files and defines the number of video outputs (by default only the main video output)
       *
       * This function initializes the camera configuration and the intrinsics parameters of the camera. At end, the window is created.
       * It's possible to extend the window by mini windows of size two times lower than the main video window, setting the two last parameters
       * \param camConfigName camera configuration path file
       * \param camParamName path with intrinsics parameters file of the camera
       * \param xWinNum number of mini window to be created to left of the main video
       *    \note the main video are two mini windows in x-direction (ie xWinNum <= 2 no effect)
       * \param yWinNum number of mini window to be created below the video
       *    \note in y-direction, yWinNum > 0 has effect
       */
      void setCameraFiles(const std::string& camConfigName, const std::string& camParamName, int xWinNum = 0, int yWinNum = 0);

      // User set display callback
      //! sets visibility callback
      void setVisibilityCallback  (void (*visibilityFunction)(int visible));
      //! sets special keyboard events callback
      void setSpecialCallback     (void (*specialFunction)(int key, int x, int y));
      //! sets keyboard events callback
      void setKeyCallback         (void (*keyEvent)(unsigned char key, int x, int y));
      //! sets motion events callback
      void setMotionCallback      (void (*motionEvent)(int x, int y));
      //! sets mouse events callback
      void setMouseCallback       (void (*mouseEvent)(int button, int state, int x, int y));

      //! main loop application
      void mainLoop();

      //! Starts main loop application (called after sets the files, patterns and callbacks for start application)
      void start();

      //! Stops video captures and exit application (called in the end of the applications)
      void stop();

      // Set and Print information of applications in the prompt
      /** \brief sets project informations (optional)
       *
       * \param projectName application name
       * \param authors authors names of application
       * \param info more informations
       * \param requiredMarkers required markers by application
       */
      void setProjectInfo(const std::string& projectName, const std::string& authors, const std::string& info, const std::string& requiredMarkers);
      //! shows project information in the terminal
      void printProjectInfo();
};

#endif // AVR_APPLICATION_H
