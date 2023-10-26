#include "lio_encoder.h"
#include <stdlib.h>
#include <string.h>

void LioEncoderInit(LioEncoder *encoder_p, const char *profile_name, int width, int height, int i_csp, int fps)
{

    /*设置编码帧分辨率格式*/
    x264_param_default(&encoder_p->param);
    encoder_p->profile_name = profile_name;
    encoder_p->param.i_width = width;
    encoder_p->param.i_height = height;
    encoder_p->param.i_csp = i_csp;
    encoder_p->param.i_fps_num = fps;
    encoder_p->param.i_fps_den = 1;

    /*配置编码参数*/
    if (x264_param_apply_profile(&encoder_p->param, profile_name) < 0)
    {
        perror("apply profile error");
        exit(0);
    }

    /*打开一个编码器实例*/
    encoder_p->x264_encoder_p = x264_encoder_open(&encoder_p->param);
    if (!encoder_p->x264_encoder_p)
    {
        perror("x264 encoder open error");
    }

    /*为存储帧图像数据分配空间*/
    x264_picture_init(&encoder_p->pic_in);
    if (x264_picture_alloc(&encoder_p->pic_in, encoder_p->param.i_csp, encoder_p->param.i_width, encoder_p->param.i_height) < 0)
    {
        perror("picture alloc error");
        exit(0);
    }
    x264_picture_init(&encoder_p->pic_out);

    /*Y,U,V大小设置*/
    encoder_p->luma_size = width * height;
    encoder_p->chroma_size = encoder_p->luma_size >> 2;
    encoder_p->pic_in.i_pts = 0;
}

void LioEncoderOutput(LioEncoder *encoder_p, unsigned char *frame_stream, FILE *output_ptr)
{

    /*将编码数据传入接受数据结构体*/
    memcpy(encoder_p->pic_in.img.plane[0], frame_stream, encoder_p->luma_size);
    memcpy(encoder_p->pic_in.img.plane[1], frame_stream + encoder_p->luma_size, encoder_p->chroma_size);
    memcpy(encoder_p->pic_in.img.plane[2], frame_stream + encoder_p->luma_size + encoder_p->chroma_size, encoder_p->chroma_size);
    int i_frame_size = x264_encoder_encode(encoder_p->x264_encoder_p, &encoder_p->nal, &encoder_p->i_nal, &encoder_p->pic_in, &encoder_p->pic_out);
    if (i_frame_size < 0)
    {
        perror("x264 encode error");
        exit(0);
    }
    else if (i_frame_size)
    {
        // if (!fwrite(encoder_p->nal->p_payload, i_frame_size, 1, output_ptr))
        // {
        //     perror("fwrite error");
        //     exit(0);
        // }
        for (int i = 0; i < encoder_p->i_nal; i++)
        {
            fwrite(encoder_p->nal[i].p_payload, encoder_p->nal[i].i_payload, 1, output_ptr);
        }
    }

    encoder_p->pic_in.i_pts++;
}

void LioEncoderFlush(LioEncoder *encoder_p, FILE *output_ptr)
{
    int i_frame_size = 0;

    /*编码延迟剩余帧*/
    while (x264_encoder_delayed_frames(encoder_p->x264_encoder_p))
    {
        i_frame_size = x264_encoder_encode(encoder_p->x264_encoder_p, &encoder_p->nal, &encoder_p->i_nal, NULL, &encoder_p->pic_out);
        if (i_frame_size < 0)
        {
            perror("x264 encode error");
            exit(0);
        }
        else if (i_frame_size)
        {
            if (!fwrite(encoder_p->nal->p_payload, i_frame_size, 1, output_ptr))
            {
                perror("fwrite error");
                exit(0);
            }
        }
    }
}

void LioEncoderDestroy(LioEncoder *encoder_p)
{
    x264_encoder_close(encoder_p->x264_encoder_p);
    x264_picture_clean(&encoder_p->pic_in);
}
