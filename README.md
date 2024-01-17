# YJC

## base

TODO: 其余移出 base 模块

`base` 为基础组件

头文件|功能|说明
:---:|:---:|:---:
defs.h|预定义头|包含下面头文件
math.h|
platform.h|
export.h|
types.h|常用类型|变量以t结尾

## event

`event` 为事件库。

`event_t` 为事件结构体，主要成员包括：

- 事件所属 eventloop
- 事件类型
- 事件回调函数
- 事件状态

## network

`network` 为网络编程基础库

头文件|功能|说明
:---:|:---:|:---:
`socket.h`|基本操作函数封装|
`ipaddr.h`|IP 地址操作|
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
base64.h|BASE64 编解码|标准编解码操作
checksum.h|Internet 校验和|取自 FRR
crc.h|CRC 循环冗余校验|计算法（多项式、初始值、输入字节比特反转、输出比特反转、输出异或），速度没有查表快。
md5.h|MD5 数字摘要|标准 128 位数字摘要实现，支持二进制或字符串输出
sha1.h|SHA1 安全散列算法|标准 160 位安全散列实现，支持二进制或字符串输出
sha256.h|SHA256 安全散列算法|标准 256 位安全散列实现，支持二进制或字符串输出

### 工具

头文件 | 名称 | 说明
:---:|:---:|:---:
args.h|命令行参数解析|[dmulholl/Args](https://github.com/dmulholl/args)
iniparser.h|ini 配置文件读写|[N.Devillard/iniparser](todo)
xml.h|xml 文件生成/解析|TODO
json.h|json 文件生成/解析|TODO
cmd.h|命令行控制台库|TODO
protobuf.h|protobuf 相关|TODO，查看protobuf-c
thread.h|pthread 线程相关|简单包装了常用 API
thpool.h|基于 pthread 线程的线程池基本实现|[Pithikos/C-Thread-Pool](https://github.com/Pithikos/C-Thread-Pool)

## 数据结构

头文件 | 名称 | 说明
:---:|:---:|:---:
array.h|动态数组|暂时用于框架
vector.h|动态数组|support quueue-like and stack-like operations
list.h|链表|Linux 内核双向链表简化实现，未采用
linklist.h|链表|Zebra 双向链表实现，采用
hashmap.h|哈希表|[tidwall/hashmap.c](https://github.com/tidwall/hashmap.c)
hash.h|哈希表|Zebra的哈希表实现，未采用。
str.h|字符串|支持字符串去白、分割、构造、数值转换等功能，代替换
sds.h|字符串|Redis的动态字符串实现。
heap.h|二叉堆|Linux 内核风格堆实现（优先队列）
rbtree.h|红黑树|Linux 内核红黑树简化实现
ringbuf.h|循环缓存|摘自 FRR，通常用于在生产者和消费者之间共享数据的场景中
ptable.h|ASCII风格表打印|[marchelzo/libtable](https://github.com/marchelzo/libtable)

### 哈希表 (hashmap)

摘自 [tidwall/hashmap.c](https://github.com/tidwall/hashmap.c)，MIT Licence。

选择理由：

- 无复杂依赖，仅需 C99 特性。
- 支持自定义 items。
- 使用 Open addressing - Robin Hood 哈希，代码简洁易懂。
- API 易用。

API 如下所示，具体参见项目主页。

```sh
hashmap_new      # allocate a new hash map
hashmap_free     # free the hash map
hashmap_count    # returns the number of items in the hash map
hashmap_set      # insert or replace an existing item and return the previous
hashmap_get      # get an existing item
hashmap_del      # delete and return an item
hashmap_clear    # clear the hash map
hashmap_iter     # loop based iteration over all items in hash map 
hashmap_scan     # callback based iteration over all items in hash map
```

### 总结

API|链表|哈希表
:---:|:---:|:---:
数据结构|struct list*|struct hashmap*
新建容器|list_new<br>list_create|hashmap_new
删除容器|list_free|hashmap_free
遍历容器（循环）|list_foreach<br>list_foreach_ro|hashmap_iter
遍历容器（回调）|list_scan|hashmap_scan
获取节点数量|list_count|hashmap_count
添加节点|list_add<br>list_add_*|hashmap_set
更新节点|No API|hashmap_set
删除节点|list_del|hashmap_del
获取节点|list_get|hashmap_get

## TODO

- 完善 atomic
- 完善 netllink 应用
- 信号机制
- 抽象为 sevl.h：极简事件循环库。
- 更改 Makefile，可读性，降低依赖，加速编译速度。
- 如何统一 DB 的API，建议分模块操作。sqlite3 / redis
- 基于 cmd.h 开发 ecmd.h，主要增加非阻塞回调。
- 统一 base 库，将采纳的常用模块全部放入 base 库中，base 库必须是无依赖的，且需要编译成 lib 文件，目前 base.c 中仍有混乱部分，主要是宏定义和 wrapper 部分。
- 未来分为 base 库：大概率会用到的基础函数及功能，包括通用的宏定义、类型定义、多线程编程、配置文件读写、命令行参数解析、常用数据结构等。可以直接扔到别的项目中编译。
  - event 库：改进或者替换。
  - network 库：网络相关的基础库。
  - netlink 应用库：Netlink 编程库，依赖 base、network，需要编译成 lib 文件。
  - util 库：不太常用的算法和工具函数，util 库最好是无依赖的。
- grpc 和 protobuf，不着急
- cspf.ch 受限最短路径，不着急。
- UI 界面开发，放到后面，基于 nurses。
