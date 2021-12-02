# Simple WebServer 
简单的web服务器，主要用于学习网络服务器相关的知识

## 构建 & 编译
```shell
$ mkdir build && cd build
$ cmake .. 
$ make -j4
```

## 运行测试
进入build目录下，运行第10天的 示例
```shell
$ cd build
$ ./bin/html_server_conf ../tests/simpleweb.conf
```
在浏览器中访问 http://127.0.0.1:8080
