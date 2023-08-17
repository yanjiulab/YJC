# Networking

## sk buff

<http://vger.kernel.org/~davem/skb.html>
<http://vger.kernel.org/~davem/skb_data.html>

uint8_t *head
uint8_t*data
uint8_t *tail
uint8_t*end

skb = alloc_skb(len, GFP_KERNEL); 是用来分配总空间的，len 一般为一个比较大的值
skb_reserve(skb, header_len); 是用来预留 header 的空间的，一般为 header 的最大值
skb_put(skb, user_data_len); 是用来增加 tail 指针的，即填充 data 到 tail 之间的空间。
skb_push(skb, sizeof(struct udphdr)); 是用来减少 data 指针的，即目前的 data 指针向 head 指针“压入”了多少长度。

## 简介

在 TCP/IP ⽹络分层模型⾥，整个协议栈被分成了物理层、链路层、⽹络层，传输层和应⽤层。物理层对应的是⽹卡和⽹线，应⽤层对应的是我们常⻅的 Nginx， FTP 等等各种应⽤。Linux 实现的是链路层、⽹络层和传输层这三层。

在 Linux 内核实现中，链路层协议靠⽹卡驱动来实现，内核协议栈来实现⽹络层和传输层。内核对更上层的应⽤层提供 socket 接⼝来供⽤户进程访问。我们⽤ Linux 的视⻆来看到的 TCP/IP ⽹络分层模型应该是下⾯这个样⼦的。

## 数据包接收

### 准备工作

Linux 驱动，内核协议栈等等模块在具备接收⽹卡数据包之前，要做很多的准备⼯作才⾏。⽐
如要提前创建好ksoftirqd内核线程，要注册好各个协议对应的处理函数，⽹卡设备⼦系统要
提前初始化好，⽹卡要启动好。只有这些都Ready之后，我们才能真正开始接收数据包。那
么我们现在来看看这些准备⼯作都是怎么做的。

#### 创建 ksoftirqd 内核进程

Linux 的软中断都是在专⻔的内核线程（ksoftirqd）中进⾏的，该进程数量不是 1 个，⽽是 N 个，其中 N 等于你的机器的核数。

当 ksoftirqd 被创建出来以后，它就会进⼊⾃⼰的线程循环函数 ksoftirqd_should_run 和 run_ksoftirqd 了。不停地判断有没有软中断需要被处理。这⾥需要注意的⼀点是，软中断不仅仅只有⽹络软中断，还有其它类型。

```c
enum {
    HI_SOFTIRQ = 0,
    TIMER_SOFTIRQ,
    NET_TX_SOFTIRQ,
    NET_RX_SOFTIRQ,
    BLOCK_SOFTIRQ,
    BLOCK_IOPOLL_SOFTIRQ,
    TASKLET_SOFTIRQ,
    SCHED_SOFTIRQ,
    HRTIMER_SOFTIRQ,
    RCU_SOFTIRQ, /* Preferable RCU should always be the last softirq */
    NR_SOFTIRQS
};
```

#### 网络子系统初始化

linux 内核通过调⽤ subsys_initcall 来初始化各个⼦系统，⽹络⼦系统的初始化，会执⾏到 net_dev_init 函数。

```c
// file: net/core/dev.c
static int __init net_dev_init(void) {
    ......
    for_each_possible_cpu(i) {
        struct softnet_data *sd = &per_cpu(softnet_data, i);
        memset(sd, 0, sizeof(*sd));
        skb_queue_head_init(&sd->input_pkt_queue);
        skb_queue_head_init(&sd->process_queue);
        sd->completion_queue = NULL;
        INIT_LIST_HEAD(&sd->poll_list);
        ......
    }
    ......
    open_softirq(NET_TX_SOFTIRQ, net_tx_action);
    open_softirq(NET_RX_SOFTIRQ, net_rx_action);
}
subsys_initcall(net_dev_init);
```

在这个函数⾥，会为每个 CPU 都申请⼀个 softnet_data 数据结构，在这个数据结构⾥的 poll_list 是等待驱动程序将其 poll 函数注册进来。

另外 open_softirq 注册了每⼀种软中断都注册⼀个处理函数。NET_TX_SOFTIRQ 的处理函数为 net_tx_action， NET_RX_SOFTIRQ 的为 net_rx_action。

#### 协议栈注册

内核实现了⽹络层的 ip 协议，也实现了传输层的 tcp 协议和 udp 协议。 这些协议对应的实现函数分别是 ip_rcv(), tcp_v4_rcv()和 udp_rcv()。

和我们平时写代码的⽅式不⼀样的是，内核是通过注册的⽅式来实现的。 Linux 内核中的 fs_initcall 和 subsys_initcall 类似，也是初始化模块的⼊⼝。fs_initcall 调⽤ inet_init 后开始⽹络协议栈注册。 通过 inet_init ，将这些函数注册到了 inet_protos 和 ptype_base 数据结构中了。

首先，系统定义了一些 handler，例如 udp_protocol 结构体中的 handler 是 udp_rcv，tcp_protocol 结构体中的 handler 是tcp_v4_rcv。

```c
// file: net/ipv4/af_inet.c
static struct packet_type ip_packet_type __read_mostly = {
    .type = cpu_to_be16(ETH_P_IP),
    .func = ip_rcv,
};
static const struct net_protocol udp_protocol = {
    .handler = udp_rcv,
    .err_handler = udp_err,
    .no_policy = 1,
    .netns_ok = 1,
};
static const struct net_protocol tcp_protocol = {
    .early_demux = tcp_v4_early_demux,
    .handler = tcp_v4_rcv,
    .err_handler = tcp_v4_err,
    .no_policy = 1,
    .netns_ok = 1,
};
```

然后，在 inet_init 中，先是注册一些上层协议，例如 ICMP、TCP、UDP 等，然后再注册 IP 协议。在 `...` 中，省略了 ARP、IP、UDP、TCP 等各模块的初始化过程以及一些错误处理代码。

```c
static int __init inet_init(void) {
    ...... 
    if (inet_add_protocol(&icmp_protocol, IPPROTO_ICMP) < 0)
        pr_crit("%s: Cannot add ICMP protocol\n", __func__);
    if (inet_add_protocol(&udp_protocol, IPPROTO_UDP) < 0)
        pr_crit("%s: Cannot add UDP protocol\n", __func__);
    if (inet_add_protocol(&tcp_protocol, IPPROTO_TCP) < 0)
        pr_crit("%s: Cannot add TCP protocol\n", __func__);
    ...... 
    dev_add_pack(&ip_packet_type);
}
fs_initcall(inet_init);
```

通过 inet_add_protocol 函数将 TCP 和 UDP 这些 IP 提供的上层协议的处理函数都注册到 inet_protos 数组中。

```c
int inet_add_protocol(const struct net_protocol *prot, unsigned char protocol) {
    if (!prot->netns_ok) {
        pr_err("Protocol %u is not namespace aware, cannot
               register.\n ",
               protocol);
        return -EINVAL;
    }
    return !cmpxchg((const struct net_protocol **)&inet_protos[protocol], NULL,
                    prot)
               ? 0
               : -1;
}
```

最后，dev_add_pack(&ip_packet_type) 将 ip_packet_type 结构体注册到 ptype_base 哈希表中。

```c
// file: net/core/dev.c
void dev_add_pack(struct packet_type *pt) {
    struct list_head *head = ptype_head(pt);
    ......
}
static inline struct list_head *ptype_head(const struct packet_type *pt) {
    if (pt->type == htons(ETH_P_ALL))
        return &ptype_all;
    else
        return &ptype_base[ntohs(pt->type) & PTYPE_HASH_MASK];
}
```

简单总结一下就是：

- inet_protos[] 记录着 udp， tcp 等协议的处理函数地址，
- ptype_base[] 存储着 ip 的处理函数地址。

软中断中会通过 ptype_base 找到 ip_rcv 函数地址，进⽽将 ip 包正确地送到 ip_rcv() 中执⾏。在 ip_rcv 中将会通过 inet_protos 找到 tcp 或者 udp 的处理函数，再⽽把包转发给 udp_rcv() 或 tcp_v4_rcv() 函数。

> ip_rcv 和 udp_rcv 等函数的代码能看到很多协议的处理过程。例如， ip_rcv 中会处理 netfilter 和 iptable 过滤，如果你有很多或者很复杂的 netfilter 或 iptables 规则，这些规则都是在软中断的上下⽂中执⾏的，会加⼤⽹络延迟。

#### 网卡驱动初始化

每⼀个驱动程序（不仅仅只是⽹卡驱动）会使⽤ module_init 向内核注册⼀个初始化函数，当驱动被加载时，内核会调⽤这个函数，这样，内核就知道关于驱动的相关信息了。⽐如 igb ⽹卡驱动的 igb_driver_name 和 igb_probe 函数地址等等。

网卡设备被识别以后，内核会调⽤其驱动的 probe ⽅法让网卡做一些准备工作，包括：

- 注册 ethtool 实现函数：当 ethtool 发起⼀个系统调⽤之后，内核会找到对应操作的回调函数。因此，该命令之所以能查看⽹卡收发包统计、能修改⽹卡⾃适应模式、能调整 RX 队列的数量和⼤⼩，是因为 ethtool 命令最终调⽤到了⽹卡驱动的相应⽅法，⽽不是 ethtool 本身有这个超能⼒。
- 注册网卡操作结构体 igb_netdev_ops：包含了各种操作使用的函数地址。

```c
// file: drivers/net/ethernet/intel/igb/igb_main.c
static const struct net_device_ops igb_netdev_ops = {
    .ndo_open = igb_open,
    .ndo_stop = igb_close,
    .ndo_start_xmit = igb_xmit_frame,
    .ndo_get_stats64 = igb_get_stats64,
    .ndo_set_rx_mode = igb_set_rx_mode,
    .ndo_set_mac_address = igb_set_mac,
    .ndo_change_mtu = igb_change_mtu,
    .ndo_do_ioctl = igb_ioctl,
    ...
}
```

- 注册 NAPI 机制所需要的 poll 函数地址：NAPI 的核心是不采用中断的方式读取数据，而是通过中断唤醒数据接收的服务程序，然后通过 poll 的方式来轮询数据。

#### 启动网卡

当上⾯的初始化都完成以后，就可以启动⽹卡了。回忆前⾯⽹卡驱动初始化时，我们提到了驱动向内核注册了 structure net_device_ops 变量，它包含着⽹卡启⽤、发包、设置 mac 地址等回调函数（函数指针）。当启⽤⼀个⽹卡时（例如，通过 ifconfig eth0 up），net_device_ops 中的 igb_open ⽅法会被调⽤。它通常会做以下事情：

- 分配内存资源：分配 RingBuffer，并建⽴内存和 Rx 队列的映射关系。（Rx Tx 队列的数量和⼤⼩可以通过 ethtool 进⾏配置）
- 注册中断：对于多队列的⽹卡，为每⼀个队列都注册了中断，其对应的中断处理函数是 igb_msix_ring，从⽹卡硬件中断的层⾯就可以设置让收到的包被不同的 CPU 处理。

当做好以上准备⼯作以后，就可以开⻔迎客（数据包）了！

### 数据到来

#### 硬中断处理

#### 软中断处理

#### 网络协议栈处理

#### IP 协议层处理

## 数据包发送

## Netfilter

- Xtables
  - ip
  - ip6
  - eb
  - arp
- nftables

## BPF、eBPF、XDP

重构的思路很显然有两个：

upload方法：别让应用程序等内核了，让应用程序自己去网卡直接拉数据。

offload方法：别让内核处理网络逻辑了，让网卡自己处理。

总之，绕过内核就对了，内核协议栈背负太多历史包袱。

DPDK让用户态程序直接处理网络流，bypass掉内核，使用独立的CPU专门干这个事。

XDP让灌入网卡的eBPF程序直接处理网络流，bypass掉内核，使用网卡NPU专门干这个事。

尽管 BPF 自 1992 年就存在，扩展的 Berkeley Packet Filter (eBPF) 版本首次出现在 Kernel3.18中，如今被称为“经典”BPF (cBPF) 的版本已过时。许多人都知道 cBPF是tcpdump使用的数据包过滤语言。现在Linux内核只运行 eBPF，并且加载的 cBPF 字节码在程序执行之前被透明地转换为内核中的eBPF表示。除非指出 eBPF 和 cBPF 之间的明确区别，一般现在说的BPF就是指eBPF。

XDP的全称是： eXpress Data Path

XDP 是Linux 内核中提供高性能、可编程的网络数据包处理框架。

## 参考

- <https://forsworns.github.io/zh/blogs/20210311/>
- <https://www.kernel.org/doc/Documentation/networking/>
