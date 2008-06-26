#ifndef __APPLE__
#include <malloc.h>
#else
#include <stdlib.h>
#endif
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

	imax = 0;
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
