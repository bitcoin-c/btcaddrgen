#include "compatible.h"
#include "satoshi_protocol.h"
#include "util.h"

#define SATOSHI_PROTOCOL_VERSION (70002)
const char * ch_default_user_agent = "/chehw:0.1.0.alpha/";
const unsigned char zero_byte_checksum[4] = {0x5D, 0xF6, 0xE0, 0xE2};
const uint32_t satoshi_null_checksum = 0xE2E0F65D;

const unsigned char ipv4_mapped_pack[12] = {0,0,0,0,0,0,0,0,0,0,0xff,0xff};

const uint32_t satoshi_magic_main = 0xD9B4BEF9; //	 F9 BE B4 D9
const uint32_t satoshi_magic_testnet = 0xDAB5BFFA; //	 FA BF B5 DA
const uint32_t satoshi_magic_testnet3 = 0x0709110B; //	 0B 11 09 07
const uint32_t satoshi_magic_namecoin = 0xFEB4BEF9; //	 F9 BE B4 FE

const unsigned char satoshi_genesis_hash[32] = {
    0x6f, 0xe2, 0x8c, 0x0a, 0xb6, 0xf1, 0xb3, 0x72,
    0xc1, 0xa6, 0xa2, 0x46, 0xae, 0x63, 0xf7, 0x4f,
    0x93, 0x1e, 0x83, 0x65, 0xe1, 0x5a, 0x08, 0x9c,
    0x68, 0xd6, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00
};

uint32_t satoshi_magic_number; // = (uint32_t)satoshi_magic_testnet3;
const int32_t SATOSHI_DEFAULT_TX_VERSION = 1;

uint32_t chutil_calc_merkel_tree_size(uint32_t h_count)
{
    uint32_t c = h_count;
    uint32_t size = h_count;

    while(c > 1)
    {
        if(c % 2) c++;
        c /= 2;

        size += c;
    }
    return size;
}

BOOL chutil_get_merkel_tree(const struct _uint256 * tx_hashes, uint32_t h_count, struct _satoshi_merkle_tree * p_tree)
{
    assert(tx_hashes != NULL || p_tree != NULL);
    uint32_t c;
    uint32_t i;
    struct _uint256 buffer[2];
    struct _uint256 * p_hash, * p_start;

    if(h_count == 0) return FALSE;

    bzero(buffer, sizeof(buffer));

    p_tree->leafs = h_count;
    p_tree->size = chutil_calc_merkel_tree_size(h_count);

    p_tree->h_list = (struct _uint256 *)malloc(sizeof(struct _uint256) * p_tree->size);
    if(NULL == p_tree->h_list) return FALSE;


    if(h_count == 1)
    {
        memcpy(p_tree->h_list, tx_hashes, sizeof(struct _uint256));
        return TRUE;
    }else
    {
        memcpy(p_tree->h_list, tx_hashes, sizeof(struct _uint256) * h_count);
        p_hash = p_tree->h_list + h_count;


        p_start = p_tree->h_list;

        c = h_count;

        while(c > 1)
        {
            for(i = 0; i < (c / 2); i++)
            {
                Hash256((unsigned char *)p_start, 2 * sizeof(struct _uint256), &p_hash->vch[0]);
                p_start += 2;
                p_hash++;
            }
            if(c % 2)
            {
                memcpy(&buffer[0], p_start, sizeof(struct _uint256));
                memcpy(&buffer[1], p_start, sizeof(struct _uint256));
                Hash256((unsigned char *)&buffer, 2 * sizeof(struct _uint256), &p_hash->vch[0]);
                p_hash++;
                p_start++;
            }
            if(c == 1) break;
            if(c % 2) c++;
            c /= 2;


        }
        return TRUE;
    }
}



uint32_t SATOSHI_MSG_recv_data(int sockfd, void * data, uint32_t cbData)
{
    uint32_t cbTotal = cbData;
    int32_t cb;

    unsigned char * p = (unsigned char *)data;

    if(cbData == 0) return 0;
    do
    {
        cb = recv(sockfd, (char *)p, cbTotal, 0);
        if(cb <= 0) break;
        if(cb == cbTotal) return cbData;
        p += cb;
        cbTotal -= cb;
    }while(cbTotal);
    return 0;

}

uint32_t SATOSHI_MSG_send_data(int sockfd, const void * data, uint32_t cbData)
{
    uint32_t cbTotal = cbData;
    int32_t cb;
    char * p = (char *)data;
    do
    {
        cb = send(sockfd, (char *)p, cbTotal, 0);
        if(cb <= 0) break;
        if(cb == cbTotal) return cbData;
        p += cb;
        cbTotal -= cb;
    }while(TRUE);
    return 0;
}


uint32_t SATOSHI_MSG_send_header(int sockfd, const struct _satoshi_msg_header * p_header)
{
    return SATOSHI_MSG_send_data(sockfd, (const unsigned char *)p_header, sizeof(struct _satoshi_msg_header));
}
uint32_t SATOSHI_MSG_recv_header(int sockfd, struct _satoshi_msg_header * p_header)
{
    return SATOSHI_MSG_recv_data(sockfd, (unsigned char *)p_header, sizeof(struct _satoshi_msg_header));
}

uint32_t SATOSHI_MSG_send_cmd(int sockfd, const char cmd[12], uint32_t cbPayload, uint32_t checksum)
{
    struct _satoshi_msg_header hdr;
    hdr.magic = satoshi_magic_number;
    hdr.length = cbPayload;
    strncpy(hdr.cmd, cmd, 12);
    if(cbPayload == 0) memcpy(&hdr.checksum, &zero_byte_checksum[0], 4);
    else memcpy(&hdr.checksum, &checksum, 4);
    return SATOSHI_MSG_send_data(sockfd, (const unsigned char *)&hdr, sizeof(struct _satoshi_msg_header));
}


uint32_t SATOSHI_MSG_send(int sockfd, const char cmd[12], const unsigned char * payload, uint32_t cbPayload)
{
    struct _satoshi_msg_header hdr;
    uint32_t cb;
    struct _uint256 hash;

    bzero(&hdr, sizeof(hdr));
    hdr.magic = satoshi_magic_number;
    hdr.length = cbPayload;
    strcpy(hdr.cmd, cmd);
    if(cbPayload)
    {
        if(NULL == payload) return 0; // invalid parameters
        Hash256(payload, cbPayload, &hash.vch[0]);
        memcpy(&hdr.checksum, &hash.vch[0], 4);
    }else
    {
        memcpy(&hdr.checksum, &zero_byte_checksum[0], 4);
    }

    cb = SATOSHI_MSG_send_header(sockfd, &hdr);
    if(cb == 0) return 0;

    if(cbPayload)
    {
        cb = SATOSHI_MSG_send_data(sockfd, payload, cbPayload);
        if(cb != cbPayload) return 0;
    }
    return (sizeof(struct _satoshi_msg_header) + cbPayload);
}
uint32_t SATOSHI_MSG_recv(int sockfd, char cmd[12], unsigned char * payload, uint32_t cbPayload)
{
/*
    if(strcmp(cmd, "version") == 0)
    {
        return SATOSHI_MSG_VERSION_recv(sockfd, payload, cbPayload);
    }else if(strcmp(cmd, "verack") == 0)
    {
        return SATOSHI_MSG_VERACK_recv(sockfd, payload, cbPayload);
    }else if(strcmp(cmd, "addr") == 0)
    {
        return SATOSHI_MSG_ADDR_recv(sockfd, payload, cbPayload);
    }else if(strcmp(cmd, "inv") == 0)
    {
        return SATOSHI_MSG_INV_recv(sockfd, payload, cbPayload);
    }else if(strcmp(cmd, "getdata") == 0)
    {
        return SATOSHI_MSG_GETDATA_recv(sockfd, payload, cbPayload);
    }else if(strcmp(cmd, "notfound") == 0)
    {
        return SATOSHI_MSG_NOTFOUND_recv(sockfd, payload, cbPayload);
    }else if(strcmp(cmd, "getblocks") == 0)
    {
        return SATOSHI_MSG_GETBLOCKS_recv(sockfd, payload, cbPayload);
    }else if(strcmp(cmd, "getheaders") == 0)
    {
        return SATOSHI_MSG_GETHEADERS_recv(sockfd, payload, cbPayload);
    }else if(strcmp(cmd, "tx") == 0)
    {

    }else if(strcmp(cmd, "block") == 0)
    {

    }else if(strcmp(cmd, "headers") == 0)
    {

    }else if(strcmp(cmd, "getaddr") == 0)
    {

    }else if(strcmp(cmd, "block") == 0)
    {

    }else if(strcmp(cmd, "mempool") == 0)
    {

    }else if(strcmp(cmd, "ping") == 0)
    {

    }else if(strcmp(cmd, "pong") == 0)
    {

    }else if(strcmp(cmd, "reject") == 0)
    {

//CCodes
//
//Value	 Name	                Description
//0x01	 REJECT_MALFORMED
//0x10	 REJECT_INVALID
//0x11	 REJECT_OBSOLETE
//0x12	 REJECT_DUPLICATE
//0x40	 REJECT_NONSTANDARD
//0x41	 REJECT_DUST
//0x42	 REJECT_INSUFFICIENTFEE
//0x43	 REJECT_CHECKPOINT


    }else if(strcmp(cmd, "alert") == 0)
    {

    }
    else
    {
    // filterload, filteradd, filterclear, merkleblock
    // These messages are related to Bloom filtering of connections and are defined in BIP 0037.
    }
*/
    return 0;
}

SOCKET SATOSHI_PROTOCOL_connect(const char * ipv4, unsigned short port)
{
    int sockfd = INVALID_SOCKET;
    struct sockaddr_in in_addr;
    int rc;

    bzero(&in_addr, sizeof(struct sockaddr_in));
    in_addr.sin_family = AF_INET;
    in_addr.sin_port = htons(port);
    in_addr.sin_addr.S_un.S_addr = inet_addr(ipv4);

    sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sockfd == INVALID_SOCKET) return INVALID_SOCKET;

    printf("prepare connect to %s:%u...\n", ipv4, port);

    rc = connect(sockfd, (struct sockaddr *)&in_addr, sizeof(in_addr));
    if(rc < 0)
    {
        closesocket(sockfd);
        sockfd = INVALID_SOCKET;
    }
    return sockfd;
}

SOCKET SATOSHI_PROTOCOL_handshake(SOCKET sockfd, const char * ipv4, unsigned short port,
                               const char * user_agent, uint32_t magic_number,
                               uint32_t height, bool relay,
                               struct _satoshi_msg_version ** pp_peerversion
                               )
{
    struct _satoshi_msg_header hdr;
    struct _satoshi_msg_version * p_version;


    uint32_t cb;
    uint32_t cbVersion;
    struct _uint256 hash;

    if(magic_number == 0) magic_number = satoshi_magic_main;
    if(NULL == user_agent) user_agent = ch_default_user_agent;

    //** Get my version
    p_version = SATOSHI_MSG_VERSION_new(user_agent, strlen(user_agent));
    if(NULL == p_version) return 0;
    cbVersion = SATOSHI_MSG_VERSION_get_size(p_version);
    p_version->version = SATOSHI_PROTOCOL_VERSION;
    p_version->services = NODE_NETWORK;
    p_version->timestamp = GetTime();
    SATOSHI_ADDR_set_ipv4(&p_version->addr_recv, ipv4, port);
//    SATOSHI_ADDR_set_ipv4(&p_version->addr_from, "127.0.0.1", port);
    p_version->nonce = GetTimeMicro();
    SATOSHI_MSG_VERSION_set_height(p_version, height);
    SATOSHI_MSG_VERSION_set_relay(p_version, relay);
    cbVersion = SATOSHI_MSG_VERSION_get_size(p_version);


    //** Init socket connection
    if(sockfd == INVALID_SOCKET) sockfd = SATOSHI_PROTOCOL_connect(ipv4, port);
    if(INVALID_SOCKET == sockfd) goto label_errexit;

    // Init msg header
    bzero(&hdr, sizeof(hdr));
    hdr.magic = magic_number;
    strcpy(hdr.cmd, "version");
    hdr.length = cbVersion;

    Hash256((unsigned char *)p_version, cbVersion, &hash.vch[0]);
    memcpy(&hdr.checksum, &hash.vch[0], 4);

    cb = SATOSHI_MSG_send_header(sockfd, &hdr);
    if(cb == 0) goto label_errexit;

    cb = SATOSHI_MSG_send_data(sockfd, p_version, cbVersion);
    if(cb != cbVersion) goto label_errexit;
    free(p_version);
    p_version = NULL;

//** recv node version
    bzero(&hdr, sizeof(hdr));
    cb = recv(sockfd, (char *)&hdr, sizeof(hdr), 0);
    if(cb != sizeof(hdr)) goto label_errexit;

    if(strcmp(hdr.cmd, "version") != 0) goto label_errexit;

    cbVersion = hdr.length;
    p_version = (struct _satoshi_msg_version *)malloc(cbVersion);
    cb = SATOSHI_MSG_recv_data(sockfd, p_version, cbVersion);
    if(cb != cbVersion) goto label_errexit;

    //** verify checksum
    Hash256((unsigned char *)p_version, cbVersion, &hash.vch[0]);
    if(memcmp(&hash.vch[0], &hdr.checksum, 4) != 0)
    {
        printf("version: checksum error!\n");
        goto label_errexit;
    }

    bzero(&hdr.cmd[0], 12);
    strcpy(hdr.cmd, "verack");
    hdr.length = 0;
    memcpy(&hdr.checksum, &zero_byte_checksum[0], 4);

    cb = SATOSHI_MSG_send_header(sockfd, &hdr);
    if(cb != sizeof(struct _satoshi_msg_header)) goto label_errexit;

    bzero(&hdr, sizeof(hdr));
    cb = SATOSHI_MSG_recv_data(sockfd, &hdr, sizeof(hdr));
    if(cb != sizeof(hdr)) goto label_errexit;

    printf("verack\n");
    if(NULL != pp_peerversion) *pp_peerversion = p_version;
    else
    {
        free(p_version);
        p_version = NULL;
    }
    return sockfd;
 label_errexit:
    printf("handshake error.\n");
    if(NULL != p_version) free(p_version);
    if(sockfd != INVALID_SOCKET) closesocket(sockfd);
    return INVALID_SOCKET;
}



//*****************************************
//** SATOSHI_NET_ADDR
uint32_t SATOSHI_UTIL_get_addrtype(const unsigned char ip[16])
{
    if(memcmp(&ip[0], &ipv4_mapped_pack[0], 12) == 0) return 4;
    return 16;
}
uint32_t SATOSHI_NET_ADDR_set_ipv4(struct _satoshi_net_addr * p_addr, const char * ipv4, unsigned short port)
{
    struct in_addr addr;
    p_addr->time = (uint32_t)GetTime();
    p_addr->services = NODE_NETWORK;

    addr.s_addr = inet_addr(ipv4);
    memcpy(&p_addr->ip[12], &addr, 4);
    memcpy(&p_addr->ip[0],&ipv4_mapped_pack[0], 12);

    p_addr->port = htons(port);

    return 4;
}
uint32_t SATOSHI_NET_ADDR_set_ipv6(struct _satoshi_net_addr * p_addr, const char * ipv6, unsigned short port)
{
    return 0;
}


uint32_t SATOSHI_UTIL_set_ip(unsigned char ip[16], const char * szIp, uint32_t type)
{
    if(type == 4) // ipv6
    {
        struct in_addr *addr = (struct in_addr *)&ip[12];

        memcpy(&ip[0], ipv4_mapped_pack, 12);
        addr->s_addr = inet_addr(szIp);
        return type;
    }else if (type == 16) // ipv6
    {
    #if defined(WIN32) && defined(__WIN32_INET_SUPPORT_IPV6)
        struct in6_addr *addr = (struct in6_addr *)&ip[0];
        int rc;
        rc = inet_pton(AF_INET6, szIp, addr);
        if(rc == 1) return type;
    #else
        printf("not support ipv6.\n");
    #endif

    }
    return 0;
}

uint32_t SATOSHI_UTIL_get_ip(const unsigned char ip[16], char * szIp)
{
    uint32_t type = SATOSHI_UTIL_get_addrtype(ip);
    uint32_t cb = 0;
    if(NULL == szIp) return 0;

    szIp[0] = 0;
    if(type == 4)
    {
        char * p = NULL;
        Lock();
        p = inet_ntoa(*(struct in_addr *)&ip[12]);
        if(NULL != p)
        {
            cb = strlen(szIp);
            strcpy(szIp, p);

        }
        Unlock();

    }else
    {
    #if defined(WIN32) && defined(__WIN32_INET_SUPPORT_IPV6)
        if( NULL != inet_ntop(AF_INET6, &ip[0], szIp, 100))
        {
            cb = strlen(szIp);
        }
    #else
        printf("not support ipv6\n");
    #endif
    }
    return cb;
}

void SATOSHI_ADDR_dump(const struct _satoshi_addr * p_addr, FILE *fp)
{
    char szLine[200] = "";
    uint32_t cbLine;
    uint32_t port;


    if(NULL == p_addr) return;

    if(NULL == fp) fp = stdout;


    cbLine = snprintf(szLine, 200, "services: "PRI_llu"\n", p_addr->services);
    fwrite(szLine, 1, cbLine, fp);

    SATOSHI_UTIL_get_ip(p_addr->ip, szLine);
    cbLine = strlen(szLine);
    fwrite(szLine, 1, cbLine, fp);

    port = ntohs(p_addr->port);
    cbLine = snprintf(szLine, 200, "\nport: %hu\n", port);
    fwrite(szLine, 1, cbLine, fp);
}

uint32_t SATOSHI_ADDR_set_ipv4(struct _satoshi_addr * p_addr, const char * ipv4, unsigned short port)
{
    p_addr->services = NODE_NETWORK;
    SATOSHI_UTIL_set_ip(p_addr->ip, ipv4, 4);
    p_addr->port = htons(port);
    return sizeof(struct _satoshi_addr);

}
uint32_t SATOSHI_ADDR_set_ipv6(struct _satoshi_addr * p_addr, const char * ipv6, unsigned short port)
{
    p_addr->services = NODE_NETWORK;
    SATOSHI_UTIL_set_ip(p_addr->ip, ipv6, 16);
    p_addr->port = htons(port);
    return sizeof(struct _satoshi_addr);
}

//********************************************************
//** SATOSHI_MSG_VERSION
struct _satoshi_msg_version * SATOSHI_MSG_VERSION_new(const char * user_agent, uint32_t cbUserAgent)
{
    struct _satoshi_msg_version * p_version;
    uint32_t size;
    uint32_t cbVarint;

    if(NULL == user_agent) cbUserAgent = 0;
    else
    {
        if(0 == cbUserAgent) cbUserAgent = strlen(user_agent);
    }

    size = sizeof(struct _satoshi_msg_version) -1;

    cbVarint = VARINT_calc_size(cbUserAgent);

    size += cbVarint + cbUserAgent;
    size += sizeof(int32_t); // start_height;
    size += 1; //sizeof(bool); // relay

    p_version = (struct _satoshi_msg_version *)malloc(size);
    if(NULL == p_version) return NULL;

    bzero(p_version, size);
    VARINT_set_value((struct _varint *)&p_version->user_agent[0], cbUserAgent);
    if(cbUserAgent) memcpy(&p_version->user_agent[cbVarint], user_agent, cbUserAgent);
    return p_version;
}
void SATOSHI_MSG_VERSION_free(struct _satoshi_msg_version *p_version)
{
    if(NULL != p_version) free(p_version);
}
BOOL SATOSHI_MSG_VERSION_set_height(struct _satoshi_msg_version *p_version, const int32_t height)
{
    uint32_t vstr_size = 0;
    int32_t * p_height = NULL;
    if(NULL == p_version) return FALSE;
    vstr_size = VARSTR_get_size((struct _varstr *)&p_version->user_agent[0]);
    if(vstr_size == 0) return FALSE; // error format

    p_height = (int32_t *)((unsigned char *)&p_version->user_agent[0] + vstr_size);
    if(NULL == p_height) return FALSE; // error format

    *p_height = height;

    return TRUE;
}
BOOL SATOSHI_MSG_VERSION_set_relay(struct _satoshi_msg_version *p_version, const bool relay)
{
    uint32_t vstr_size = 0;
    bool * p_relay = NULL;
    if(NULL == p_version) return FALSE;
    vstr_size = VARSTR_get_size((struct _varstr *)&p_version->user_agent[0]);
    if(vstr_size == 0) return FALSE; // error format

    p_relay = (bool *)((unsigned char *)&p_version->user_agent[0] + vstr_size + sizeof(int32_t));
    if(NULL == p_relay) return FALSE; // error format

    *p_relay = relay;

    return TRUE;
}
int32_t SATOSHI_MSG_VERSION_get_height(const struct _satoshi_msg_version *p_version)
{
    uint32_t vstr_size = 0;
    int32_t * p_height = NULL;
    if(NULL == p_version) return 0;
    vstr_size = VARSTR_get_size((struct _varstr *)&p_version->user_agent[0]);
    if(vstr_size == 0) return 0; // error format

    p_height = (int32_t *)((unsigned char *)&p_version->user_agent[0] + vstr_size);
    if(NULL == p_height) return 0; // error format

    return (*p_height);
}
bool SATOSHI_MSG_VERSION_get_relay(const struct _satoshi_msg_version *p_version)
{
    uint32_t vstr_size = 0;
    bool * p_relay = NULL;
    if(NULL == p_version) return 0;
    vstr_size = VARSTR_get_size((struct _varstr *)&p_version->user_agent[0]);
    if(vstr_size == 0) return 0; // error format

    p_relay = (bool *)((unsigned char *)&p_version->user_agent[0] + vstr_size + sizeof(int32_t));
    if(NULL == p_relay) return 0; // error format
    return (*p_relay);
}

uint32_t SATOSHI_MSG_VERSION_get_size(const struct _satoshi_msg_version * p_version)
{
    uint32_t cb = sizeof(struct _satoshi_msg_version) - 1;
    struct _varstr * p_useragent;

    p_useragent = (struct _varstr *)p_version->user_agent;
    cb += VARSTR_get_size(p_useragent);

    cb += sizeof(int32_t) + sizeof(bool);
    return cb;

}

void SATOSHI_MSG_VERSION_dump(const struct _satoshi_msg_version *p_version, FILE * fp)
{
    char szLine[200] = "";
    uint32_t cbLine;
    SYSTEMTIME st;
    bzero(&st, sizeof(st));
    if(NULL == fp) fp = stdout;

    cbLine = snprintf(szLine, 200, "version: "PRI_d"\n", p_version->version);
    fwrite(szLine, 1, cbLine, fp);

    cbLine = snprintf(szLine, 200, "service: "PRI_llu"\n", p_version->services);
    fwrite(szLine, 1, cbLine, fp);

    UnixtimeToSystemTime(p_version->timestamp, &st);
    cbLine = snprintf(szLine, 200, "timestamp(%.8x%.8x): %.4d-%.2d-%.2d %.2d:%.2d:%.2d UTC\n",
        (uint32_t)(p_version->timestamp >>32), (uint32_t)(p_version->timestamp &0xffffffff),
        st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

    fwrite(szLine, 1, cbLine, fp);

    SATOSHI_ADDR_dump(&p_version->addr_recv, fp);
    SATOSHI_ADDR_dump(&p_version->addr_from, fp);

    cbLine = snprintf(szLine, 200, "nonce: (%.8x%.8x)\n",
        (uint32_t)(p_version->nonce >>32), (uint32_t)(p_version->nonce &0xffffffff));
    fwrite(szLine, 1, cbLine, fp);

    cbLine = sprintf(szLine, "user_agent: ");
    cbLine += VARSTR_to_string((struct _varstr *)&p_version->user_agent[0], &szLine[cbLine], 200 - cbLine);
    szLine[cbLine++] = '\n';
    fwrite(szLine, 1, cbLine, fp);

    cbLine = snprintf(szLine, 200, "start_height: %u\n", SATOSHI_MSG_VERSION_get_height(p_version));
    fwrite(szLine, 1, cbLine, fp);

    cbLine = snprintf(szLine, 200, "relay: "PRI_hhu"\n", SATOSHI_MSG_VERSION_get_relay(p_version));
    fwrite(szLine, 1, cbLine, fp);

}



