#include <cassert>
#include "i_network.h"
#include "graph.h"
#include "global.h"
#include "IGTree.h"

using namespace INetwork;
using namespace std;

static int id_x[n]; //  sorted by x
static auto x_cmp = [] (int a, int b) { return coordinate[a].x < coordinate[b].x; };
static vector<int> all;

void INetwork::init()
{
    cerr << "init network" << endl;
    auto fin = fopen("data/bnode.txt", "r");
    coordinate.resize(n);
    for (int i = 0; i < n; i++) {
        int id;
        double x, y;
        fscanf(fin, "%d%lf%lf", &id, &x, &y);
        coordinate[id] = coor(x, y);
    }
    fclose(fin);

    assert((int)coordinate.size() == n);

    for (int i = 0; i < n; i++)
        id_x[i] = i;

    sort(id_x, id_x + n, x_cmp);
}

int INetwork::closest(const coor& p)
{
    // cerr << "origina: " << p << endl;
    // auto lower_bound = [&] {
    //     int l = 0, r = n - 1, ret = n;
    //     while (l <= r) {
    //         int m = (l + r) >> 1;
    //         if (coordinate[id_x[m]].x >= p.x - 0.1) {
    //             ret = m;
    //             r = m - 1;
    //         } else {
    //             l = m + 1;
    //         }
    //     }
    //     return ret;
    // };

    // auto upper_bound = [&] {
    //     int l = 0, r = n - 1, ret = 0;
    //     while (l <= r) {
    //         int m = (l + r) >> 1;
    //         if (coordinate[id_x[m]].x <= p.x + 0.1) {
    //             ret = m;
    //             l = m + 1;
    //         } else {
    //             r = m - 1;
    //         }
    //     }
    //     return ret;
    // };

    // int l = lower_bound(), r = upper_bound(), ret = l;
    int l = 0, r = n - 1, ret = l;
    double val = 1e100;
    for (int i = l; i <= r; i++) {
        double cur = get_dis(coordinate[i], p);
        if (cur < val) {
            val = cur;
            ret = i;
        }
    }
    return ret;
}

int INetwork::search(int a, int b) { return  IGTree::tree.search(a, b); }

static void dfs(int id, int pre, const vector<int>& points, Route& cur, Route& ans, bool vis[]) {
    if (cur.dis > ans.dis)
        return;
    if (id == points.size()) {
        ans = cur;
        return;
    }

    for (int i = 0; i < points.size(); i++) if(!vis[i]) {
        int pos = points[i];
        vis[i] = 1;
        cur.points.push_back(pos);
        int dis = search(pre, pos);
        cur.dis += dis;

        dfs(id + 1, pos, points, cur, ans, vis);

        cur.dis -= dis;
        cur.points.pop_back();
        vis[i] = 0;
    }
}

Route INetwork::hamilton(int origin, const vector<int>& points) {
    static bool vis[5];
    for (int i = 0; i < points.size(); i++)
        vis[i] = 0;

    Route ans(origin), cur(origin);
    ans.dis = 0x7fffffff;

    ::dfs(0, origin, points, cur, ans, vis);
    return ans;
}

vector<int> INetwork::get_path(int a, int b) {
    vector<int> ret;
    IGTree::tree.find_path(a, b, ret);
    return ret;
}