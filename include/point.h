#include "global.h"

struct Point : public coor {
    Point(double x = 0, double y = 0) : coor(x, y) {}
    Point(const coor& c) : coor(c) {}
    std::string to_json() const 
    {
        static char buf[1024];
        sprintf(buf, "{\"x\":%lf,\"y\":%lf}", x, y);
        return buf;
        // return "{\"x\":" + std::to_string(x) + ",\"y\":" + to_string(y) + "}"
    }
};

