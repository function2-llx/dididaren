#pragma once

#include <cstdio>
#include "global.h"
#include <iostream>

struct Matrix//矩阵
{
	Matrix():n(0),a(NULL){}
	~Matrix(){clear();}
	int n;//矩阵长宽
	int **a;
	void save(FILE* fout)
	{
		fprintf(fout, "%d\n",n);
		for(int i=0;i<n;i++)
		{
			for(int j=0;j<n;j++)
				fprintf(fout, "%d ",a[i][j]);
			fprintf(fout, "\n");
		}
	}
	void load(FILE* fin)
	{
		fscanf(fin, "%d",&n);
		a=new int*[n];
		for(int i=0;i<n;i++)a[i]=new int[n];
		for(int i=0;i<n;i++)
			for(int j=0;j<n;j++)
				fscanf(fin, "%d",&a[i][j]);
	}
	void cover(int x)
	{
		for(int i=0;i<n;i++)
			for(int j=0;j<n;j++)
				a[i][j]=x;
	}
	void init(int N)
	{
		clear();
		n=N;
		a=new int*[n];
		for(int i=0;i<n;i++)a[i]=new int[n];
		for(int i=0;i<n;i++)
			for(int j=0;j<n;j++)
				a[i][j]=INF;
		for(int i=0;i<n;i++)a[i][i]=0;
	}
	void clear()
	{
		for(int i=0;i<n;i++)delete [] a[i];
		delete [] a;
	}
	void floyd()//对矩阵a进行floyd
	{
		int i,j,k;
		for(k=0;k<n;k++)
			for(i=0;i<n;i++)
				for(j=0;j<n;j++)
					if(a[i][j]>a[i][k]+a[k][j])a[i][j]=a[i][k]+a[k][j];
	}
	void floyd(Matrix &order)//对矩阵a进行floyd,将方案记录到order中
	{
		int i,j,k;
		for(k=0;k<n;k++)
			for(i=0;i<n;i++)
				for(j=0;j<n;j++)
					if(a[i][j]>a[i][k]+a[k][j])
					{
						a[i][j]=a[i][k]+a[k][j];
						order.a[i][j]=k;
					}
	}
	// void write()
	// {
	// 	fprintf(stderr, "n=%d\n",n);
	// 	for(int i=0;i<n;i++,cerr<<endl)
	// 		for(int j=0;j<n;j++)fprintf(stderr, "%d ",a[i][j]);
	// }
	Matrix& operator =(const Matrix &m)
	{
		if(this!=(&m))
		{
			init(m.n);
			for(int i=0;i<n;i++)
				for(int j=0;j<n;j++)
					a[i][j]=m.a[i][j];
		}
		return *this;
	}
};