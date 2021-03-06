#define BUILDING_NODE_EXTENSION

#include "EmiNodeUtil.h"

#include "EmiError.h"
#include "slab_allocator.h"

#include "../core/EmiNetUtil.h"
#include <netinet/in.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <node_buffer.h>

using namespace v8;

static const int SLAB_SIZE = 1024 * 1024;

static node::SlabAllocator slab_allocator(SLAB_SIZE);

static void close_cb(uv_handle_t* handle) {
    free(handle);
}

static uv_buf_t alloc_cb(uv_handle_t* handle, size_t suggested_size) {
    char *buf = slab_allocator.Allocate(Context::GetCurrent()->Global(), suggested_size);
    return uv_buf_init(buf, suggested_size);
}

static void send_cb(uv_udp_send_t* req, int status) {
    uv_buf_t *buf = (uv_buf_t *)&req[1];
    
    free(buf->base);
    
    free(req);
}

static void recv_cb(uv_udp_t *handle,
                    ssize_t nread,
                    uv_buf_t buf,
                    struct sockaddr *addr,
                    unsigned flags) {
    HandleScope scope;
    
    Local<Object> slab = slab_allocator.Shrink(Context::GetCurrent()->Global(),
                                               buf.base,
                                               nread < 0 ? 0 : nread);
    if (nread == 0) return;
    
    EmiNodeUtil::EmiNodeUtilRecvCb *recvCb = *(reinterpret_cast<EmiNodeUtil::EmiNodeUtilRecvCb**>(handle+1));
    
    // Invoke recvCb if there is an error (nread < 0) or (if
    // we did receive data AND the data we received was complete)
    if (nread < 0 ||
        (nread > 0 && !(flags & UV_UDP_PARTIAL))) {
        recvCb(handle,
               *((struct sockaddr_storage *)addr),
               nread,
               slab,
               buf.base - node::Buffer::Data(slab));
    }
}

void EmiNodeUtil::parseIp(const char* host,
                          uint16_t port,
                          int family,
                          sockaddr_storage *out) {
    if (AF_INET == family) {
        struct sockaddr_in address4(uv_ip4_addr(host, port));
        memcpy(out, &address4, sizeof(struct sockaddr_in));
    }
    else if (AF_INET6 == family) {
        struct sockaddr_in6 address6(uv_ip6_addr(host, port));
        memcpy(out, &address6, sizeof(struct sockaddr_in));
    }
    else {
        ASSERT(0 && "unexpected address family");
        abort();
    }
}

void EmiNodeUtil::ipName(char *buf, size_t bufLen, const sockaddr_storage& addr) {
    if (AF_INET == addr.ss_family) {
        uv_ip4_name((struct sockaddr_in *)&addr, buf, bufLen);
    }
    else if (AF_INET6 == addr.ss_family) {
        uv_ip6_name((struct sockaddr_in6 *)&addr, buf, bufLen);
    }
    else {
        ASSERT(0 && "unexpected address family");
    }
}

bool EmiNodeUtil::parseAddressFamily(const char* typeStr, int *family) {
    if (0 == strcmp(typeStr, "udp4")) {
        *family = AF_INET;
        return true;
    }
    else if (0 == strcmp(typeStr, "udp6")) {
        *family = AF_INET6;
        return true;
    }
    else {
        return false;
    }
}

Handle<String> EmiNodeUtil::errStr(uv_err_t err) {
    HandleScope scope;
    
    Local<String> errStr;
    
    if (err.code == UV_UNKNOWN) {
        char errno_buf[100];
        snprintf(errno_buf, 100, "Unknown system errno %d", err.sys_errno_);
        errStr = String::New(errno_buf);
    } else {
        errStr = String::NewSymbol(uv_err_name(err));
    }
    
    return scope.Close(errStr);
}

void EmiNodeUtil::closeSocket(uv_udp_t *socket) {
    uv_close((uv_handle_t *)socket, close_cb);
}

uv_udp_t *EmiNodeUtil::openSocket(const sockaddr_storage& address,
                                  EmiNodeUtilRecvCb *recvCb,
                                  void *data,
                                  EmiError& error) {
    int err;
    uv_udp_t *socket = (uv_udp_t *)malloc(sizeof(uv_udp_t)+sizeof(EmiNodeUtilRecvCb*));
    *(reinterpret_cast<EmiNodeUtilRecvCb**>(socket+1)) = recvCb;
    
    err = uv_udp_init(uv_default_loop(), socket);
    if (0 != err) {
        goto error;
    }
    
    if (AF_INET == address.ss_family) {
        struct sockaddr_in& addr(*((struct sockaddr_in *)&address));
        
        char buf[100];
        uv_ip4_name(&addr, buf, sizeof(buf));
        
        err = uv_udp_bind(socket, addr, /*flags:*/0);
        if (0 != err) {
            goto error;
        }
    }
    else if (AF_INET6 == address.ss_family) {
        struct sockaddr_in6& addr6(*((struct sockaddr_in6 *)&address));
        
        err = uv_udp_bind6(socket, addr6, /*flags:*/0);
        if (0 != err) {
            goto error;
        }
    }
    else {
        ASSERT(0 && "unexpected address family");
        abort();
    }
    
    err = uv_udp_recv_start(socket, alloc_cb, recv_cb);
    if (0 != err) {
        goto error;
    }
    
    socket->data = data;
    
    return socket;
    
error:
    free(socket);
    return NULL;
}

void EmiNodeUtil::sendData(uv_udp_t *socket,
                           const sockaddr_storage& address,
                           const uint8_t *data,
                           size_t size) {
    uv_udp_send_t *req = (uv_udp_send_t *)malloc(sizeof(uv_udp_send_t)+
                                                 sizeof(uv_buf_t)+
                                                 sizeof(size_t));
    uv_buf_t      *buf = (uv_buf_t *)&req[1];
    size_t        *sizePtr = (size_t *)&buf[1];
    
    // TODO This copies the packet data. We might want to redesign
    // this part of the code so that this is not required.
    //
    // TODO It would probably be better and faster to use the slab
    // allocator here.
    char *bufData = (char *)malloc(size);
    memcpy(bufData, data, size);
    
    *buf = uv_buf_init((char *)bufData, size);
    *sizePtr = size;
    
    if (AF_INET == address.ss_family) {
        if (0 != uv_udp_send(req,
                             socket,
                             buf,
                             /*bufcnt:*/1,
                             *((struct sockaddr_in *)&address),
                             send_cb)) {
            free(req);
        }
    }
    else if (AF_INET6 == address.ss_family) {
        if (0 != uv_udp_send6(req,
                              socket,
                              buf,
                              /*bufcnt:*/1,
                              *((struct sockaddr_in6 *)&address),
                              send_cb)) {
            free(req);
        }
    }
    else {
        ASSERT(0 && "unexpected address family");
    }
}

EmiTimeInterval EmiNodeUtil::now() {
    return ((double)uv_hrtime())/NSECS_PER_SEC;
}
