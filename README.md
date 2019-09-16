## 互联网出行

### 主要文件存放及定义

- `src/` 为 `c++` 代码存放目录，主要提供后端查询算法支持，`include/` 为对应头文件目录；
- `site/` 为 web 静态资源存放目录；
- `data/` 目录存放路网数据，出租车数据等数据；
- `binding.gyp` 用于将`c++` 代码编译为可供 `nodejs` 调用的动态链接库；
- `server.js` 为打车界面的后端代码。

### 构建与运行方法

首先，安装 `nodejs` 与 `npm`，以及需要的一些依赖包（如果安装过可以跳过这一步）：

```bash
$ sudo apt-get install nodejs
$ sudo apt-get install npm
$ npm install express
$ npm install node-gyp
```

然后构建 `c++` 工程，生成用于 `nodejs` 调用的  `.node`  动态链接库：

```bash
$ node-gyp configure
$ node-gyp build
```

启动服务器，等待控制台输出如下信息（如果没有加载成功，可能会输出额外的大量信息）：

```bash
$ node server.js
init GTree
load_begin
init network
cars loaded
Interface ok
App listening at port 8080
```

打开浏览器，输入网址 `localhost:8080/index.html`，可以看到 UI 界面。