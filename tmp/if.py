import socket
import struct

# 定义Netlink消息头结构体
NLMSG_HEADER_FORMAT = "IHHII"

# 定义ifinfomsg消息结构体
IFINFOMSG_FORMAT = "BBHiI"

# 定义Netlink消息类型常量
RTM_GETLINK = 0x12

# 定义NLM_F_REQUEST和NLM_F_ACK消息标志位
NLM_F_REQUEST = 0x1
NLM_F_ACK = 0x4

# 定义Netlink消息头长度
NLMSG_HDRLEN = 16

# 创建Socket
s = socket.socket(socket.AF_NETLINK, socket.SOCK_RAW, socket.NETLINK_ROUTE)

# 绑定Socket
s.bind((0, 0))

# 构造Netlink消息头
nlmsg_header = struct.pack(NLMSG_HEADER_FORMAT, NLMSG_HDRLEN, NLM_F_REQUEST | NLM_F_ACK, 1, RTM_GETLINK, 0)

# 构造ifinfomsg消息体
ifinfomsg_body = struct.pack(IFINFOMSG_FORMAT, 0, 0, 0, 0, 0)

# 发送Netlink消息
s.send(nlmsg_header + ifinfomsg_body)

# 接收Netlink消息
data = s.recv(65535)

# 解析Netlink消息
while len(data) > 0:
    nlmsg_len, nlmsg_type, nlmsg_flags, nlmsg_seq, nlmsg_pid = struct.unpack(NLMSG_HEADER_FORMAT, data[:NLMSG_HDRLEN])
    if nlmsg_type == socket.NLMSG_DONE:
        break
    elif nlmsg_type == socket.NLMSG_ERROR:
        print("Error: ", -struct.unpack("i", data[NLMSG_HDRLEN:NLMSG_HDRLEN+4])[0])
        break
    else:
        rtattr_offset = NLMSG_HDRLEN
        while rtattr_offset < nlmsg_len:
            rtattr_len, rtattr_type = struct.unpack("HH", data[rtattr_offset:rtattr_offset+4])
            if rtattr_type == socket.IFLA_IFNAME:
                print("Interface name: ", data[rtattr_offset+4:rtattr_offset+rtattr_len].decode())
            rtattr_offset += ((rtattr_len + 3) & ~3)
    data = data[nlmsg_len:]
    
# 关闭Socket
s.close()
