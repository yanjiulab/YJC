# YJC

## 基础库 base

`base` 为基础组件

头文件|功能|说明
:---:|:---:|:---:
defs.h|预定义头|包含下面头文件
math.h|
platform.h|
export.h|
types.h|常用类型|变量以t结尾
iniparser.h|ini 配置文件读写|[N.Devillard/iniparser](todo)
thread.h|pthread 线程相关|简单包装了常用 API
thpool.h|基于 pthread 线程的线程池基本实现|[Pithikos/C-Thread-Pool](https://github.com/Pithikos/C-Thread-Pool)
cmdf.h|命令行控制台库|
csv.h|CSV 文件读写|

### 并发

并发主要涉及以下方面内容：

- 原子性：atomic.h
- 线程：thread.h
- 线程池：thpool.h
- 进程：proc.h

### 事件循环

### 常用工具

## 网络编程库 network

`network` 为网络编程基础库

头文件|功能|说明
:---:|:---:|:---:
`socket.h`|基本操作函数封装|
`ipaddr.h`|IP 地址操作|
`netdev.h`|网卡操作（添加/删除/读取网卡信息）|基于 ip 命令实现

### 网络地址相关

在编程中，常常涉及到与网络地址相关的地方，通常情况下，网络地址是指 IPv4 或 IPv6 地址。涉及到的数据结构包括：

- 整型：IP 地址本身就是一个整型数据，其中 IPv4 地址占用 32 位，IPv5 地址占用 128 位，**用于实际存储使用**，例如 IPv4 头中使用 4 字节存储。
  - `uint32_t`：IPv4 地址可直接使用 int 表示。
  - `typedef uint32_t in_addr_t;`：`in.h` 中定义用于 IPv4 的整型的别名。
  - `struct in_addr {in_addr_t s_addr;};`：`in.h` 中定义的 IPv4 结构体。
  - `struct in6_addr {...};`：`in.h` 中定义的 IPv6 结构体。
- 字符串：点分十进制表示的用户友好的显示格式，例如 `127.0.0.1`，或 `:1/128`，**用于日志打印及用户交互使用**。
- 套接字地址族：**用于套接字操作时与内核进行交互**的 IP 地址族结构体，例如 `bind()`、`accept()`、`sendto()`、`recvfrom()` 等函数中需要传入套接字地址结构。其不仅包含了表示 IP 地址的结构体，还增加了地址族，或端口号等其他信息。
  - `struct sockaddr`：通用结构。
  - `struct sockaddr_in`：IPv4 地址族结构。
  - `struct sockaddr_in6`：IPv6 地址族结构。

格式|定义|说明
:---:|:---:|:---:
IPv4 地址| `struct in_addr v4;` 或者 `uint32_t v4;`|4 字节
IPv6 地址| `struct in6_addr v6;` |16 字节
IPv4 字符串|`char *ip_str;` 或者 `char ip_str[]` |一般采用点分十进制，占用 16 字节，如果加上后缀
IPv6 字符串|`char *ip_str;` 或者 `char ip_str[]` |一般采用点分十六进制，占用 16 字节
通用套接字地址结构|`struct sockaddr saddr;`|发送、接收、绑定、连接等函数需要使用该类型
IPv4 套接字地址结构|`struct sockaddr_in6 saddr;`|实际使用 IPv4 通信需要定义的地址，主要包含协议族、端口号、IPv4 地址信息
IPv6 套接字地址结构|`struct sockaddr_in6 s6addr;`|实际使用 IPv6 通信需要定义的地址，主要包含协议族、端口号、IPv6 地址信息等。

### 套接字操作相关

### 数据读取相关

## 数据包嗅探库 sniffer

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

## 工具库 util

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
xml.h|xml 文件生成/解析|TODO
json.h|json 文件生成/解析|TODO

protobuf.h|protobuf 相关|TODO，查看protobuf-c

## 数据结构库 container

C 语言缺少基础的容器支持，许多 C 语言的项目中或多或少都实现了一些数据结构，用于项目内部容器。大部分的实现可以分为以下两类：

- 通用风格：定义**容器**和**容器单元**两种数据结构，其中，容器由若干容器单元组成，**而容器单元包括容器内部使用部分和数据部分**，其中数据部分通常实现为一个 `void *` 来指向用户自定义的结构体。简而言之，用户定义数据都包含在容器单元中，这也符合人类对于容器的直观认知。用户获取到节点后，需要自行将 `void *` 转换为自定义数据结构体指针，从而访问数据。
- 宏定义风格：与通用风格类似，但不使用 `void *` 而是使用宏实现，一般在创建容器时，以宏的方式将容器单元的用户数据类型替换为用户定义的数据类型，这样用户可以不用进行类型转换，且容器仅可用包含该类型数据。
- 嵌入风格：与以上两种风格不同，可以不包括容器数据结构（或容器数据结构非常简单），而仅定义容器单元，且容器单元仅包括容器内部使用部分，反而是用户自定义的数据需要包含容器单元。对容器的操作就是对自定义数据结构中容器单元的操作，Linux 内核大量使用了该风格的容器。

特征|通用风格|嵌入风格
:--:|:--:|:--:
容器单元组成|包括内部数据和外部数据|仅包括内部数据
外部数据组织方式|包含数据的结构体|包含数据和容器单元的结构体
包含关系|容器单元包含自定义类型|自定义类型包含容器单元
支持基本类型|支持|不支持
支持自定义类型|支持|支持
容器单元内存管理|容器负责容器单元内存，用户负责数据部分内存|用户负责数据部分内存
容器单元新增|先分配数据部分内存，再分配容器单元本身内存|只需分配数据部分。
支持混合数据类型|支持|支持
内存占用|多|少
使用方式|较灵活|十分灵活
实现方式|通用|某些数据结构无法实现

### Generic

头文件 | 名称 | 说明
:---:|:---:|:---:
list.h|双向链表|Linux 内核双向链表简化实现
heap.h|二叉堆|Linux 内核风格堆实现（优先队列）
rbtree.h|红黑树|Linux 内核红黑树简化实现

### Embedded

头文件 | 名称 | 说明
:---:|:---:|:---:
array.h|动态数组|暂时用于框架
str.h|字符串|支持字符串去白、分割、构造、数值转换等功能，代替换
sds.h|字符串|Redis的动态字符串实现。
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

## 数据库操作库 dbo（TODO）

## 事件循环库 event (TODO)

`event` 为事件库。

`event_t` 为事件结构体，主要成员包括：

- 事件所属 eventloop
- 事件类型
- 事件回调函数
- 事件状态

## TODO

- 完善 cmdf 中关于 readline 的操作。
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

## 分库

- libseventh
