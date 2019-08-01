#ifndef PPCS_H
#define PPCS_H
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

typedef struct{
    int magic;
    int version;
    int seq;
    int cmd;
    char token[32];
    int length;
}PPCS_Header;

typedef struct
{
    unsigned int channel; // Camera Index
    unsigned char reserved[4];
} SMsgAVIoctrlAVStream;

typedef struct 
{
    unsigned int   magic; /**<Media header: 0x56565099*/
    unsigned int   videoid; /**<Video Source No*/
    unsigned int   streamid; /**<Stream type: 0: The main stream 1: Sub-stream*/
    unsigned int media_format; /**<Media encoding format 0x01=H264 0x02=mpeg4 0x03=mjpeg 0x04=hevc 0x81=aac 0x82=g711u 0x83=g711a 0x84=g726_16 0x85=G726_32*/
    unsigned char frame_type; /**<0xF0- video frame type main frame 0xF1 = video fill the frame 0xF2 = pps 0xF3 = sps 0xFA = audio frame*/
    union _media{
        struct{
            unsigned char frame_rate; /**<Frame rate*/
            unsigned char width; /**<Video width (a multiple of 8)*/
            unsigned char height; /**<Video High (a multiple of 8)*/
        }video;
        struct{
            unsigned char sample_rate; /**<Sampling Rate 0=8000 1=12000 2=11025 3=16000 4=22050 5=24000 6=32000 7=44100 8=48000*/
            unsigned char bit_rate; /**<Audio of bits*/
            unsigned char channels; /**<Number of channels*/
        }audio;
    }media;
    unsigned int timestamp; /**<Timestamp, millisecond*/
    unsigned int datetime; /**<Utc time the frame data, second grade*/
    unsigned int size; /**<The length of the frame data*/
}PPSDEV_MEDIA_HEADER,*PPSDEV_MEDIA_HEADER_PTR;

#define SDK_SYSHEAD_DATA       (int)0 
#define SDK_STREAM_DATA_VIDEO  (int)1  

int startlive(void*arg);
int stoplive(void);
void disconnectIpc(void);
int connectIpc(char* uid,char* password,char* initstring);
    
#endif