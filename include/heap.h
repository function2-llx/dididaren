#pragma once

#include <vector>

using namespace std;

struct Heap//双指针大根堆
{
	Heap(){clear();}
	int n;
	vector<int>id,iid,a;//id[i]表示编号为i的元素在a中的位置，iid[i]表示a[i]的id（id:[0~n-1],a/iid:[1~n]）
	void clear()
	{
		n=1;
		id.clear();
		iid.clear();
		a.clear();
		iid.push_back(0);
		a.push_back(0);
	}
	void swap_(int x,int y)//交换a,id,iid
	{
		swap(a[x],a[y]);
		id[iid[x]]=y;
		id[iid[y]]=x;
		swap(iid[x],iid[y]);
	}
	void up(int x)//向上调整a[x]至合适位置
	{
		while(x>1)
		{
			if(a[x>>1]<a[x]){swap_(x>>1,x);x>>=1;}
			else return;
		}
	}
	void down(int x)//向上调整a[x]至合适位置
	{
		while((x<<1)<n)
		{
			int k;
			if((x<<1)+1>=n||a[x<<1]>a[(x<<1)+1])k=x<<1;
			else k=(x<<1)+1;
			if(a[x]<a[k]){swap_(x,k);x=k;}
			else return;
		}
	}
	int top(){return a[1];}
	int top_id(){return iid[1];}
	int size(){return n;}
	void change(int x,int num)//将编号x的点改为num
	{
		a[id[x]]=num;
		up(id[x]);
		down(id[x]);
	}
	void add(int x,int num)//向编号x的点加num
	{
		a[id[x]]+=num;
		up(id[x]);
		down(id[x]);
	}
	void push(int num)//压入一个数值为num的元素
	{
		id.push_back(n);
		iid.push_back(n-1);
		a.push_back(num);
		n++;
		up(n-1);
	}
};
