#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <uv.h>

static void uv_alloc_buf(uv_handle_t *handle,
                         size_t suggested_size,
                         uv_buf_t *buf)
{
    if (buf->base == NULL)
    {
        if (handle->data != NULL)
        {
            free(handle->data);
            handle->data = NULL;
        }
        *buf = uv_buf_init((char *)malloc(suggested_size), suggested_size);
        handle->data = buf->base;
        printf("uv_alloc_buf:%d\n", suggested_size);
    }
    else
    {
        printf("alloc_buf:%d\n", buf->len);
    }
}

static void
on_close(uv_handle_t *handle)
{
    printf("on_close\n");
    if (handle->data)
    {
        free(handle->data);
        handle->data = NULL;
    }
}

static void
after_read(uv_stream_t *stream,
           ssize_t nread,
           const uv_buf_t *buf)
{
    if (nread < 0)
    {
        // UV_ECONNRESET
        uv_close((uv_handle_t *)stream, on_close);
        return;
    }

    buf->base[nread] = 0;
    printf("recv %d\n", nread);
    printf("%s\n", buf->base);
    free(buf->base);
    // test send
    //     uv_write_t *w_req = malloc(sizeof(uv_write_t));
    //     uv_buf_t *w_buf = malloc(sizeof(uv_buf_t));
    //     w_buf->base = buf->base;
    //     w_buf->len = nread;
    //     w_req->data = w_buf;
    //     uv_write(w_req, stream, w_buf, 1, after_write);
}

void on_new_connection(uv_stream_t *server, int status)
{
    if (status < 0)
    {
        fprintf(stderr, "New connection error %s\n", uv_strerror(status));
        // error!
        return;
    }

    uv_tcp_t *client = (uv_tcp_t *)malloc(sizeof(uv_tcp_t));
    uv_tcp_init(uv_default_loop(), client);
    if (uv_accept(server, (uv_stream_t *)client) == 0)
    {
        printf("uv_accept success\n");
        uv_read_start((uv_stream_t *)client, uv_alloc_buf, after_read);
    }
    else
    {
        uv_close((uv_handle_t *)client, NULL);
    }
}

static uv_tcp_t server;

int net_sever_open()
{

    uv_tcp_init(uv_default_loop(), &server);

    struct sockaddr_in addr = {0};
    uv_ip4_addr("0.0.0.0", 9999, &addr);

    uv_tcp_bind(&server, (const struct sockaddr *)&addr, 0);
    int r = uv_listen((uv_stream_t *)&server, SOMAXCONN, on_new_connection);
    if (r)
    {
        fprintf(stderr, "Listen error %s\n", uv_strerror(r));
        return -1;
    }

    return 0;
}
void net_sever_close()
{
    uv_close((uv_handle_t *)&server, on_close);
}
//-----------------------------------------------------
uv_buf_t writebuf[] = {
    {.base = "1", .len = 1},
    {.base = "2", .len = 1}};
static uv_tcp_t* tcp_client;

void writecb(uv_write_t *req, int status)
{
    if (status < 0)
    {
        printf("write error\n");
        //uv_close((uv_handle_t*)req,NULL);
        return;
    }

    printf("write succeed!\n");
}
void on_connect(uv_connect_t *req, int status)
{
    if (status < 0)
    {
        fprintf(stderr, "connect error %s\n", uv_strerror(status));
        // error!
        return;
    }

    struct sockaddr addr;
    struct sockaddr_in addrin;
    int addrlen = sizeof(addr);
    char sockname[17] = {0};

    struct sockaddr addrpeer;
    struct sockaddr_in addrinpeer;
    int addrlenpeer = sizeof(addrpeer);
    char socknamepeer[17] = {0};

    if (0 == uv_tcp_getsockname((uv_tcp_t *)req->handle, &addr, &addrlen) &&
        0 == uv_tcp_getpeername((uv_tcp_t *)req->handle, &addrpeer, &addrlenpeer))
    {
        addrin = *((struct sockaddr_in *)&addr);
        addrinpeer = *((struct sockaddr_in *)&addrpeer);
        uv_ip4_name(&addrin, sockname, addrlen);
        uv_ip4_name(&addrinpeer, socknamepeer, addrlenpeer);
        printf("%s:%d sendto %s:%d\n", sockname, ntohs(addrin.sin_port), socknamepeer, ntohs(addrinpeer.sin_port));
    }
    else
        printf("get socket fail!\n");

    printf(" connect succeed!\n");
    uv_buf_t buf = uv_buf_init("HELLO", 5);

    uv_write_t writereq;
    uv_write(&writereq, (uv_stream_t *)tcp_client, &buf, 1, writecb);
}

int net_client_open()
{
    tcp_client = (uv_tcp_t *)malloc(sizeof(uv_tcp_t));
    uv_tcp_init(uv_default_loop(),tcp_client);

    uv_connect_t *connect = (uv_connect_t *)malloc(sizeof(uv_connect_t));

    struct sockaddr_in dest;
    uv_ip4_addr("127.0.0.1", 9999, &dest);

    if (uv_tcp_connect(connect, tcp_client, (struct sockaddr *)&dest, on_connect) == 0)
    {
        printf("uv_tcp_connect\n");

    }
    return 0;
}

void net_client_close()
{
    uv_close((uv_handle_t *)tcp_client, NULL);
}