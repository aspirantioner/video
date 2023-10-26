#ifndef _IMAGE_ROTATE_H
#define _IMAGE_ROTATE_H

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/fcntl.h>
#include <stdint.h>

void reverse(uint8_t *start_ptr, int size);
void yuv_rotate_180(const char *yuvfilename, int width, int height);
void yuv_reverse(const char *yuvfilename, int width, int height);

#endif

