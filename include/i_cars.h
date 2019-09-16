#ifndef I_CARS
#define I_CARS

#include <vector>
#include "point.h"

namespace ICars {
    const int n = 100000;

    struct Car {
        int id;
        int pos;
        vector<int> dests;
        // int dis;
        int d1, d2, d3, d4;
        vector<int> route;
        // INetwork::Route route;

        void read(FILE* fin)
        {
            int k;
            fscanf(fin, "%d %d ", &id, &k);
            dests.resize(k);
            // scanf("%d", &k);
            double tmp;
            fscanf(fin, "%lf,%lf,%d", &tmp, &tmp, &pos);
            for (int i = 0; i < k; i++) {
                fscanf(fin, "%lf,%lf,%d", &tmp, &tmp, &dests[i]);
            }
        }

        std::string to_json() const {
            std::string ret = "{";

            ret += quote("pos") + ":" + Point(coordinate[pos]).to_json();
            ret += ",";

            ret += quote("dis") + ":" + to_string(d2);
            ret += ",";

            ret += quote("dest") + ":[";
            for (int i = 0; i < dests.size(); i++) {
                ret += Point(coordinate[dests[i]]).to_json();
                if (i + 1 < dests.size()) {
                    ret += ",";
                }
            }
            ret += "]";
            ret += ",";

            ret += quote("detour1") + ":" + to_string(detour_on_car());
            ret += ",";

            ret += quote("detour2") + ":" + to_string(detour_passenger());
            ret += ",";

            ret += quote("route") + ":[";
            for (int i = 0; i < route.size(); i++) {
                ret += Point(coordinate[route[i]]).to_json();

                if (i + 1 < route.size())
                    ret += ",";
            }
            ret += "]";

            ret += "}";
            return ret;
        }

        void pick(int origin, int dest);
        int detour_on_car() const {
            if (this->dests.empty())
                return 0;
            return d2 + d3 - d1;
        }
        int detour_passenger() const { return d3 - d4; }
        bool legal(int lim) const { return d2 < lim && detour_on_car() < lim && detour_passenger() < lim; }
        void get_route(int origin);
    };

    extern Car cars[n];

    void init();

    vector<int> get_cars(const Point& origin, const Point& dest);

    Car& get_car(int id);
}

#endif