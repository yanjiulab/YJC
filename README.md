# YJC

## base

`base` 为基础组件

## net

`net` 为网络编程库

## util

`util` 为工具库，包含了各种常用工具、数据结构及算法，每个功能均使用一对 `.c` 和 `.h` 文件实现，个别功能仅用 `.h` 实现，**没有外部依赖**，可以根据需要直接拖到自己的项目中使用。

### 算法

头文件 | 名称 | 说明
:---:|:---:|:---:
base64.h|BASE64编解码|标准编解码操作
checksum.h|Internet校验和|取自 FRR
crc.h|CRC循环冗余校验|计算法（多项式、初始值、输入字节比特反转、输出比特反转、输出异或），速度没有查表快。
md5.h|MD5数字摘要|标准 128 位数字摘要实现，支持二进制或字符串输出
sha1.h|SHA1安全散列算法|标准 160 位安全散列实现，支持二进制或字符串输出
sha256.h|SHA256安全散列算法|标准 256 位安全散列实现，支持二进制或字符串输出

### 数据结构

头文件 | 名称 | 说明
:---:|:---:|:---:
array.h|动态数组|TODO
vector.h|动态数组|TODO
list.h|链表|Linux 内核双向链表简化实现
map.h|哈希表|key 为字符串的哈希表实现
str.h|字符串|支持字符串去白、分割、构造、数值转换等高级功能
heap.h|二叉堆|Linux 内核风格堆实现（优先队列）
rbtree.h|红黑树|Linux 内核红黑树简化实现
ringbuf.h|循环缓存|TODO
ptable.h|ASCII风格表打印|[marchelzo/libtable](https://github.com/marchelzo/libtable)

### 工具

头文件 | 名称 | 说明
:---:|:---:|:---:
args.h|命令行参数解析|TODO
log.h|日志系统|6 个日志等级，支持日志文件
ini.h|ini 配置文件读取|标准 ini 文件读取
xml.h|xml 文件生成/解析|TODO
json.h|json 文件生成/解析|TODO

## TODO

- atomic
- thread pool
- grpc
- protobuf
- netllink
- 数据库 db.ch sqlite3 / redis
- 控制台命令系统 vtysh
- cspf.ch 受限最短路径
- py.ch
- 标准化 makefile 文件（build 系统）
- libpcap
- libnet
- ncurses
- ui 界面
