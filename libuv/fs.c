#include <string.h>
#include <stdio.h>
#include <uv.h>


static uv_fs_t open_req, read_req, write_req;
static char buffer[1024] = "qwer123456";
uv_buf_t iov;

void on_read(uv_fs_t *req);
void on_write(uv_fs_t *req);

void on_open(uv_fs_t *req)
{
    // The request passed to the callback is the same as the one the call setup
    // function was passed.
    if (req->result >= 0)
    {
        iov = uv_buf_init(buffer, sizeof(buffer));
        iov.len = strlen(buffer);
        // uv_fs_read(uv_default_loop(), &read_req, req->result,
        //            &iov, 1, -1, on_read);
        uv_fs_write(uv_default_loop(), &write_req, req->result, &iov, 1, -1, on_write);

    }
    else
    {
        fprintf(stderr, "error opening file: %s\n", uv_strerror((int)req->result));
    }
}

void on_read(uv_fs_t *req)
{
    printf("on_read:%d\n",req->result );
    if (req->result < 0)
    {
        fprintf(stderr, "Read error: %s\n", uv_strerror(req->result));
    }
    else if (req->result == 0)
    {
        uv_fs_t close_req;
        // synchronous
        uv_fs_close(uv_default_loop(), &close_req, open_req.result, NULL);
    }
    else if (req->result > 0)
    {
        // iov.len = req->result;
        // uv_fs_write(uv_default_loop(), &write_req, 1, &iov, 1, -1, on_write);
        printf("%s,%d\n", iov.base, iov.len);
    }
}

void on_write(uv_fs_t *req)
{
    printf("on_write:%d\n",req->result );
    if (req->result < 0)
    {
        fprintf(stderr, "Write error: %s\n", uv_strerror((int)req->result));
    }
    else
    {

        uv_fs_read(uv_default_loop(), &read_req, open_req.result, &iov, 1, 0, on_read);
    }
}
int fs_open()
{
    printf("fs_open\n");
    uv_fs_open(uv_default_loop(), &open_req, "uv_file", O_RDWR | O_CREAT, 0, on_open);

    return 0;
}
void fs_close()
{
    uv_fs_req_cleanup(&open_req);
    uv_fs_req_cleanup(&read_req);
    uv_fs_req_cleanup(&write_req);
}