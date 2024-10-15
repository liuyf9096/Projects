用于连接 server 的 jsonrpc 通讯库，适用window,mac,arm平台。

编译：
编译工程后，把 lib 文件夹 和 include 文件夹 拷贝至工程目录，导入工程即可使用

使用：
1.调用connectServer( ip地址， 端口号， 是否短线重连 ) 即可连接
2.断线重连默认1000ms，可通过调用函数修改。
3.给server发送消息，有string，jsonobject，packet 三种方式可选，接收也一样
4.该库可MoveToThread到子线程中运行，不影响其他模块，已通过10万以上通讯检测，放心使用。
5.如有不明白的可以看example的demo。如要运行demo最好用自己的环境重新编译lib再运行。

编辑于 2021-11-12 liuyufei