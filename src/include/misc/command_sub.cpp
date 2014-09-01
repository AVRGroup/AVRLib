#ifdef _WIN32
#include <windows.h>
#endif
#include <cstdio>
#include <cstdlib>
#include <string>
#include <cmath>

#include "command_sub.h"
#include "util.h"

#define  SHAKE_BUF_SIZE     10
#define  PUNCH_BUF_SIZE      5
#define  PUSH_BUF_SIZE       2

typedef struct {
    double   mat[3][4];
    int      f;
} SHAKE_BUF_T;

typedef struct {
    double   x, y, z;
    int      f;
} PUNCH_BUF_T;

typedef struct {
    double   x, y, z;
    int      f;
} PUSH_BUF_T;

static SHAKE_BUF_T    shake_buf[SHAKE_BUF_SIZE];
static int            shake_buf_num = 0;
//static PUNCH_BUF_T    punch_buf[PUNCH_BUF_SIZE];
//static int            punch_buf_num = 0;
//static PUSH_BUF_T     push_buf[PUNCH_BUF_SIZE];
//static int            push_buf_num = 0;


int check_shake(const avrMatrix3x4& cardTrans, int f )
{
   double  lxy, lz;
   int     i, j, k;

   if( shake_buf_num < SHAKE_BUF_SIZE ) {
      if( f ) {
         for( j = 0; j < 3; j++ ) {
            for( i = 0; i < 4; i++ ) {
               shake_buf[shake_buf_num].mat[j][i] = cardTrans.acess(j, i);
            }
         }
         shake_buf[shake_buf_num].f = 1;
      }
      else {
         shake_buf[shake_buf_num].f = 0;
      }
      shake_buf_num++;

      return 0;
   }
   for( i = 1; i < shake_buf_num; i++ ) {
      shake_buf[i-1] = shake_buf[i];
   }
   if( f ) {
      for( j = 0; j < 3; j++ ) {
         for( i = 0; i < 4; i++ ) {
            shake_buf[shake_buf_num-1].mat[j][i] = cardTrans.acess(j, i);
         }
      }
      shake_buf[shake_buf_num-1].f = 1;
   }
   else {
      shake_buf[shake_buf_num-1].f = 0;
      return 0;
   }

   if( shake_buf[SHAKE_BUF_SIZE-3].f == 0 || shake_buf[0].f == 0 ) return 0;

   avrMatrix3x4 invCardTrans = cardTrans.inverse();
   avrMatrix3x4 matAux, relation;

   for( k = 0 ; k < SHAKE_BUF_SIZE-3; k++ ) {
      if( shake_buf[k].f == 0 ) continue;

      matAux = shake_buf[k].mat;
      relation = invCardTrans * matAux;

      lxy = sqrt( pow(relation.X(), 2) + pow(relation.Y(), 2) );
      lz  = relation.Z();

      if( lxy < 20.0 && lz < 20.0 ) break;
   }

   if( k == SHAKE_BUF_SIZE-3 )
      return 0;

   for( ; k < SHAKE_BUF_SIZE-1; k++ ) {
      if( shake_buf[k].f == 0 ) continue;

      matAux = shake_buf[k].mat;
      relation = invCardTrans * matAux;

      lxy = sqrt( pow(relation.X(), 2) + pow(relation.Y(), 2) );
      lz  = relation.Z();

      if( lxy > 60.0 && lz < 20.0 ) break;
   }

   if( k < SHAKE_BUF_SIZE-1 ) {
      shake_buf_num = 0;
      return 1;
   }

   return 0;
}

int check_incline (const avrMatrix3x4& cardTrans, const avrMatrix3x4& baseTrans, double *angle )
{
   avrMatrix3x4 relation = cardTrans.getRelationWith(baseTrans);

   double  a, b, c;
   get_angle( (double(*)[4])relation.matrix(), &a, &b, &c );

   if( b > 0.4 ) {
      *angle = a + 3.141592;
      return 1;
   }

   return 0;
}

int check_pickup(const avrMatrix3x4& cardTrans, const avrMatrix3x4& baseTrans, ItemList* itlist, double* angle)
{
   double   x, y, z;
   double   lx, ly;

   avrMatrix3x4 relation = baseTrans.getRelationWith(cardTrans);

   x = relation.X();
   y = relation.Y();
   z = relation.Z();

   int ret = -1;
   for(int i = 0; i < itlist->itemnum; i ++ ){
      lx = x - itlist->item[i].pos[0];
      ly = y - itlist->item[i].pos[1];

      //MB increased by a factor of 10
      if( lx*lx + ly*ly < 1000.0 && z < 20.0 ) {
         ret = i;
      }
   }

   double   a, b, c;
   if( ret >= 0 ) {
      get_angle( (double(*)[4])relation.matrix(), &a, &b, &c );
      *angle = -c;
   }

   return ret;
}
