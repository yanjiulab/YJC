# Linux 常用命令

## 发包

发送 UDP 包，如果不是本地地址，则会触发 ARP 解析。

1. 使用 echo 命令和设备地址，使用 `-n` 参数可以不发送换行符。
```sh
echo -n "hello" > /dev/udp/127.0.0.1/6663
echo -n "hello" > /dev/udp/192.168.0.1/6663
echo -n "hello" > /dev/udp/192.168.0.1/6663
```

2. 使用 nc 命令
```sh
echo -n "hello" | nc -4u -w1 127.0.0.1 6666
```