/*
mDet
mInv
mMul
mPCA
mSelfInv
mTrans
vHouse
vInnerP
vTridiag
*/

#include <cstdlib>
#include <cstdio>
#include <cmath>
///
#include <avrMath.h>
///

#define CHECK_CALC  0
#define MD_PI       3.14159265358979323846

#define MATRIX(name,x,y,width)  ( *(name + (width) * (x) + (y)) )

#define     VZERO           1e-16
#define     EPS             1e-6
#define     MAX_ITER        100

// ARVec
ARVec *arVecAlloc( int clm )
{
    ARVec     *v;

    v = (ARVec *)malloc(sizeof(ARVec));
    if( v == NULL ) return NULL;

    v->v = (double *)malloc(sizeof(double) * clm);
    if( v->v == NULL ) {
        free(v);
        return NULL;
    }

    v->clm = clm;

    return v;
}

int arVecDisp( ARVec *v )
{
    int    c;

    if( v == NULL ) return -1;

    printf(" === vector (%d) ===\n", v->clm);
    printf(" |");
    for( c = 0; c < v->clm; c++ ){
	printf( " %10g", v->v[c] );
    }
    printf(" |\n");
    printf(" ===================\n");

    return 0;
}

int arVecFree( ARVec *v )
{
    free( v->v );
    free( v );
    return 0;
}

double arVecHousehold( ARVec *x )
{
    double s, t;
    int    i;

    s = sqrt( arVecInnerproduct(x,x) );

    if( s != 0.0 ) {
        if(x->v[0] < 0) s = -s;
        x->v[0] += s;
        t = 1 / sqrt(x->v[0] * s);
        for( i = 0; i < x->clm; i++ ) {
            x->v[i] *= t;
        }
    }

    return(-s);
}

double arVecInnerproduct( ARVec *x, ARVec *y )
{
    double   result = 0.0;
    int      i;

    if( x->clm != y->clm ) exit(0);

    for( i = 0; i < x->clm; i++ ) {
        result += x->v[i] * y->v[i];
    }

    return( result );
}

int arVecTridiagonalize( ARMat *a, ARVec *d, ARVec *e )
{
    ARVec     wv1, wv2;
    double  *v;
    double  s, t, p, q;
    int     dim;
    int     i, j, k;

    if( a->clm != a->row )   return(-1);
    if( a->clm != d->clm )   return(-1);
    if( a->clm != e->clm+1 ) return(-1);
    dim = a->clm;

    for( k = 0; k < dim-2; k++ ) {
        v = &(a->m[k*dim]);
        d->v[k] = v[k];

        wv1.clm = dim-k-1;
        wv1.v = &(v[k+1]);
        e->v[k] = arVecHousehold(&wv1);
        if( e->v[k] == 0.0 ) continue;

        for( i = k+1; i < dim; i++ ) {
            s = 0.0;
            for( j = k+1; j < i; j++ ) {
                s += a->m[j*dim+i] * v[j];
            }
            for( j = i; j < dim; j++ ) {
                s += a->m[i*dim+j] * v[j];
            }
            d->v[i] = s;
        }

        wv1.clm = wv2.clm = dim-k-1;
        wv1.v = &(v[k+1]);
        wv2.v = &(d->v[k+1]);
        t = arVecInnerproduct( &wv1, &wv2 ) / 2;
        for( i = dim-1; i > k; i-- ) {
            p = v[i];
            q = d->v[i] -= t*p;
            for( j = i; j < dim; j++ ) {
                a->m[i*dim+j] -= p*(d->v[j]) + q*v[j];
            }
        }
    }

    if( dim >= 2) {
        d->v[dim-2] = a->m[(dim-2)*dim+(dim-2)];
        e->v[dim-2] = a->m[(dim-2)*dim+(dim-1)];
    }

    if( dim >= 1 ) d->v[dim-1] = a->m[(dim-1)*dim+(dim-1)];

    for( k = dim-1; k >= 0; k-- ) {
        v = &(a->m[k*dim]);
        if( k < dim-2 ) {
            for( i = k+1; i < dim; i++ ) {
                wv1.clm = wv2.clm = dim-k-1;
                wv1.v = &(v[k+1]);
                wv2.v = &(a->m[i*dim+k+1]);
                t = arVecInnerproduct( &wv1, &wv2 );
                for( j = k+1; j < dim; j++ ) a->m[i*dim+j] -= t * v[j];
            }
        }
        for( i = 0; i < dim; i++ ) v[i] = 0.0;
        v[k] = 1;
    }

    return(0);
}

// ARMat
/* === matrix definition ===

Input:
  <---- clm (Data dimention)--->
  [ 10  20  30 ] ^
  [ 20  10  15 ] |
  [ 12  23  13 ] row
  [ 20  10  15 ] |(Sample number)
  [ 13  14  15 ] v

Evec:
  <---- clm (Eigen vector)--->
  [ 10  20  30 ] ^
  [ 20  10  15 ] |
  [ 12  23  13 ] row
  [ 20  10  15 ] |(Number of egen vec)
  [ 13  14  15 ] v

Ev:
  <---- clm (Number of eigen vector)--->
  [ 10  20  30 ] eigen value

Mean:
  <---- clm (Data dimention)--->
  [ 10  20  30 ] mean value

=========================== */
ARMat *arMatrixAlloc(int row, int clm)
{
	ARMat *m;

	m = (ARMat *)malloc(sizeof(ARMat));
	if( m == NULL ) return NULL;

	m->m = (double *)malloc(sizeof(double) * row * clm);
	if(m->m == NULL) {
		free(m);
		return NULL;
	}
	else {
		m->row = row;
		m->clm = clm;
	}

	return m;
}

ARMat *arMatrixAllocDup(ARMat *source)
{
	ARMat *dest;

	dest = arMatrixAlloc(source->row, source->clm);
        if( dest == NULL ) return NULL;

	if( arMatrixDup(dest, source) < 0 ) {
		arMatrixFree(dest);
		return NULL;
	}

	return dest;
}

int arMatrixDisp(ARMat *m)
{
	int r, c;

	printf(" === matrix (%d,%d) ===\n", m->row, m->clm);
	for(r = 0; r < m->row; r++) {
		printf(" |");
		for(c = 0; c < m->clm; c++) {
			printf(" %10g", ARELEM0(m, r, c));
		}
		printf(" |\n");
	}
	printf(" ======================\n");

	return 0;
}

int arMatrixDup(ARMat *dest, ARMat *source)
{
	int r,c;

	if(dest->row != source->row || dest->clm != source->clm) {
		return -1;
	}
	for(r = 0; r < source->row; r++) {
		for(c = 0; c < source->clm; c++) {
			ARELEM0(dest, r, c) = ARELEM0(source, r, c);
		}
	}
	return 0;
}

int arMatrixFree(ARMat *m)
{
	free(m->m);
	free(m);

	return 0;
}

int arMatrixInv(ARMat *dest, ARMat *source)
{
	if(arMatrixDup(dest, source) < 0) return -1;

	return arMatrixSelfInv(dest);
}

int arMatrixMul(ARMat *dest, ARMat *a, ARMat *b)
{
	int r, c, i;

	if(a->clm != b->row || dest->row != a->row || dest->clm != b->clm) return -1;

	for(r = 0; r < dest->row; r++) {
		for(c = 0; c < dest->clm; c++) {
			ARELEM0(dest, r, c) = 0.0;
			for(i = 0; i < a->clm; i++) {
				ARELEM0(dest, r, c) += ARELEM0(a, r, i) * ARELEM0(b, i, c);
			}
		}
	}

	return 0;
}

ARMat *arMatrixAllocInv(ARMat *source)
{
	ARMat *dest;

	dest = arMatrixAlloc(source->row, source->row);
	if( dest == NULL ) return NULL;

	if( arMatrixInv(dest, source) < 0 ) {
		arMatrixFree( dest );
		return NULL;
	}

	return dest;
}

ARMat *arMatrixAllocMul(ARMat *a, ARMat *b)
{
	ARMat *dest;

	dest = arMatrixAlloc(a->row, b->clm);
	if( dest == NULL ) return NULL;

	if( arMatrixMul(dest, a, b) < 0 ) {
		arMatrixFree(dest);
		return NULL;
	}

	return dest;
}

ARMat *arMatrixAllocTrans(ARMat *source)
{
	ARMat *dest;

	dest = arMatrixAlloc(source->clm, source->row);
	if( dest == NULL ) return NULL;

	if( arMatrixTrans(dest, source) < 0 ) {
		arMatrixFree(dest);
		return NULL;
	}

	return dest;
}

ARMat *arMatrixAllocUnit(int dim)
{
	ARMat *m;

	m = arMatrixAlloc(dim, dim);
	if( m == NULL ) return NULL;

	if( arMatrixUnit(m) < 0 ) {
		arMatrixFree(m);
		return NULL;
	}

	return m;
}

static double mdet( double *ap, int dimen, int rowa );

double arMatrixDet(ARMat *m)
{

	if(m->row != m->clm) return 0.0;

	return mdet(m->m, m->row, m->row);
}

int arMatrixTrans(ARMat *dest, ARMat *source)
{
	int r, c;

	if(dest->row != source->clm || dest->clm != source->row) return -1;

	for(r = 0; r < dest->row; r++) {
		for(c = 0; c < dest->clm; c++) {
			ARELEM0(dest, r, c) = ARELEM0(source, c, r);
		}
	}

	return 0;
}

int arMatrixUnit(ARMat *unit)
{
	int r, c;

	if(unit->row != unit->clm) return -1;

	for(r = 0; r < unit->row; r++) {
		for(c = 0; c < unit->clm; c++) {
			if(r == c) {
				ARELEM0(unit, r, c) = 1.0;
			}
			else {
				ARELEM0(unit, r, c) = 0.0;
			}
		}
	}

	return 0;
}

static double mdet(double *ap, int dimen, int rowa)
/*  double  *ap;          input matrix */
/*  int     dimen;        Dimension of linre and row, those must be equal,
                          that is square matrix.       */
/*  int     rowa;         ROW Dimension of matrix A    */
{
    double det = 1.0;
    double work;
    int    is = 0;
    int    mmax;
    int    i, j, k;

    for(k = 0; k < dimen - 1; k++) {
        mmax = k;
        for(i = k + 1; i < dimen; i++)
            if (fabs(MATRIX(ap, i, k, rowa)) > fabs(MATRIX(ap, mmax, k, rowa)))
                mmax = i;
        if(mmax != k) {
            for (j = k; j < dimen; j++) {
                work = MATRIX(ap, k, j, rowa);
                MATRIX(ap, k, j, rowa) = MATRIX(ap, mmax, j, rowa);
                MATRIX(ap, mmax, j, rowa) = work;
            }
            is++;
        }
        for(i = k + 1; i < dimen; i++) {
            work = MATRIX(ap, i, k, rowa) / MATRIX(ap, k, k, rowa);
            for (j = k + 1; j < dimen; j++)
                MATRIX(ap, i, j, rowa) -= work * MATRIX(ap, k, j, rowa);
        }
    }
    for(i = 0; i < dimen; i++)
        det *= MATRIX(ap, i, i, rowa);
    for(i = 0; i < is; i++)
        det *= -1.0;
    return(det);
}

static int EX( ARMat *input, ARVec *mean );
static int CENTER( ARMat *inout, ARVec *mean );
static int PCA( ARMat *input, ARMat *output, ARVec *ev );
static int x_by_xt( ARMat *input, ARMat *output );
static int xt_by_x( ARMat *input, ARMat *output );
static int EV_create( ARMat *input, ARMat *u, ARMat *output, ARVec *ev );
static int QRM( ARMat *u, ARVec *ev );

int arMatrixPCA( ARMat *input, ARMat *evec, ARVec *ev, ARVec *mean )
{
    ARMat     *work;
    double  srow, sum;
    int     row, clm;
    int     check, rval;
    int     i;

    row = input->row;
    clm = input->clm;
    check = (row < clm)? row: clm;
    if( row < 2 || clm < 2 ) return(-1);
    if( evec->clm != input->clm || evec->row != check ) return(-1);
    if( ev->clm   != check )      return(-1);
    if( mean->clm != input->clm ) return(-1);

    work = arMatrixAllocDup( input );
    if( work == NULL ) return -1;

    srow = sqrt((double)row);
    if( EX( work, mean ) < 0 ) {
        arMatrixFree( work );
        return(-1);
    }
    if( CENTER( work, mean ) < 0 ) {
        arMatrixFree( work );
        return(-1);
    }
    for(i=0; i<row*clm; i++) work->m[i] /= srow;

    rval = PCA( work, evec, ev );
    arMatrixFree( work );

    sum = 0.0;
    for( i = 0; i < ev->clm; i++ ) sum += ev->v[i];
    for( i = 0; i < ev->clm; i++ ) ev->v[i] /= sum;

    return( rval );
}

int arMatrixPCA2( ARMat *input, ARMat *evec, ARVec *ev )
{
    ARMat   *work;
    // double  srow; // unreferenced
    double  sum;
    int     row, clm;
    int     check, rval;
    int     i;

    row = input->row;
    clm = input->clm;
    check = (row < clm)? row: clm;
    if( row < 2 || clm < 2 ) return(-1);
    if( evec->clm != input->clm || evec->row != check ) return(-1);
    if( ev->clm   != check )      return(-1);

    work = arMatrixAllocDup( input );
    if( work == NULL ) return -1;

/*
    srow = sqrt((double)row);
    for(i=0; i<row*clm; i++) work->m[i] /= srow;
*/

    rval = PCA( work, evec, ev );
    arMatrixFree( work );

    sum = 0.0;
    for( i = 0; i < ev->clm; i++ ) sum += ev->v[i];
    for( i = 0; i < ev->clm; i++ ) ev->v[i] /= sum;

    return( rval );
}

static int PCA( ARMat *input, ARMat *output, ARVec *ev )
{
    ARMat     *u;
    double  *m1, *m2;
    int     row, clm, min;
    int     i, j;

    row = input->row;
    clm = input->clm;
    min = (clm < row)? clm: row;
    if( row < 2 || clm < 2 )        return(-1);
    if( output->clm != input->clm ) return(-1);
    if( output->row != min )        return(-1);
    if( ev->clm != min )            return(-1);

    u = arMatrixAlloc( min, min );
    if( u->row != min || u->clm != min ) return(-1);
    if( row < clm ) {
        if( x_by_xt( input, u ) < 0 ) { arMatrixFree(u); return(-1); }
    }
    else {
        if( xt_by_x( input, u ) < 0 ) { arMatrixFree(u); return(-1); }
    }

    if( QRM( u, ev ) < 0 ) { arMatrixFree(u); return(-1); }

    if( row < clm ) {
        if( EV_create( input, u, output, ev ) < 0 ) {
            arMatrixFree(u);
            return(-1);
        }
    }
    else{
        m1 = u->m;
        m2 = output->m;
	for( i = 0; i < min; i++){
	    if( ev->v[i] < VZERO ) break;
            for( j = 0; j < min; j++ ) *(m2++) = *(m1++);
        }
	for( ; i < min; i++){
            ev->v[i] = 0.0;
            for( j = 0; j < min; j++ ) *(m2++) = 0.0;
        }
    }

    arMatrixFree(u);

    return( 0 );
}

static int EX( ARMat *input, ARVec *mean )
{
    double  *m, *v;
    int     row, clm;
    int     i, j;

    row = input->row;
    clm = input->clm;
    if( row <= 0 || clm <= 0 )  return(-1);
    if( mean->clm != clm )      return(-1);

    for( i = 0; i < clm; i++ ) mean->v[i] = 0.0;

    m = input->m;
    for( i = 0; i < row; i++ ) {
        v = mean->v;
        for( j = 0; j < clm; j++ ) *(v++) += *(m++);
    }

    for( i = 0; i < clm; i++ ) mean->v[i] /= row;

    return(0);
}

static int CENTER( ARMat *inout, ARVec *mean )
{
    double  *m, *v;
    int     row, clm;
    int     i, j;

    row = inout->row;
    clm = inout->clm;
    if( mean->clm != clm ) return(-1);

    m = inout->m;
    for( i = 0; i < row; i++ ) {
        v = mean->v;
        for( j = 0; j < clm; j++ ) *(m++) -= *(v++);
    }

    return(0);
}

static int x_by_xt( ARMat *input, ARMat *output )
{
    double  *in1, *in2, *out;
    int     row, clm;
    int     i, j, k;

    row = input->row;
    clm = input->clm;
    if( output->row != row || output->clm != row ) return(-1);

    out = output->m;
    for( i = 0; i < row; i++ ) {
        for( j = 0; j < row; j++ ) {
            if( j < i ) {
                *out = output->m[j*row+i];
            }
            else {
                in1 = &(input->m[clm*i]);
                in2 = &(input->m[clm*j]);
                *out = 0.0;
                for( k = 0; k < clm; k++ ) {
                    *out += *(in1++) * *(in2++);
                }
            }
            out++;
        }
    }

    return(0);
}

static int xt_by_x( ARMat *input, ARMat *output )
{
    double  *in1, *in2, *out;
    int     row, clm;
    int     i, j, k;

    row = input->row;
    clm = input->clm;
    if( output->row != clm || output->clm != clm ) return(-1);

    out = output->m;
    for( i = 0; i < clm; i++ ) {
        for( j = 0; j < clm; j++ ) {
            if( j < i ) {
                *out = output->m[j*clm+i];
            }
            else {
                in1 = &(input->m[i]);
                in2 = &(input->m[j]);
                *out = 0.0;
                for( k = 0; k < row; k++ ) {
                    *out += *in1 * *in2;
                    in1 += clm;
                    in2 += clm;
                }
            }
            out++;
        }
    }

    return(0);
}

static int EV_create( ARMat *input, ARMat *u, ARMat *output, ARVec *ev )
{
    double  *m, *m1, *m2;
    double  sum, work;
    int     row, clm;
    int     i, j, k;

    row = input->row;
    clm = input->clm;
    if( row <= 0 || clm <= 0 ) return(-1);
    if( u->row != row || u->clm != row ) return(-1);
    if( output->row != row || output->clm != clm ) return(-1);
    if( ev->clm != row ) return(-1);

    m = output->m;
    for( i = 0; i < row; i++ ) {
        if( ev->v[i] < VZERO ) break;
        work = 1 / sqrt(fabs(ev->v[i]));
        for( j = 0; j < clm; j++ ) {
            sum = 0.0;
            m1 = &(u->m[i*row]);
            m2 = &(input->m[j]);
            for( k = 0; k < row; k++ ) {
                sum += *m1 * *m2;
                m1++;
                m2 += clm;
            }
            *(m++) = sum * work;
        }
    }
    for( ; i < row; i++ ) {
        ev->v[i] = 0.0;
        for( j = 0; j < clm; j++ ) *(m++) = 0.0;
    }

    return(0);
}

static int QRM( ARMat *a, ARVec *dv )
{
    ARVec     *ev, ev1;
    double  w, t, s, x, y, c;
    double  *v1, *v2;
    int     dim, iter;
    int     i, j, k, h;

    dim = a->row;
    if( dim != a->clm || dim < 2 ) return(-1);
    if( dv->clm != dim ) return(-1);

    ev = arVecAlloc( dim );
    if( ev == NULL ) return(-1);

    ev1.clm = dim-1;
    ev1.v = &(ev->v[1]);
    if( arVecTridiagonalize( a, dv, &ev1 ) < 0 ) {
        arVecFree( ev );
        return(-1);
    }

    ev->v[0] = 0.0;
    for( h = dim-1; h > 0; h-- ) {
        j = h;
        while(j>0 && fabs(ev->v[j]) > EPS*(fabs(dv->v[j-1])+fabs(dv->v[j]))) j--;
        if( j == h ) continue;

        iter = 0;
        do{
            iter++;
            if( iter > MAX_ITER ) break;

            w = (dv->v[h-1] - dv->v[h]) / 2;
            t = ev->v[h] * ev->v[h];
            s = sqrt(w*w+t);
            if( w < 0 ) s = -s;
            x = dv->v[j] - dv->v[h] + t/(w+s);
            y = ev->v[j+1];
            for( k = j; k < h; k++ ) {
                if( fabs(x) >= fabs(y) ) {
		    if( fabs(x) > VZERO ) {
			t = -y / x;
			c = 1 / sqrt(t*t+1);
			s = t * c;
		    }
		    else{
			c = 1.0;
			s = 0.0;
		    }
                }
                else{
		    t = -x / y;
		    s = 1.0 / sqrt(t*t+1);
		    c = t * s;
                }
                w = dv->v[k] - dv->v[k+1];
                t = (w * s + 2 * c * ev->v[k+1]) * s;
                dv->v[k]   -= t;
                dv->v[k+1] += t;
                if( k > j) ev->v[k] = c * ev->v[k] - s * y;
                ev->v[k+1] += s * (c * w - 2 * s * ev->v[k+1]);

                for( i = 0; i < dim; i++ ) {
                    x = a->m[k*dim+i];
                    y = a->m[(k+1)*dim+i];
                    a->m[k*dim+i]     = c * x - s * y;
                    a->m[(k+1)*dim+i] = s * x + c * y;
                }
                if( k < h-1 ) {
                    x = ev->v[k+1];
                    y = -s * ev->v[k+2];
                    ev->v[k+2] *= c;
                }
            }
        } while(fabs(ev->v[h]) > EPS*(fabs(dv->v[h-1])+fabs(dv->v[h])));
    }

    for( k = 0; k < dim-1; k++ ) {
        h = k;
        t = dv->v[h];
        for( i = k+1; i < dim; i++ ) {
            if( dv->v[i] > t ) {
                h = i;
                t = dv->v[h];
            }
        }
        dv->v[h] = dv->v[k];
        dv->v[k] = t;
        v1 = &(a->m[h*dim]);
        v2 = &(a->m[k*dim]);
        for( i = 0; i < dim; i++ ) {
            w = *v1;
            *v1 = *v2;
            *v2 = w;
            v1++;
            v2++;
        }
    }

    arVecFree( ev );
    return(0);
}

static double *minv( double *ap, int dimen, int rowa );

int arMatrixSelfInv(ARMat *m)
{
	if(minv(m->m, m->row, m->row) == NULL) return -1;

	return 0;
}

/********************************/
/*                              */
/*    MATRIX inverse function   */
/*                              */
/********************************/
static double *minv( double *ap, int dimen, int rowa )
{
        double *wap, *wcp, *wbp;/* work pointer                 */
        int i,j,n,ip,nwork;
        int nos[50];
        double epsl;
        double p,pbuf,work;
        //double  fabs();

        epsl = 1.0e-10;         /* Threshold value      */

        switch (dimen) {
                case (0): return(NULL);                 /* check size */
                case (1): *ap = 1.0 / (*ap);
                          return(ap);                   /* 1 dimension */
        }

        for(n = 0; n < dimen ; n++)
                nos[n] = n;

        for(n = 0; n < dimen ; n++) {
                wcp = ap + n * rowa;

                for(i = n, wap = wcp, p = 0.0; i < dimen ; i++, wap += rowa)
                        if( p < ( pbuf = fabs(*wap)) ) {
                                p = pbuf;
                                ip = i;
                        }
                if (p <= epsl)
                        return(NULL);

                nwork = nos[ip];
                nos[ip] = nos[n];
                nos[n] = nwork;

                for(j = 0, wap = ap + ip * rowa, wbp = wcp; j < dimen ; j++) {
                        work = *wap;
                        *wap++ = *wbp;
                        *wbp++ = work;
                }

                for(j = 1, wap = wcp, work = *wcp; j < dimen ; j++, wap++)
                        *wap = *(wap + 1) / work;
                *wap = 1.0 / work;

                for(i = 0; i < dimen ; i++) {
                        if(i != n) {
                                wap = ap + i * rowa;
                                for(j = 1, wbp = wcp, work = *wap;
                                                j < dimen ; j++, wap++, wbp++)
                                        *wap = *(wap + 1) - work * (*wbp);
                                *wap = -work * (*wbp);
                        }
                }
        }

        for(n = 0; n < dimen ; n++) {
                for(j = n; j < dimen ; j++)
                        if( nos[j] == n) break;
                nos[j] = nos[n];
                for(i = 0, wap = ap + j, wbp = ap + n; i < dimen ;
                                        i++, wap += rowa, wbp += rowa) {
                        work = *wap;
                        *wap = *wbp;
                        *wbp = work;
                }
        }
        return(ap);
}

// RA Math
double arModifyMatrix( double rot[3][3], double trans[3], double cpara[3][4], double vertex[][3], double pos2d[][2], int num )
{
    double    factor;
    double    a, b, c;
    double    a1, b1, c1;
    double    a2, b2, c2;
    double    ma = 0.0, mb = 0.0, mc = 0.0;
    double    combo[3][4];
    double    hx, hy, h, x, y;
    double    err, minerr;
    int       t1, t2, t3;
    int       s1 = 0, s2 = 0, s3 = 0;
    int       i, j;

    arGetAngle( rot, &a, &b, &c );

    a2 = a;
    b2 = b;
    c2 = c;
    factor = 10.0*MD_PI/180.0;
    for( j = 0; j < 10; j++ ) {
        minerr = 1000000000.0;
        for(t1=-1;t1<=1;t1++) {
        for(t2=-1;t2<=1;t2++) {
        for(t3=-1;t3<=1;t3++) {
            a1 = a2 + factor*t1;
            b1 = b2 + factor*t2;
            c1 = c2 + factor*t3;
            arGetNewMatrix( a1, b1, c1, trans, NULL, cpara, combo );

            err = 0.0;
            for( i = 0; i < num; i++ ) {
                hx = combo[0][0] * vertex[i][0]
                   + combo[0][1] * vertex[i][1]
                   + combo[0][2] * vertex[i][2]
                   + combo[0][3];
                hy = combo[1][0] * vertex[i][0]
                   + combo[1][1] * vertex[i][1]
                   + combo[1][2] * vertex[i][2]
                   + combo[1][3];
                h  = combo[2][0] * vertex[i][0]
                   + combo[2][1] * vertex[i][1]
                   + combo[2][2] * vertex[i][2]
                   + combo[2][3];
                x = hx / h;
                y = hy / h;

                err += (pos2d[i][0] - x) * (pos2d[i][0] - x) + (pos2d[i][1] - y) * (pos2d[i][1] - y);
            }

            if( err < minerr ) {
                minerr = err;
                ma = a1;
                mb = b1;
                mc = c1;
                s1 = t1; s2 = t2; s3 = t3;
            }
        }
        }
        }

        if( s1 == 0 && s2 == 0 && s3 == 0 ) factor *= 0.5;
        a2 = ma;
        b2 = mb;
        c2 = mc;
    }

    arGetRot( ma, mb, mc, rot );

    return minerr/num;
}

int arGetAngle( double rot[3][3], double *wa, double *wb, double *wc )
{
    double      a, b, c;
    double      sina, cosa, sinb, cosb, sinc, cosc;
#if CHECK_CALC
double   w[3];
int      i;
for(i=0;i<3;i++) w[i] = rot[i][0];
for(i=0;i<3;i++) rot[i][0] = rot[i][1];
for(i=0;i<3;i++) rot[i][1] = rot[i][2];
for(i=0;i<3;i++) rot[i][2] = w[i];
#endif

    if( rot[2][2] > 1.0 ) {
        /* printf("cos(beta) = %f\n", rot[2][2]); */
        rot[2][2] = 1.0;
    }
    else if( rot[2][2] < -1.0 ) {
        /* printf("cos(beta) = %f\n", rot[2][2]); */
        rot[2][2] = -1.0;
    }
    cosb = rot[2][2];
    b = acos( cosb );
    sinb = sin( b );
    if( b >= 0.000001 || b <= -0.000001) {
        cosa = rot[0][2] / sinb;
        sina = rot[1][2] / sinb;
        if( cosa > 1.0 ) {
            /* printf("cos(alph) = %f\n", cosa); */
            cosa = 1.0;
            sina = 0.0;
        }
        if( cosa < -1.0 ) {
            /* printf("cos(alph) = %f\n", cosa); */
            cosa = -1.0;
            sina =  0.0;
        }
        if( sina > 1.0 ) {
            /* printf("sin(alph) = %f\n", sina); */
            sina = 1.0;
            cosa = 0.0;
        }
        if( sina < -1.0 ) {
            /* printf("sin(alph) = %f\n", sina); */
            sina = -1.0;
            cosa =  0.0;
        }
        a = acos( cosa );
        if( sina < 0 ) a = -a;

        sinc =  (rot[2][1]*rot[0][2]-rot[2][0]*rot[1][2])
              / (rot[0][2]*rot[0][2]+rot[1][2]*rot[1][2]);
        cosc =  -(rot[0][2]*rot[2][0]+rot[1][2]*rot[2][1])
               / (rot[0][2]*rot[0][2]+rot[1][2]*rot[1][2]);
        if( cosc > 1.0 ) {
            /* printf("cos(r) = %f\n", cosc); */
            cosc = 1.0;
            sinc = 0.0;
        }
        if( cosc < -1.0 ) {
            /* printf("cos(r) = %f\n", cosc); */
            cosc = -1.0;
            sinc =  0.0;
        }
        if( sinc > 1.0 ) {
            /* printf("sin(r) = %f\n", sinc); */
            sinc = 1.0;
            cosc = 0.0;
        }
        if( sinc < -1.0 ) {
            /* printf("sin(r) = %f\n", sinc); */
            sinc = -1.0;
            cosc =  0.0;
        }
        c = acos( cosc );
        if( sinc < 0 ) c = -c;
    }
    else {
        a = b = 0.0;
        cosa = cosb = 1.0;
        sina = sinb = 0.0;
        cosc = rot[0][0];
        sinc = rot[1][0];
        if( cosc > 1.0 ) {
            /* printf("cos(r) = %f\n", cosc); */
            cosc = 1.0;
            sinc = 0.0;
        }
        if( cosc < -1.0 ) {
            /* printf("cos(r) = %f\n", cosc); */
            cosc = -1.0;
            sinc =  0.0;
        }
        if( sinc > 1.0 ) {
            /* printf("sin(r) = %f\n", sinc); */
            sinc = 1.0;
            cosc = 0.0;
        }
        if( sinc < -1.0 ) {
            /* printf("sin(r) = %f\n", sinc); */
            sinc = -1.0;
            cosc =  0.0;
        }
        c = acos( cosc );
        if( sinc < 0 ) c = -c;
    }

    *wa = a;
    *wb = b;
    *wc = c;

    return 0;
}

int arGetRot( double a, double b, double c, double rot[3][3] )
{
    double   sina, sinb, sinc;
    double   cosa, cosb, cosc;
#if CHECK_CALC
    double   w[3];
    int      i;
#endif

    sina = sin(a); cosa = cos(a);
    sinb = sin(b); cosb = cos(b);
    sinc = sin(c); cosc = cos(c);
    rot[0][0] = cosa*cosa*cosb*cosc+sina*sina*cosc+sina*cosa*cosb*sinc-sina*cosa*sinc;
    rot[0][1] = -cosa*cosa*cosb*sinc-sina*sina*sinc+sina*cosa*cosb*cosc-sina*cosa*cosc;
    rot[0][2] = cosa*sinb;
    rot[1][0] = sina*cosa*cosb*cosc-sina*cosa*cosc+sina*sina*cosb*sinc+cosa*cosa*sinc;
    rot[1][1] = -sina*cosa*cosb*sinc+sina*cosa*sinc+sina*sina*cosb*cosc+cosa*cosa*cosc;
    rot[1][2] = sina*sinb;
    rot[2][0] = -cosa*sinb*cosc-sina*sinb*sinc;
    rot[2][1] = cosa*sinb*sinc-sina*sinb*cosc;
    rot[2][2] = cosb;

#if CHECK_CALC
    for(i=0;i<3;i++) w[i] = rot[i][2];
    for(i=0;i<3;i++) rot[i][2] = rot[i][1];
    for(i=0;i<3;i++) rot[i][1] = rot[i][0];
    for(i=0;i<3;i++) rot[i][0] = w[i];
#endif

    return 0;
}

int arGetNewMatrix( double a, double b, double c, double trans[3], double trans2[3][4], double cpara[3][4], double ret[3][4] )
{
    double   cpara2[3][4];
    double   rot[3][3];
    int      i, j;

    arGetRot( a, b, c, rot );

    if( trans2 != NULL ) {
        for( j = 0; j < 3; j++ ) {
            for( i = 0; i < 4; i++ ) {
                cpara2[j][i] = cpara[j][0] * trans2[0][i]
                             + cpara[j][1] * trans2[1][i]
                             + cpara[j][2] * trans2[2][i];
            }
        }
    }
    else {
        for( j = 0; j < 3; j++ ) {
            for( i = 0; i < 4; i++ ) {
                cpara2[j][i] = cpara[j][i];
            }
        }
    }

    for( j = 0; j < 3; j++ ) {
        for( i = 0; i < 3; i++ ) {
            ret[j][i] = cpara2[j][0] * rot[0][i]
                      + cpara2[j][1] * rot[1][i]
                      + cpara2[j][2] * rot[2][i];
        }
        ret[j][3] = cpara2[j][0] * trans[0]
                  + cpara2[j][1] * trans[1]
                  + cpara2[j][2] * trans[2]
                  + cpara2[j][3];
    }

    return(0);
}
