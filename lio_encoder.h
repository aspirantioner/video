#ifndef _LIO_ENCODER_H
#define _LIO_ENCODER_H
#include <stdint.h>
#include <x264.h>
#include <unistd.h>
#include <stdio.h>

typedef struct
{
    x264_param_t param;       // 配置编码参数的结构体
    const char *profile_name; // 编码配置文件名
    x264_picture_t pic_in;    // 待编码帧的图像数据
    x264_picture_t pic_out;   // 编码后帧的图像数据
    x264_nal_t *nal;          // 编码后的NALU数据
    int i_nal;                // 编码后生成NALU的数量
    int luma_size;            // Y大小
    int chroma_size;          // U,V大小
    x264_t *x264_encoder_p;   // 编码器句柄
} LioEncoder;

/*配置编码器参数*/
void LioEncoderInit(LioEncoder *encoder_p, const char *profile_name, int width, int height, int i_csp, int fps);

/*对目标帧进行编码*/
void LioEncoderOutput(LioEncoder *encoder_p, unsigned char *frame_stream, FILE *output_ptr);

/*刷新编码器,对剩余(延迟)帧进行编码*/
void LioEncoderFlush(LioEncoder *encoder_p, FILE *output_ptr);

/*销毁编码器*/
void LioEncoderDestroy(LioEncoder *encoder_p);
#endif