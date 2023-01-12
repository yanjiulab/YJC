# ev

## Select

fd_set 实际上是一个 long 型数组 `long fds_bits[FD_SETSIZE / NFDBITS]`，其中 `FD_SETSIZE` 为 1024，`NFDBITS` 为 `(8 * (int) sizeof (long))` 即 64，因此 fd_set 实际上表达了一个长度为 FD_SETSIZE 比特的数组，所以也称为位矢量，其中每位表示一个文件描述符。

当调用 select() 时，内核根据 IO 状态修改 fd_set 的内容，由此来通知进程哪一个 fd 可读。


## Poll


