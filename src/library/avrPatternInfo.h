/*
  Name:        avrPatternInfo.h
  Version      1.0.1
  Author:      Douglas Oliveira, Rodrigo L. S. Silva
  Date:        10/10/2012
  Last Update: 09/10/2014
  Description: class in substitution to structure ARMarkerInfo
*/

#ifndef AVR_PATTERN_INFO_H_INCLUDED
#define AVR_PATTERN_INFO_H_INCLUDED

/** \class avrPatternInfo avrPatternInfo.h "avrPatternInfo.h"
*   \brief main structure for detected patterns of the library internal use.
*
* Store information after contour detection (in ideal screen coordinate, after distortion compensated).
* \note Lines are represented by 3 values a,b,c for ax+by+c=0
*/
class avrPatternInfo{
   public:
      int     area;
      int     id;
      int     dir;
      double  cf;
      double  pos[2];
      double  line[4][3];
      double  vertex[4][2];
};

#endif // AVR_PATTERN_INFO_H_INCLUDED
