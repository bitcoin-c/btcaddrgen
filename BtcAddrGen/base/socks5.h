#ifndef SOCKS5_H_INCLUDED
#define SOCKS5_H_INCLUDED

#include "compatible.h"
typedef enum{
    SOCKS5_ADDR_TYPE_IPV4 = 0x01,
    SOCKS5_ADDR_TYPE_DOMAIN_NAME = 0x03,
    SOCKS5_ADDR_TYPE_IPV6 = 0x04
}SOCKS5_ADDR_TYPE;

#define MAX_SOCKS5_NAME_STRING_LEN (256)


#pragma pack(push)
#pragma pack(1)
struct socks5_init_req
{
    unsigned char version;
    unsigned char count;
    unsigned char methods[255];
};
struct socks5_init_resp
{
    unsigned char version;
    unsigned char method;
};


struct socks5_req
{
    unsigned char version;
    unsigned char cmd;
    unsigned char reserved;
    unsigned char addr_type;
    union
    {
        struct in_addr ipv4;
        struct in6_addr ipv6;
        struct
        {
            unsigned char cb_dns;
            char dns[MAX_SOCKS5_NAME_STRING_LEN];
        };
    }dest_addr;
    unsigned short port;

};


struct socks5_resp
{
    unsigned char version;
    unsigned char reply;
    unsigned char reserved;
    unsigned char addr_type;
    union
    {
        struct in_addr ipv4;
        struct in6_addr ipv6;
        struct
        {
            unsigned char cb_dns;
            char dns[256];
        };
    }bind_addr;
    unsigned short port;

};
#pragma pack(pop)

BOOL socks5_init(SOCKET sockfd);
BOOL socks5_connect(SOCKET sockfd, const struct socks5_req *req, struct socks5_resp *resp);
BOOL socks5_connect_ipv4(SOCKET sockfd, const char * ipv4, unsigned short port);

BOOL socks5_handshake(SOCKET sockfd, SOCKS5_ADDR_TYPE addr_type, const char * szAddr, size_t cbAddr, unsigned short port);

#endif // SOCKS5_H_INCLUDED
