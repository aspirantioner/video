#include "image_rotate.h"

void reverse(uint8_t *start_ptr, int size)
{
    int mid = size / 2;
    uint8_t tmp;
    for (int i = 0; i < mid; i++)
    {
        tmp = start_ptr[i];
        start_ptr[i] = start_ptr[size - 1 - i];
        start_ptr[size - i - 1] = tmp;
    }
}

void yuv_rotate_180(const char *yuvfilename, int width, int height)
{

    int file_fd = open(yuvfilename, O_RDWR);
    uint8_t *start_ptr = mmap(NULL, WIDTH * HEIGHT * 3 / 2, PROT_READ | PROT_WRITE, MAP_SHARED, file_fd, 0);
    reverse(start_ptr, WIDTH * HEIGHT);
    reverse(start_ptr + WIDTH * HEIGHT, WIDTH * HEIGHT / 4);
    reverse(start_ptr + WIDTH * HEIGHT * 5 / 4, WIDTH * HEIGHT / 4);
    munmap(start_ptr, WIDTH * HEIGHT * 3 / 2);
    return;
}

void yuv_reverse(const char *yuvfilename, int width, int height)
{
    int file_fd = open(yuvfilename, O_RDWR);
    uint8_t *start_ptr = mmap(NULL, WIDTH * HEIGHT * 3 / 2, PROT_READ | PROT_WRITE, MAP_SHARED, file_fd, 0);
    for (int i = 0; i < height; i++)
    {
        reverse(start_ptr + i * width, width);
    }
    uint8_t *u_ptr = start_ptr + height * width;
    for (int i = 0; i < height / 2; i++)
    {
        reverse(u_ptr + i * width / 2, width / 2);
    }
    uint8_t *v_ptr = start_ptr + height * width * 5 / 4;
    for (int i = 0; i < height / 2; i++)
    {
        reverse(v_ptr + i * width / 2, width / 2);
    }
    munmap(start_ptr, width * height * 3 / 2);
    return;
}