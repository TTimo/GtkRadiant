
#include <stdlib.h>
#include <limits.h>
#include <float.h>

#include "mathlib.h"

#define TINY FLT_MIN

void lubksb(float **a, int n, int *indx, float b[])
// Solves the set of n linear equations A.X=B. Here a[n][n] is input, not as the matrix
// A but rather as its LU decomposition determined by the routine ludcmp. indx[n] is input
// as the permutation vector returned by ludcmp. b[n] is input as the right-hand side vector
// B, and returns with the solution vector X. a, n and indx are not modified by this routine
// and can be left in place for successive calls with different right-hand sides b. This routine takes
// into account the possibility that b will begin with many zero elements, so it is efficient for use
// in matrix inversion
{
	int i,ii=-1,ip,j;
	float sum;

	for (i=0;i<n;i++) {
		ip=indx[i];
		sum=b[ip];
		b[ip]=b[i];
		if (ii>=0)
			for (j=ii;j<i;j++) sum -= a[i][j]*b[j];
		else if (sum) ii=i;
		b[i]=sum;
	}
	for (i=n-1;i>=0;i--) {
		sum=b[i];
		for (j=i+1;j<n;j++) sum -= a[i][j]*b[j];
		b[i]=sum/a[i][i];
	}
}
/* (C) Copr. 1986-92 Numerical Recipes Software */


int ludcmp(float **a, int n, int *indx, float *d)
// given a matrix a[n][n] this routine replaces it with the LU decomposition of a rowwise
// permutation of itself. a and n are input. a is output, arranged as in above equation;
// indx[n] is an output vector that records the row permutation effected by the partial
// pivoting; d is output as +/-1 depending on whether the number of row interchanges was even
// or odd, respectively. This routine is used in combination with lubksb to solve linear
// equations or invert a matrix.
{
	int i,imax,j,k;
	float big,dum,sum,temp;
	float *vv;

	vv=(float*)malloc(sizeof(float)*n);
	*d=1.0;
	for (i=0;i<n;i++) {
		big=0.0;
		for (j=0;j<n;j++)
			if ((temp=(float)fabs(a[i][j])) > big) big=temp;
		if (big == 0.0) return 1;
		vv[i]=1.0f/big;
	}
	for (j=0;j<n;j++) {
		for (i=0;i<j;i++) {
			sum=a[i][j];
			for (k=0;k<i;k++) sum -= a[i][k]*a[k][j];
			a[i][j]=sum;
		}
		big=0.0;
		for (i=j;i<n;i++) {
			sum=a[i][j];
			for (k=0;k<j;k++)
				sum -= a[i][k]*a[k][j];
			a[i][j]=sum;
			if ( (dum=vv[i]*(float)fabs(sum)) >= big) {
				big=dum;
				imax=i;
			}
		}
		if (j != imax) {
			for (k=0;k<n;k++) {
				dum=a[imax][k];
				a[imax][k]=a[j][k];
				a[j][k]=dum;
			}
			*d = -(*d);
			vv[imax]=vv[j];
		}
		indx[j]=imax;
		if (a[j][j] == 0.0) a[j][j]=TINY;
		if (j != n) {
			dum=1.0f/(a[j][j]);
			for (i=j+1;i<n;i++) a[i][j] *= dum;
		}
	}
	free(vv);
  return 0;
}
/* (C) Copr. 1986-92 Numerical Recipes Software */


/*
void ludcmp(float **a, int n, int *indx, float *d)
//Given a matrix a[1..n][1..n], this routine replaces it by the LU decomposition of a rowwise
//permutation of itself. a and n are input. a is output, arranged as in equation (2.3.14) above;
//indx[1..n] is an output vector that records the row permutation eected by the partial
//pivoting; d is output as .1 depending on whether the number of row interchanges was even
//or odd, respectively. This routine is used in combination with lubksb to solve linear equations
//or invert a matrix.
{
  int i,imax,j,k;
  float big,dum,sum,temp;
  float *vv; //vv stores the implicit scaling of each row.
  vv=vector(1,n);
  *d=1.0; //No row interchanges yet.
  for (i=1;i<=n;i++) { //Loop over rows to get the implicit scaling information.
    big=0.0;
    for (j=1;j<=n;j++)
      if ((temp=fabs(a[i][j])) > big) big=temp;
      if (big == 0.0) nrerror("Singular matrix in routine ludcmp");
      //No nonzero largest element.
      vv[i]=1.0/big; //Save the scaling.
  }
  for (j=1;j<=n;j++) { //This is the loop over columns of Crout's method.
    for (i=1;i<j;i++) { //This is equation (2.3.12) except for i = j.
      sum=a[i][j];
      for (k=1;k<i;k++) sum -= a[i][k]*a[k][j];
      a[i][j]=sum;
    }
    big=0.0; //Initialize for the search for largest pivot element.
    for (i=j;i<=n;i++) { //This is i = j of equation (2.3.12) and i = j+1 : ::N
      of equation (2.3.13). sum=a[i][j];
      for (k=1;k<j;k++)
        sum -= a[i][k]*a[k][j];
      a[i][j]=sum;
      if ( (dum=vv[i]*fabs(sum)) >= big) {
        //Is the figure of merit for the pivot better than the best so far?
        big=dum;
        imax=i;
      }
    }
    if (j != imax) { //Do we need to interchange rows?
      for (k=1;k<=n;k++) { Yes, do so...
        dum=a[imax][k];
      a[imax][k]=a[j][k];
      a[j][k]=dum;
      }
      *d = -(*d); //...and change the parity of d.
      vv[imax]=vv[j]; //Also interchange the scale factor.
    }
    indx[j]=imax;
    if (a[j][j] == 0.0) a[j][j]=TINY;
    //If the pivot element is zero the matrix is singular (at least to the precision of the
    //  algorithm). For some applications on singular matrices, it is desirable to substitute
    //  TINY for zero.
    if (j != n) { //Now, finally, divide by the pivot element.
      dum=1.0/(a[j][j]);
      for (i=j+1;i<=n;i++) a[i][j] *= dum;
    }
  } //Go back for the next column in the reduction.
  free_vector(vv,1,n);
}

void lubksb(float **a, int n, int *indx, float b[])
//Solves the set of n linear equations A.X = B. Here a[1..n][1..n] is input, not as the matrix
//A but rather as its LU decomposition, determined by the routine ludcmp. indx[1..n] is input
//as the permutation vector returned by ludcmp. b[1..n] is input as the right-hand side vector
//B, and returns with the solution vector X. a, n, and indx are not modied by this routine
//and can be left in place for successive calls with dierent right-hand sides b. This routine takes
//into account the possibility that b will begin with many zero elements, so it is e.cient for use
//in matrix inversion.
{
  int i,ii=0,ip,j;
  float sum;
  for (i=1;i<=n;i++) { //When ii is set to a positive value, it will become the
    //index of the first nonvanishing element of b. Wenow
    //do the forward substitution, equation (2.3.6). The
    //only new wrinkle is to unscramble the permutation
    //as we go.
    ip=indx[i];
    sum=b[ip];
    b[ip]=b[i];
    if (ii)
      for (j=ii;j<=i-1;j++) sum -= a[i][j]*b[j];
    else if (sum) ii=i; //A nonzero element was encountered, so from now on we
      //will have to do the sums in the loop above. b[i]=sum;
  }
  for (i=n;i>=1;i--) { //Now we do the backsubstitution, equation (2.3.7).
    sum=b[i];
    for (j=i+1;j<=n;j++) sum -= a[i][j]*b[j];
    b[i]=sum/a[i][i]; //Store a component of the solution vector X.
  } //All done!
}

void bleh()
{
  #define N ...
  float **a,**y,d,*col;
  int i,j,*indx;
  ...
  ludcmp(a,N,indx,&d); //Decompose the matrix just once.
  for(j=1;j<=N;j++) { //Find inverse by columns.
    for(i=1;i<=N;i++) col[i]=0.0;
    col[j]=1.0;
    lubksb(a,N,indx,col);
    for(i=1;i<=N;i++) y[i][j]=col[i];
  }
}
*/
