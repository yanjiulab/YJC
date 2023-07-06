# IP

## 策略路由（IP rule）

IP rule 是策略路由（Routing policy database， RPDB）管理工具。

每条策略路由规则包括一个选择器（selector）和一个动作指示（action），数据包到来时，根据优先级降序（数字越小，优先级越高）依次搜索策略路由规则，如果 selector 匹配成功，则执行相应的动作。如果动作执行了，则返回路由或者失败，同时结束查找。否则，继续搜索下一条规则。

在系统启动时，内核配置了默认的 RPDB，包括三个规则：

优先级|选择器|动作|说明
:---:|:---:|:---:|:---:
0|匹配所有|查找 local (0) 路由表|特殊路由表，用于匹配广播和本地路由，该规则不可删除。
32766|匹配所有|查找 main (254) 路由表|用于匹配系统主路由表，该规则可以由管理员修改或删除。
32767|匹配所有|查找 default (253) 路由表|用于没有匹配策略之后的 post-processing，表默认为空，该规则可以删除。

### 规则类型

策略路由规则有 5 种类型：

- unicast：返回单播路由表项。
- blackhole：丢弃数据包。
- unreachable：生成“网络不可达”错误。
- prohibit：生成“通信被系统保护”错误。
- nat：修改数据包源地址。

### 规则匹配器

策略路由规则匹配器包括：

- from：根据源地址匹配。
- to：根据目的地址匹配。
- iif：如果是 lo，则可以为本地数据包和转发数据包制定不同的数据包。
- oif：只应用于本地绑定了设备的套接字发出的数据包。
- tos/dsfield：根据 tos 字段匹配。
- fwmark：根据 firewall mark 匹配。iptables 可以用来标记，从而用于策略路由。
- ipproto：根据 IP 协议匹配。
- sport：根据源端口匹配，支持范围匹配。
- dport：根据目的端口匹配，支持范围匹配。

### 规则动作

规则动作包括：

- table/lookup：查询路由表。
- protocol：指示添加该规则的路由协议，例如 zebra。
- nat：转换的地址。
- realms：TODO。
- goto：转到指定规则。

## VLAN

```shell
# ip link add link eth0 name eth0.100 type vlan id 100
# ip -d addr show

# ip addr add 192.168.100.1/24 brd 192.168.100.255 dev eth0.100
# ip link set dev eth0.100 up

# ip link set dev eth0.100 down

# ip link delete eth0.100
```

## Intro interface

### Bridge

![bridge](iproute2.asset/bridge.png)

```
# ip link add br0 type bridge
# ip link set eth0 master br0
# ip link set tap1 master br0
# ip link set tap2 master br0
# ip link set veth1 master br0
```

This creates a bridge device named br0 and sets two TAP devices (tap1, tap2), a VETH device (veth1), and a physical device (eth0) as its slaves, as shown in the diagram above.

### Bonded interface

```
ip link add bond1 type bond miimon 100 mode active-backup
ip link set eth0 master bond1
ip link set eth1 master bond1
```

The Linux bonding driver provides a method for aggregating multiple network interfaces into a single logical "bonded" interface. The behavior of the bonded interface depends on the mode; generally speaking, modes provide either hot standby or load balancing services.

<https://www.kernel.org/doc/Documentation/networking/bonding.txt>

### team device

The main thing to realize is that a team device is not trying to replicate or mimic a bonded interface. What it does is to solve the same problem using a different approach, using, for example, a lockless (RCU) TX/RX path and modular design.

But there are also some functional differences between a bonded interface and a team. For example, a team supports LACP load-balancing, NS/NA (IPV6) link monitoring, D-Bus interface, etc., which are absent in bonding. For further details about the differences between bonding and team, see Bonding vs. Team features.

Use a team when you want to use some features that bonding doesn't provide.

```
# teamd -o -n -U -d -t team0 -c '{"runner": {"name": "activebackup"},"link_watch": {"name": "ethtool"}}'
# ip link set eth0 down
# ip link set eth1 down
# teamdctl team0 port add eth0
# teamdctl team0 port add eth1
```

[Bonding vs. Team features](https://github.com/jpirko/libteam/wiki/Bonding-vs.-Team-features)

### VLAN

A VLAN, aka virtual LAN, separates broadcast domains by adding tags to network packets. VLANs allow network administrators to group hosts under the same switch or between different switches.

```
# ip link add link eth0 name eth0.2 type vlan id 2
# ip link add link eth0 name eth0.3 type vlan id 3
```

Note: When configuring a VLAN, you need to make sure the switch connected to the host is able to handle VLAN tags, for example, by setting the switch port to trunk mode.

### VXLAN

VXLAN (Virtual eXtensible Local Area Network) is a tunneling protocol designed to solve the problem of limited VLAN IDs (4,096) in IEEE 802.1q. It is described by IETF RFC 7348.

With a 24-bit segment ID, aka VXLAN Network Identifier (VNI), VXLAN allows up to 2^24 (16,777,216) virtual LANs, which is 4,096 times the VLAN capacity.

VXLAN encapsulates Layer 2 frames with a VXLAN header into a UDP-IP packet, which looks like this:

VXLAN is typically deployed in data centers on virtualized hosts, which may be spread across multiple racks.

```
# ip link add vx0 type vxlan id 100 local 1.1.1.1 remote 2.2.2.2 dev eth0 dstport 4789
```

For reference, you can read the [VXLAN kernel documentation](https://www.kernel.org/doc/Documentation/networking/vxlan.txt) or this [VXLAN introduction](https://vincent.bernat.ch/en/blog/2017-vxlan-linux).

### MACVLAN

With VLAN, you can create multiple interfaces on top of a single one and filter packages based on a VLAN tag. With MACVLAN, you can create multiple interfaces with different Layer 2 (that is, Ethernet MAC) addresses on top of a single one.

Before MACVLAN, if you wanted to connect to physical network from a VM or namespace, you would have needed to create TAP/VETH devices and attach one side to a bridge and attach a physical interface to the bridge on the host at the same time, as shown below.

br-ns

Now, with MACVLAN, you can bind a physical interface that is associated with a MACVLAN directly to namespaces, without the need for a bridge.

macvlan

共有 5 种类型：

1. Private: doesn't allow communication between MACVLAN instances on the same physical interface, even if the external switch supports hairpin mode.
2. VEPA: data from one MACVLAN instance to the other on the same physical interface is transmitted over the physical interface. Either the attached switch needs to support hairpin mode or there must be a TCP/IP router forwarding the packets in order to allow communication.
3. Bridge: all endpoints are directly connected to each other with a simple bridge via the physical interface.
4. Passthru: allows a single VM to be connected directly to the physical interface.
5. Source: the source mode is used to filter traffic based on a list of allowed source MAC addresses to create MAC-based VLAN associations. Please see the commit message.

hairpin中文翻译为发卡。bridge不允许包从收到包的端口发出，比如bridge从一个端口收到一个广播报文后，会将其广播到所有其他端口。bridge的某个端口打开hairpin mode后允许从这个端口收到的包仍然从这个端口发出。这个特性用于NAT场景下，比如docker的nat网络，一个容器访问其自身映射到主机的端口时，包到达bridge设备后走到ip协议栈，经过iptables规则的dnat转换后发现又需要从bridge的收包端口发出，需要开启端口的hairpin mode.

brctl:

hairpin <bridge> <port> {on|off} turn hairpin on/off

The type is chosen according to different needs. Bridge mode is the most commonly used.

Use a MACVLAN when you want to connect directly to a physical network from containers.

```
# ip link add macvlan1 link eth0 type macvlan mode bridge
# ip link add macvlan2 link eth0 type macvlan mode bridge
# ip netns add net1
# ip netns add net2
# ip link set macvlan1 netns net1
# ip link set macvlan2 netns net2
```

### IPVLAN

IPVLAN is similar to MACVLAN with the difference being that the endpoints have the same MAC address.
[0]

IPVLAN supports L2 and L3 mode. IPVLAN L2 mode acts like a MACVLAN in bridge mode. The parent interface looks like a bridge or switch.
[1]

In IPVLAN L3 mode, the parent interface acts like a router and packets are routed between endpoints, which gives better scalability.
[2]

Regarding when to use an IPVLAN, the IPVLAN kernel documentation says that MACVLAN and IPVLAN "are very similar in many regards and the specific use case could very well define which device to choose. if one of the following situations defines your use case then you can choose to use ipvlan

- (a) The Linux host that is connected to the external switch / router has policy configured that allows only one mac per port.
- (b) No of virtual devices created on a master exceed the mac capacity and puts the NIC in promiscuous mode and degraded performance is a concern.
- (c) If the slave device is to be put into the hostile / untrusted network namespace where L2 on the slave could be changed / misused."

```
# ip netns add ns0
# ip link add name ipvl0 link eth0 type ipvlan mode l2
# ip link set dev ipvl0 netns ns0
```

### MACVTAP/IPVTAP

MACVTAP/IPVTAP is a new device driver meant to simplify virtualized bridged networking. When a MACVTAP/IPVTAP instance is created on top of a physical interface, the kernel also creates a character device/dev/tapX to be used just like a TUN/TAP device, which can be directly used by KVM/QEMU.

With MACVTAP/IPVTAP, you can replace the combination of TUN/TAP and bridge drivers with a single module:
[0]

Typically, MACVLAN/IPVLAN is used to make both the guest and the host show up directly on the switch to which the host is connected. The difference between MACVTAP and IPVTAP is same as with MACVLAN/IPVLAN.

Here's how to create a MACVTAP instance:

```
# ip link add link eth0 name macvtap0 type macvtap
```

### MACsec

MACsec
MACsec (Media Access Control Security) is an IEEE standard for security in wired Ethernet LANs. Similar to IPsec, as a layer 2 specification, MACsec can protect not only IP traffic but also ARP, neighbor discovery, and DHCP. The MACsec headers look like this:

[MACsec header]

The main use case for MACsec is to secure all messages on a standard LAN including ARP, NS, and DHCP messages.

[MACsec configuration]

Here's how to set up a MACsec configuration:

```
# ip link add macsec0 link eth1 type macsec
```

Note: This only adds a MACsec device called macsec0 on interface eth1. For more detailed configurations, please see the "Configuration example" section in this MACsec introduction by Sabrina Dubroca.

### VETH

The VETH (virtual Ethernet) device is a local Ethernet tunnel. Devices are created in pairs, as shown in the diagram below.

Packets transmitted on one device in the pair are immediately received on the other device. When either device is down, the link state of the pair is down.

Use a VETH configuration when namespaces need to communicate to the main host namespace or between each other.

Here's how to set up a VETH configuration:

```
# ip netns add net1
# ip netns add net2
# ip link add veth1 netns net1 type veth peer name veth2 netns net2
```

### VCAN

```
# ip link add dev vcan1 type vcan
```

### VXCAN

```
# ip netns add net1
# ip netns add net2
# ip link add vxcan1 netns net1 type vxcan peer name vxcan2 netns net2
```

### IPOIB

An IPOIB device supports the IP-over-InfiniBand protocol. This transports IP packets over InfiniBand (IB) so you can use your IB device as a fast NIC.

### NLMON

NLMON is a Netlink monitor device.

Use an NLMON device when you want to monitor system Netlink messages.

Here's how to create an NLMON device:

```
# ip link add nlmon0 type nlmon
# ip link set nlmon0 up
# tcpdump -i nlmon0 -w nlmsg.pcap
```

### Dummy interface

A dummy interface is entirely virtual like, for example, the loopback interface. The purpose of a dummy interface is to provide a device to route packets through without actually transmitting them.

Use a dummy interface to make an inactive SLIP (Serial Line Internet Protocol) address look like a real address for local programs. Nowadays, a dummy interface is mostly used for testing and debugging.

Here's how to create a dummy interface:

```
# ip link add dummy1 type dummy
# ip addr add 1.1.1.1/24 dev dummy1
# ip link set dummy1 up
```

### IFB

The IFB (Intermediate Functional Block) driver supplies a device that allows the concentration of traffic from several sources and the shaping incoming traffic instead of dropping it.

Use an IFB interface when you want to queue and shape incoming traffic.

Here's how to create an IFB interface:

```
# ip link add ifb0 type ifb
# ip link set ifb0 up
# tc qdisc add dev ifb0 root sfq
# tc qdisc add dev eth0 handle ffff: ingress
# tc filter add dev eth0 parent ffff: u32 match u32 0 0 action mirred egress redirect dev ifb0
```

This creates an IFB device named ifb0 and replaces the root qdisc scheduler with SFQ (Stochastic Fairness Queueing), which is a classless queueing scheduler. Then it adds an ingress qdisc scheduler on eth0 and redirects all ingress traffic to ifb0.

For more IFB qdisc use cases, please refer to this Linux Foundation wiki on IFB.


### netdevsim interface
netdevsim is a simulated networking device which is used for testing various networking APIs. At this time it is particularly focused on testing hardware offloading, tc/XDP BPF and SR-IOV.

## 参考

- [Introduction to Linux interfaces for virtual networking](https://developers.redhat.com/blog/2018/10/22/introduction-to-linux-interfaces-for-virtual-networking)
