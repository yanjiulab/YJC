# YJC

## base

`base` 为基础组件

头文件|功能|说明
:---:|:---:|:---:
defs.h|预定义头|包含下面头文件
math.h|
platform.h|
export.h|
types.h|常用类型|变量以t结尾

## network

`network` 为网络编程基础库

头文件|功能|说明
:---:|:---:|:---:
`socket.h`|基本操作函数封装|
`ip_address.h`|IP 地址操作|
`netdev.h`|网卡操作（添加/删除/读取网卡信息）|基于 ip 命令实现

### IP 地址

格式|定义|说明
:---:|:---:|:---:
IPv4 地址| `struct in_addr v4;` 或者 `uint32_t v4;`|4 字节
IPv6 地址| `struct in6_addr v6;` |16 字节
IPv4 字符串|`char *ip_str;` 或者 `char ip_str[]` |一般采用点分十进制，占用 16 字节，如果加上后缀
IPv6 字符串|`char *ip_str;` 或者 `char ip_str[]` |一般采用点分十六进制，占用 16 字节
通用套接字地址结构|`struct sockaddr saddr;`|发送、接收、绑定、连接等函数需要使用该类型
IPv4 套接字地址结构|`struct sockaddr_in6 saddr;`|实际使用 IPv4 通信需要定义的地址，主要包含协议族、端口号、IPv4 地址信息
IPv6 套接字地址结构|`struct sockaddr_in6 s6addr;`|实际使用 IPv6 通信需要定义的地址，主要包含协议族、端口号、IPv6 地址信息等。

### 数据包

头文件 | 名称 | 说明
:---:|:---:|:---:
ifi.h|网络接口相关|读取接口，设置接口，
rt.h|路由表相关|
arp.h|ARP表相关|

## sniffer

`sniffer` 为网络数据包嗅探库，主要包括网卡数据包接收（基于 Linux packet socket）、协议解析、数据包处理；

头文件 | 名称 | 说明
:---:|:---:|:---:
`packet.h`|数据包结构|定义数据包 metadata
`packet_socket.h`|链路层套接字操作|嗅探基本机制，提供过滤机制
`packet_header.h`|数据包头文件集合|定义或引用常用协议的包头定义
`packet_parser.h`|数据包解析|字节->结构体
`packet_stringify.h`|数据包字符串化|结构体->字符串
`packet_generator.h`|数据包构造|根据 API 构造或根据特定字符串语法构造

TODO：支持自定义协议热挂载。

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
array.h|动态数组|暂时用于框架
vector.h|动态数组|support quueue-like and stack-like operations
list.h|链表|Linux 内核双向链表简化实现
map.h|哈希表|key 为字符串的哈希表实现
str.h|字符串|支持字符串去白、分割、构造、数值转换等功能
heap.h|二叉堆|Linux 内核风格堆实现（优先队列）
rbtree.h|红黑树|Linux 内核红黑树简化实现
ringbuf.h|循环缓存|摘自 FRR，通常用于在生产者和消费者之间共享数据的场景中
ptable.h|ASCII风格表打印|[marchelzo/libtable](https://github.com/marchelzo/libtable)

### 工具

头文件 | 名称 | 说明
:---:|:---:|:---:
args.h|命令行参数解析|TODO
log.h|日志系统|6 个日志等级，支持日志文件
ini.h|ini 配置文件读取|标准 ini 文件读取
xml.h|xml 文件生成/解析|TODO
json.h|json 文件生成/解析|TODO
protobuf.h|protobuf 相关|TODO，查看protobuf-c
thread.h|pthread 线程相关|简单包装了常用 API
thpool.h|基于 pthread 线程的线程池基本实现|[Pithikos/C-Thread-Pool](https://github.com/Pithikos/C-Thread-Pool)

## TODO

- atomic
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
