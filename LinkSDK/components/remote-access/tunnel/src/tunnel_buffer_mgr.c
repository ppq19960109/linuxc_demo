#include "core_stdinc.h"
#include "tunnel_buffer_mgr.h"
#include "core_sysdep.h"


/*********************************************************
 * 接口名称：create_tunnel_buffer
 * 描       述：创建_tunnel_buffer
 * 输入参数：RA_BUFFER_INFO_S *RAbuffer
 *           int size
 * 输出参数：
 * 返  回 值：
 * 说       明：
 *
 *********************************************************/
int create_tunnel_buffer(RA_BUFFER_INFO_S *RAbuffer, int size)
{
    if(RAbuffer == NULL || size <= 0)
    {
        return -1;
    }
    aiot_sysdep_portfile_t *sysdep = aiot_sysdep_get_portfile();
    RAbuffer->buffer = (char*)sysdep->core_sysdep_malloc(size, "REMOTE_ACCESS");
    if(RAbuffer->buffer == NULL)
    {
        return -1;
    }
    memset((void *)RAbuffer->buffer, 0x00, sizeof(char)*size);
    RAbuffer->size = size;
    RAbuffer->read_index = 0;
    RAbuffer->write_index = 0;

    return 0;
}

/*********************************************************
 * 接口名称：release_tunnel_buffer
 * 描       述：释放RABuffer
 * 输入参数：RA_BUFFER_INFO_S* buffer
 * 输出参数：
 * 返  回 值：
 * 说       明：
 *
 *********************************************************/
void release_tunnel_buffer(RA_BUFFER_INFO_S* buffer)
{
    if(NULL == buffer)
        return;

    if(buffer->buffer != NULL)
    {
        aiot_sysdep_portfile_t *sysdep = aiot_sysdep_get_portfile();
        sysdep->core_sysdep_free(buffer->buffer);
    }

    buffer->read_index = 0;
    buffer->write_index = 0;
    buffer->size = 0;

    return;
}

/*********************************************************
 * 接口名称：RAbuffer_reset_buffer
 * 描       述：重置RABuffer
 * 输入参数：RA_BUFFER_INFO_S* buffer
 * 输出参数：
 * 返  回 值：
 * 说       明：
 *
 *********************************************************/
int reset_tunnel_buffer(RA_BUFFER_INFO_S* buffer)
{
    buffer->read_index = 0;
    buffer->write_index = 0;
    memset(buffer->buffer, 0x00, buffer->size);

    return 0;
}

/*********************************************************
 * 接口名称：write_tunnel_buffer
 * 描       述：将数据写入Buffer中
 * 输入参数：RA_BUFFER_INFO_S* buffer
 * 输出参数：
 * 返  回 值：
 * 说       明：此接口保证buffer数据完整性，
 *           如果buffer中已有数据则在原数据后添加
 *********************************************************/
int write_tunnel_buffer(RA_BUFFER_INFO_S* buffer, const char*data, int len)
{
    if(buffer->write_index + len > buffer->size)
    {
        //超出buffer的长度限制
        return -1;
    }

    //copy数据
    memcpy(buffer->buffer+buffer->write_index,data,len);

    //移动写下标
    buffer->write_index += len;

    return 0;
}

/*********************************************************
 * 接口名称：RABuffer_get_read_pointer
 * 描       述：获取Buffer的读地址
 * 输入参数：RA_BUFFER_INFO_S* buffer
 * 输出参数：
 * 返  回 值：
 * 说       明：
 *********************************************************/
char* get_tunnel_buffer_read_pointer(RA_BUFFER_INFO_S* buffer)
{
    char *pointer = buffer->buffer + buffer->read_index;
    return pointer;
}

/*********************************************************
 * 接口名称：get_tunnel_buffer_write_pointer
 * 描       述：获取Buffer的写地址
 * 输入参数：RA_BUFFER_INFO_S* buffer
 * 输出参数：
 * 返  回 值：
 * 说       明：
 *********************************************************/
char* get_tunnel_buffer_write_pointer(RA_BUFFER_INFO_S* buffer)
{
    char *pointer = buffer->buffer + buffer->write_index;
    return pointer;
}

/*********************************************************
 * 接口名称：get_tunnel_buffer_read_len
 * 描       述：获取Buffer的可读长度
 * 输入参数：RA_BUFFER_INFO_S* buffer
 * 输出参数：
 * 返  回 值：
 * 说       明：
 *********************************************************/
int get_tunnel_buffer_read_len(RA_BUFFER_INFO_S* buffer)
{
    int len = buffer->write_index - buffer->read_index;
    return len;
}

/*********************************************************
 * 接口名称：RABuffer_move_read_pointer
 * 描       述：移动buffer读指针
 * 输入参数：RA_BUFFER_INFO_S* buffer
 * 输出参数：
 * 返  回 值：
 * 说       明：
 *********************************************************/
int move_tunnel_buffer_read_pointer(RA_BUFFER_INFO_S* buffer, int offset_len)
{
    if(offset_len > 0) // 右移
    {
        if(buffer->read_index + offset_len > buffer->size)
        {
            return -1;
        }

        buffer->read_index += offset_len;
    }
    else // 左移
    {
        if(buffer->read_index < -offset_len)
            return -1; // 溢出

        buffer->read_index +=  offset_len;
    }

    return 0;
}

/*********************************************************
 * 接口名称：move_tunnel_buffer_write_pointer
 * 描       述：移动buffer读指针
 * 输入参数：RA_BUFFER_INFO_S* buffer
 * 输出参数：
 * 返  回 值：
 * 说       明：
 *********************************************************/
int move_tunnel_buffer_write_pointer(RA_BUFFER_INFO_S* buffer, int offset_len)
{
    if(offset_len > 0) // 右移
    {
        if(buffer->write_index + offset_len > buffer->size)
            return -1; // 溢出

        buffer->write_index +=   offset_len;
    }
    else  // 左移
    {
        if(buffer->write_index < -offset_len)
            return -1; // 溢出

        buffer->write_index +=  offset_len;
    }

    return 0;
}

/*********************************************************
 * 接口名称：memmove_tunnel_buffer
 * 描       述：移动buffer数据位置
 * 输入参数：RA_BUFFER_INFO_S* buffer
 * 输出参数：
 * 返  回 值：
 * 说       明：
 *********************************************************/
int memmove_tunnel_buffer(RA_BUFFER_INFO_S* buffer, int offset_len)
{
    int len = get_tunnel_buffer_read_len(buffer);
    if(offset_len > 0)  //右移
    {
        if(buffer->write_index + offset_len > buffer->size)
        {
            return -1; // 超出范围
        }

        memmove(buffer->buffer + buffer->read_index + offset_len, buffer->buffer + buffer->read_index, len);
        buffer->read_index += offset_len;
        buffer->write_index += offset_len;
    }
    else  //左移
    {
        if(buffer->read_index < -offset_len)
        {
            return -1; //超出范围
        }

        memmove(buffer->buffer + (buffer->read_index + offset_len), buffer->buffer + buffer->read_index, len);
        buffer->read_index += offset_len;
        buffer->write_index += offset_len;
    }

    return 0;

}

/*********************************************************
 * 接口名称：RABuffer_reset_read_point
 * 描       述：重置RABuffer的读索引
 * 输入参数：RA_BUFFER_INFO_S* buffer
 * 输出参数：
 * 返  回 值：
 * 说       明：
 *********************************************************/
void reset_tunnel_buffer_read_point(RA_BUFFER_INFO_S* buffer)
{
    buffer->read_index = 0;
    return;
}

/*********************************************************
 * 接口名称：reset_tunnel_buffer_write_point
 * 描       述：重置RABuffer的写索引
 * 输入参数：RA_BUFFER_INFO_S* buffer
 * 输出参数：
 * 返  回 值：
 * 说       明：
 *********************************************************/
void reset_tunnel_buffer_write_point(RA_BUFFER_INFO_S* buffer)
{
    buffer->write_index = 0;
    return;
}

/*********************************************************
 * 接口名称：join_content_before_tunnel_buffer
 * 描       述：拼接内容放在BABuffer当前内容之前
 * 输入参数：RA_BUFFER_INFO_S* buffer
 * 输出参数：
 * 返  回 值：
 * 说       明：
 *********************************************************/
int join_content_before_tunnel_buffer(char*data, int len, RA_BUFFER_INFO_S *channel_buffer)
{
    int remain_len = get_tunnel_buffer_read_len(channel_buffer);

    if( 0 != memmove_tunnel_buffer(channel_buffer,len))
    {
        return -1;
    }

    reset_tunnel_buffer_read_point(channel_buffer);
    reset_tunnel_buffer_write_point(channel_buffer);

    if(0 != write_tunnel_buffer(channel_buffer,data,len))
    {
        return -1;
    }

    if(0 != move_tunnel_buffer_write_pointer(channel_buffer,remain_len))
    {
        return -1;
    }

    return 0;
}


