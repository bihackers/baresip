#include "PPCS_API.h"
#include "PPCS_Error.h"  
#include "PPCS_Type.h"
#include "cJSON.h"
#include "md5.h"
#include "ppcs.h"
#include "ppsdevice.h"

static int g_ppcsinit=0;
static int g_session=0;
static int g_run=0;
static char g_password[128]={0};
static int g_seq=0;
static pthread_t g_threadhandle;
static int initppcs(char* initstring)
{
    if(g_ppcsinit)
    {
        return 1; 
    }
    
    PPCS_Initialize(initstring);
    g_ppcsinit=1;
    
    return 1;
}

static void destoryppcs(void)
{
    if(g_ppcsinit)
    {
        PPCS_DeInitialize();
        g_ppcsinit=0;
    }
}

int connectIpc(char* uid,char* password,char* initstring)
{
    int ret=0;
    int retry=3;
    initppcs(initstring);
    while(retry--)
    {
        ret=PPCS_ConnectByServer(uid, 0x7E, 0,initstring);
        if(ret>=0)
        {
            g_session=ret;
            break;
        }
        sys_usleep(1000*1000);
    }
    
    return 0;
}

void disconnectIpc(void)
{
    if(g_session>=0)
    {
        PPCS_Close(g_session);
    }
    destoryppcs();
}

static void encrypt(PPCS_Header *header)
{
    char token[256]={0};
    char outsign[16]={0};
    //default 1
    header->version=htonl(1);
    char password[17]={0};
    if (strlen(g_password)) {
         memcpy(password, g_password, strlen(g_password)>16?16:strlen(g_password));
    }
    //md5(username|password|magic|seq|cmd|length|meari.p2p.ppcs)
    sprintf(token,"%s|%s|%d|%d|%d|%d|meari.p2p.ppcs","admin",g_password,ntohl(header->magic),ntohl(header->seq),ntohl(header->cmd),ntohl(header->length));
    MD5_CTX ctx;
    MD5Init(&ctx);
    MD5Update(&ctx,(unsigned char*)token,(unsigned int)strlen(token));
    MD5Final((unsigned char*)outsign,&ctx);
    char out[64]={0};
    for(int i=0;i<16;i++)
    {
        sprintf(out+i*2,"%02x",(int)(uint8_t)outsign[i]);
    }
    memcpy(header->token, out, 32);
}

static int findIFrame(int *IFind,char *buf,int buflen,PPSDEV_MEDIA_HEADER_PTR header)
{
    int ret=-1;
    switch (header->media_format)
    {
        case 1:
            if (buflen>5&&((buf[4] & 0x1f) ==7))
            {
                *IFind = 1;
                ret=1;
            }
            break;
        case 4:
            if (buflen>5&&buf[4]==0x40)
            {
                *IFind = 1;
                ret=1;
            }
            break;
        default:
            break;
    }
    return ret;
}

static int m_read(INT32 g_Session,UCHAR Channel,char * buf,INT32 *outBufSize) {
    int normalsize = *outBufSize;
    int totalsize = *outBufSize;
    int tempsize = 0;
    int ret = PPCS_Read(g_Session,Channel,buf, outBufSize, 3000);
    if (ret<0&&ret!=-3)
    {
        return -1;
    }
    while (totalsize!=*outBufSize) {
        tempsize = *outBufSize;
        totalsize -= *outBufSize;
        outBufSize = &totalsize;
        ret = PPCS_Read(g_Session,Channel,buf+tempsize, outBufSize, 3000);
        if (ret<0&&ret!=-3)
        {
            return -1;
        }
    }
    outBufSize = &normalsize;
    return 0;
}

static void* read_thread(void* arg)
{
    //struct vidsrc_st* p = (struct vidsrc_st*)arg;
    char *buf = (char*)malloc(1024*1024);
    PPSDEV_MEDIA_HEADER frameInfo = {0};
    int outBufSize = 0,I_frame_find=0;
    int type = SDK_STREAM_DATA_VIDEO;
    int tempsize =0;
    int totalsize =0;
    unsigned int froNo=0;
    //int frametype=0;
    while (g_run)
    {
        outBufSize = sizeof(PPSDEV_MEDIA_HEADER);
        int result = m_read(g_session,1, buf, &outBufSize);
        if (result<0) {
            break;
        }
        if (!outBufSize) {
            continue;
        }
        memcpy(&frameInfo,buf,sizeof(PPSDEV_MEDIA_HEADER));
        outBufSize = frameInfo.size;
        totalsize=outBufSize;
        tempsize = 0;
        while (totalsize>0) {
            if(g_run)
            {
                break;
            }
            result = m_read(g_session,1, buf+sizeof(PPSDEV_MEDIA_HEADER)+tempsize, &outBufSize);
            if (result<0) {
                free(buf);
                return NULL;
            } else {
                break;
            }
        }
        if(frameInfo.frame_type == 0xF0||frameInfo.frame_type == 0xF1) {
            if(type!=SDK_STREAM_DATA_VIDEO)
            {
                I_frame_find = 0;
            }
            if(!I_frame_find&&frameInfo.frame_type == 0xF0)
            {
                if (findIFrame(&I_frame_find,buf+sizeof(PPSDEV_MEDIA_HEADER),outBufSize-sizeof(PPSDEV_MEDIA_HEADER),&frameInfo)==1) {
                    froNo=frameInfo.magic;
                }else{
                    continue;
                }
            } else if(!I_frame_find) {
                continue;
            }
            
            if((froNo+1)!=frameInfo.magic&&frameInfo.frame_type!=0xF0)
            {
                //frametype=0xF1;
            }else{
                //please recall video data
                froNo=frameInfo.magic;
                //frametype=frameInfo.frame_type;
            }
        } 
        sys_usleep(1000);
    }
    free(buf);
    return NULL;
}

int startlive(void*arg)
{
    PPCS_Header header = {0};
    header.magic = htonl(0x56565099);
    header.cmd = htonl(0x11FF);
    header.length = htonl(sizeof(SMsgAVIoctrlAVStream));
    header.seq = htonl(g_seq++);
    SMsgAVIoctrlAVStream ioMsg={0};
    ioMsg.channel = 0;
    ioMsg.reserved[0] = 0;
    encrypt(&header);
    char buf[1024]={0};
    memcpy(buf,(char*)&header,sizeof(PPCS_Header));
    memcpy(buf+sizeof(PPCS_Header),(char*)&ioMsg,sizeof(SMsgAVIoctrlAVStream));
    int ret=PPCS_Write(g_session,0, buf, sizeof(PPCS_Header)+sizeof(SMsgAVIoctrlAVStream));
    if(ret<0)
   {
        return ret;
   }
    g_run=1;
    pthread_create(&g_threadhandle, NULL, read_thread, arg);
    return 1;
}
