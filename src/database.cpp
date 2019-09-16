#include "IGTree.h"
#include "global.h"
#include "i_network.h"
#include <v8.h>
#include <node.h>
#include "i_cars.h"

using namespace v8;
using namespace std;

void test_interface(const FunctionCallbackInfo<Value>& args)
{
    Isolate *isolate = args.GetIsolate();
    args.GetReturnValue().Set(String::NewFromUtf8(isolate, "Interface ok"));
}

void get_pos(const FunctionCallbackInfo<Value>& args)
{
    Isolate *isolate = args.GetIsolate();
    double x = args[0]->NumberValue(), y = args[1]->NumberValue();
    auto callback = Local<Function>::Cast(args[2]);
    auto pos = coordinate[INetwork::closest(coor(x, y))];

    Local<Value> argv[] = {Number::New(isolate, pos.x), Number::New(isolate, pos.y)};
    Local<Array> points;
    callback->Call(Null(isolate), 2, argv);
}

void get_cars(const FunctionCallbackInfo<Value>& args)
{
    Isolate *isolate = args.GetIsolate();
    // double origin_x = args[0]->NumberValue(), origin_y = args[1]->NumberValue();
    Point origin(args[0]->NumberValue(), args[1]->NumberValue()), dest(args[2]->NumberValue(), args[3]->NumberValue());
    auto callback = Local<Function>::Cast(args[4]);
    auto cars_id = ICars::get_cars(origin, dest);
    int cars_num = cars_id.size();

    cerr << "get cars success, size: " << cars_num << endl;
    for (auto id: cars_id)
        cerr << id << endl;

    std::string ret = "[";
    for (int i = 0; i < cars_num; i++) {
        auto &car = ICars::get_car(cars_id[i]);
        ret += car.to_json();
        if (i < cars_num - 1)
            ret += ",";
        
        vector<int>().swap(car.route);
    }
    ret += "]";

    int argc = 1;
    Local<Value> argv[1] = {String::NewFromUtf8(isolate, ret.c_str())};

    callback->Call(Null(isolate), argc, argv);
}

void init(Local<Object> exports)
{
    IGTree::init();
    INetwork::init();
    ICars::init();

    NODE_SET_METHOD(exports, "testInterface", test_interface);
    NODE_SET_METHOD(exports, "getPos", get_pos);
    NODE_SET_METHOD(exports, "getCars", get_cars);
}

NODE_MODULE(NODE_GYP_MODULE_NAME, init);