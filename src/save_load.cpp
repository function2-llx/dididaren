#include <map>
#include <cstdio>
#include <vector>

#include "save_load.h"

using namespace std;

void save_vector(FILE* fout, const vector<int> &v)
{
	fprintf(fout, "%d ",(int)v.size());
	for(int i=0;i<(int)v.size();i++)fprintf(fout, "%d ",v[i]);
	fprintf(fout, "\n");
}
void load_vector(FILE* fin, vector<int> &v)
{
	v.clear();
	int n,i,j;
	fscanf(fin, "%d",&n);
	for(i=0;i<n;i++)
	{
		fscanf(fin, "%d",&j);
		v.push_back(j);
	}
}
void save_vector_vector(FILE* fout, const vector<vector<int> > &v)
{
	fprintf(fout, "%d\n",(int)v.size());
	for(int i=0;i<(int)v.size();i++)save_vector(fout, v[i]);
	fprintf(fout, "\n");
}
void load_vector_vector(FILE* fin, vector<vector<int> > &v)
{
	v.clear();
	int n,i,j;
	fscanf(fin, "%d",&n);
	vector<int>ls;
	for(i=0;i<n;i++)
	{
		load_vector(fin, ls);
		v.push_back(ls);
	}
}
void save_vector_pair(FILE* fout, const vector<pair<int,int> > &v)
{
	fprintf(fout, "%d ",(int)v.size());
	for(int i=0;i<(int)v.size();i++)fprintf(fout, "%d %d ",v[i].first,v[i].second);
	fprintf(fout, "\n");
}
void load_vector_pair(FILE* fin, vector<pair<int,int> > &v)
{
	v.clear();
	int n,i,j,k;
	fscanf(fin, "%d",&n);
	for(i=0;i<n;i++)
	{
		fscanf(fin, "%d%d",&j,&k);
		v.push_back(make_pair(j,k));
	}
}
void save_map_int_pair(FILE* fout, map<int,pair<int,int> > &h)
{
	fprintf(fout, "%lu\n",h.size());
	map<int,pair<int,int> >::iterator iter;
	for(iter=h.begin();iter!=h.end();iter++)
		fprintf(fout, "%d %d %d\n",iter->first,iter->second.first,iter->second.second);
}
void load_map_int_pair(FILE* fin, map<int,pair<int,int> > &h)
{
	int n,i,j,k,l;
	fscanf(fin, "%d",&n);
	for(i=0;i<n;i++)
	{
		fscanf(fin, "%d%d%d",&j,&k,&l);
		h[j]=make_pair(k,l);
	}
}
void save_map_int_int(FILE* fout, map<int,int> &h)
{
	fprintf(fout, "%lu\n",h.size());
	map<int,int>::iterator iter;
	for(iter=h.begin();iter!=h.end();iter++)
		fprintf(fout, "%d %d\n",iter->first,iter->second);
}
void load_map_int_int(FILE* fin, map<int,int> &h)
{
	int n,i,j,k;
	fscanf(fin, "%d",&n);
	for(i=0;i<n;i++)
	{
		fscanf(fin, "%d%d",&j,&k);
		h[j]=k;
	}
}