# YJC

## 目录结构

### base

`base` 为基础组件

### net

`net` 为网络编程库

### util

`util` 为工具库，包含了各种常用工具、数据结构及算法，每个功能均使用一对 `.c` 和 `.h` 文件实现，个别功能仅用 `.h` 实现，**没有外部依赖**，可以根据需要直接拖到自己的项目中使用。

头文件 | 名称 | 说明
:---:|:---:|:---:
args.h|命令行参数解析|TODO
log.h|日志系统|6 个日志等级，支持日志文件
ini.h|ini 配置文件读取|标准 ini 文件读取
base64.h|BASE64编解码|标准编解码操作
crc.h|CRC循环冗余校验|支持 21 种 CRC 格式
md5.h|MD5数字摘要|标准 128 位数字摘要实现，支持二进制或字符串输出
sha1.h|SHA1安全散列算法|标准 160 位安全散列实现，支持二进制或字符串输出
str.h|字符串|支持字符串去白、分割、构造、数值转换等高级功能
list.h|链表|Linux 内核双向链表简化实现
map.h|哈希表|key 为字符串的哈希表实现
heap.h|二叉堆|Linux 内核风格堆实现（优先队列）
rbtree.h|红黑树|Linux 内核红黑树简化实现

## TODO

- change makefile 这可以解决 cjson 的警告
- 数据库
- ui 界面
- 控制台命令系统 vtysh
- db.ch sqlite3 / redis
- cspf.ch 受限最短路径
- py.ch
- prettytable [marchelzo/libtable](https://github.com/marchelzo/libtable)
- 标准化 makefile 文件
- 参考 frr 的 hash.h/c
- netllink
- protobuf
- grpc
- libpcap
- libnet
- ncurses
- config check
