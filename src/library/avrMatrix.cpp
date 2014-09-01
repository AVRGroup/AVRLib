#include <iostream>
#include <exception>
#include <cstdio>
#include <cstdlib>
#include <cmath>

#include <avrMath.h>
#include <avrUtil.h>
#include <avrMatrix.h>

#define  MEMORY_ERROR      0

using namespace std;

static ARMat      *avrMatrixToArMat(double *m, int rows, int clms);
static avrMatrix  *arMatToAvrMatrix(ARMat *m);
static int        pos(int i, int j, int d);

//Constructors
avrMatrix::avrMatrix()
{
   this->matrixx = NULL;
}
avrMatrix::avrMatrix(int row, int clm)
{
	try{
	   this->matrixx = new double[row * clm];
	}catch(bad_alloc& ba){
      cerr << ba.what() << endl;
      exit(EXIT_FAILURE);
	}

   this->rows = row;
   this->clms = clm;
   this->setIdentityMatrix();
}

//Destructor
avrMatrix::~avrMatrix(){
//   delete[] this->matrixx;
}


/* ***** Operations ***** */

avrMatrix& avrMatrix::operator* (double scalar)
{
   avrMatrix *mat2 = new avrMatrix(this->row(), this->column());
   for(int i = 0; i < this->row(); i++){
      for(int j = 0; j < this->column(); j++){
         double element = scalar * this->access(i, j);
         mat2->add(element,i,j);
      }
   }

   return *mat2;
}

avrMatrix& avrMatrix::operator* (const avrMatrix& mat2) throw(std::invalid_argument&)
{
   if(this->clms != mat2.row())
      throw invalid_argument("column dimension different of the row dimension");

   avrMatrix *mat3 = new avrMatrix(this->rows, mat2.column());
   for(int i = 0; i < this->rows; i++){
      for(int j = 0; j < mat2.column(); j++){
         double element = 0.0;
         for(int k = 0; k < this->clms; k++){
            element += this->access(i,k) * mat2.access(k,j);
         }
         mat3->add(element,i,j);
      }
   }
   return *mat3;
}

avrMatrix& avrMatrix::operator+ (const avrMatrix& mat2) throw(std::invalid_argument&)
{
   if(this->column() != mat2.column() || this->row() != mat2.row())
      throw invalid_argument("dimension of the row and column different");

   avrMatrix *mat3 = new avrMatrix(this->row(), this->column());
   for(int i = 0; i < this->row(); i++){
      for(int j = 0; j < mat2.column(); j++){
         double element = this->access(i,j) + mat2.access(i,j);
         mat3->add(element,i,j);
      }
   }
   return *mat3;
}

avrMatrix& avrMatrix::operator- (const avrMatrix& mat2) throw(std::invalid_argument&)
{
   if(this->column() != mat2.column() || this->row() != mat2.row())
      throw invalid_argument("dimension of the row and column different");

   avrMatrix *mat3 = new avrMatrix(this->row(), this->column());
   for(int i = 0; i < this->row(); i++){
      for(int j = 0; j < mat2.column(); j++){
         double element = this->access(i,j) - mat2.access(i,j);
         mat3->add(element,i,j);
      }
   }
   return *mat3;
}

avrMatrix& avrMatrix::operator= (const avrMatrix& mat2)
{
   if(!this->matrixx){
      create:
         try{
            this->matrixx = new double[mat2.row() * mat2.column()];
         }catch(bad_alloc& ba){
            cerr << ba.what() << endl;
            exit(EXIT_FAILURE);
         }
         this->rows = mat2.row();
         this->clms = mat2.column();
   }
   else if(this->row() != mat2.row() || this->column() != mat2.column()){
      delete[] this->matrixx;
      this->matrixx = NULL;
      goto create;
   }

   for(int i = 0; i < this->rows; i++){
      for(int j = 0; j < this->clms; j++){
         this->matrixx[i * this->clms + j] = mat2.access(i, j);
      }
   }

   return *this;
}

double* avrMatrix::operator[](unsigned int index) throw(std::out_of_range&)
{
   if(index >= row()) throw out_of_range("invalid index");
   return &(this->matrixx[index * clms]);
}

avrMatrix& avrMatrix::inverse() const throw(std::domain_error&)
{
   ARMat *inverse = avrMatrixToArMat(this->matrixx, this->rows, this->clms);
   int sucess = arMatrixSelfInv(inverse);
   sucess++;      // retorno arMatrixSelfInv é -1 ou 0, esta linha converte para 0 ou 1
   if(!sucess)
      throw domain_error("not inverse");

   return *arMatToAvrMatrix(inverse);
}

avrMatrix& avrMatrix::transposed() const
{
   avrMatrix *mat2 = new avrMatrix(this->clms, this->rows);
   for(int i = 0; i < this->row(); i++)
      for(int j = 0; j < this->column(); j++)
         mat2->add(this->access(i, j), j, i);

   return *mat2;
}

double avrMatrix::determinant() const
{
   ARMat *mat;
   try{
      mat = avrMatrixToArMat(this->matrixx, this->rows, this->clms);
   }catch(ARMat* e){
      cout << "bad allocated\n";
      exit(EXIT_FAILURE);
   }
   return arMatrixDet(mat);
}


/* ***** Auxiliary ***** */

double avrMatrix::access(int row,int clm) const throw(std::out_of_range&)
{
   if(row < 0 || clm < 0 || row >= this->rows || clm >= this->clms) throw out_of_range("invalid index");
   return this->matrixx[row * column() + clm];
}

void avrMatrix::add(double element, int row, int clm) throw(std::out_of_range&)
{
   if(row < 0 || clm < 0 || row >= this->rows || clm >= this->clms) throw out_of_range("invalid index");
   this->matrixx[row * column() + clm] = element;
}

void avrMatrix::print(string name, int decimals) const
{
   if(name != "")
      cout << "  " << name << " " << this->rows << "x" << this->clms << "\n";
   else
      cout << "  avrMatrix " << this->rows << "x" << this->clms << "\n";
	for(int r = 0; r < this->rows; r++) {
		cout << " | ";
		for(int c = 0; c < this->clms; c++) {
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

int avrMatrix::row() const
{
   return this->rows;
}

int avrMatrix::column() const
{
   return this->clms;
}

double** avrMatrix::matrix() const
{
   double ret[this->rows][this->clms];

   for(int i = 0; i < this->rows; i++)
      for(int j = 0; j < this->clms; j++)
         ret[i][j] = this->access(i, j);

   return (double**) ret;
}


/* ***** Sets ***** */

void avrMatrix::setIdentityMatrix()
{
   for(int i = 0; i < this->rows; i++){
      for(int j = 0; j < this->clms; j++){
         if(i == j)  avrMatrix::add(1.0, i, j);
         else        avrMatrix::add(0.0, i, j);
      }
   }
}


/* ***** Statics ***** */

static ARMat *avrMatrixToArMat(double *mat, int rows, int clms){
   ARMat *dest = arMatrixAlloc(rows, clms);
   if(!dest)   throw NULL;

   for(int i = 0; i < dest->row; i++)
      for(int j = 0; j < dest->clm; j++)
         dest->m[i * dest->clm + j] = mat[i * clms + j];

   return dest;
}

static avrMatrix *arMatToAvrMatrix(ARMat *mat){
   avrMatrix *dest = new avrMatrix(mat->row, mat->clm);

   for(int i = 0; i < dest->row(); i++)
      for(int j = 0; j < dest->column(); j++)
         dest->add(mat->m[i * dest->column() + j], i, j);

   return dest;
}
