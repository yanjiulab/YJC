# P&G

## Protobuf

protobuf 即 Protocol Buffers，是一种轻便高效的结构化数据存储格式，与语言、平台无关，可扩展可序列化。protobuf 性能和效率大幅度优于 JSON、XML 等其他的结构化数据格式。protobuf 是以二进制方式存储，占用空间小，但也带来了可读性差的缺点（二进制协议，因为不可读而难以调试，不好定位问题）。

在序列化协议中，JSON，protobuf 以及 msgpack 都是业界常用的协议，正因为 protobuf 的轻量级以及效率极其优秀，因此在众多后台项目中广泛使用。在对外的接口，我们用 http 协议支持对方的服务调用，而对内的服务间 rpc 调用，我们倾向于使用 protobuf 这种轻量级、效率优先的协议。

### Proto 语法

Protobuf 官方实现了一门语言，专门用来自定义数据结构。protoc 是这门语言的编译工具，可编译生成指定编程语言（如C++、Java、Golang、Python、C# 等）的源代码，然后开发者可以轻松在这些语言中使用该源代码进行编程。

以下 test.proto 就是一个 Protobuf 语言的示例。

```protobuf
//选择  proto2 或者 proto3 的语法，这里指定了 proto3 的语法
syntax = "proto3";
​
//包名，在 C++ 里表现为 namespace
package mytest;
//option optimize_for = LITE_RUNTIME;
​
//依赖的其他 proto 源文件，
//在依赖的数据类型在其他 proto 源文件中定义的情况下，
//需要通过 import 导入其他 proto 源文件
import "google/protobuf/any.proto";
​
//message 是消息体，它就是一个结构体/类
message SubTest {
  int32              i32      =   1;
}
​
message Test {
//数据类型            字段          field-number (还是用英文原文好一点)
  int32              i32      =   1;
  int64              i64      =   2;
  uint32             u32      =   3;
  uint64             u64      =   4;
  sint32             si32     =   5;
  sint64             si64     =   6;
  fixed32            fx32     =   7;
  fixed64            fx64     =   8;
  sfixed32           sfx32    =   9;
  sfixed64           sfx64    =   10;
  bool               bl       =   11;
  float              f32      =   12;
  double             d64      =   13;
  string             str      =   14;
  bytes              bs       =   15;
  repeated int32     vec      =   16;
  map<int32, int32>  mp       =   17;
  SubTest            test     =   18;
  oneof object {
    float            obj_f32  =   19;
    string           obj_str  =   20;
  }
  google.protobuf.Any any     =   21;
}
```

## 协议定义

protobuf 有2个版本，默认版本是 proto2，如果需要 proto3，则需要在非空非注释第一行使用 syntax = "proto3" 标明版本。

## gRPC

gRPC 是互联网后台常用的 RPC 框架，而 protobuf 是一个常用的通信协议，而 gRPC 中，protobuf 常用作其服务间的协议通信，因此很有必要一块掌握这两个技术点。
