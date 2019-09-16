#include "i_cars.h"

#include "i_network.h"
#include "IGTree.h"
#include "global.h"

#include <cstdio>

using namespace ICars;
using namespace std;

Car ICars::cars[n];
static vector<int> cars_pos;

void ICars::init()
{
    auto fin = fopen("./data/car.txt", "r");

    for (int i = 0; i < n; i++) {
        auto &car = cars[i];
        car.read(fin);
        cars_pos.push_back(car.pos);
    }

    cerr << "cars loaded" << endl;
}


vector<int> ICars::get_cars(const Point& origin_p, const Point& dest_p)
{
    int origin = INetwork::closest(origin_p), dest = INetwork::closest(dest_p);
    const int filt = 200;  //  选200个
    const int dis_lim = 10 * 1000 / Unit;
    vector<int> tmp = IGTree::tree.KNN(origin, filt, cars_pos);

    vector<int> ret;

    for (auto x: tmp) {
        auto &car = cars[x];
        if (car.dests.size() < 4) {
            // cerr << x << endl;
            car.pick(origin, dest);
            if (car.legal(dis_lim)) {
                ret.push_back(x);
                if (ret.size() == 5)
                    break;
            }
        }
    }

    assert(ret.size() <= 5);

    sort(ret.begin(), ret.end(), [&](int a, int b) { return cars[a].d2 < cars[b].d2; });
    for (auto x: ret) {
        cars[x].get_route(origin);
    }

    return ret;
}

Car& ICars::get_car(int id) { return cars[id]; }

void Car::pick(int origin, int dest) {
    this->d1 = INetwork::hamilton(pos, dests).dis;
    this->d2 = INetwork::search(pos, origin);
    dests.push_back(dest);
    auto plan = INetwork::hamilton(origin, dests);
    this->route = plan.points;
    auto points = plan.points;
    this->d3 = plan.dis;
    dests.pop_back();
    this->d4 = INetwork::search(origin, dest);
}

void Car::get_route(int origin) {
    auto tmp = INetwork::get_path(pos, origin);
    for (int i = 0; i + 1 < route.size(); i++) {
        auto cur = INetwork::get_path(route[i], route[i + 1]);
        tmp.insert(tmp.end(), cur.begin(), cur.end());
    }
    tmp.resize(unique(tmp.begin(), tmp.end()) - tmp.begin());
    // this->route = INetwork::get_path(pos, origin);
    this->route = tmp;

    // for (int i = 0; i < route.size(); i++)
    //     cerr << route[i] << ' ';
    // cerr << endl;
}