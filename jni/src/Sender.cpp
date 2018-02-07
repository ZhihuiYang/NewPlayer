/*!
 * \file Sender.cpp
 *
 * \date Created on: Aug 8, 2016
 * \author: eyngzui
 */

#include "include/h264_RtpPacket.h"
#include "include/logger.h"
#include "include/memoryPool.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "include/rtpheader.h"

#define ERR_EXIT(m) \
    do { \
        perror(m); \
        LOG_ERROR(m); \
        exit(EXIT_FAILURE); \
    } while (0)

#define BUF_LENGTH 1500
int sock;
struct sockaddr_in servaddr;
static void successFunc(u_char *buf, int length)
{
    RTP_HDR_T * rtp_h = (RTP_HDR_T *) buf;
    LOG_INFO("seq = %d, ts = %d", ntohs(rtp_h->seq), ntohl(rtp_h->ts));
    LOG_INFO("Success after doPackage H264Packet length = %d", length);
    int n = sendto(sock, (const void *) buf, length, 0,
            (struct sockaddr *) &servaddr, sizeof(servaddr));
    if (n == -1)
    {
        ERR_EXIT("sendto");
    }
}

static void failureFunc(int errNo)
{
    LOG_INFO("failure in doPackage! errNo: %d", errNo);
    
}

char * generateMockData(int length)
{
    char *data = new char[length];
    
    return data;
}
void echo_cli(int sock, char *addr, char *port)
{
    LOG_INFO("[LispMediaPlayer] in echo_cli, addr is %s, port is %s", addr,
            port);
    const char* ipAddr = addr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(port));
    servaddr.sin_addr.s_addr = inet_addr(ipAddr);
    
    int ret;
    //char sendbuf[BUF_LENGTH] = {0};
    u_char *sendbuf = NULL;
    PMEMORYPOOL pMemoryPool = generateRTPMemoryPool(100, 1500);
    H264RtpPacket* pmyPacket = new H264RtpPacket(pMemoryPool, 30);
    
    u_char *data[2];
    data[0] = (u_char *) malloc(1000);
    memset(data[0], 0, 1000);
    data[1] = (u_char *) malloc(3000);
    memset(data[1], 0, 3000);
    unsigned char nalu_start_prefix1[5] =
            { 0x00, 0x00, 0x00, 0x01, 0x6B };
    unsigned char nalu_start_prefix2[4] =
            { 0x00, 0x00, 0x01, 0x4C };
    memcpy(data[0], nalu_start_prefix1, 5);
    memcpy(data[1], nalu_start_prefix2, 4);
    
    int lengths[2] =
            { 1000, 3000 };
    int count = 10;
    while (count-- > 0)
    {
        for (int i = 0; i < 2; i++)
        {
            sendbuf = data[i];
            LOG_INFO("[LispMediaPlayer] will package RTP packet!");
            pmyPacket->doPackage((u_char *) sendbuf, lengths[i], 0, successFunc,
                    failureFunc);
        }
    }
    close(sock);
    LOG_INFO("memypool is full %d", pMemoryPool->isFullPool(pMemoryPool));
    LOG_INFO("memypool is full %d", pMemoryPool->isEmptyPool(pMemoryPool));
    destoryRTPMemoryPool(pMemoryPool);
    
}

int sender(int argc, char *argv[])
{
    if (argc != 2)
        ERR_EXIT("Invalid Input");
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        ERR_EXIT("socket");
    echo_cli(sock, argv[0], argv[1]);
    LOG_INFO("This is sender() in c!");
    return 0;
}

