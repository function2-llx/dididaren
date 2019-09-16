#pragma once

#include <cstdio>
#include "global.h"
// #include "point.h"

class Point;

namespace INetwork {
    const int n = 338024;
    const int m = 440525;

    // extern Point bcoor[n];

    struct Route {
        int dis = 0;
        std::vector<int> points;

        Route(int origin) { points.push_back(origin); }
    };

    void init();
    int closest(const coor&);
    int search(int a, int b);
    vector<int> get_path(int a, int b);
    Route hamilton(int origin, const std::vector<int>& points);
    vector<int> KNN_search(int s);
};