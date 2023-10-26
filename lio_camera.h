#ifndef _LIO_CAMERA_H
#define _LIO_CAMERA_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <linux/videodev2.h>

typedef struct
{
    unsigned char *mptr; // 映射用户空间首地址
    int map_size;        // 映射空间大小
} MapBufferInfo;         // 申请内核空间所映射到的用户空间信息

typedef struct
{
    const char *dev_name;       // 设备名称
    int dev_fd;                 // 设备文件描述符
    int dev_type;               // 设备类型
    struct v4l2_format fmt;     // 拍摄帧格式分辨率大小
    struct v4l2_streamparm stp; // 拍摄帧率大小
    struct v4l2_buffer mbf;     // 取出放回内存中间量
    int map_buff_num;           // 申请内核缓存数
    MapBufferInfo *map_array;   // 记录申请的内存信息数组
} LioCamera;

/*打开设备*/
void LioCameraOpen(LioCamera *camera_p, const char *dev_name);

/*设置拍摄帧格式、分辨率大小*/
void LioCameraSetFormat(LioCamera *camera_p, int pix_format, int width, int height);

/*设置拍摄帧大小*/
void LioCameraSetFps(LioCamera *camera_p, int denominator, int numerator);

/*申请映射内存*/
void LioCameraBufRequest(LioCamera *camera_p, int map_num); // 申请一定的数量的内核空间

/*开始抓取帧*/
void LioCameraStartStream(LioCamera *camera_p);

/*从队列取出存放的拍摄到的一帧内存*/
unsigned char *LioCameraFetchStream(LioCamera *camera_p);

/*将取出的存放帧内存重新放入队列*/
void LioCameraPutStream(LioCamera *camera_p);

/*停止拍摄*/
void LioCameraStopStream(LioCamera *camera_p);

/*关闭设备*/
void LioCameraDestroy(LioCamera *camera_p);

#endif

