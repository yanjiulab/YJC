# IP

## ip link

### VXLAN

VXLAN (Virtual eXtensible Local Area Network) is a tunneling protocol designed to solve the problem of limited VLAN IDs (4,096) in IEEE 802.1q. It is described by IETF RFC 7348.

With a 24-bit segment ID, aka VXLAN Network Identifier (VNI), VXLAN allows up to 2^24 (16,777,216) virtual LANs, which is 4,096 times the VLAN capacity.

VXLAN encapsulates Layer 2 frames with a VXLAN header into a UDP-IP packet, which looks like this:

VXLAN is typically deployed in data centers on virtualized hosts, which may be spread across multiple racks.

```
# ip link add vx0 type vxlan id 100 local 1.1.1.1 remote 2.2.2.2 dev eth0 dstport 4789
```

For reference, you can read the [VXLAN kernel documentation](https://www.kernel.org/doc/Documentation/networking/vxlan.txt) or this [VXLAN introduction](https://vincent.bernat.ch/en/blog/2017-vxlan-linux).

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
