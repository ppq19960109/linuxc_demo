#include "file.h"
#include "commom.h"

int file_test(char* file_buf, int file_len) {
    FILE* ffd = fopen("./test_file", "w+");
    if (ffd < 0) {
        log_debug("ffd:%d\n", ffd);
        goto end;
    }
    fwrite(file_buf, file_len, file_len, ffd);

    fclose(ffd);

    int fd = open("./test_file", O_RDWR | O_CREAT, 0777);
    if (fd < 0) {
        log_debug("fd:%d\n", fd);
        goto end;
    }
    read(fd, file_buf, file_len);

    char* mapbuf =
        mmap(NULL, file_len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mapbuf == MAP_FAILED) {
        log_debug("errno:%s\n", errno);
    }
    log_debug("mmap buf:%s\n", mapbuf);
    munmap(mapbuf, file_len);

    // char* mmapbuf = mmap(NULL, total, PROT_READ|PROT_WRITE, MAP_PRIVATE
    // |MAP_ANONYMOUS, -1, 0); if(mmapbuf==MAP_FAILED){
    //     log_debug("errno:%s\n", errno);
    // }

    // memcpy(mmapbuf,buf,total);
    // log_debug("mmap buf:%s\n", mmapbuf);
    // munmap(mmapbuf, total);

    log_debug("read buf:%s\n", file_buf);
    close(fd);
    return 0;
end:

    return -1;
}