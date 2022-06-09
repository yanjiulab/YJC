# YJC

YJC (Yet another Jet C project) is a lightweight, elegant and powerful C project template, which contains a [`makefile`](./Makefile) which by defaults builds a `C` project using `GCC` which has the following structure.

```
project
│   README.md
│   Makefile
│
└───src
│   │
│   └─ app
│   │  main.c
│   │
│   └─ folder
│   |  *.c
│   └─ another_folder
│       *.c
│
└───include
│   │
│   └─ folder
│       *.h
│    *.h
│
└───log
|   *.log
|
└───build
    *.out
```

## Commands

- **all**: `make` rule runs `comipile` and `run`
- **start**: starts a new project using C project template
- **compile**: `make compile` compiles source files
- **run**: `make run` executes the program
- **leaks**: `make leaks` runs `compile` and runs the program in Valgrind, loggin the result in `log/leaks.log`
- **threads**: `make threads` runs `compile` and runs the program in Valgrind with the tool `Hellgrind`
- **clean**: deletes executable
- **cleanLogs**: deletes the `log` folder
- **remove**: runs `clean` and `cleanLogs`

## package

### util

|       模块名        |       功能       | 备注                                  |
| :-----------------: | :--------------: | ------------------------------------- |
|   `args.h/args.c`   |  命令行参数解析  | TODO https://github.com/dmulholl/args |
|    `ini.h/ini.c`    | ini 配置文件解析 | OK                                    |
|    `log.h/log.c`    |     日志系统     | OK                                    |
| `debug.h/debug.c`   |  异常/错误打印   | OK                                    |
|  `timer.h/timer.c`  |     定时队列     | OK                                    |
|    `str.h/str.c`    |    字符串操作    | Working                               |

### container

|     模块名      |        功能         | 备注 |
| :-------------: | :-----------------: | ---- |
|  `vector.h/vec.c`  |      柔性数组       | OK   |
| `linklist.h/linklist.c` |        链表         | TODO |
| `hashmap.h/hashmap.c` | 字典（哈希+开链法） | TODO |

### net

C 语言实现了众多网络编程接口，在构造我们自己的 net 包时，首先应当熟悉标准库为我们提供了哪些功能，避免重复造轮子。

|        模块名         |                             功能                             | 备注 |
| :-------------------: | :----------------------------------------------------------: | :--- |
| `wrapper.h/wrapper.c` |                           包裹函数                           |      |
|    `sock.h/sock.c`    | 常用套接字创建（tcp，udp，raw，packet）、初始化、注册、读取、处理 |      |
|     `ifi.h/ifi.c`     |                         获取网卡信息                         |      |
|     `ipa.h/ipa.c`     |                              IP                              |      |
|     `mac.h/mac.c`     |                             MAC                              | TODO |
|  `bridge.h/bridge.c`  |                             网桥                             |      |
|      `rt.h/rt.c`      |                            路由表                            |      |
|   `neigh.h/neigh.c`   |                          ARP 邻居表                          |      |
|  `server.h/server.c`  |                            服务器                            |      |
|  `client.h/client.c`  |                            客户端                            |      |
|                       |                                                              |      |
|                       |                                                              |      |

TODO：

- 名字与地址转换
- netlink套接字
- ping
- tracerout
- 校验和

- 框架类
    - IO复用
    - cmd --- 支持面向行的命令解释器

### ns

- netns
- 

### proto

| 模块名              |               |      |
| ------------------- | ------------- | ---- |
| `packet.h/packet.c` | 数据包定义    |      |
| `parser.h/parser.c` | 数据包解析    |      |
| `sender.h/sender.c` | 数据包发送    |      |
| `ether.h/ether.c`   | Ethernet 协议 |      |
| `arp.h/arp.c`       | ARP 协议      |      |
| `ip.h/ip.c`         | IP 协议       |      |
| `icmp.h/icmp.c`     | ICMP 协议     |      |
| `tcp.h/tcp.c`       | TCP 协议      |      |
| `udp.h/udp.c`       | UDP 协议      |      |
| `pim.h/pim.c`       | PIM 协议      |      |
| `rip.h/rip.c`       | RIP 协议      |      |
|                     |               |      |


### others



## Inspired

- [c-project-template](https://github.com/pantuza/c-project-template)

