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

## Netfilter

- Xtables
  - ip
  - ip6
  - eb
  - arp
- nftables

## XFRM

XFRM的正确读音是transform(转换), 这表示内核协议栈收到的IPsec报文需要经过转换才能还原为原始报文；

同样地，要发送的原始报文也需要转换为IPsec报文才能发送出去。

IPsec（Internet协议安全）应该很多人都听过，IPsec是一组协议，他们通过对通信会话中的每个数据包进行身份验证和加密，以确保IP流量的安全。

XFRM框架是IPsec的“基础设施”，IPsec通过XFRM框架实现的。XFRM源自USAGI项目，该项目旨在提供适用于生产环境的IPv6和IPsec协议栈。自内核2.5之后引入了XFRM框架，这个“基础设施”独立于协议簇，包含可同时应用于IPv4和IPv6的通用部分，位于源代码的net/xfrm/目录下。

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
