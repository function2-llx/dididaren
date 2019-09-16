#pragma once

#include <vector>
#include <cstdio>
#include <iostream>
#include "save_load.h"
#include "global.h"
#include <metis.h>
#include <cstring>
#include "heap.h"
#include <queue>
#include <algorithm>

using namespace std;

struct Graph//无向图结构 
{
	int n,m;//n个点m条边 点从0编号到n-1
	int tot;
	vector<int>id;//id[i]为子图中i点在原图中的真实编号
	vector<int>head,list,next,cost;//邻接表
	Graph(){clear();}
	~Graph(){clear();}
	void save(FILE* fout)//保存结构信息
	{
		fprintf(fout, "%d %d %d\n",n,m,tot);
		save_vector(fout, id);
		save_vector(fout, head);
		save_vector(fout, list);
		save_vector(fout, next);
		save_vector(fout, cost);
	}
	void load(FILE* fin)//读取结构信息
	{
		fscanf(fin, "%d%d%d",&n,&m,&tot);
        // cerr << "n: " << n << "m :" << m << endl;
		load_vector(fin, id);
		load_vector(fin, head);
		load_vector(fin, list);
		load_vector(fin, next);
		load_vector(fin, cost);
	}
	void add_D(int a,int b,int c)//加入一条a->b权值为c的有向边
	{
		tot++;
		list[tot]=b;
		cost[tot]=c;
		next[tot]=head[a];
		head[a]=tot;
	}
	void add(int a,int b,int c)//加入一条a<->b权值为c的无向边
	{
		add_D(a,b,c);
		add_D(b,a,c);
	}
	void init(int N,int M,int t=1)
	{
		clear();
		n=N;m=M;
		tot=t;
		head=vector<int>(N);
		id=vector<int>(N);
		list=vector<int>(M*2+2);
		next=vector<int>(M*2+2);
		cost=vector<int>(M*2+2);
	}
	void clear()
	{
		n=m=tot=0;
		head.clear();
		list.clear();
		next.clear();
		cost.clear();
		id.clear();
	}
	// void draw()//输出图结构
	// {	
	// 	fprintf(stderr, "Graph:%p n=%d m=%d\n",this,n,m);
	// 	for(int i=0;i<n;i++)
    //         cerr<<id[i]<<' ';
    //     cerr<<endl;
	// 	for(int i=0;i<n;i++)
	// 	{
	// 		fprintf(stderr, "%d:",i);
	// 		for(int j=head[i];j;j=next[j])fprintf(stderr, " %d",list[j]);
	// 		cerr<<endl;
	// 	}
	// 	fprintf(stderr, "Graph_draw_end\n");
	// }
	//图划分算法
	vector<int>color;//01染色数组
	vector<int>con;//连通性
	vector<int> Split(Graph *G[],int nparts)//将子图一分为二返回color数组，并将两部分分别存至G1，G2 METIS algorithm,npart表示划分块数
	{
		
		vector<int>color(n);
		int i;
		/*if(n<Naive_Split_Limit)
		{
			return Split_Naive(*G[0],*G[1]);
		}*/

		if(DEBUG1)fprintf(stderr, "Begin-Split\n");
		if(n==nparts)
		{
			for(i=0;i<n;i++)color[i]=i;
		}
		else
		{
			idx_t options[METIS_NOPTIONS];
			memset(options,0,sizeof(options));
			{
				METIS_SetDefaultOptions(options);
				options[METIS_OPTION_PTYPE] = METIS_PTYPE_KWAY; // _RB
				options[METIS_OPTION_OBJTYPE] = METIS_OBJTYPE_CUT; // _VOL
				options[METIS_OPTION_CTYPE] = METIS_CTYPE_SHEM; // _RM
				options[METIS_OPTION_IPTYPE] = METIS_IPTYPE_RANDOM; // _GROW _EDGE _NODE
				options[METIS_OPTION_RTYPE] = METIS_RTYPE_FM; // _GREEDY _SEP2SIDED _SEP1SIDED
				// options[METIS_OPTION_NCUTS] = 1;
				// options[METIS_OPTION_NITER] = 10;
				/* balance factor, used to be 500 */
				options[METIS_OPTION_UFACTOR] = 500;
				// options[METIS_OPTION_MINCONN];
				options[METIS_OPTION_CONTIG] = 1;
				// options[METIS_OPTION_SEED];
				options[METIS_OPTION_NUMBERING] = 0;
				// options[METIS_OPTION_DBGLVL] = 0;
			}
			idx_t nvtxs=n;
			idx_t ncon=1;
			//transform
			int *xadj = new idx_t[n + 1];
			int *adj=new idx_t[n+1];
			int *adjncy = new idx_t[tot-1];
			int *adjwgt = new idx_t[tot-1];
			int *part = new idx_t[n];


			int xadj_pos = 1;
			int xadj_accum = 0;
			int adjncy_pos = 0;

			// xadj, adjncy, adjwgt
			xadj[0] = 0;
			int i = 0;
			for (int i=0;i<n;i++){
				// init node map

				/*int fanout = Nodes[nid].adjnodes.size();
				for ( int j = 0; j < fanout; j++ ){
					int enid = Nodes[nid].adjnodes[j];
					// ensure edges within
					if ( nset.find( enid ) != nset.end() ){
						xadj_accum ++;

						adjncy[adjncy_pos] = enid;
						adjwgt[adjncy_pos] = Nodes[nid].adjweight[j];
						adjncy_pos ++;
					}
				}
				xadj[xadj_pos++] = xadj_accum;*/
				for(int j=head[i];j;j=next[j])
				{
					int enid = list[j];
					xadj_accum ++;
					adjncy[adjncy_pos] = enid;
					adjwgt[adjncy_pos] = cost[j];
					adjncy_pos ++;
				}
				xadj[xadj_pos++] = xadj_accum;
			}


			// adjust nodes number started by 0

			// adjwgt -> 1
			for ( int i = 0; i < adjncy_pos; i++ ){
				adjwgt[i] = 1;
			}

			// nparts
			int objval=0;
			//METIS
			METIS_PartGraphKway(
				&nvtxs,
				&ncon,
				xadj,
				adjncy,
				NULL,
				NULL,
				adjwgt,
				&nparts,
				NULL,
				NULL,
				options,
				&objval,
				part
			);
			for(int i=0;i<n;i++)color[i]=part[i];
			delete [] xadj;
			delete [] adj;
			delete [] adjncy;
			delete [] adjwgt;
			delete [] part;
		}
		//划分
		int j;
		vector<int>new_id;
		vector<int>tot(nparts,0),m(nparts,0);
		for(i=0;i<n;i++)
			new_id.push_back(tot[color[i]]++);
		for(i=0;i<n;i++)
			for(j=head[i];j;j=next[j])
				if(color[list[j]]==color[i])
					m[color[i]]++;
		for(int t=0;t<nparts;t++)
		{
			(*G[t]).init(tot[t],m[t]);
			for(i=0;i<n;i++)
				if(color[i]==t)
					for(j=head[i];j;j=next[j])
						if(color[list[j]]==color[i])
							(*G[t]).add_D(new_id[i],new_id[list[j]],cost[j]);
		}
		for(i=0;i<tot.size();i++)tot[i]=0;
		for(i=0;i<n;i++)
			(*G[color[i]]).id[tot[color[i]]++]=id[i];
		if(DEBUG1)fprintf(stderr, "Split_over\n");
		return color;
	}
	vector<int> Split_Naive(Graph &G1,Graph &G2)//将子图一分为二返回color数组，并将两部分分别存至G1，G2 Kernighan-Lin algorithm
	{
		color.clear();
		con.clear();
		if(DEBUG1)fprintf(stderr, "Begin Split\n");
		int i,j,k=n/2,l;
		for(i=0;i<n;i++)//初始随机染色
		{
			if(n-i==k/*||(k&&rand()&1)*/){color.push_back(1);k--;}
			else color.push_back(0);
		}
		if(DEBUG1)fprintf(stderr, "Split_Color over\n");

		for(i=0;i<n;i++)
		{
			k=0;
			for(j=head[i];j;j=next[j])
				if(color[list[j]]^color[i])k++;
			con.push_back(k);
		}
		if(DEBUG1)fprintf(stderr, "Split_Con over\n");
		
		//优先队列划分,每个点的分数邻域不同数量-相同数量
		Heap q[2];int ans=0;
		for(i=0;i<n;i++)
		{
			k=0;
			for(j=head[i];j;j=next[j])
				if(color[list[j]]^color[i])ans++,k++;
				else k--;
			q[color[i]].push(k);
			q[color[i]^1].push(-k-INF);
		}
		ans/=2;
		// if(DEBUG1)fprintf(stderr, "Split_Build_Heap over\n");
		//for(i=0;i<n;i++)fprintf(fout, "%d %d %d\n",i,color[i],con[i]);
		// if(DEBUG1)cerr<<"start_ans="<<ans<<endl;
		while(q[0].top()+q[1].top()>0)
		{
			int save_ans=ans;
			for(l=0;l<2;l++)
			{
				i=q[l].top_id();
				k=0;
				for(j=head[i];j;j=next[j])
				{
					if(color[list[j]]^color[i])k--;
					else k++;
					q[color[i]^1].add(list[j],-2);
					q[color[i]].add(list[j],2);
				}
				ans+=k;
				color[i]^=1;
				q[color[i]].change(i,k);
				q[color[i]^1].change(i,-k-INF);
			}
			if(DEBUG1)if(rand()%100==0)fprintf(stderr, "now_ans=%d\n",save_ans);
			if(save_ans==ans)break;
		}
		//DEBUG
		if(DEBUG1) {
			int border_num=0;
			for(i=0;i<n;i++)
				for(j=head[i];j;j=next[j]) {
					if(color[i]!=color[list[j]]) {
                        border_num++;break;
                    }
                }
            fprintf(stderr, "边连通度ans=%d border_number=%d\n",ans,border_num);
		}
		//for(i=0;i<n;i++)cerr<<"i="<<i<<" color="<<color[i]<<endl;

		//划分
		vector<int>new_id;
		int tot0=0,tot1=0;
		int m1=0,m0=0;
		for(i=0;i<n;i++)
		{
			if(color[i]==0)new_id.push_back(tot0++);
			else new_id.push_back(tot1++);
		}
		for(i=0;i<n;i++)
				for(j=head[i];j;j=next[j])
					if(1^color[list[j]]^color[i])
					{
						if(color[i]==0)m0++;
						else m1++;
					}
		G1.init(tot0,m0);
		for(i=0;i<n;i++)
			if(color[i]==0)
				for(j=head[i];j;j=next[j])
					if(color[list[j]]==color[i])
						G1.add_D(new_id[i],new_id[list[j]],cost[j]);
		G2.init(tot1,m1);
		for(i=0;i<n;i++)
			if(color[i]==1)
				for(j=head[i];j;j=next[j])
					if(color[list[j]]==color[i])
						G2.add_D(new_id[i],new_id[list[j]],cost[j]);
		tot0=tot1=0;
		for(i=0;i<n;i++)
		{
			if(color[i]==0)G1.id[tot0++]=id[i];
			else G2.id[tot1++]=id[i];
		}
		if(DEBUG1)fprintf(stderr, "Split_over\n");
		return color;
	}
	int Split_Borders(int nparts)//将该图划分为nparts块后会产生的border数
	{
		if(n<Naive_Split_Limit)
		{
			return Naive_Split_Limit;
		}
		idx_t options[METIS_NOPTIONS];
		memset(options,0,sizeof(options));
		{
			METIS_SetDefaultOptions(options);
			options[METIS_OPTION_PTYPE] = METIS_PTYPE_KWAY; // _RB
			options[METIS_OPTION_OBJTYPE] = METIS_OBJTYPE_CUT; // _VOL
			options[METIS_OPTION_CTYPE] = METIS_CTYPE_SHEM; // _RM
			options[METIS_OPTION_IPTYPE] = METIS_IPTYPE_RANDOM; // _GROW _EDGE _NODE
			options[METIS_OPTION_RTYPE] = METIS_RTYPE_FM; // _GREEDY _SEP2SIDED _SEP1SIDED
			// options[METIS_OPTION_NCUTS] = 1;
			// options[METIS_OPTION_NITER] = 10;
			/* balance factor, used to be 500 */
			options[METIS_OPTION_UFACTOR] = 500;
			// options[METIS_OPTION_MINCONN];
			options[METIS_OPTION_CONTIG] = 1;
			// options[METIS_OPTION_SEED];
			options[METIS_OPTION_NUMBERING] = 0;
			// options[METIS_OPTION_DBGLVL] = 0;
		}
		idx_t nvtxs=n;
		idx_t ncon=1;
		vector<int>color(n);
		int *xadj = new idx_t[n + 1];
		int *adj=new idx_t[n+1];
		int *adjncy = new idx_t[tot-1];
		int *adjwgt = new idx_t[tot-1];
		int *part = new idx_t[n];


		int xadj_pos = 1;
		int xadj_accum = 0;
		int adjncy_pos = 0;


		xadj[0] = 0;
		int i = 0;
		for (int i=0;i<n;i++){
			for(int j=head[i];j;j=next[j])
			{
				int enid = list[j];
				xadj_accum ++;
				adjncy[adjncy_pos] = enid;
				adjwgt[adjncy_pos] = cost[j];
				adjncy_pos ++;
			}
			xadj[xadj_pos++] = xadj_accum;
		}
		for ( int i = 0; i < adjncy_pos; i++ ){
			adjwgt[i] = 1;
		}
		int objval=0;
		METIS_PartGraphKway(
			&nvtxs,
			&ncon,
			xadj,
			adjncy,
			NULL,
			NULL,
			adjwgt,
			&nparts,
			NULL,
			NULL,
			options,
			&objval,
			part
		);
		for(int i=0;i<n;i++)color[i]=part[i];
		//划分
		int j,re=0;
		for(i=0;i<n;i++)
			for(j=head[i];j;j=next[j])
				if(color[i]!=color[list[j]])
				{
					re++;
					break;
				}
		delete [] xadj;
		delete [] adj;
		delete [] adjncy;
		delete [] adjwgt;
		delete [] part;
		return re;
	}
	struct state{state(int a=0,int b=0,int c=0):id(a),len(b),index(c){}int id,len,index;};//用于dijkstra的二元组
	struct cmp{bool operator()(const state &a,const state &b){return a.len>b.len;}};//重载priority_queue的比较函数
	void dijkstra(int S,vector<int> &dist)//依据本图计算以S为起点的全局最短路将结果存入dist
	{
		priority_queue<state,vector<state>,cmp>q;
		state now;
		int i;
		dist.clear();
		while((int)dist.size()<n)dist.push_back(INF);
		q.push(state(S,0));
		while(q.size())
		{
			now=q.top();
			q.pop();
			if(dist[now.id]==INF)
			{
				dist[now.id]=now.len;
				for(i=head[now.id];i;i=next[i])
					if(dist[list[i]]==INF)q.push(state(list[i],dist[now.id]+cost[i]));
			}
		}
	}
	vector<int> KNN(int S,int K,vector<int>T)//暴力dijkstra计算S到T数组中的前K小并返回其在T数组中的下标
	{
		int i;
		vector<int>dist(n,INF),Cnt(n,0);
		for(i=0;i<T.size();i++)Cnt[T[i]]++;
		priority_queue<state,vector<state>,cmp>q;
		state now;
		q.push(state(S,0));
		int bound,cnt=0;
		while(q.size()&&cnt<K)
		{
			now=q.top();
			q.pop();
			if(dist[now.id]==INF)
			{
				dist[now.id]=now.len;
				cnt+=Cnt[now.id];
				if(cnt>=K)bound=now.len;
				for(i=head[now.id];i;i=next[i])
					if(dist[list[i]]==INF)q.push(state(list[i],dist[now.id]+cost[i]));
			}
		}
		vector<int>re;
		for(int i=0;i<T.size()&&re.size()<K;i++)
			if(dist[T[i]]<=bound)
				re.push_back(i);
		return re;
	}
	vector<int> find_path(int S,int T)//依据本图计算以S为起点的全局最短路将结果存入dist
	{
		vector<int>dist,re,last;
		priority_queue<state,vector<state>,cmp>q;
		state now;
		int i;
		dist.clear();
		last.clear();
		re.clear();
		while((int)dist.size()<n)
		{
			dist.push_back(INF);
			last.push_back(0);
		}
		q.push(state(S,0));
		while(q.size())
		{
			now=q.top();
			q.pop();
			if(dist[now.id]==INF)
			{
				dist[now.id]=now.len;
				for(i=head[now.id];i;i=next[i])
				{
					if(dist[list[i]]==INF)q.push(state(list[i],dist[now.id]+cost[i]));
					if(dist[list[i]]+cost[i]==dist[now.id])last[now.id]=list[i];
				}
			}
		}
		if(dist[T]==INF)return re;
		else
		{
			for(i=T;i!=S;i=last[i])re.push_back(i);
			re.push_back(S);
			reverse(re.begin(),re.end());
			return re;
		}
	}
	int real_node()
	{
		int ans=0;
		for(int i=0;i<n;i++)
		{
			int k=0;
			for(int j=head[i];j;j=next[j])k++;
			if(k!=2)ans++;
		}
		return ans;
	}

	//给定起点集合S，处理到每个结点的前K短路长度以及起点编号在list中的编号
	vector<vector<int> >K_Near_Dist,K_Near_Order;
	void KNN_init(const vector<int> &S , int K)
	{
		priority_queue<state,vector<state>,cmp>q;
		state now;
		int i;
		vector<int>empty;
		K_Near_Dist.clear();
		K_Near_Order.clear();
		while((int)K_Near_Dist.size()<n)
		{
			K_Near_Dist.push_back(empty);
			K_Near_Order.push_back(empty);
		}
		for(int i=0;i<S.size();i++)
		q.push(state(S[i],0,i));
		while(q.size())
		{
			now=q.top();
			q.pop();
			if(K_Near_Dist[now.id].size()<K)
			{
				K_Near_Dist[now.id].push_back(now.len);
				K_Near_Order[now.id].push_back(now.index);
				for(i=head[now.id];i;i=next[i])
					if(K_Near_Dist[list[i]].size()<K)q.push(state(list[i],now.len+cost[i]));
			}
		}
	}
	vector<int>* KNN_Dijkstra(int S){return &K_Near_Order[S];}
};

extern Graph G;