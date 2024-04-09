
// #include "base.h"
// #include "sev.h"
// // #include "iowatcher.h"
// #include "log.h"


// static void __connect_timeout_cb(evtimer_t* timer) {
//     evio_t* io = (evio_t*)timer->privdata;
//     if (io) {
//         char localaddrstr[SU_ADDRSTRLEN] = {0};
//         char peeraddrstr[SU_ADDRSTRLEN] = {0};
//         log_warn("connect timeout [%s] <=> [%s]",
//                  SOCKADDR_STR(io->localaddr, localaddrstr),
//                  SOCKADDR_STR(io->peeraddr, peeraddrstr));
//         io->error = ETIMEDOUT;
//         evio_close(io);
//     }
// }

// static void __close_timeout_cb(evtimer_t* timer) {
//     evio_t* io = (evio_t*)timer->privdata;
//     if (io) {
//         char localaddrstr[SU_ADDRSTRLEN] = {0};
//         char peeraddrstr[SU_ADDRSTRLEN] = {0};
//         log_warn("close timeout [%s] <=> [%s]",
//                  SOCKADDR_STR(io->localaddr, localaddrstr),
//                  SOCKADDR_STR(io->peeraddr, peeraddrstr));
//         io->error = ETIMEDOUT;
//         evio_close(io);
//     }
// }

// static void __accept_cb(evio_t* io) {
//     evio_accept_cb(io);
// }

// static void __connect_cb(evio_t* io) {
//     evio_del_connect_timer(io);
//     evio_connect_cb(io);
// }

// static void __read_cb(evio_t* io, void* buf, int readbytes) {
//     // printd("> %.*s\n", readbytes, buf);
//     io->last_read_hrtime = io->loop->cur_hrtime;
//     evio_handle_read(io, buf, readbytes);
// }

// static void __write_cb(evio_t* io, const void* buf, int writebytes) {
//     // printd("< %.*s\n", writebytes, buf);
//     io->last_write_hrtime = io->loop->cur_hrtime;
//     evio_write_cb(io, buf, writebytes);
// }

// static void __close_cb(evio_t* io) {
//     // printd("close fd=%d\n", io->fd);
//     evio_del_connect_timer(io);
//     evio_del_close_timer(io);
//     evio_del_read_timer(io);
//     evio_del_write_timer(io);
//     evio_del_keepalive_timer(io);
//     evio_del_heartbeat_timer(io);
//     evio_close_cb(io);
// }

// static void ssl_server_handshake(evio_t* io) {
//     // printd("ssl server handshake...\n");
//     // int ret = hssl_accept(io->ssl);
//     // if (ret == 0) {
//     //     // handshake finish
//     //     evio_del(io, EV_READ);
//     //     printd("ssl handshake finished.\n");
//     //     __accept_cb(io);
//     // } else if (ret == HSSL_WANT_READ) {
//     //     if ((io->events & EV_READ) == 0) {
//     //         evio_add(io, ssl_server_handshake, EV_READ);
//     //     }
//     // } else {
//     //     log_error("ssl handshake failed: %d", ret);
//     //     io->error = ERR_SSL_HANDSHAKE;
//     //     evio_close(io);
//     // }
// }

// static void ssl_client_handshake(evio_t* io) {
//     // printd("ssl client handshake...\n");
//     // int ret = hssl_connect(io->ssl);
//     // if (ret == 0) {
//     //     // handshake finish
//     //     evio_del(io, EV_READ);
//     //     printd("ssl handshake finished.\n");
//     //     __connect_cb(io);
//     // } else if (ret == HSSL_WANT_READ) {
//     //     if ((io->events & EV_READ) == 0) {
//     //         evio_add(io, ssl_client_handshake, EV_READ);
//     //     }
//     // } else {
//     //     log_error("ssl handshake failed: %d", ret);
//     //     io->error = ERR_SSL_HANDSHAKE;
//     //     evio_close(io);
//     // }
// }

// static void nio_accept(evio_t* io) {
//     // printd("nio_accept listenfd=%d\n", io->fd);
//     int connfd = 0, err = 0, accept_cnt = 0;
//     socklen_t addrlen;
//     evio_t* connio = NULL;
//     while (accept_cnt++ < 3) {
//         addrlen = sizeof(sockaddr_u);
//         connfd = accept(io->fd, io->peeraddr, &addrlen);
//         if (connfd < 0) {
//             err = socket_errno();
//             if (err == EAGAIN || err == EINTR) {
//                 return;
//             } else {
//                 perror("accept");
//                 io->error = err;
//                 goto accept_error;
//             }
//         }
//         addrlen = sizeof(sockaddr_u);
//         getsockname(connfd, io->localaddr, &addrlen);
//         connio = evio_get(io->loop, connfd);
//         // NOTE: inherit from listenio
//         connio->accept_cb = io->accept_cb;
//         connio->userdata = io->userdata;
//         if (io->unpack_setting) {
//             evio_set_unpack(connio, io->unpack_setting);
//         }

//         if (io->io_type == EVIO_TYPE_SSL) {
//             // if (connio->ssl == NULL) {
//             //     // io->ssl_ctx > g_ssl_ctx > hssl_ctx_new
//             //     hssl_ctx_t ssl_ctx = NULL;
//             //     if (io->ssl_ctx) {
//             //         ssl_ctx = io->ssl_ctx;
//             //     } else if (g_ssl_ctx) {
//             //         ssl_ctx = g_ssl_ctx;
//             //     } else {
//             //         io->ssl_ctx = ssl_ctx = hssl_ctx_new(NULL);
//             //         io->alloced_ssl_ctx = 1;
//             //     }
//             //     if (ssl_ctx == NULL) {
//             //         io->error = ERR_NEW_SSL_CTX;
//             //         goto accept_error;
//             //     }
//             //     hssl_t ssl = hssl_new(ssl_ctx, connfd);
//             //     if (ssl == NULL) {
//             //         io->error = ERR_NEW_SSL;
//             //         goto accept_error;
//             //     }
//             //     connio->ssl = ssl;
//             // }
//             // evio_enable_ssl(connio);
//             // ssl_server_handshake(connio);
//         } else {
//             // NOTE: SSL call accept_cb after handshake finished
//             __accept_cb(connio);
//         }
//     }
//     return;

// accept_error:
//     log_error("listenfd=%d accept error: %s:%d", io->fd, socket_strerror(io->error), io->error);
//     evio_close(io);
// }

// static void nio_connect(evio_t* io) {
//     // printd("nio_connect connfd=%d\n", io->fd);
//     socklen_t addrlen = sizeof(sockaddr_u);
//     int ret = getpeername(io->fd, io->peeraddr, &addrlen);
//     if (ret < 0) {
//         io->error = socket_errno();
//         goto connect_error;
//     } else {
//         addrlen = sizeof(sockaddr_u);
//         getsockname(io->fd, io->localaddr, &addrlen);

//         if (io->io_type == EVIO_TYPE_SSL) {
//             // if (io->ssl == NULL) {
//             //     // io->ssl_ctx > g_ssl_ctx > hssl_ctx_new
//             //     hssl_ctx_t ssl_ctx = NULL;
//             //     if (io->ssl_ctx) {
//             //         ssl_ctx = io->ssl_ctx;
//             //     } else if (g_ssl_ctx) {
//             //         ssl_ctx = g_ssl_ctx;
//             //     } else {
//             //         io->ssl_ctx = ssl_ctx = hssl_ctx_new(NULL);
//             //         io->alloced_ssl_ctx = 1;
//             //     }
//             //     if (ssl_ctx == NULL) {
//             //         io->error = ERR_NEW_SSL_CTX;
//             //         goto connect_error;
//             //     }
//             //     hssl_t ssl = hssl_new(ssl_ctx, io->fd);
//             //     if (ssl == NULL) {
//             //         io->error = ERR_NEW_SSL;
//             //         goto connect_error;
//             //     }
//             //     io->ssl = ssl;
//             // }
//             // if (io->hostname) {
//             //     hssl_set_sni_hostname(io->ssl, io->hostname);
//             // }
//             // ssl_client_handshake(io);
//         } else {
//             // NOTE: SSL call connect_cb after handshake finished
//             __connect_cb(io);
//         }

//         return;
//     }

// connect_error:
//     log_warn("connfd=%d connect error: %s:%d", io->fd, socket_strerror(io->error), io->error);
//     evio_close(io);
// }

// static void nio_connect_event_cb(event_t* ev) {
//     evio_t* io = (evio_t*)ev->userdata;
//     uint32_t id = (uintptr_t)ev->privdata;
//     if (io->id != id)
//         return;
//     nio_connect(io);
// }

// static int nio_connect_async(evio_t* io) {
//     event_t ev;
//     memset(&ev, 0, sizeof(ev));
//     ev.cb = nio_connect_event_cb;
//     ev.userdata = io;
//     ev.privdata = (void*)(uintptr_t)io->id;
//     eloop_post_event(io->loop, &ev);
//     return 0;
// }

// static int __nio_read(evio_t* io, void* buf, int len) {
//     int nread = 0;
//     switch (io->io_type) {
//     case EVIO_TYPE_SSL:
//         // nread = hssl_read(io->ssl, buf, len);
//         // break;
//     case EVIO_TYPE_TCP:
// #ifdef OS_UNIX
//         nread = read(io->fd, buf, len);
// #else
//         nread = recv(io->fd, buf, len, 0);
// #endif
//         break;
//     case EVIO_TYPE_UDP:
//     case EVIO_TYPE_KCP:
//     case EVIO_TYPE_IP: {
//         socklen_t addrlen = sizeof(sockaddr_u);
//         nread = recvfrom(io->fd, buf, len, 0, io->peeraddr, &addrlen);
//     } break;
//     default:
//         nread = read(io->fd, buf, len);
//         break;
//     }
//     // hlogd("read retval=%d", nread);
//     return nread;
// }

// static int __nio_write(evio_t* io, const void* buf, int len) {
//     int nwrite = 0;
//     switch (io->io_type) {
//     case EVIO_TYPE_SSL:
//         // nwrite = hssl_write(io->ssl, buf, len);
//         // break;
//     case EVIO_TYPE_TCP:
// #ifdef OS_UNIX
//         nwrite = write(io->fd, buf, len);
// #else
//         nwrite = send(io->fd, buf, len, 0);
// #endif
//         break;
//     case EVIO_TYPE_UDP:
//     case EVIO_TYPE_KCP:
//     case EVIO_TYPE_IP:
//         nwrite = sendto(io->fd, buf, len, 0, io->peeraddr, SOCKADDR_LEN(io->peeraddr));
//         break;
//     default:
//         nwrite = write(io->fd, buf, len);
//         break;
//     }
//     // hlogd("write retval=%d", nwrite);
//     return nwrite;
// }

// static void nio_read(evio_t* io) {
//     // printd("nio_read fd=%d\n", io->fd);
//     void* buf;
//     int len = 0, nread = 0, err = 0;
// read:
//     buf = io->readbuf.base + io->readbuf.tail;
//     if (io->read_flags & EVIO_READ_UNTIL_LENGTH) {
//         len = io->read_until_length - (io->readbuf.tail - io->readbuf.head);
//     } else {
//         len = io->readbuf.len - io->readbuf.tail;
//     }
//     assert(len > 0);
//     nread = __nio_read(io, buf, len);
//     // printd("read retval=%d\n", nread);
//     if (nread < 0) {
//         err = socket_errno();
//         if (err == EAGAIN) {
//             // goto read_done;
//             return;
//         } else if (err == EMSGSIZE) {
//             // ignore
//             return;
//         } else {
//             // perror("read");
//             io->error = err;
//             goto read_error;
//         }
//     }
//     if (nread == 0) {
//         goto disconnect;
//     }
//     io->readbuf.tail += nread;
//     __read_cb(io, buf, nread);
//     if (nread == len && !io->closed) {
//         // NOTE: ssl may have own cache
//         if (io->io_type == EVIO_TYPE_SSL) {
//             // read continue
//             goto read;
//         }
//     }
//     return;
// read_error:
// disconnect:
//     if (io->io_type & EVIO_TYPE_SOCK_STREAM) {
//         evio_close(io);
//     }
// }

// static void nio_write(evio_t* io) {
//     // printd("nio_write fd=%d\n", io->fd);
//     int nwrite = 0, err = 0;
//     recursive_mutex_lock(&io->write_mutex);
// write:
//     if (write_queue_empty(&io->write_queue)) {
//         recursive_mutex_unlock(&io->write_mutex);
//         if (io->close) {
//             io->close = 0;
//             evio_close(io);
//         }
//         return;
//     }
//     offset_buf_t* pbuf = write_queue_front(&io->write_queue);
//     char* base = pbuf->base;
//     char* buf = base + pbuf->offset;
//     int len = pbuf->len - pbuf->offset;
//     nwrite = __nio_write(io, buf, len);
//     // printd("write retval=%d\n", nwrite);
//     if (nwrite < 0) {
//         err = socket_errno();
//         if (err == EAGAIN) {
//             recursive_mutex_unlock(&io->write_mutex);
//             return;
//         } else {
//             // perror("write");
//             io->error = err;
//             goto write_error;
//         }
//     }
//     if (nwrite == 0) {
//         goto disconnect;
//     }
//     pbuf->offset += nwrite;
//     io->write_bufsize -= nwrite;
//     __write_cb(io, buf, nwrite);
//     if (nwrite == len) {
//         // NOTE: after write_cb, pbuf maybe invalid.
//         // EV_FREE(pbuf->base);
//         EV_FREE(base);
//         write_queue_pop_front(&io->write_queue);
//         if (!io->closed) {
//             // write continue
//             goto write;
//         }
//     }
//     recursive_mutex_unlock(&io->write_mutex);
//     return;
// write_error:
// disconnect:
//     recursive_mutex_unlock(&io->write_mutex);
//     if (io->io_type & EVIO_TYPE_SOCK_STREAM) {
//         evio_close(io);
//     }
// }

// static void evio_handle_events(evio_t* io) {
//     if ((io->events & EV_READ) && (io->revents & EV_READ)) {
//         if (io->accept) {
//             nio_accept(io);
//         } else {
//             nio_read(io);
//         }
//     }

//     if ((io->events & EV_WRITE) && (io->revents & EV_WRITE)) {
//         // NOTE: del EV_WRITE, if write_queue empty
//         recursive_mutex_lock(&io->write_mutex);
//         if (write_queue_empty(&io->write_queue)) {
//             evio_del(io, EV_WRITE);
//         }
//         recursive_mutex_unlock(&io->write_mutex);
//         if (io->connect) {
//             // NOTE: connect just do once
//             // ONESHOT
//             io->connect = 0;

//             nio_connect(io);
//         } else {
//             nio_write(io);
//         }
//     }

//     io->revents = 0;
// }

// int evio_accept(evio_t* io) {
//     io->accept = 1;
//     return evio_add(io, evio_handle_events, EV_READ);
// }

// int evio_connect(evio_t* io) {
//     int ret = connect(io->fd, io->peeraddr, SOCKADDR_LEN(io->peeraddr));
// #ifdef OS_WIN
//     if (ret < 0 && socket_errno() != WSAEWOULDBLOCK) {
// #else
//     if (ret < 0 && socket_errno() != EINPROGRESS) {
// #endif
//         perror("connect");
//         io->error = socket_errno();
//         evio_close_async(io);
//         return ret;
//     }
//     if (ret == 0) {
//         // connect ok
//         nio_connect_async(io);
//         return 0;
//     }
//     int timeout = io->connect_timeout ? io->connect_timeout : EVIO_DEFAULT_CONNECT_TIMEOUT;
//     io->connect_timer = evtimer_add(io->loop, __connect_timeout_cb, timeout, 1);
//     io->connect_timer->privdata = io;
//     io->connect = 1;
//     return evio_add(io, evio_handle_events, EV_WRITE);
// }

// int evio_read(evio_t* io) {
//     if (io->closed) {
//         log_error("evio_read called but fd[%d] already closed!", io->fd);
//         return -1;
//     }
//     evio_add(io, evio_handle_events, EV_READ);
//     if (io->readbuf.tail > io->readbuf.head &&
//         io->unpack_setting == NULL &&
//         io->read_flags == 0) {
//         evio_read_remain(io);
//     }
//     return 0;
// }

// int evio_write(evio_t* io, const void* buf, size_t len) {
//     if (io->closed) {
//         log_error("evio_write called but fd[%d] already closed!", io->fd);
//         return -1;
//     }
//     int nwrite = 0, err = 0;
//     recursive_mutex_lock(&io->write_mutex);
// #if WITH_KCP
//     if (io->io_type == EVIO_TYPE_KCP) {
//         nwrite = evio_write_kcp(io, buf, len);
//         // if (nwrite < 0) goto write_error;
//         goto write_done;
//     }
// #endif
//     if (write_queue_empty(&io->write_queue)) {
//     try_write:
//         nwrite = __nio_write(io, buf, len);
//         // printd("write retval=%d\n", nwrite);
//         if (nwrite < 0) {
//             err = socket_errno();
//             if (err == EAGAIN) {
//                 nwrite = 0;
//                 log_warn("try_write failed, enqueue!");
//                 goto enqueue;
//             } else {
//                 // perror("write");
//                 io->error = err;
//                 goto write_error;
//             }
//         }
//         if (nwrite == 0) {
//             goto disconnect;
//         }
//         if (nwrite == len) {
//             goto write_done;
//         }
//     enqueue:
//         evio_add(io, evio_handle_events, EV_WRITE);
//     }
//     if (nwrite < len) {
//         if (io->write_bufsize + len - nwrite > io->max_write_bufsize) {
//             log_error("write bufsize > %u, close it!", io->max_write_bufsize);
//             // io->error = ERR_OVER_LIMIT;
//             goto write_error;
//         }
//         offset_buf_t remain;
//         remain.len = len - nwrite;
//         remain.offset = 0;
//         // NOTE: free in nio_write
//         EV_ALLOC(remain.base, remain.len);
//         memcpy(remain.base, ((char*)buf) + nwrite, remain.len);
//         if (io->write_queue.maxsize == 0) {
//             write_queue_init(&io->write_queue, 4);
//         }
//         write_queue_push_back(&io->write_queue, &remain);
//         io->write_bufsize += remain.len;
//         if (io->write_bufsize > WRITE_BUFSIZE_HIGH_WATER) {
//             log_warn("write len=%d enqueue %u, bufsize=%u over high water %u",
//                      len, (unsigned int)(remain.len - remain.offset),
//                      (unsigned int)io->write_bufsize,
//                      (unsigned int)WRITE_BUFSIZE_HIGH_WATER);
//         }
//     }
// write_done:
//     recursive_mutex_unlock(&io->write_mutex);
//     if (nwrite > 0) {
//         __write_cb(io, buf, nwrite);
//     }
//     return nwrite;
// write_error:
// disconnect:
//     recursive_mutex_unlock(&io->write_mutex);
//     /* NOTE:
//      * We usually free resources in hclose_cb,
//      * if evio_close_sync, we have to be very careful to avoid using freed resources.
//      * But if evio_close_async, we do not have to worry about this.
//      */
//     if (io->io_type & EVIO_TYPE_SOCK_STREAM) {
//         evio_close_async(io);
//     }
//     return nwrite < 0 ? nwrite : -1;
// }

// int evio_close(evio_t* io) {
//     if (io->closed)
//         return 0;
//     if (gettid() != io->loop->tid) {
//         return evio_close_async(io);
//     }

//     recursive_mutex_lock(&io->write_mutex);
//     if (io->closed) {
//         recursive_mutex_unlock(&io->write_mutex);
//         return 0;
//     }
//     if (!write_queue_empty(&io->write_queue) && io->error == 0 && io->close == 0) {
//         io->close = 1;
//         recursive_mutex_unlock(&io->write_mutex);
//         log_warn("write_queue not empty, close later.");
//         int timeout_ms = io->close_timeout ? io->close_timeout : EVIO_DEFAULT_CLOSE_TIMEOUT;
//         io->close_timer = evtimer_add(io->loop, __close_timeout_cb, timeout_ms, 1);
//         io->close_timer->privdata = io;
//         return 0;
//     }
//     io->closed = 1;
//     recursive_mutex_unlock(&io->write_mutex);

//     evio_done(io);
//     __close_cb(io);
//     // if (io->ssl) {
//         // hssl_free(io->ssl);
//         // io->ssl = NULL;
//     // }
//     // if (io->ssl_ctx && io->alloced_ssl_ctx) {
//         // hssl_ctx_free(io->ssl_ctx);
//         // io->ssl_ctx = NULL;
//     // }
//     // SAFE_FREE(io->hostname);
//     if (io->io_type & EVIO_TYPE_SOCKET) {
//         closesocket(io->fd);
//     }
//     return 0;
// }
