#pragma once

#include <cmath>
#include <vector>
// #include "i_network.h"

// extern int times[10];//辅助计时变量；
extern int cnt_type0,cnt_type1;

using namespace std;
extern const bool DEBUG_;
extern const bool Optimization_G_tree_Search;//是否开启全连接加速算法
extern const bool Optimization_KNN_Cut;//是否开启KNN剪枝查询算法
extern const bool Optimization_Euclidean_Cut;//是否开启Catch查询中基于欧几里得距离剪枝算法
extern const char Edge_File[];//第一行两个整数n,m表示点数和边数，接下来m行每行三个整数U,V,C表示U->V有一条长度为C的边
extern const char Node_File[];//共N行每行一个整数两个实数id,x,y表示id结点的经纬度(但输入不考虑id，只顺序从0读到n-1，整数N在Edge文件里)
extern const double Unit;//路网文件的单位长度/m
const double R_earth = 6371000.0;//地球半径，用于输入经纬度转化为x,y坐标
const double limit_x = 10000.0 / R_earth;
extern const double PI;
extern const int Partition_Part;//K叉树
extern long long Additional_Memory;//用于构建辅助矩阵的额外空间(int)
extern const int Naive_Split_Limit;//子图规模小于该数值全划分
extern const int INF;//无穷大常量
extern const bool RevE;//false代表有向图，true代表无向图读入边复制反向一条边
extern const bool Distance_Offset;//KNN是否考虑车辆距离结点的修正距离
extern const bool DEBUG1;

#define TIME_TICK_START gettimeofday( &tv, NULL ); ts = tv.tv_sec * 100000 + tv.tv_usec / 10;
#define TIME_TICK_END gettimeofday( &tv, NULL ); te = tv.tv_sec * 100000 + tv.tv_usec / 10;
#define TIME_TICK_PRINT(T) fprintf(stderr, "%s RESULT: %lld (0.01MS)\r\n", (#T), te - ts );

// struct timeval tv;
using tv = timeval;
extern long long ts, te;
// static int rootp = 0;

// const int a = 1;
#include <iostream>

struct coor{coor(double a=0.0,double b=0.0):x(a),y(b){}double x,y;};

ostream& operator << (ostream& os, const coor& p);

extern vector<coor>coordinate;
class Point;
// extern Point bcoor[338024];
double Distance_(double a1,double b1,double a2,double b2 );
double Euclidean_Dist(int S,int T);//计算节点S,T的欧几里得距离在路网数据中的长度
double get_dis(const coor& a, const coor& b);

#include <string>

std::string quote(const string& a);