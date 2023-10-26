#include "lio_camera.h"
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <linux/videodev2.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <x264.h>

void LioCameraOpen(LioCamera *camera_p, const char *dev_name)
{
    camera_p->dev_name = dev_name;

    /*打开设备*/
    camera_p->dev_fd = open(camera_p->dev_name, O_RDWR);
    if (camera_p->dev_fd < 0)
    {
        perror("open video error");
        exit(0);
    }

    /*查询设备是否有拍摄能力*/
    struct v4l2_capability caps;
    if (ioctl(camera_p->dev_fd, VIDIOC_QUERYCAP, &caps) < 0)
    {
        perror("ioctl query error");
        exit(0);
    }
    if (!(caps.capabilities & V4L2_CAP_VIDEO_CAPTURE))
    {
        perror("video device can't support capture");
        exit(0);
    }
    camera_p->dev_type = V4L2_CAP_VIDEO_CAPTURE;
}
void LioCameraSetFormat(LioCamera *camera_p, int pix_format, int width, int height)
{

    /*设置帧类型以及分辨率大小*/
    memset(&camera_p->fmt, 0, sizeof(struct v4l2_format));
    camera_p->fmt.type = camera_p->dev_type;
    camera_p->fmt.fmt.pix.height = height;
    camera_p->fmt.fmt.pix.width = width;
    camera_p->fmt.fmt.pix.pixelformat = pix_format;

    /*检查是否设置成功*/
    if (ioctl(camera_p->dev_fd, VIDIOC_S_FMT, &camera_p->fmt) < 0)
    {
        perror("set format error");
        exit(0);
    }
    if (ioctl(camera_p->dev_fd, VIDIOC_G_FMT, &camera_p->fmt) < 0)
    {
        perror("get format error");
        exit(0);
    }
    if (camera_p->fmt.fmt.pix.pixelformat == pix_format && camera_p->fmt.fmt.pix.width == width && camera_p->fmt.fmt.pix.height == height)
    {
        write(STDOUT_FILENO, "set format success!\n", 21);
    }
    else
    {
        perror("set format not success");
        exit(0);
    }
}

void LioCameraSetFps(LioCamera *camera_p, int denominator, int numerator)
{
    /*设置帧率*/
    camera_p->stp.type = camera_p->dev_type;
    camera_p->stp.parm.capture.capability = V4L2_CAP_TIMEPERFRAME;
    camera_p->stp.parm.capture.timeperframe.numerator = numerator;     // 帧率分子，可以根据需要更改
    camera_p->stp.parm.capture.timeperframe.denominator = denominator; // 帧率分母，可以根据需要更改

    /*查询是否设置成功*/
    if (ioctl(camera_p->dev_fd, VIDIOC_S_PARM, &camera_p->stp) == -1)
    {
        perror("Failed to set frame rate");
        exit(0);
    }
    if (ioctl(camera_p->dev_fd, VIDIOC_G_PARM, &camera_p->stp) == -1)
    {
        perror("get stream param error");
        exit(0);
    }
    if (camera_p->stp.parm.capture.timeperframe.denominator != denominator || camera_p->stp.parm.capture.timeperframe.numerator != numerator)
    {
        write(STDOUT_FILENO, "set fps error!\n", 16);
        exit(0);
    }
}
void LioCameraBufRequest(LioCamera *camera_p, int map_num)
{

    /*记录内存空间信息的数组*/
    camera_p->map_array = (MapBufferInfo *)malloc(map_num * sizeof(MapBufferInfo));
    if (!camera_p->map_array)
    {
        perror("malloc error");
        exit(0);
    }
    camera_p->map_buff_num = map_num;

    /*申请一定数量的帧内存映射空间*/
    struct v4l2_requestbuffers reqbuffer;
    reqbuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    reqbuffer.count = camera_p->map_buff_num;
    reqbuffer.memory = V4L2_MEMORY_MMAP;
    if (ioctl(camera_p->dev_fd, VIDIOC_REQBUFS, &reqbuffer) == -1)
    {
        perror("req buffer error");
        exit(0);
    }

    /*对上述申请的内核空间进行映射*/
    camera_p->mbf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    for (int i = 0; i < camera_p->map_buff_num; i++)
    {

        camera_p->mbf.index = i;
        if (ioctl(camera_p->dev_fd, VIDIOC_QUERYBUF, &camera_p->mbf) == -1) // 检查申请的内存是否有效
        {
            perror("query buffer error");
            exit(0);
        }
        camera_p->map_array[i].mptr = (unsigned char *)mmap(NULL, camera_p->mbf.length, PROT_READ | PROT_WRITE, MAP_SHARED, camera_p->dev_fd, camera_p->mbf.m.offset); // 对申请的内存进行映射
        if (camera_p->map_array[i].mptr == MAP_FAILED)
        {
            perror("map error");
            exit(0);
        }
        camera_p->map_array[i].map_size = camera_p->mbf.length;
        if (ioctl(camera_p->dev_fd, VIDIOC_QBUF, &camera_p->mbf) == -1) // 放回内存队列中，以供帧存取
        {
            perror("qbuf error");
            exit(0);
        }
    }
}

void LioCameraStartStream(LioCamera *camera_p)
{
    if (ioctl(camera_p->dev_fd, VIDIOC_STREAMON, &camera_p->dev_type) == -1)
    {
        perror("stream on error");
        exit(0);
    }
}

unsigned char *LioCameraFetchStream(LioCamera *camera_p)
{
    if (ioctl(camera_p->dev_fd, VIDIOC_DQBUF, &camera_p->mbf) == -1) // 从队列中取出存放抓取的帧内存
    {
        perror("read error");
        exit(0);
    }
    return camera_p->map_array[camera_p->mbf.index].mptr;
}

void LioCameraPutStream(LioCamera *camera_p)
{
    /*将内存放入队列中*/
    if (ioctl(camera_p->dev_fd, VIDIOC_QBUF, &camera_p->mbf) == -1)
    {
        perror("qbuf error");
        exit(0);
    }
}

void LioCameraStopStream(LioCamera *camera_p)
{
    if (ioctl(camera_p->dev_fd, VIDIOC_STREAMOFF, &camera_p->dev_type) == -1)
    {
        perror("stream off error");
        exit(0);
    }
}

void LioCameraDestroy(LioCamera *camera_p)
{
    for (int i = 0; i < camera_p->map_buff_num; i++)
    {
        munmap(camera_p->map_array[i].mptr, camera_p->map_array[i].map_size);
    }
    free(camera_p->map_array);
    close(camera_p->dev_fd);
}
