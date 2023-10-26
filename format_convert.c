#include "format_convert.h"
#include <stdbool.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <stdio.h>

void yuyv422_to_yuv420(unsigned char *yuyv_ptr, unsigned char *yuv_ptr, int width, int height)
{
    int length = width * height;
    unsigned char *y = yuv_ptr;
    unsigned char *u = yuv_ptr + length;
    unsigned char *v = u + length / 4;
    int y_length = 2 * length;
    for (int i = 0; i < y_length; i += 2)
    {
        *y = *(yuyv_ptr + i);
        y++;
    }

    int base_h = 0;
    bool is_u = true;
    for (int i = 0; i < height; i += 2)
    {
        base_h = i * width * 2;
        for (int j = base_h + 1; j < base_h + width * 2; j += 2)
        {
            if (is_u)
            {
                *u = *(yuyv_ptr + j);
                u++;
                is_u = false;
            }
            else
            {
                *v = *(yuyv_ptr + j);
                v++;
                is_u = true;
            }
        }
    }
    return;
}

// int main(void)
// {
//     int yuv_fd = open("./yu.yuv", O_RDWR);
//     int yuyv_fd = open("./test.yuv", O_RDWR);
//     int sum = 1280 * 720 * 3 / 2;
//     printf("%d\n", sum);
//     unsigned char *yuv_ptr = (unsigned char *)mmap(NULL, sum, PROT_READ | PROT_WRITE, MAP_SHARED, yuv_fd, 0);
//     unsigned char *yuyv_ptr = (unsigned char *)mmap(NULL, 1280 * 720 * 2, PROT_READ | PROT_WRITE, MAP_SHARED, yuyv_fd, 0);
//     printf("%c\t%c\n", yuyv_ptr[1280 * 720 * 2 - 1], yuv_ptr[sum - 1]);
//     yuyv422_to_yuv420(yuyv_ptr, yuv_ptr, 1280, 720);
//     munmap(yuyv_ptr, 1280 * 720 * 2);
//     munmap(yuv_ptr, 1280 * 720 * 3 / 2);
//     return 1;
// }