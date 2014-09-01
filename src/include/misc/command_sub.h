#ifndef COMMAND_SUB_H
#define COMMAND_SUB_H

#include <avrMatrix3x4.h>

/* on item structure */
typedef struct {
  double pos[2];
  int onpaddle;
} Item;

/* list of items */
typedef struct {
  int itemnum;
  Item item[256];
} ItemList;

/* item which is on the paddle */
typedef struct {
    int     item;
    double  angle;
    double  x, y;
} PaddleItemInfo;

/* shaking gesture to remove a pattern */
int check_shake   (const avrMatrix3x4& cardTrans, int f );

/* inclining gesture to put an object on the ground*/
int check_incline (const avrMatrix3x4& cardTrans, const avrMatrix3x4& baseTrans, double *angle );

/* picking gesture to take an object from the ground*/
int check_pickup(const avrMatrix3x4& cardTrans, const avrMatrix3x4& baseTrans, ItemList* itlist, double* angle);

#endif
