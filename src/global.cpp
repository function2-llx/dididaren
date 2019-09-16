#include <cmath>
#include <vector>
#include "global.h"
#include "i_network.h"
#include "point.h"

// int times[10];//辅助计时变量；
int cnt_type0,cnt_type1;

using namespace std;
const bool DEBUG_=false;
const bool Optimization_G_tree_Search=true;//是否开启全连接加速算法
const bool Optimization_KNN_Cut=true;//是否开启KNN剪枝查询算法
const bool Optimization_Euclidean_Cut=true;//是否开启Catch查询中基于欧几里得距离剪枝算法
const char Edge_File[]="./data/road.nedge";//第一行两个整数n,m表示点数和边数，接下来m行每行三个整数U,V,C表示U->V有一条长度为C的边
const char Node_File[]="./data/road.cnode";//共N行每行一个整数两个实数id,x,y表示id结点的经纬度(但输入不考虑id，只顺序从0读到n-1，整数N在Edge文件里)
const double Unit=1.0;//路网文件的单位长度/m
// const double R_earth=6371000.0;//地球半径，用于输入经纬度转化为x,y坐标
const double PI=acos(-1.0);
const int Partition_Part=4;//K叉树
long long Additional_Memory=0;//用于构建辅助矩阵的额外空间(int)
const int Naive_Split_Limit=33;//子图规模小于该数值全划分
const int INF=0x3fffffff;//无穷大常量
const bool RevE=true;//false代表有向图，true代表无向图读入边复制反向一条边
const bool Distance_Offset=false;//KNN是否考虑车辆距离结点的修正距离
const bool DEBUG1=false;
#define TIME_TICK_START gettimeofday( &tv, NULL ); ts = tv.tv_sec * 100000 + tv.tv_usec / 10;
#define TIME_TICK_END gettimeofday( &tv, NULL ); te = tv.tv_sec * 100000 + tv.tv_usec / 10;
#define TIME_TICK_PRINT(T) fprintf(stderr, "%s RESULT: %lld (0.01MS)\r\n", (#T), te - ts );
// struct timeval tv;
long long ts, te;
// static int  = 0;


vector<coor>coordinate;
// Point bcoor[INetwork::n];

double Distance_(double a1,double b1,double a2,double b2 )
{
	double x,y,z,AB,A_B;
	if(b1 < 0)
	{
		b1 = abs(b1) + 180;
	}
	if(b2 < 0)
	{
		b2 = abs(b2) + 180;
	}
	a1 = a1 * PI /180;
	b1 = b1 * PI/180;
	a2 = a2 * PI /180;
	b2 = b2 * PI /180;
	x = R_earth * cos(b1)*cos(a1) - R_earth * cos(b2)*cos(a2);
	y = R_earth * cos(b1)*sin(a1) - R_earth * cos(b2)*sin(a2);
	z = R_earth * sin(b1) - R_earth * sin(b2);
	AB = sqrt(x*x + y*y + z*z);
	A_B= 2 * asin(AB /(2*R_earth)) * R_earth;
	return A_B;
}
double Euclidean_Dist(int S,int T)//计算节点S,T的欧几里得距离在路网数据中的长度
{
	return Distance_(coordinate[S].x,coordinate[S].y,coordinate[T].x,coordinate[T].y)/Unit;
}

double get_dis(const coor& a, const coor& b) {
    return Distance_(a.x, a.y, b.x, b.y);
}

#include <iomanip>

ostream& operator << (ostream& os, const coor& p)
{
    os << fixed << setprecision(6) << '(' << p.x << " ," << p.y << ')';
    return os;
}


string quote(const string& a)
{
    return "\"" + a + "\"";
}