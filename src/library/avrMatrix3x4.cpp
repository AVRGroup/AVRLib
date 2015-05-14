#include <cmath>
#include <iostream>

#include <avrUtil.h>
#include <avrMatrix3x4.h>

using namespace std;

avrMatrix3x4::avrMatrix3x4() : avrMatrix(4, 4)
{

}

avrMatrix3x4::avrMatrix3x4(const avrMatrix& mat) : avrMatrix(4 ,4)
{
   for(unsigned int i = 0; i < mat.row() && i < 3; i++){
      for(unsigned int j = 0; j < mat.column() && j < 4; j++){
         this->data[i * 4 + j] = mat.access(i, j);
      }
   }
}

avrMatrix3x4::avrMatrix3x4(double matrix[3][4]) : avrMatrix(4, 4)
{
   for(unsigned int i = 0; i < 3; i++){
      for(unsigned int j = 0; j < 4; j++){
         this->data[i * 4 + j] = matrix[i][j];
      }
   }
}

avrMatrix3x4::~avrMatrix3x4()
{

}

// Methods Overwritten

/* ***** Auxiliary ***** */

double avrMatrix3x4::access (int row, int clm) const throw(std::out_of_range&)
{
   if(row < 0 || clm < 0 || row >= 3 || row >= 4)
      throw out_of_range("invalid index");

   return avrMatrix::access(row, clm);
}

void avrMatrix3x4::add (double element, int row, int clm) throw(std::out_of_range&)
{
   if(row < 0 || clm < 0 || row >= 3 || row >= 4)
      throw out_of_range("invalid index");
   avrMatrix::add(element, row, clm);
}

void avrMatrix3x4::print(string name, int decimals) const
{
   if(name != "")
      cout << "  " << name << "3x4\n";
   else
      cout << "  avrMatrix3x4\n";
   for(int r = 0; r < 3; r++) {
      cout << " | ";
		for(int c = 0; c < 4; c++) {
         cout.flags(std::ios::fixed | std::ios::showpoint | std::ios::showpos);
         cout.precision(decimals);
			cout << this->access(r, c) << " | ";
			cout.flags(~std::ios::showpos);
		}
		cout << "\n";
	}
	cout << "\n";
}

/* ***** Gets ***** */
unsigned int avrMatrix3x4::row() const
{
   return 3;
}

unsigned int avrMatrix3x4::column() const
{
   return 4;
}

double** avrMatrix3x4::matrix() const
{
   double (*ret)[4] = new double[3][4];

   for(int i = 0; i < 3; i++)
      for(int j = 0; j < 4; j++)
         ret[i][j] = this->access(i, j);

   return (double**)ret;
}

// Methods Specific
avrMatrix3x4& avrMatrix3x4::operator* (const avrMatrix3x4& mat2)
{
   double mat2_acess_3[] = {0.0, 0.0, 0.0, 1.0};
   avrMatrix3x4 *mat3 = new avrMatrix3x4();

   for(int i = 0; i < 3; i++){
      for(int j = 0; j < 4; j++){
         double element = 0.0;
         for(int k = 0; k < 4; k++){
            if(k == 3)
               element += this->access(i, k) * mat2_acess_3[j];
            else
               element += this->access(i,k) * mat2.access(k,j);
         }
         mat3->add(element,i,j);
      }
   }
   return *mat3;
}

avrMatrix3x4& avrMatrix3x4::operator= (const avrMatrix3x4& mat2)
{
   for(int i = 0; i < 3; i++){
      for(int j = 0; j < 4; j++){
         this->data[i * 4 + j] = mat2.access(i, j);
      }
   }

   return *this;
}

// Retorna a matriz que leva this à mat2 (Mab = inv(A) x B)
avrMatrix3x4&  avrMatrix3x4::getRelationWith(const avrMatrix3x4& mat2) const
{
   double inv[3][4], dest[3][4];

   arUtilMatInv((double(*)[4])this->matrix(), inv);
   arUtilMatMul(inv, (double(*)[4])mat2.matrix(), dest);

   avrMatrix3x4 * ret = new avrMatrix3x4(dest);
   return *ret;
}

double avrMatrix3x4::euclidianDistanceBetween(const avrMatrix3x4& mat2) const
{
   double result = 0;
   result += pow(this->access(0, 3) - mat2.access(0, 3), 2);
   result += pow(this->access(1, 3) - mat2.access(1, 3), 2);
   result += pow(this->access(2, 3) - mat2.access(2, 3), 2);
   result = sqrt(result);

   return result;
}

void avrMatrix3x4::getMatrixGLFormat(double gl_para[16]) const
{
   for(int i = 0; i < 4; i++)
      for(int j = 0; j < 4; j++)
         gl_para[j * 4 + i] = avrMatrix::access(i, j);
}

void avrMatrix3x4::extractQuatAndPos(double quat[4], double pos[3]) const throw(std::domain_error&)
{
   double (*mat)[4] = (double(*)[4])this->matrix();
   int sucess = arUtilMat2QuatPos(mat, quat, pos);
   sucess++;      // retorno arUtilMat2QuatPos -1 ou 0, esta linha converte para 0 ou 1
   if(!sucess) throw domain_error("quaternion not normalize");
}

void avrMatrix3x4::setMatWithQuatAndPos(double quat[4], double pos[3])
{
   double mat[3][4];
   arUtilQuatPos2Mat(quat, pos, mat);

   for(int i = 0; i < 3; i++)
      for(int j = 0; j < 4; j++)
         this->add(mat[i][j], i, j);

   this->data[3 * 4 + 0] = 0.0;
   this->data[3 * 4 + 1] = 0.0;
   this->data[3 * 4 + 2] = 0.0;
   this->data[3 * 4 + 3] = 1.0;
}

// Special Gets
double   avrMatrix3x4::X() const   { return this->access(0, 3); }
double   avrMatrix3x4::Y() const   { return this->access(1, 3); }
double   avrMatrix3x4::Z() const   { return this->access(2, 3); }
