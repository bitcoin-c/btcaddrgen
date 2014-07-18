#ifndef SATOSHI_PROTOCOL_H_INCLUDED
#define SATOSHI_PROTOCOL_H_INCLUDED

#include "compatible.h"
#include "util.h"
#include "datatype.h"

enum SATOSHI_SERVICE
{
    NODE_NETWORK = 1
};

enum SATOSHI_MSG_OBJECT_TYPE
{
    SATOSHI_MSG_OBJECT_ERROR = 0,
    SATOSHI_MSG_OBJECT_MSG_TX = 1,
    SATOSHI_MSG_OBJECT_MSG_BLOCK = 2
};

extern const int32_t SATOSHI_DEFAULT_TX_VERSION;



typedef struct _satoshi_net_addr
{
    uint32_t time; // version > 31402
    uint64_t services;
    unsigned char ip[16]; // IPv4-mapped IPv6 address (12 bytes 00 00 00 00 00 00 00 00 00 00 FF FF, followed by the 4 bytes of the IPv4 address).
    unsigned char port; // network byte order
}satoshi_net_addr;

uint32_t SATOSHI_UTIL_set_ip(unsigned char ip[16], const char * szIp, uint32_t type);
uint32_t SATOSHI_UTIL_get_addrtype(const unsigned char ip[16]);
uint32_t SATOSHI_UTIL_get_ip(const unsigned char ip[16], char *szIp);

uint32_t SATOSHI_NET_ADDR_set_ipv4(struct _satoshi_net_addr * p_addr, const char * ipv4, unsigned short port);
uint32_t SATOSHI_NET_ADDR_set_ipv6(struct _satoshi_net_addr * p_addr, const char * ipv6, unsigned short port);

#pragma pack(push)
#pragma pack(1)
typedef struct _satoshi_inv
{
    uint32_t msg_obj_type;
    struct _uint256 hash;
}satoshi_inv;

typedef struct _satoshi_addr
{
    uint64_t services;
    unsigned char ip[16]; // IPv4-mapped IPv6 address (12 bytes 00 00 00 00 00 00 00 00 00 00 FF FF, followed by the 4 bytes of the IPv4 address).
    unsigned short port; // network byte order
}satoshi_addr;
uint32_t SATOSHI_ADDR_set_ipv4(struct _satoshi_addr * p_addr, const char * ipv4, unsigned short port);
uint32_t SATOSHI_ADDR_set_ipv6(struct _satoshi_addr * p_addr, const char * ipv6, unsigned short port);


typedef struct _satoshi_msg_header
{
    uint32_t magic;
    char cmd[12];
    uint32_t length;
    uint32_t checksum;
}satoshi_msg_header;


typedef struct _satoshi_msg_version
{
    int32_t version;
    uint64_t services;
    uint64_t timestamp;
    struct _satoshi_addr addr_recv;
    //version >= 106
    struct _satoshi_addr addr_from;
    uint64_t nonce;
    unsigned char user_agent[1];
    // int32_t start_height;
    // bool relay;
}satoshi_msg_version;
#pragma pack(pop)

struct _satoshi_msg_version * SATOSHI_MSG_VERSION_new(const char * user_agent, uint32_t cbUserAgent);
uint32_t SATOSHI_MSG_VERSION_get_size(const struct _satoshi_msg_version * p_version);
void SATOSHI_MSG_VERSION_free(struct _satoshi_msg_version *p_version);
BOOL SATOSHI_MSG_VERSION_set_height(struct _satoshi_msg_version *p_version, const int32_t height);
BOOL SATOSHI_MSG_VERSION_set_relay(struct _satoshi_msg_version *p_version, const bool relay);
int32_t SATOSHI_MSG_VERSION_get_height(const struct _satoshi_msg_version *p_version);
bool SATOSHI_MSG_VERSION_get_relay(const struct _satoshi_msg_version *p_version);
void SATOSHI_MSG_VERSION_dump(const struct _satoshi_msg_version *p_version, FILE * fp);



uint32_t SATOSHI_MSG_recv_data(int sockfd, void * data, uint32_t cbData);
uint32_t SATOSHI_MSG_send_data(int sockfd, const void * data, uint32_t cbData);
uint32_t SATOSHI_MSG_send_header(int sockfd, const struct _satoshi_msg_header * p_header);
uint32_t SATOSHI_MSG_recv_header(int sockfd, struct _satoshi_msg_header * p_header);


uint32_t SATOSHI_MSG_send(int sockfd, const char cmd[12], const unsigned char * payload, uint32_t cbPayload);
uint32_t SATOSHI_MSG_recv(int sockfd, char cmd[12], unsigned char * payload, uint32_t cbPayload);

SOCKET SATOSHI_PROTOCOL_connect(const char * ipv4, unsigned short port);
SOCKET SATOSHI_PROTOCOL_handshake(SOCKET sockfd, const char * ipv4, unsigned short port,
                               const char * user_agent, uint32_t magic_number,
                               uint32_t height, bool relay,
                               struct _satoshi_msg_version ** pp_peerversion
                               );

typedef struct _satoshi_merkle_tree
{
    uint32_t leafs;
    uint32_t size;
    struct _uint256 * h_list;
}satoshi_merkle_tree;

BOOL chutil_get_merkel_tree(const struct _uint256 * tx_hashes, uint32_t h_count, struct _satoshi_merkle_tree * p_tree);
uint32_t chutil_calc_merkel_tree_size(uint32_t h_count);

extern const unsigned char ipv4_mapped_pack[12];
extern const unsigned char zero_byte_checksum[4];
extern uint32_t satoshi_magic_number;
extern const uint32_t satoshi_magic_main; // = 0xD9B4BEF9; //	 F9 BE B4 D9
extern const uint32_t satoshi_magic_testnet; // = 0xDAB5BFFA; //	 FA BF B5 DA
extern const uint32_t satoshi_magic_testnet3; // = 0x0709110B; //	 0B 11 09 07
extern const uint32_t satoshi_magic_namecoin; // = 0xFEB4BEF9; //	 F9 BE B4 FE

extern const unsigned char satoshi_genesis_hash[32];


#endif // SATOSHI_PROTOCOL_H_INCLUDED
