用于创建 server 的 jsonrpc 通讯库，适用window,mac,arm平台。

编译：
编译工程后，把 lib 文件夹 和 include 文件夹 拷贝至工程目录，导入工程即可使用

使用：
1. listen 端口后，开始监听所有的连接请求；
2. setClientUserName 用于设置自定义客户端名称，仅用于打印信息方便显示；
3. sendMessage、sendJsonObject、sendPacket 任选3中方式，向指定的 client_id 发送信息，推荐用 sendPacket。
4. 该库已放置独立线程运行，不用再设置多线程。
5.如有不明白的可以看example的demo。如要运行demo最好用自己的环境重新编译lib再运行。

编辑于 2021-11-12 liuyufei