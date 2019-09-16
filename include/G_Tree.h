#pragma once

#include <queue>
#include <vector>
#include <cassert>
#include <exception>
#include <algorithm>
#include "graph.h"
#include "matrix.h"

using namespace std;

const static int lim = 100;

static int rootp = 0;

struct G_Tree
{
	int root;
	vector<int>id_in_node;//真实结点所在的叶子结点编号
	vector<vector<int> >car_in_node;//用于挂border法KNN，记录每个节点上车的编号
	vector<int>car_offset;//用于记录车id距离车所在的node的距离
	struct Node
	{
		Node(){clear();}
		int part;//结点的儿子个数
		int n,father,*son=nullptr,deep;//n:子图结点数,father父节点编号,son[2]左右儿子编号,deep结点所在树深度
		Graph G;//子图
		vector<int>color;//结点分别在那个儿子中
		Matrix dist,order;//border距离,以及border做floyd的中间点k方案,order=(-1:直接相连)|(-2:在父节点中相连)|(-3:在子结点中相连)|(-INF:无方案)
		map<int,pair<int,int> >borders;//first:真实border编号;second:<border序列的编号,对应子图中的编号(0~n-1)>
		vector<int>border_in_father,border_in_son,border_id,border_id_innode;//borders在父亲与儿子borders列表中的编号,border在原图中的编号,border在结点中的编号
		vector<int>path_record;//路径查询辅助数组，无意义
		int catch_id,catch_bound;//当前catch所保存的起点编号，当前catch所保存的已更新的catch的bound(<bound的begin已更新end)
		vector<int>catch_dist;//当前catch_dist保存的是从图G中结点catch_id到每个border的距离，其中只有值小于等于catch_bound的部分值是正确的
		vector<int>border_son_id;//当前border所在的儿子结点的编号
		int min_border_dist;//当前结点随catch缓存的边界点最小的距离(用于KNN剪枝)
		vector<pair<int,int> >min_car_dist;//车辆集合中距离每个border最近的<car_dist,node_id>
        
		void save(FILE* fout)
		{
			fprintf(fout, "%d %d %d %d %d %d %d\n",n,father,part,deep,catch_id,catch_bound,min_border_dist);
			for(int i=0;i<part;i++)fprintf(fout, "%d ",son[i]);
            fputc('\n', fout);
			save_vector(fout, color);
			dist.save(fout);
			order.save(fout);
			save_map_int_pair(fout, borders);
			save_vector(fout, border_in_father);
			save_vector(fout, border_in_son);
			save_vector(fout, border_id);
			save_vector(fout, border_id_innode);
			save_vector(fout, path_record);
			save_vector(fout, catch_dist);
			save_vector_pair(fout, min_car_dist);
		}
		void load(FILE* fin)
		{
			fscanf(fin, "%d%d%d%d%d%d%d",&n,&father,&part,&deep,&catch_id,&catch_bound,&min_border_dist);
			if(son!=nullptr)delete[] son;
            // if (part > lim)
            //     cerr << "size: " << part << endl;
			son=new int[part];
			for(int i=0;i<part;i++)fscanf(fin, "%d",&son[i]);
			load_vector(fin, color);
			dist.load(fin);
			order.load(fin);
			load_map_int_pair(fin, borders);
			load_vector(fin, border_in_father);
			load_vector(fin, border_in_son);
			load_vector(fin, border_id);
			load_vector(fin, border_id_innode);
			load_vector(fin, path_record);
			load_vector(fin, catch_dist);
			load_vector_pair(fin, min_car_dist);
		}
		void init(int n)
		{
			part=n;
            // if (n > lim)
            //     cerr << "size: " << n << endl;
			son=new int[n];
			for(int i=0;i<n;i++)son[i]=0;
		}
		void clear()
		{
			part=n=father=deep=0;
            if (son != nullptr)
                delete[] son;
			dist.clear();
			order.clear();
			G.clear();
			color.clear();
			borders.clear();
			border_in_father.clear();
			border_in_son.clear();
			border_id.clear();
			border_id_innode.clear();
			catch_dist.clear();
			catch_id=-1;
		}
		void make_border_edge()//将border之间直接相连的边更新至dist(build_dist1)
		{
			int i,j;
			map<int,pair<int,int> >::iterator iter;
			for(iter=borders.begin();iter!=borders.end();iter++)
			{
				i=iter->second.second;
				for(j=G.head[i];j;j=G.next[j])
					if(color[i]!=color[G.list[j]])
					{
						int id1,id2;
						id1=iter->second.first;
						id2=borders[G.id[G.list[j]]].first;
						if(dist.a[id1][id2]>G.cost[j])
						{
							dist.a[id1][id2]=G.cost[j];
							order.a[id1][id2]=-1;
						}
					}
			}
		}
	};
	int node_tot,node_size;
	Node *node;
	void save(FILE* fout)
	{
		G.save(fout);

		fprintf(fout, "%d %d %d\n",root,node_tot,node_size);
		save_vector(fout, id_in_node);
		save_vector_vector(fout, car_in_node);
		save_vector(fout, car_offset);
		for(int i=0;i<node_size;i++)
		{
			fprintf(fout, "\n");
			node[i].save(fout);
		}
	}
	void load(FILE* fin)
	{
		fprintf(stderr, "load_begin\n");
		G.load(fin);
        // cerr << "G loaded" << endl;
		fscanf(fin, "%d%d%d",&root,&node_tot,&node_size);
		load_vector(fin, id_in_node);
		load_vector_vector(fin, car_in_node);
		load_vector(fin, car_offset);
        // cerr << "G.n: " << G.n << endl;
        // node_size
        // cerr << "node size: " << node_size << endl;
		node=new Node[G.n*2+2];
        // cerr << "you de mei de" << endl;
		for(int i=0;i<node_size;i++)
		{
            // cerr << i << endl;
			node[i].load(fin);
		}
	}

	void add_border(int x,int id,int id2)//向x点的border集合中加入一个新真实id,在子图的虚拟id为id2,并对其标号为border中的编号
	{
		map<int,pair<int,int> >::iterator iter;
		iter=node[x].borders.find(id);
		if(iter==node[x].borders.end())
		{
			pair<int,int> second=make_pair((int)node[x].borders.size(),id2);
			node[x].borders[id]=second;
		}
	}
	void make_border(int x,const vector<int> &color)//计算点x的border集合，二部图之间的边集为E
	{
		for(int i=0;i<node[x].G.n;i++)
		{
			int id=node[x].G.id[i];
			for(int j=node[x].G.head[i];j;j=node[x].G.next[j])
				if(color[i]!=color[node[x].G.list[j]])
				{
					add_border(x,id,i);
					break;
				}
		}
	}
	int partition_root(int x=1)//返回该结点在不超过Additional_Memory限制下最多可以划分为多少块
	{
		if((long long)node[x].G.n*node[x].G.n<=Additional_Memory)return node[x].G.n;
		int l=2,r=max(2,(int)sqrt(Additional_Memory)),mid;//二分块数
		while(l<r)
		{
			mid=(l+r+1)>>1;
			int num=node[x].G.Split_Borders(mid);
			if(num*num>Additional_Memory)r=mid-1;
			else l=mid;
		}
		return l;
	}
	void build(int x=1,int f=1,const Graph &g=G)//递归建树，当前结点x,叶子规模f,当前结点的子图g
	{
		if(x==1)//x为根
		{
            // if (G.n * 2 + 2 > lim)
            //     cerr << "size: " << G.n * 2 + 2 << endl;
			node=new Node[G.n*2+2];
			node_size=G.n*2;
			node_tot=2;
			root=1;
			node[x].deep=1;
			node[1].G=g;
		}
		else//x非根
		{
			node[x].deep=node[node[x].father].deep+1;
		}
		node[x].n=node[x].G.n;
		//决策根节点的儿子数
		if(x==root&&Optimization_G_tree_Search)
		{
			node[x].init(partition_root(x));
			fprintf(stderr, "root's part:%d\n",node[x].part);
			rootp = node[x].part;
		}
		else if(node[x].G.n<Naive_Split_Limit)node[x].init(node[x].n);
		else node[x].init(Partition_Part);
		if(node[x].n>50)fprintf(stderr, "x=%d deep=%d n=%d ",x,node[x].deep,node[x].G.n);
		if(node[x].n>f)
		{
			//子结点标号
			int top=node_tot;
			for(int i=0;i<node[x].part;i++)
			{
				node[x].son[i]=top+i;
				node[top+i].father=x;
			}
			node_tot+=node[x].part;
			//添加介于两块之间的border
			Graph **graph;
            // if (node[x].part > lim)
            //     cerr << "size: " << node[x].part << endl;
			graph=new Graph*[node[x].part];
			for(int i=0;i<node[x].part;i++)graph[i]=&node[node[x].son[i]].G;
			node[x].color=node[x].G.Split(graph,node[x].part);
			delete [] graph;
			make_border(x,node[x].color);
			if(node[x].n>50)fprintf(stderr, "border=%lu\n",node[x].borders.size());
			//传递border至子结点
			map<int,pair<int,int> >::iterator iter;
			for(iter=node[x].borders.begin();iter!=node[x].borders.end();iter++)
			{
				//fprintf(fout, "(%d,%d,%d)",iter->first,iter->second.first,iter->second.second);
				node[x].color[iter->second.second]=-node[x].color[iter->second.second]-1;
			}
			//cerr<<endl;
			vector<int>tot(node[x].part,0);
			for(int i=0;i<node[x].n;i++)
			{
				if(node[x].color[i]<0)
				{
					node[x].color[i]=-node[x].color[i]-1;
					add_border(node[x].son[node[x].color[i]],node[x].G.id[i],tot[node[x].color[i]]);
				}
				tot[node[x].color[i]]++;
			}
			//递归子结点
			for(int i=0;i<node[x].part;i++)
				build(node[x].son[i]);
		}
		else if(node[x].n>50)cerr<<endl;
		node[x].dist.init(node[x].borders.size());
		node[x].order.init(node[x].borders.size());
		node[x].order.cover(-INF);
		if(x==1)//x为根建立dist
		{
			for(int i=1;i<min(1000,node_tot-1);i++)
				if(node[i].n>50)
				{
					fprintf(stderr, "x=%d deep=%d n=%d ",i,node[i].deep,node[i].G.n);
					fprintf(stderr, "border=%lu real_border=%d\n",node[i].borders.size(),real_border_number(i));
				}
			fprintf(stderr, "begin_build_border_in_father_son\n");
			build_border_in_father_son();
			fprintf(stderr, "begin_build_dist\n");
			build_dist1(root);
			fprintf(stderr, "begin_build_dist2\n");
			build_dist2(root);
			//计算查询每个结点所在的叶子编号
			id_in_node.clear();
			for(int i=0;i<node[root].G.n;i++)id_in_node.push_back(-1);
			for(int i=1;i<node_tot;i++)
				if(node[i].G.n==1)
					id_in_node[node[i].G.id[0]]=i;
			//建立catch
			for(int i=1;i<=node_tot;i++)
			{
				node[i].catch_id=-1;
				for(int j=0;j<node[i].borders.size();j++)
				{
					node[i].catch_dist.push_back(0);
					node[i].min_car_dist.push_back(make_pair(INF,-1));
				}
			}
			{
				//建立car_in_node;
				vector<int>empty_vector;
				empty_vector.clear();
				car_in_node.clear();
				for(int i=0;i<G.n;i++)car_in_node.push_back(empty_vector);
			}
		}
		
	}
	void build_dist1(int x=1)//自下而上归并子图内部dist
	{
		//计算子结点内部dist并传递给x
		for(int i=0;i<node[x].part;i++)if(node[x].son[i])build_dist1(node[x].son[i]);
		if(node[x].son[0])//非叶子
		{
			//建立x子结点之间的边
			node[x].make_border_edge();
			//计算内部真实dist矩阵
			node[x].dist.floyd(node[x].order);
		}
		// else ;//叶子
		//向父节点传递内部边权
		if(node[x].father)
		{
			int y=node[x].father,i,j;
			map<int,pair<int,int> >::iterator x_iter1,y_iter1;
			vector<int>id_in_fa(node[x].borders.size());
			//计算子图border在父节点border序列中的编号,不存在为-1
			for(x_iter1=node[x].borders.begin();x_iter1!=node[x].borders.end();x_iter1++)
			{
				y_iter1=node[y].borders.find(x_iter1->first);
				if(y_iter1==node[y].borders.end())id_in_fa[x_iter1->second.first]=-1;
				else id_in_fa[x_iter1->second.first]=y_iter1->second.first;
			}
			//将子图内部的全连接边权传递给父亲
			for(i=0;i<(int)node[x].borders.size();i++)
				for(j=0;j<(int)node[x].borders.size();j++)
					if(id_in_fa[i]!=-1&&id_in_fa[j]!=-1)
					{
						int *p=&node[y].dist.a[id_in_fa[i]][id_in_fa[j]];
						if((*p)>node[x].dist.a[i][j])
						{
							(*p)=node[x].dist.a[i][j];
							node[y].order.a[id_in_fa[i]][id_in_fa[j]]=-3;
						}
					}
		}
		return;
	}
	void build_dist2(int x=1)//自上而下修正子图外部dist
	{
		if(x!=root)node[x].dist.floyd(node[x].order);
		if(node[x].son[0])
		{
			//计算此节点border编号在子图中border序列的编号
			vector<int>id_(node[x].borders.size());
			vector<int>color_(node[x].borders.size());
			map<int,pair<int,int> >::iterator iter1,iter2;
			for(iter1=node[x].borders.begin();iter1!=node[x].borders.end();iter1++)
			{
				int c=node[x].color[iter1->second.second];
				color_[iter1->second.first]=c;
				int y=node[x].son[c];
				id_[iter1->second.first]=node[y].borders[iter1->first].first;
			}
			//修正子图边权
			for(int i=0;i<(int)node[x].borders.size();i++)
				for(int j=0;j<(int)node[x].borders.size();j++)
					if(color_[i]==color_[j])
					{
						int y=node[x].son[color_[i]];
						int *p=&node[y].dist.a[id_[i]][id_[j]];
						if((*p)>node[x].dist.a[i][j])
						{
							(*p)=node[x].dist.a[i][j];
							node[y].order.a[id_[i]][id_[j]]=-2;
						}
					}
			//递归子节点
			for(int i=0;i<node[x].part;i++)
				if(node[x].son[i])build_dist2(node[x].son[i]);
		}
	}
	void build_border_in_father_son()//计算每个结点border在父亲/儿子borders数组中的编号
	{
		int i,j,x,y;
		for(x=1;x<node_tot;x++)
		{
			for(i=0;i<node[x].borders.size();i++)node[x].border_id.push_back(0);
			for(i=0;i<node[x].borders.size();i++)node[x].border_id_innode.push_back(0);
			for(map<int,pair<int,int> >::iterator iter=node[x].borders.begin();iter!=node[x].borders.end();iter++)
			{
				node[x].border_id[iter->second.first]=iter->first;
				node[x].border_id_innode[iter->second.first]=iter->second.second;
			}
			node[x].border_in_father.clear();
			node[x].border_in_son.clear();
			for(i=0;i<node[x].borders.size();i++)
			{
				node[x].border_in_father.push_back(-1);
				node[x].border_in_son.push_back(-1);
			}
			if(node[x].father)
			{
				y=node[x].father;
				map<int,pair<int,int> >::iterator iter;
				for(iter=node[x].borders.begin();iter!=node[x].borders.end();iter++)
				{
					map<int,pair<int,int> >::iterator iter2;
					iter2=node[y].borders.find(iter->first);
					if(iter2!=node[y].borders.end())
						node[x].border_in_father[iter->second.first]=iter2->second.first;
				}
			}
			if(node[x].son[0])
			{
				map<int,pair<int,int> >::iterator iter;
				for(iter=node[x].borders.begin();iter!=node[x].borders.end();iter++)
				{
					y=node[x].son[node[x].color[iter->second.second]];
					map<int,pair<int,int> >::iterator iter2;
					iter2=node[y].borders.find(iter->first);
					if(iter2!=node[y].borders.end())
						node[x].border_in_son[iter->second.first]=iter2->second.first;
				}
				for (int i = 0; i<node[x].borders.size(); i++)
					node[x].border_son_id.push_back(node[x].son[node[x].color[node[x].border_id_innode[i]]]);
			}
		}
	}
	void push_borders_up(int x, vector<int> &dist1, int type)//将S到结点x边界点的最短路长度记录在dist1中，计算S到x.father真实border的距离更新dist1 type==0上推,type==1下推
	{
        // cerr << "x: " << x << endl;
        // cerr << "time test in push borders up: " << times[5] << endl;


        if (node[x].father == 0) {
            // cerr << "tui chu la!!" << endl;
            return;
        }

        // cerr << "mei tui chu" << endl;
        // cerr << times << endl;
        // times[5] -= clock();
        // try
        // {
        //     /* code */
        // }
        // catch(const std::exception& e)
        // {
        //     std::cerr << e.what() << '\n';
        // }
              
        // cerr << "wo na cuo le" << endl;
		int y = node[x].father;
        // cerr << "y: " << y << endl;
		vector<int>dist2(node[y].borders.size(), INF);
		for (int i = 0; i<node[x].borders.size(); i++)
			if (node[x].border_in_father[i] != -1)
				dist2[node[x].border_in_father[i]] = dist1[i];
		//fprintf(fout, "dist2:");save_vector(dist2);
		int **dist = node[y].dist.a;
		//vector<int>begin,end;//已算出的序列编号,未算出的序列编号
		int *begin, *end;
        // if (node[x].borders.size() > lim)
        //     cerr << "size: " << node[x].borders.size() << endl;
        begin = new int[node[x].borders.size()];
        // if(node[y].borders.size() > lim)
        //     cerr << "size: " << node[y].borders.size() << endl;
		end = new int[node[y].borders.size()];
		int tot0 = 0, tot1 = 0;
		// times[5] += clock();
		// times[6] -= clock();
		for (int i = 0; i<dist2.size(); i++)
		{
			if (dist2[i]<INF)begin[tot0++] = i;
			else if (node[y].border_in_father[i] != -1)end[tot1++] = i;
		}
		// times[6] += clock();
		// times[7] -= clock();
		if (type == 0)
		{
			for (int i = 0; i<tot0; i++)
			{
				int i_ = begin[i];
				for (int j = 0; j<tot1; j++)
				{
					int j_ = end[j];
					if (dist2[j_]>dist2[i_] + dist[i_][j_])
						dist2[j_] = dist2[i_] + dist[i_][j_];
				}
			}
		}
		else{
			for (int i = 0; i<tot0; i++)
			{
				int i_ = begin[i];
				for (int j = 0; j<tot1; j++)
				{
					int j_ = end[j];
					if (dist2[j_]>dist2[i_] + dist[j_][i_])
						dist2[j_] = dist2[i_] + dist[j_][i_];
				}
			}
		}
		/*
		int bit[2]={0,0xffffffff};
		int i,i_,j,j_,ls1,ls2;
		for(i=0;i<tot0;++i)
		{
		i_=begin[i];
		ls1=dist2[i_];
		for(j=0;j<tot1;++j)
		{
		j_=end[j];
		ls2=ls1+dist[i_][j_];
		dist2[j_]+=((ls2-dist2[j_])&bit[dist2[j_]>ls2]);
		}
		}
		*/
		// times[7] += clock();
		dist1 = dist2;
		delete[] begin;
		delete[] end;
	}
	void push_borders_up_catch(int x, int bound = INF)//将S到结点x边界点缓存在x.catch_dist的最短路长度，计算S到x.father真实border的距离更新x.father.catch
	{
		if (node[x].father == 0)return;
		int y = node[x].father;
		if (node[x].catch_id == node[y].catch_id&&bound <= node[y].catch_bound)return;
		node[y].catch_id = node[x].catch_id;
		node[y].catch_bound = bound;
		vector<int> *dist1 = &node[x].catch_dist, *dist2 = &node[y].catch_dist;
		for (int i = 0; i<(*dist2).size(); i++)(*dist2)[i] = INF;
		for (int i = 0; i<node[x].borders.size(); i++)
			if (node[x].border_in_father[i] != -1)
			{
				if (node[x].catch_dist[i]<bound)//bound界内的begin
					(*dist2)[node[x].border_in_father[i]] = (*dist1)[i];
				else (*dist2)[node[x].border_in_father[i]] = -1;//bound界外的begin
			}
		int **dist = node[y].dist.a;
		int *begin, *end;//已算出的序列编号,未算出的序列编号
        // if (node[x].borders.size() > lim)
        //     cerr << "size: " << node[x].borders.size() << endl;
		begin = new int[node[x].borders.size()];
        // if (node[y].borders.size() > lim)
        //     cerr << "size: " << node[y].borders.size() << endl;
		end = new int[node[y].borders.size()];
		int tot0 = 0, tot1 = 0;
		for (int i = 0; i<(*dist2).size(); i++)
		{
			if ((*dist2)[i] == -1)(*dist2)[i] = INF;
			else if ((*dist2)[i]<INF)begin[tot0++] = i;
			else if (node[y].border_in_father[i] != -1)
			{
				if (Optimization_Euclidean_Cut == false || Euclidean_Dist(node[x].catch_id, node[y].border_id[i])<bound)
					end[tot1++] = i;
			}
		}
		for (int i = 0; i<tot0; i++)
		{
			int i_ = begin[i];
			for (int j = 0; j<tot1; j++)
			{
				if ((*dist2)[end[j]]>(*dist2)[i_] + dist[i_][end[j]])
					(*dist2)[end[j]] = (*dist2)[i_] + dist[i_][end[j]];
			}
		}
		delete[] begin;
		delete[] end;
		node[y].min_border_dist = INF;
		for (int i = 0; i<node[y].catch_dist.size(); i++)
			if (node[y].border_in_father[i] != -1)
				node[y].min_border_dist = min(node[y].min_border_dist, node[y].catch_dist[i]);
	}
	void push_borders_down_catch(int x, int y, int bound = INF)//将S到结点x边界点缓存在x.catch_dist的最短路长度，计算S到x的儿子y真实border的距离更新y.catch
	{
		if (node[x].catch_id == node[y].catch_id&&bound <= node[y].catch_bound)return;
		node[y].catch_id = node[x].catch_id;
		node[y].catch_bound = bound;
		vector<int> *dist1 = &node[x].catch_dist, *dist2 = &node[y].catch_dist;
		for (int i = 0; i<(*dist2).size(); i++)(*dist2)[i] = INF;
		for (int i = 0; i<node[x].borders.size(); i++)
			if (node[x].son[node[x].color[node[x].border_id_innode[i]]] == y)
			{
				if (node[x].catch_dist[i]<bound)//bound界内的begin
					(*dist2)[node[x].border_in_son[i]] = (*dist1)[i];
				else (*dist2)[node[x].border_in_son[i]] = -1;//bound界外的begin
			}
		int **dist = node[y].dist.a;
		int *begin, *end;//已算出的序列编号,未算出的序列编号
        // if (node[y].borders.size() > lim)
        //     cerr << "size: " << node[y].borders.size() << endl;
		begin = new int[node[y].borders.size()];
        // if (node[y].borders.size())
        // cerr << "size: " << node[y].borders.size() << endl;
		end = new int[node[y].borders.size()];
		int tot0 = 0, tot1 = 0;
		for (int i = 0; i<(*dist2).size(); i++)
		{
			if ((*dist2)[i] == -1)(*dist2)[i] = INF;
			else if ((*dist2)[i]<INF)begin[tot0++] = i;
			else
			{
				if (Optimization_Euclidean_Cut == false || Euclidean_Dist(node[x].catch_id, node[y].border_id[i])<bound)
					end[tot1++] = i;
			}
		}
		for (int i = 0; i<tot0; i++)
		{
			int i_ = begin[i];
			for (int j = 0; j<tot1; j++)
			{
				if ((*dist2)[end[j]]>(*dist2)[i_] + dist[i_][end[j]])
					(*dist2)[end[j]] = (*dist2)[i_] + dist[i_][end[j]];
			}
		}
		delete[] begin;
		delete[] end;
		node[y].min_border_dist = INF;
		for (int i = 0; i<node[y].catch_dist.size(); i++)
			if (node[y].border_in_father[i] != -1)
				node[y].min_border_dist = min(node[y].min_border_dist, node[y].catch_dist[i]);
	}
	void push_borders_brother_catch(int x, int y, int bound = INF)//将S到结点x边界点缓存在x.catch_dist的最短路长度，计算S到x的兄弟结点y真实border的距离更新y.catch
	{
		int S = node[x].catch_id, LCA = node[x].father, i, j;
		if (node[y].catch_id == S&&node[y].catch_bound >= bound)return;
		int p;
		node[y].catch_id = S;
		node[y].catch_bound = bound;
		vector<int>id_LCA[2], id_now[2];//子结点候选border在LCA中的border序列编号,子结点候选border在内部的border序列的编号
		for (int t = 0; t<2; t++)
		{
			if (t == 0)p = x;
			else p = y;
			for (i = j = 0; i<(int)node[p].borders.size(); i++)
				if (node[p].border_in_father[i] != -1)
					if ((t == 1 && (Optimization_Euclidean_Cut == false || Euclidean_Dist(node[x].catch_id, node[p].border_id[i])<bound)) || (t == 0 && node[p].catch_dist[i]<bound))
					{
						id_LCA[t].push_back(node[p].border_in_father[i]);
						id_now[t].push_back(i);
					}
		}
		for (int i = 0; i<node[y].catch_dist.size(); i++)node[y].catch_dist[i] = INF;
		for (int i = 0; i<id_LCA[0].size(); i++)
			for (int j = 0; j<id_LCA[1].size(); j++)
			{
				int k = node[x].catch_dist[id_now[0][i]] + node[LCA].dist.a[id_LCA[0][i]][id_LCA[1][j]];
				if (k<node[y].catch_dist[id_now[1][j]])node[y].catch_dist[id_now[1][j]] = k;
			}
		int **dist = node[y].dist.a;
		//vector<int>begin,end;//已算出的序列编号,未算出的序列编号
		int *begin, *end;
        // if (node[y].borders.size() > lim)
        // cerr << "size: " << node[y].borders.size() << endl;
		begin = new int[node[y].borders.size()];
        // if (node[y].borders.size() > lim)
        // cerr << "size: " << node[y].borders.size() << endl;
		end = new int[node[y].borders.size()];
		int tot0 = 0, tot1 = 0;
		for (int i = 0; i<node[y].catch_dist.size(); i++)
		{
			if (node[y].catch_dist[i]<bound)begin[tot0++] = i;
			else if (node[y].catch_dist[i] == INF)
			{
				if (Optimization_Euclidean_Cut == false || Euclidean_Dist(node[x].catch_id, node[y].border_id[i])<bound)
					end[tot1++] = i;
			}
		}
		for (int i = 0; i<tot0; i++)
		{
			int i_ = begin[i];
			for (int j = 0; j<tot1; j++)
			{
				if (node[y].catch_dist[end[j]]>node[y].catch_dist[i_] + dist[i_][end[j]])
					node[y].catch_dist[end[j]] = node[y].catch_dist[i_] + dist[i_][end[j]];
			}
		}
		delete[] begin;
		delete[] end;
		node[y].min_border_dist = INF;
		for (int i = 0; i<node[y].catch_dist.size(); i++)
			if (node[y].border_in_father[i] != -1)
				node[y].min_border_dist = min(node[y].min_border_dist, node[y].catch_dist[i]);
	}
	void push_borders_up_path(int x, vector<int> &dist1)//将S到结点x边界点的最短路长度记录在dist1中，计算S到x.father真实border的距离更新dist1,并将到x.father的方案记录到x.father.path_record中(>=0表示结点，<0表示传递于那个结点儿子,-INF表示无前驱)
	{
		if (node[x].father == 0)return;
		// times[5] -= clock();
		int y = node[x].father;
		vector<int>dist3(node[y].borders.size(), INF);
		vector<int> *order = &node[y].path_record;
		(*order).clear();
		for (int i = 0; i<node[y].borders.size(); i++)(*order).push_back(-INF);
		for (int i = 0; i<node[x].borders.size(); i++)
			if (node[x].border_in_father[i] != -1)
			{
				dist3[node[x].border_in_father[i]] = dist1[i];
				(*order)[node[x].border_in_father[i]] = -x;
			}
		//fprintf(fout, "dist3:");save_vector(dist3);
		int **dist = node[y].dist.a;
		//vector<int>begin,end;//已算出的序列编号,未算出的序列编号
		int *begin, *end;
        // if (node[x].borders.size() > lim)
        // cerr << "size: " << node[x].borders.size() << endl;
		begin = new int[node[x].borders.size()];
        // if (node[y].borders.size() > lim)
        // cerr << "size: " << node[y].borders.size() << endl;
		end = new int[node[y].borders.size()];

		int tot0 = 0, tot1 = 0;
		// times[5] += clock();
		// times[6] -= clock();
		for (int i = 0; i<dist3.size(); i++)
		{
			if (dist3[i]<INF)begin[tot0++] = i;
			else if (node[y].border_in_father[i] != -1)end[tot1++] = i;
		}
		// times[6] += clock();
		// times[7] -= clock();
		for (int i = 0; i<tot0; i++)
		{
			int i_ = begin[i];
			for (int j = 0; j<tot1; j++)
			{
				if (dist3[end[j]]>dist3[i_] + dist[i_][end[j]])
				{
					dist3[end[j]] = dist3[i_] + dist[i_][end[j]];
					(*order)[end[j]] = i_;
				}
			}
		}
		// times[7] += clock();
		dist1 = dist3;
		delete[] begin;
		delete[] end;
	}
	int find_LCA(int x, int y)//计算树上两节点xy的LCA
	{
		if (node[x].deep<node[y].deep)swap(x, y);
		while (node[x].deep>node[y].deep)x = node[x].father;
		while (x != y){ x = node[x].father; y = node[y].father; }
		return x;
	}
	int search(int S, int T)//查询S-T最短路长度
	{

		if (S == T)return 0;
        // cerr << "lets test time" << times << " and a value: " << times[5] << endl;
		//计算LCA
		int i, j, k, p;
        assert(S < id_in_node.size());
        assert(T < id_in_node.size());
		int LCA, x = id_in_node[S], y = id_in_node[T];
		if (node[x].deep<node[y].deep)swap(x, y);
		while (node[x].deep>node[y].deep) {
            x = node[x].father;

        }
		while (x != y){ 
            // cerr << "x: " << x << " y: " << y << endl;
            x = node[x].father; y = node[y].father; 
        }
		LCA = x;
        // cerr << "LCA: " << LCA << endl;
		vector<int>dist[2], dist_;
		dist[0].push_back(0);
		dist[1].push_back(0);
        // cerr << "S: " << S << " T: " << T << endl;
        // cerr << "node size: " << id_in_node.size() << endl;
		x = id_in_node[S], y = id_in_node[T];
        // cerr << "im here" << endl;

		//朴素G-Tree计算
		for (int t = 0; t<2; t++)
		{
            // cerr << "i'm here 9" << endl;
			if (t == 0)p = x;
			else p = y;
			while (node[p].father != LCA)
			{
                // cerr << "i'm here 9" << endl;
				push_borders_up(p, dist[t], t);
                // cerr << "i'm here 10" << endl;
				p = node[p].father;
			}
			if (t == 0)x = p;
			else y = p;
		}
		vector<int>id[2];//子结点border在LCA中的border序列编号
		for (int t = 0; t<2; t++)
		{
			if (t == 0)p = x;
			else p = y;
			for (i = j = 0; i<(int)dist[t].size(); i++)
				if (node[p].border_in_father[i] != -1)
				{
					id[t].push_back(node[p].border_in_father[i]);
					dist[t][j] = dist[t][i];
					j++;
				}
			while (dist[t].size()>id[t].size()){ dist[t].pop_back(); }
		}

		//最终配对
		int MIN = INF;
		for (i = 0; i<dist[0].size(); i++)
		{
			int i_ = id[0][i];
			for (j = 0; j<dist[1].size(); j++)
			{
				k = dist[0][i] + dist[1][j] + node[LCA].dist.a[i_][id[1][j]];
				if (k<MIN)MIN = k;
			}
		}
		return MIN;
	}
	int search_catch(int S, int T, int bound = INF)//查询S-T最短路长度,并将沿途结点的catch处理为S的结果，其中不计算权值>=bound的部分，若没有则剪枝返回INF
	{
		//朴素G-Tree计算,维护catch
		if (S == T)return 0;
		//计算LCA
		int i, j, k, p;
		int x = id_in_node[S], y = id_in_node[T];
		int LCA = find_LCA(x, y);

		//计算两个叶子到LCA沿途结点编号
		vector<int>node_path[2];//点x/y到LCA之前依次会经过的树结点编号
		for (int t = 0; t<2; t++)
		{
			if (t == 0)p = x;
			else p = y;
			while (node[p].father != LCA)
			{
				node_path[t].push_back(p);
				p = node[p].father;
			}
			node_path[t].push_back(p);
			if (t == 0)x = p;
			else y = p;
		}

		//将S从叶子push到LCA下层
		node[id_in_node[S]].catch_id = S;
		node[id_in_node[S]].catch_bound = bound;
		node[id_in_node[S]].min_border_dist = 0;
		node[id_in_node[S]].catch_dist[0] = 0;
		for (i = 0; i + 1<node_path[0].size(); i++)
		{
			if (node[node_path[0][i]].min_border_dist >= bound)return INF;
			push_borders_up_catch(node_path[0][i]);
		}

		//计算T在LCA下层结点的catch
		if (node[x].min_border_dist >= bound)return INF;
		push_borders_brother_catch(x, y);
		//将T在LCA下层的数据push到底层结点T
		for (int i = node_path[1].size() - 1; i>0; i--)
		{
			if (node[node_path[1][i]].min_border_dist >= bound)return INF;
			push_borders_down_catch(node_path[1][i], node_path[1][i - 1]);
		}

		//最终答案
		return node[id_in_node[T]].catch_dist[0];
	}
	int find_path(int S, int T, vector<int> &order)//返回S-T最短路长度，并将沿途经过的结点方案存储到order数组中
	{
		order.clear();
		if (S == T)
		{
			order.push_back(S);
			return 0;
		}
		//计算LCA
		// times[0] -= clock();
		// times[4] -= clock();
		int i, j, k, p;
		int LCA, x = id_in_node[S], y = id_in_node[T];
		if (node[x].deep<node[y].deep)swap(x, y);
		while (node[x].deep>node[y].deep)x = node[x].father;
		while (x != y){ x = node[x].father; y = node[y].father; }
		LCA = x;
		vector<int>dist[2], dist_;
		dist[0].push_back(0);
		dist[1].push_back(0);
		x = id_in_node[S], y = id_in_node[T];
		//朴素G-Tree计算
		cnt_type1++;
		//fprintf(fout, "LCA=%d x=%d y=%d\n",LCA,x,y);
		// times[4] += clock();
		// times[2] -= clock();
		for (int t = 0; t<2; t++)
		{
			if (t == 0)p = x;
			else p = y;
			while (node[p].father != LCA)
			{
				push_borders_up_path(p, dist[t]);
				p = node[p].father;
			}
			if (t == 0)x = p;
			else y = p;
		}
		// times[2] += clock();
		// times[3] -= clock();
		vector<int>id[2];//子结点border在LCA中的border序列编号
		for (int t = 0; t<2; t++)
		{
			if (t == 0)p = x;
			else p = y;
			for (i = j = 0; i<(int)dist[t].size(); i++)
				if (node[p].border_in_father[i] != -1)
				{
					id[t].push_back(node[p].border_in_father[i]);
					dist[t][j] = dist[t][i];
					j++;
				}
			while (dist[t].size()>id[t].size()){ dist[t].pop_back(); }
		}
		// times[3] += clock();
		//最终配对
		// times[1] -= clock();
		int MIN = INF;
		int S_ = -1, T_ = -1;//最优路径在LCA中borders连接的编号
		for (i = 0; i<(int)dist[0].size(); i++)
			for (j = 0; j<(int)dist[1].size(); j++)
			{
				k = dist[0][i] + dist[1][j] + node[LCA].dist.a[id[0][i]][id[1][j]];
				if (k<MIN)
				{
					MIN = k;
					S_ = id[0][i];
					T_ = id[1][j];
				}
			}
		if (MIN<INF)//存在路径，恢复路径
		{
			for (int t = 0; t<2; t++)
			{
				int p, now;
				if (t == 0)p = x, now = node[LCA].border_in_son[S_];
				else p = y, now = node[LCA].border_in_son[T_];
				while (node[p].n>1)
				{
					//fprintf(fout, "t=%d p=%d now=%d node[p].path_record[now]=%d\n",t,p,now,node[p].path_record[now]);
					if (node[p].path_record[now] >= 0)
					{
						find_path_border(p, now, node[p].path_record[now], order, 0);
						now = node[p].path_record[now];
					}
					else if (node[p].path_record[now]>-INF)
					{
						int temp = now;
						now = node[p].border_in_son[now];
						p = -node[p].path_record[temp];
					}
					else break;
				}
				if (t == 0)//翻转，补充连接路径
				{
					reverse(order.begin(), order.end());
					order.push_back(node[LCA].border_id[S_]);
					find_path_border(LCA, S_, T_, order, 0);
				}
			}
		}
		// times[1] += clock();
		// times[0] += clock();
		return MIN;
		//cerr<<"QY5";
	}
	int real_border_number(int x)//计算x真实的border数(忽略内部子图之间的border)
	{
		int i, j, re = 0, id;
		map<int, int>vis;
		for (i = 0; i<node[x].G.n; i++)vis[node[x].G.id[i]] = 1;
		for (i = 0; i<node[x].G.n; i++)
		{
			id = node[x].G.id[i];
			for (j = G.head[id]; j; j = G.next[j])
				if (vis[G.list[j]] == 0)
				{
					re++;
					break;
				}
		}
		return re;
	}
	void find_path_border(int x, int S, int T, vector<int> &v, int rev)//返回结点x中编号为S到T的border的结点路径，存储在vector<int>中，将除了起点S以外的部分S+1~T，push到v尾部,rev=0表示正序，rev=1表示逆序
	{
		/*fprintf(fout, "find:x=%d S=%d T=%d\n",x,S,T);
		fprintf(fout, "node:%d\n",x);
		node[x].write();
		fprintf(fout, "\n\n\n\n");*/
		if (node[x].order.a[S][T] == -1)
		{
			if (rev == 0)v.push_back(node[x].border_id[T]);
			else v.push_back(node[x].border_id[S]);
		}
		else if (node[x].order.a[S][T] == -2)
		{
			find_path_border(node[x].father, node[x].border_in_father[S], node[x].border_in_father[T], v, rev);
		}
		else if (node[x].order.a[S][T] == -3)
		{
			find_path_border(node[x].son[node[x].color[node[x].border_id_innode[S]]], node[x].border_in_son[S], node[x].border_in_son[T], v, rev);
		}
		else if (node[x].order.a[S][T] >= 0)
		{
			int k = node[x].order.a[S][T];
			if (rev == 0)
			{
				find_path_border(x, S, k, v, rev);
				find_path_border(x, k, T, v, rev);
			}
			else
			{
				find_path_border(x, k, T, v, rev);
				find_path_border(x, S, k, v, rev);
			}
		}
	}
	vector<int> KNN(int S, int K, const vector<int>&T)//计算S到T数组中的前K小并返回其在T数组中的下标
	{
		priority_queue<int>K_Value;//保存K小值
		vector<pair<int, int> >query;//first:该查询的优先级(越低越靠前)，second该查询在原序列T中的id
		for (int i = 0; i<T.size(); i++)query.push_back(make_pair(-node[find_LCA(id_in_node[S], id_in_node[T[i]])].deep, i));
		sort(query.begin(), query.end());
		vector<int>re, ans;
		if (K <= 0)return re;
		for (int i = 0; i<T.size(); i++)
		{
			int bound = K_Value.size()<K ? INF : K_Value.top();
			//if(bound==INF)fprintf(fout, "- ");else fprintf(fout, "%d ",bound);
			if (Optimization_KNN_Cut)ans.push_back(search_catch(S, T[query[i].second], bound));
			else ans.push_back(search_catch(S, T[query[i].second]));
			if (K_Value.size()<K)K_Value.push(ans[i]);
			else if (ans[i]<K_Value.top())
			{
				K_Value.pop();
				K_Value.push(ans[i]);
			}
		}
		int bound = (ans.size() <= K) ? INF : K_Value.top();
		for (int i = 0; i<T.size() && re.size()<K; i++)
			if (ans[i] <= bound)
				re.push_back(query[i].second);
		sort(re.begin(), re.end());
		return re;
	}
	vector<int> KNN(int S, int K, const vector<int>&T, const vector<int>&offset)//计算S到T数组中的前K小并返回其在T数组中的下标,考虑车到结点距离offset
	{
		priority_queue<int>K_Value;//保存K小值
		vector<pair<int, int> >query;//first:该查询的优先级(越低越靠前)，second该查询在原序列T中的id
		for (int i = 0; i<T.size(); i++)query.push_back(make_pair(-node[find_LCA(id_in_node[S], id_in_node[T[i]])].deep, i));
		sort(query.begin(), query.end());
		vector<int>re, ans;
		if (K <= 0)return re;
		for (int i = 0; i<T.size(); i++)
		{
			int bound = K_Value.size()<K ? INF : K_Value.top();
			//if(bound==INF)fprintf(fout, "- ");else fprintf(fout, "%d ",bound);
			if (Optimization_KNN_Cut)ans.push_back(search_catch(S, T[query[i].second], bound) + offset[query[i].second]);
			else ans.push_back(search_catch(S, T[query[i].second]) + offset[query[i].second]);
			if (K_Value.size()<K)K_Value.push(ans[i]);
			else if (ans[i]<K_Value.top())
			{
				K_Value.pop();
				K_Value.push(ans[i]);
			}
		}
		int bound = (ans.size() <= K) ? INF : K_Value.top();
		for (int i = 0; i<T.size() && re.size()<K; i++)
			if (ans[i] <= bound)
				re.push_back(query[i].second);
		sort(re.begin(), re.end());
		return re;
	}
	vector<int> KNN_bound(int S, int K, const vector<int>&T, int bound)//计算S到T数组中的前K小并返回其在T数组中的下标
	{
		priority_queue<int>K_Value;//保存K小值
		vector<pair<int, int> >query;//first:该查询的优先级(越低越靠前)，second该查询在原序列T中的id
		for (int i = 0; i<T.size(); i++)query.push_back(make_pair(-node[find_LCA(id_in_node[S], id_in_node[T[i]])].deep, i));
		sort(query.begin(), query.end());
		vector<int>re, ans;
		if (K <= 0)return re;
		for (int i = 0; i<T.size(); i++)
		{
			if (Optimization_KNN_Cut)ans.push_back(search_catch(S, T[query[i].second], bound));
			else ans.push_back(search_catch(S, T[query[i].second]));
			if (K_Value.size()<K)K_Value.push(ans[i]);
			else if (ans[i]<K_Value.top())
			{
				K_Value.pop();
				K_Value.push(ans[i]);
			}
		}
		for (int i = 0; i<T.size() && re.size()<K; i++)
			if (ans[i] <= bound)
				re.push_back(query[i].second);
		sort(re.begin(), re.end());
		return re;
	}

    // #include <cstring>
	vector<int> KNN_bound(int S, int K, const vector<int>&T, int bound, vector<int>offset)//计算S到T数组中的前K小并返回其在T数组中的下标,考虑车到结点距离offset
	{
        
		priority_queue<int>K_Value;//保存K小值
		vector<pair<int, int> >query;//first:该查询的优先级(越低越靠前)，second该查询在原序列T中的id
		for (int i = 0; i<T.size(); i++)query.push_back(make_pair(-node[find_LCA(id_in_node[S], id_in_node[T[i]])].deep, i));
		sort(query.begin(), query.end());
		vector<int>re, ans;
		if (K <= 0)return re;
		for (int i = 0; i<T.size(); i++)
		{
			if (Optimization_KNN_Cut)ans.push_back(search_catch(S, T[query[i].second], bound) + offset[query[i].second]);
			else ans.push_back(search_catch(S, T[query[i].second]) + offset[query[i].second]);
			if (K_Value.size()<K)K_Value.push(ans[i]);
			else if (ans[i]<K_Value.top())
			{
				K_Value.pop();
				K_Value.push(ans[i]);
			}
		}
		for (int i = 0; i<T.size() && re.size()<K; i++)
			if (ans[i] <= bound)
				re.push_back(query[i].second);
		sort(re.begin(), re.end());
		return re;
	}
	vector<int> Range(int S, int R, const vector<int>&T)//计算S到T数组中距离小于R的终点T并返回其在T数组中的下标
	{
		vector<int>re;
		for (int i = 0; i<T.size(); i++)
		{
			if (search_catch(S, T[i], Optimization_KNN_Cut ? R : INF)<R)re.push_back(i);
		}
		return re;
	}
	vector<int> Range(int S, int R, const vector<int>&T, const vector<int>&offset)//计算S到T数组中距离小于R的终点T并返回其在T数组中的下标,考虑车到结点距离offset
	{
		vector<int>re;
		for (int i = 0; i<T.size(); i++)
		{
			if (offset[i] + search_catch(S, T[i], Optimization_KNN_Cut ? R : INF)<R)re.push_back(i);
		}
		return re;
	}
	void add_car(int node_id, int car_id)//向车辆集合中增加一辆位于结点编号：node_id的车，车的编号为car_id
	{
		car_in_node[node_id].push_back(car_id);
		if (car_in_node[node_id].size() == 1)
		{
			//增加node_id更新距离border最小距离
			int S = id_in_node[node_id];
			node[S].min_car_dist[0] = make_pair(0, node_id);
			for (int p = S; push_borders_up_add_min_car_dist(p, node_id); p = node[p].father);
		}
	}
	void del_car(int node_id, int car_id)//从车辆集合中删除一辆位于结点编号：node_id的车，车的编号为car_id
	{
		int i;
		for (i = 0; i<car_in_node[node_id].size(); i++)
			if (car_in_node[node_id][i] == car_id)break;
		if (i == car_in_node[node_id].size())fprintf(stderr, "Error: del_car find none car!");
		while (i + 1<car_in_node[node_id].size()){ swap(car_in_node[node_id][i], car_in_node[node_id][i + 1]); i++; }
		car_in_node[node_id].pop_back();
		if (car_in_node[node_id].size() == 0)
		{
			//删除node_id更新距border最小距离
			int S = id_in_node[node_id];
			node[S].min_car_dist[0] = make_pair(INF, -1);
			for (int p = S; push_borders_up_del_min_car_dist(p, node_id); p = node[p].father);
		}
	}
	void change_car_offset(int car_id, int dist)//修改车car_id到其所在结点的距离dist
	{
		while (car_offset.size() <= car_id)car_offset.push_back(0);
		car_offset[car_id] = dist;
	}
	int get_car_offset(int car_id)//查询车car_id的距离偏移量
	{
		while (car_offset.size() <= car_id)car_offset.push_back(0);
		return car_offset[car_id];
	}
	int begin[10000], end[10000];//已算出的序列编号,未算出的序列编号
	bool push_borders_up_add_min_car_dist(int x, int start_id)//用结点x的min_car_dist更新x.father的，其中只更新node_id=start_id的部分，若不存在则返回false，否则返回true
	{
		int re = false;
		if (node[x].father == 0)return re;
		int y = node[x].father;
		vector<pair<int, int> > *dist1 = &node[x].min_car_dist, *dist2 = &node[y].min_car_dist;
		for (int i = 0; i<node[x].borders.size(); i++)
			if (node[x].min_car_dist[i].second == start_id)
			{
				re = true;
				if (node[x].border_in_father[i] != -1)
				{
					if ((*dist2)[node[x].border_in_father[i]].first>(*dist1)[i].first)
						(*dist2)[node[x].border_in_father[i]] = (*dist1)[i];
				}
			}
		if (y != root)
		{
			int **dist = node[y].dist.a;
			int tot0 = 0, tot1 = 0;
			for (int i = 0; i<(*dist2).size(); i++)
			{
				if ((*dist2)[i].second == start_id)begin[tot0++] = i;
				else end[tot1++] = i;
			}
			for (int i = 0; i<tot0; i++)
			{
				int i_ = begin[i];
				for (int j = 0; j<tot1; j++)
				{
					if ((*dist2)[end[j]].first>(*dist2)[i_].first + dist[end[j]][i_])
					{
						(*dist2)[end[j]].first = (*dist2)[i_].first + dist[end[j]][i_];
						(*dist2)[end[j]].second = (*dist2)[i_].second;
					}
				}
			}
		}
		return re;
	}
	bool push_borders_up_del_min_car_dist(int x, int start_id)//删除x.father中node_id=start_id的min_car_dist,并用其他值更新,若不存在返回false，否则返回true
	{
		if (DEBUG_)fprintf(stderr, "push_borders_up_del_min_car_dist x=%d id=%d \n", x, start_id);
		int re = false;
		if (node[x].father == 0)return false;
		int y = node[x].father;
		//fprintf(fout, "min_car_dist ");for(int i=0;i<node[y].min_car_dist.size();i++)fprintf(fout, "(i:%d,D:%d,id:%d)",i,node[y].min_car_dist[i].first,node[y].min_car_dist[i].second);fprintf(fout, "\n");
		vector<pair<int, int> > *dist1 = &node[x].min_car_dist, *dist2 = &node[y].min_car_dist;
		if (DEBUG_)for (int i = 0; i<node[x].borders.size(); i++)if (node[x].min_car_dist[i].second == start_id){ fprintf(stderr, "WRong!!!!%d %d\n", x, start_id); while (1); }
		int tot0 = 0, tot1 = 0;
		if (y == root)//删除x.father中关于node_id的数据
		{
			for (int i = 0; i<node[x].borders.size(); i++)
				if (node[x].border_in_father[i] != -1)
				{
					if ((*dist2)[node[x].border_in_father[i]].second == start_id)
					{
						(*dist2)[node[x].border_in_father[i]] = make_pair(INF, -1);
						re = true;
					}
				}
			for (int i = 0; i<node[x].borders.size(); i++)
			{
				if (node[x].border_in_father[i] != -1)
				{
					if ((*dist2)[node[x].border_in_father[i]].first>(*dist1)[i].first)
						(*dist2)[node[x].border_in_father[i]] = (*dist1)[i];
				}
			}
		}
		else
		{
			int SIZE = node[y].borders.size();
			for (int i = 0; i<SIZE; i++)
			{
				if ((*dist2)[i].second == start_id)
				{
					(*dist2)[i] = node[node[y].border_son_id[i]].min_car_dist[node[y].border_in_son[i]];
					end[tot1++] = i;
					re = true;
				}
				else begin[tot0++] = i;
			}
		}
		if (re)
		{
			if (y != root)
			{
				int **dist = node[y].dist.a;
				for (int i = 0; i<tot0; i++)
				{
					int i_ = begin[i];
					for (int j = 0; j<tot1; j++)
					{
						if ((*dist2)[end[j]].first>(*dist2)[i_].first + dist[end[j]][i_])
						{
							(*dist2)[end[j]].first = (*dist2)[i_].first + dist[end[j]][i_];
							(*dist2)[end[j]].second = (*dist2)[i_].second;
						}
					}
				}
				//二次DP
				while (tot1)
				{
					for (int i = 0; i<tot1 - 1; i++)
						if ((*dist2)[end[i]].first<(*dist2)[end[i + 1]].first)swap(end[i], end[i + 1]);
					tot1--;
					//用end[tot1]重新增广dist2
					int i_ = end[tot1];
					for (int j = 0; j<tot1; j++)
					{
						if ((*dist2)[end[j]].first>(*dist2)[i_].first + dist[i_][end[j]])
						{
							(*dist2)[end[j]].first = (*dist2)[i_].first + dist[i_][end[j]];
							(*dist2)[end[j]].second = (*dist2)[i_].second;
						}
					}
				}
			}
		}
		//if(DEBUG_)fprintf(fout, "re=%d\n",re);
		return re;
	}
	int push_borders_up_catch_KNN_min_dist_car(int x)//将S到结点x边界点缓存在x.catch_dist的最短路长度，计算S到x.father真实border的距离更新x.father.catch，并返回父亲结点border中最远距离(用于KNN扩张剪枝)
	{
		if (node[x].father == 0)return INF;
		int re = INF + 1;
		int y = node[x].father;
		node[y].catch_id = node[x].catch_id;
		node[y].catch_bound = -1;
		vector<int> *dist1 = &node[x].catch_dist, *dist2 = &node[y].catch_dist;
		for (int i = 0; i<(*dist2).size(); i++)(*dist2)[i] = INF;
		for (int i = 0; i<node[x].borders.size(); i++)
			if (node[x].border_in_father[i] != -1)
				(*dist2)[node[x].border_in_father[i]] = (*dist1)[i];
		int **dist = node[y].dist.a;
		int *begin, *end;//已算出的序列编号,未算出的序列编号
        // if (node[x].borders.size() > lim)
        // cerr << "size: " << node[x].borders.size() << endl;
		begin = new int[node[x].borders.size()];
        // if (node[y].borders.size() > lim)
        // cerr << "size: " << node[y].borders.size() << endl;
		end = new int[node[y].borders.size()];
		int tot0 = 0, tot1 = 0;
		for (int i = 0; i<(*dist2).size(); i++)
		{
			if ((*dist2)[i]<INF)begin[tot0++] = i;
			else end[tot1++] = i;
		}
		for (int i = 0; i<tot0; i++)
		{
			int i_ = begin[i];
			for (int j = 0; j<tot1; j++)
			{
				if ((*dist2)[end[j]]>(*dist2)[i_] + dist[i_][end[j]])
					(*dist2)[end[j]] = (*dist2)[i_] + dist[i_][end[j]];
			}
		}
		if (y == root)re = INF + 1;
		else
			for (int i = 0; i<node[y].borders.size(); i++)
				if (node[y].border_in_father[i] != -1)
					re = min(re, (*dist2)[i]);
		delete[] begin;
		delete[] end;
		return re;
	}
	vector<int> KNN_min_dist_car(int S, int K)//计算S到car集合中的前K小并返回其车辆编号
	{
		//动态扩张优化：catch计算到某一层，只有K距离超过当前层最远点，或当前层无车时扩张
		int Now_Catch_P = id_in_node[S], Now_Catch_Dist = 0;//现在的S的catch做到那个结点，动态扩张
		priority_queue<pair<int, pair<int, int> > >q;//保存K小值<-dist,<node_id,border_id>>
		{//构建S的catch
			node[id_in_node[S]].catch_id = S;
			node[id_in_node[S]].catch_bound = INF;
			node[id_in_node[S]].min_border_dist = 0;
			node[id_in_node[S]].catch_dist[0] = 0;
			/*for(int p=id_in_node[S];p!=root;p=node[p].father)
			push_borders_up_catch_KNN_min_dist_car(p);*/
		}
		//建立PQ
		for (int i = 0; i<node[Now_Catch_P].borders.size(); i++)
			q.push(make_pair(-(node[Now_Catch_P].catch_dist[i] + node[Now_Catch_P].min_car_dist[i].first), make_pair(Now_Catch_P, i)));
		vector<int>ans, ans2;//车的编号及其所在结点
		if (Distance_Offset == false)
		{
			while (K)
			{
				int Dist = q.top().first;
				int node_id = q.top().second.first;
				int border_id = q.top().second.second;
				int real_Dist = -(node[node_id].catch_dist[border_id] + node[node_id].min_car_dist[border_id].first);
				if (Dist != real_Dist)
				{
					q.pop();
					q.push(make_pair(real_Dist, make_pair(node_id, border_id)));
					continue;
				}
				if (-Dist>Now_Catch_Dist && Now_Catch_P != root)
				{
					Now_Catch_Dist = push_borders_up_catch_KNN_min_dist_car(Now_Catch_P);
					Now_Catch_P = node[Now_Catch_P].father;
					for (int i = 0; i<node[Now_Catch_P].borders.size(); i++)
					{
						q.push(make_pair(-(node[Now_Catch_P].catch_dist[i] + node[Now_Catch_P].min_car_dist[i].first), make_pair(Now_Catch_P, i)));
					}
					continue;
				}
				int real_node_id = node[node_id].min_car_dist[border_id].second;

				if (real_node_id == -1)break;
				int car_id = car_in_node[real_node_id][0];
				q.pop();
				ans.push_back(car_id);
				ans2.push_back(real_node_id);
				del_car(real_node_id, car_id);
				q.push(make_pair(-(node[node_id].catch_dist[border_id] + node[node_id].min_car_dist[border_id].first), make_pair(node_id, border_id)));
				K--;
			}
			for (int i = 0; i<ans.size(); i++)add_car(ans2[i], ans[i]);
		}
		else
		{
			priority_queue<int>KNN_Dist;
			vector<int>ans3;
			while (KNN_Dist.size()<K || KNN_Dist.top() <= -q.top().first)
			{
				int Dist = q.top().first;
				int node_id = q.top().second.first;
				int border_id = q.top().second.second;
				int real_Dist = -(node[node_id].catch_dist[border_id] + node[node_id].min_car_dist[border_id].first);
				if (Dist != real_Dist)
				{
					q.pop();
					q.push(make_pair(real_Dist, make_pair(node_id, border_id)));
					continue;
				}
				if (-Dist>Now_Catch_Dist && Now_Catch_P != root)
				{
					Now_Catch_Dist = push_borders_up_catch_KNN_min_dist_car(Now_Catch_P);
					Now_Catch_P = node[Now_Catch_P].father;
					for (int i = 0; i<node[Now_Catch_P].borders.size(); i++)
						q.push(make_pair(-(node[Now_Catch_P].catch_dist[i] + node[Now_Catch_P].min_car_dist[i].first), make_pair(Now_Catch_P, i)));
					continue;
				}
				int real_node_id = node[node_id].min_car_dist[border_id].second;

				if (real_node_id == -1)break;
				int car_id = car_in_node[real_node_id][0];
				q.pop();
				del_car(real_node_id, car_id);
				q.push(make_pair(-(node[node_id].catch_dist[border_id] + node[node_id].min_car_dist[border_id].first), make_pair(node_id, border_id)));
				int car_dist = get_car_offset(car_id) - real_Dist;
				if (KNN_Dist.size()<K)KNN_Dist.push(car_dist);
				else if (KNN_Dist.top()>car_dist)
				{
					KNN_Dist.pop();
					KNN_Dist.push(car_dist);
				}
				ans.push_back(car_id);
				ans2.push_back(real_node_id);
				ans3.push_back(car_dist);
			}
			for (int i = 0; i<ans.size(); i++)add_car(ans2[i], ans[i]);
			int j = 0;
			for (int i = 0; i<ans.size(); i++)
				if (ans3[i] <= KNN_Dist.top())
					ans[j++] = ans[i];
			while (ans.size()>K)ans.pop_back();
		}
		return ans;
	}
	bool check_min_car_dist(int x_ = -1)//检查x的min_car_dist是否DP成立
	{
		for (int x = (x_ == -1 ? node_tot : x_ + 1) - 1; x >= (x_ == -1 ? root : x_); x--)
		{
			if (x == root)continue;
			if (node[x].borders.size() == 1)continue;
			int i, j, ans;
			for (i = 0; i<node[x].borders.size(); i++)
			{
				ans = INF; int ans_id = -1, order = -1;
				if (ans>node[node[x].son[node[x].color[node[x].border_id_innode[i]]]].min_car_dist[node[x].border_in_son[i]].first)
				{
					ans = node[node[x].son[node[x].color[node[x].border_id_innode[i]]]].min_car_dist[node[x].border_in_son[i]].first;
					ans_id = node[node[x].son[node[x].color[node[x].border_id_innode[i]]]].min_car_dist[node[x].border_in_son[i]].second;
					order = -2;
				}
				for (j = 0; j<node[x].borders.size(); j++)
					if (j != i)
					{
						if (ans>node[x].min_car_dist[j].first + node[x].dist.a[i][j])
						{
							ans = node[x].min_car_dist[j].first + node[x].dist.a[i][j];
							ans_id = node[x].min_car_dist[j].second;
							order = j;
						}
					}

				if (ans != node[x].min_car_dist[i].first/*||ans_id!=node[x].min_car_dist[i].second*/)
				{
					fprintf(stderr, "x=%d i=%d ans=%d ans_id=%d min=%d min_id=%d order=%d\n", x, i, ans, ans_id, node[x].min_car_dist[i].first, node[x].min_car_dist[i].second, order);
					fprintf(stderr, "node[x].son[node[x].color[node[x].border_id_innode[i]]]=%d node[x].border_in_son[i]=%d\n", node[x].son[node[x].color[node[x].border_id_innode[i]]], node[x].border_in_son[i]);
					return false;
				}
			}
		}
		return true;
	}
};