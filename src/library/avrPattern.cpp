#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <avrPatt.h>
#include <avrPattern.h>

using namespace std;

avrPattern::avrPattern(){
    this->patt_width    = 0.0;
    this->patt_visible  = false;
    this->patt_accuracy = 0.0;

    this->patt_center[0] = 0.0;
    this->patt_center[1] = 0.0;

    this->patt_id = -1;

    this->patt_trans = new avrMatrix3x4();
    this->patt_pos3D = new avrMatrix(4, 3);
}

avrPattern::avrPattern(const char *filename, double patt_width, double *patt_center)
{
    this->patt_width    = patt_width;
    this->patt_visible  = false;
    this->patt_accuracy = 0.0;

    this->patt_trans = new avrMatrix3x4();
    this->patt_pos3D = new avrMatrix(4, 3);

    // Change if a patt_center is passed as parameter (default value is NULL)
    if(patt_center) {
        this->patt_center[0] = patt_center[0];
        this->patt_center[1] = patt_center[1];
    }else{
        this->patt_center[0] = 0.0;
        this->patt_center[1] = 0.0;
    }

    if(filename){
        if((this->patt_id = arLoadPatt(filename)) < 0 ) {
            cout << "pattern load error !!\n";
            exit(0);
        } else {
            //cout << "-" << patt_id << endl;
        }
    }
}

// Static Members
static char *get_buff( char *buf, int n, FILE *fp );

avrPattern *avrPattern::avrReadConfigFileNotRelation(const char *filename, int *numberPatts){
   FILE        *fp;
   avrPattern  *object;
   char        buf[256], buf1[256];
   int         i;

   printf("Opening Data File %s\n", filename);

   if( (fp=fopen(filename, "r")) == NULL ) {
      printf("Can't find the file - quitting \n");
      return NULL;
   }

   get_buff(buf, 256, fp);
   if( sscanf(buf, "%d", numberPatts) != 1 ){
      fclose(fp);
      return NULL;
   }

   cout << "About to load " << *numberPatts << " Models" << endl;

   object = new avrPattern[*numberPatts];
   if( object == NULL ) return NULL;

   for( i = 0; i < *numberPatts; i++ ) {
      object[i].setVisible(false);
      get_buff(buf, 256, fp);
      if( sscanf(buf, "%s", buf1) != 1 ){
         fclose(fp);
         free(object);
         return NULL;
      }
      object[i].setName((char*)buf1);

      cout << "Read in No." << i+1 << endl;

      get_buff(buf, 256, fp);
      if( sscanf(buf, "%s", buf1) != 1 ){
         fclose(fp);
         free(object);
         return NULL;
      }

      object[i].setID(arLoadPatt(buf1));
      if( object[i].id() < 0 ){
         fclose(fp);
         free(object);
         return NULL;
      }

      get_buff(buf, 256, fp);
      double width = object[i].width();
      if( sscanf(buf, "%lf", &width) != 1 ){
         fclose(fp);
         free(object);
         return NULL;
      }
      object[i].setWidth(width);

      get_buff(buf, 256, fp);
      double *center = object[i].center();
      if( sscanf(buf, "%lf %lf", center, (center + 1)) != 2 ){
         fclose(fp);
         free(object);
         return NULL;
      }
      object[i].setCenter(center);
   }

   fclose(fp);

   return object;

}

avrPattern *avrPattern::avrReadConfigFileWithRelation(const char *filename, int * numberPatts)
{
    FILE            *fp;
    double          wpos3d[4][2];
    char            buf[256], buf1[256];
    int             i, j;
    avrPattern      *object;

    if( (fp=fopen(filename,"r")) == NULL ) return NULL;

    get_buff(buf, 256, fp);
    if( sscanf(buf, "%d", numberPatts) != 1 ){
        fclose(fp);
        return NULL;
    }

    object = new avrPattern[*numberPatts];
    if( object == NULL ) return NULL;

    for( i = 0; i < *numberPatts; i++ ) {
        get_buff(buf, 256, fp);
        if( sscanf(buf, "%s", buf1) != 1 ) {
            fclose(fp); free(object); return NULL;
        }

        object[i].setID(arLoadPatt(buf1));
        if( object[i].id() < 0 ){
            fclose(fp);
            free(object);
            return NULL;
        }

        get_buff(buf, 256, fp);
        double width = object[i].width();
        if( sscanf(buf, "%lf", &width) != 1 ){
            fclose(fp);
            free(object);
            return NULL;
        }
        object[i].setWidth(width);


        get_buff(buf, 256, fp);
        double *center = object[i].center();
        if( sscanf(buf, "%lf %lf", center, (center + 1)) != 2 ){
            fclose(fp);
            free(object);
            return NULL;
        }
        object[i].setCenter(center);

        double (*trans)[4] = (double(*)[4])object[i].trans().matrix();
        for( j = 0; j < 3; j++ ) {
            get_buff(buf, 256, fp);
            if( sscanf(buf, "%lf %lf %lf %lf", &trans[j][0], &trans[j][1], &trans[j][2], &trans[j][3]) != 4 ) {
                fclose(fp);
                free(object);
                return NULL;
            }
            object[i].trans() = trans;
        }

        wpos3d[0][0] = (object[i].center())[0] - object[i].width()/2.0;
        wpos3d[0][1] = (object[i].center())[1] + object[i].width()/2.0;
        wpos3d[1][0] = (object[i].center())[0] + object[i].width()/2.0;
        wpos3d[1][1] = (object[i].center())[1] + object[i].width()/2.0;
        wpos3d[2][0] = (object[i].center())[0] + object[i].width()/2.0;
        wpos3d[2][1] = (object[i].center())[1] - object[i].width()/2.0;
        wpos3d[3][0] = (object[i].center())[0] - object[i].width()/2.0;
        wpos3d[3][1] = (object[i].center())[1] - object[i].width()/2.0;
        for( j = 0; j < 4; j++ ) {
            object[i].pos3D().add(trans[0][0] * wpos3d[j][0] + trans[0][1] * wpos3d[j][1] + trans[0][3], j, 0);
            object[i].pos3D().add(trans[1][0] * wpos3d[j][0] + trans[1][1] * wpos3d[j][1] + trans[1][3], j, 1);
            object[i].pos3D().add(trans[2][0] * wpos3d[j][0] + trans[2][1] * wpos3d[j][1] + trans[2][3], j, 2);
        }
    }

    fclose(fp);

    return object;
}

static char *get_buff( char *buf, int n, FILE *fp )
{
    char *ret;

    for(;;) {
        ret = fgets( buf, n, fp );
        if( ret == NULL ) return(NULL);
        if( buf[0] != '\n' && buf[0] != '#' ) return(ret);
    }
}
