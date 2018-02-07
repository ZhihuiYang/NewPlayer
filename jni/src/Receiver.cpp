/*!
 * \file Recver.cpp
 *
 * \date Created on: Aug 8, 2016
 * \author: eyngzui
 */
#include "include/h264_RtpPacket.h"
#include "include/logger.h"
#include "include/memoryPool.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFF_LEN 1500

struct sockaddr_in addr_serv, addr_client;
int handler = socket(AF_INET, SOCK_DGRAM, 0);

static void successFunc(Lisp_AVPacket data)
{
    LOG_INFO("unpack success seq = %d", data.seq);
    LOG_INFO("unpack success isNalBeginning =  %d", data.isNalBeginning);
    LOG_INFO("unpack success ts = %d", data.ts);
}

static void failureFunc(int errNo, void *)
{
    
}

void usage(char *program)
{
    LOG_INFO("usage:\n");
    LOG_INFO("%s local_ip local_port peer_ip peer_port\n", program);
}

int receiver(int argc, char *argv[])
{
    if (argc != 4)
    {
        LOG_ERROR("wrong parameters, exit!\n");
        usage(argv[0]);
        return 0;
    }
    
    bzero(&addr_serv, sizeof(addr_serv));
    int local_port = atoi(argv[1]);
    addr_serv.sin_family = AF_INET;
    addr_serv.sin_port = htons(local_port);
    addr_serv.sin_addr.s_addr = inet_addr(argv[0]);
    
    bzero(&addr_client, sizeof(addr_client));
    int peer_port = atoi(argv[3]);
    addr_client.sin_family = AF_INET;
    addr_client.sin_port = htons(peer_port);
    addr_client.sin_addr.s_addr = inet_addr(argv[2]);
    
    u_char buff[BUFF_LEN];
    PMEMORYPOOL pPool = generateRTPMemoryPool(0, 10);
    int socklen_t = sizeof(addr_client);
    
    if (handler == -1)
    {
        LOG_ERROR("create socket error");
        return -1;
    }
    
    int ret = bind(handler, (struct sockaddr *) &addr_serv,
            sizeof(struct sockaddr));
    if (ret == -1)
    {
        close(handler);
        LOG_ERROR("error when trying to bind local ip/port");
        return -1;
    }
    
    H264RtpPacket * packet = new H264RtpPacket(pPool, 0);
    while (1)
    {
//        int n = recvfrom(handler, buff, BUFF_LEN, 0,
//                (struct sockaddr *) &addr_client,
//                &sock_client_len);
//        packet->doParse(buff, n, successFunc, failureFunc);
    }
    
    return 0;
}
