
#include "socks5.h"

/*
The client connects to the server, and sends a version
   identifier/method selection message:
                   +----+----------+----------+
                   |VER | NMETHODS | METHODS  |
                   +----+----------+----------+
                   | 1  |    1     | 1 to 255 |
                   +----+----------+----------+
The VER field is set to X'05' for this version of the protocol.  The
   NMETHODS field contains the number of method identifier octets that
   appear in the METHODS field.

   The server selects from one of the methods given in METHODS, and
   sends a METHOD selection message:
                         +----+--------+
                         |VER | METHOD |
                         +----+--------+
                         | 1  |   1    |
                         +----+--------+
The values currently defined for METHOD are:

          o  X'00' NO AUTHENTICATION REQUIRED
          o  X'01' GSSAPI
          o  X'02' USERNAME/PASSWORD
          o  X'03' to X'7F' IANA ASSIGNED
          o  X'80' to X'FE' RESERVED FOR PRIVATE METHODS
          o  X'FF' NO ACCEPTABLE METHODS
*/


/*
Requests

   Once the method-dependent subnegotiation has completed, the client
   sends the request details.  If the negotiated method includes
   encapsulation for purposes of integrity checking and/or
   confidentiality, these requests MUST be encapsulated in the method-
   dependent encapsulation.

   The SOCKS request is formed as follows:

        +----+-----+-------+------+----------+----------+
        |VER | CMD |  RSV  | ATYP | DST.ADDR | DST.PORT |
        +----+-----+-------+------+----------+----------+
        | 1  |  1  | X'00' |  1   | Variable |    2     |
        +----+-----+-------+------+----------+----------+

     Where:

          o  VER    protocol version: X'05'
          o  CMD
             o  CONNECT X'01'
             o  BIND X'02'
             o  UDP ASSOCIATE X'03'
          o  RSV    RESERVED
          o  ATYP   address type of following address
             o  IP V4 address: X'01'
             o  DOMAINNAME: X'03'
             o  IP V6 address: X'04'
          o  DST.ADDR       desired destination address
          o  DST.PORT desired destination port in network octet
             order

   The SOCKS server will typically evaluate the request based on source
   and destination addresses, and return one or more reply messages, as
   appropriate for the request type.
*/

/*
Replies

   The SOCKS request information is sent by the client as soon as it has
   established a connection to the SOCKS server, and completed the
   authentication negotiations.  The server evaluates the request, and
   returns a reply formed as follows:

        +----+-----+-------+------+----------+----------+
        |VER | REP |  RSV  | ATYP | BND.ADDR | BND.PORT |
        +----+-----+-------+------+----------+----------+
        | 1  |  1  | X'00' |  1   | Variable |    2     |
        +----+-----+-------+------+----------+----------+

     Where:

          o  VER    protocol version: X'05'
          o  REP    Reply field:
             o  X'00' succeeded
             o  X'01' general SOCKS server failure
             o  X'02' connection not allowed by ruleset
             o  X'03' Network unreachable
             o  X'04' Host unreachable
             o  X'05' Connection refused
             o  X'06' TTL expired
             o  X'07' Command not supported
             o  X'08' Address type not supported
             o  X'09' to X'FF' unassigned
          o  RSV    RESERVED
          o  ATYP   address type of following address
             o  IP V4 address: X'01'
             o  DOMAINNAME: X'03'
             o  IP V6 address: X'04'
          o  BND.ADDR       server bound address
          o  BND.PORT       server bound port in network octet order

   Fields marked RESERVED (RSV) must be set to X'00'.

*/

BOOL socks5_init(SOCKET sockfd)
{
    int rc;
    struct socks5_init_req req;
    struct socks5_init_resp resp;
    if(INVALID_SOCKET == sockfd)
    {
        printf("invalid socket\n");
        return FALSE;
    }

    memset(&req, 0, sizeof(req));
    memset(&resp, 0, sizeof(resp));

    req.version = 0x05;
    req.count = 0x01;
    req.methods[0] = 0x00;

    rc = send(sockfd, (char *)&req, 2 + req.count, 0);
    if(rc < 0) return FALSE;

    rc = recv(sockfd, (char *)&resp, sizeof(resp), 0);
    if(rc != sizeof(resp)) return FALSE;

    if(resp.version != 0x05)
    {
        printf("unsupported version: %.2x\n", resp.version);
        return FALSE;
    }
    if(resp.method == 0xff)
    {
        printf("NO ACCEPTABLE METHODS.\n");
        return FALSE;
    }

    if(resp.method != 0x00)
    {
        printf("unsupported method: %.2x\n", resp.method);
        return FALSE;
    }

    return TRUE;
}

BOOL socks5_connect(SOCKET sockfd, const struct socks5_req *req, struct socks5_resp *resp)
{
    int cb;
    memset(resp, 0, sizeof(struct socks5_resp));

    cb = send(sockfd, (char *)req, 4, 0);
    if(cb != 4) return FALSE;

    switch(req->addr_type)
    {
    case 0x01: /* ipv4 */
        cb = send(sockfd, (char *)&req->dest_addr.ipv4, sizeof(struct in_addr),0);
        if(cb != sizeof(struct in_addr)) return FALSE;
        break;
    case 0x03: /* domain name */
        cb = send(sockfd, (char *)&req->dest_addr.cb_dns, req->dest_addr.cb_dns + 1, 0);
        if(cb != (req->dest_addr.cb_dns + 1)) return FALSE;
        break;
    case 0x04: /* ipv6 */
        cb = send(sockfd, (char *)&req->dest_addr.ipv6, sizeof(struct in6_addr), 0);
        if(cb != sizeof(struct in6_addr)) return FALSE;
        break;
    default:
        printf("unknown req address type: %.2x\n", req->addr_type);
        return FALSE;
    }

    cb = send(sockfd, (char *)&req->port, 2, 0);
    if(cb != 2) return FALSE;

    cb = recv(sockfd, (char *)resp, 4, 0);
    if(cb != 4) return FALSE;

    switch(resp->addr_type)
    {
    case 0x01: /* ipv4 */
        cb = recv(sockfd, (char *)&resp->bind_addr.ipv4, sizeof(struct in_addr),0);
        if(cb != sizeof(struct in_addr)) return FALSE;
        break;
    case 0x03: /* domain name */
        cb = recv(sockfd, (char *)&resp->bind_addr.cb_dns, resp->bind_addr.cb_dns + 1, 0);
        if(cb != (resp->bind_addr.cb_dns + 1)) return FALSE;
        break;
    case 0x04: /* ipv6 */
        cb = recv(sockfd, (char *)&resp->bind_addr.ipv6, sizeof(struct in6_addr), 0);
        if(cb != sizeof(struct in6_addr)) return FALSE;
        break;
    default:
        printf("unknown resp address type: %.2x\n", resp->addr_type);
        return FALSE;
    }

    cb = recv(sockfd, (char *)&resp->port, 2, 0);
    if(cb != 2) return FALSE;


    return TRUE;
}

BOOL socks5_connect_ipv4(SOCKET sockfd, const char * ipv4, unsigned short port)
{
    BOOL rc;
    struct in_addr addr;
    struct socks5_req req;
    struct socks5_resp resp;

    addr.S_un.S_addr = inet_addr(ipv4);

    memset(&req, 0, sizeof(req));
    memset(&resp, 0, sizeof(resp));

    if(!socks5_init(sockfd))
    {
        printf("socks5 init error.\n");
        return FALSE;
    }

    req.version = 0x05;
    req.cmd = 0x01;
    req.reserved = 0x00;
    req.addr_type = 0x01;

    memcpy(&req.dest_addr.ipv4, &addr, sizeof(struct in_addr));
    req.port = htons(port);

    rc = socks5_connect(sockfd, &req, &resp);
    if(!rc) return FALSE;

    if(resp.reply != 0x00)
    {
        printf("socks5 connect failed: %.2x", resp.reply);
        return FALSE;
    }

    return TRUE;
}

BOOL socks5_handshake(SOCKET sockfd, SOCKS5_ADDR_TYPE addr_type, const char * szAddr, size_t cbAddr, unsigned short port)
{
    BOOL rc;
    struct socks5_req req;
    struct socks5_resp resp;
    if(INVALID_SOCKET == sockfd) return FALSE;

    bzero(&req, sizeof(req));
    bzero(&resp, sizeof(resp));

    rc = socks5_init(sockfd);
    if(!rc) goto label_errexit;

    req.version = 0x05;
    req.cmd = 0x01;
    req.reserved = 0x00;
    req.addr_type = addr_type;

    switch(addr_type)
    {
    case SOCKS5_ADDR_TYPE_IPV4: // 0x01
        #ifdef WIN32
        req.dest_addr.ipv4.S_un.S_addr = inet_addr(szAddr);
        #else
        req.dest_addr.ipv4.S_addr = inet_addr(szAddr);
        #endif
        break;
    case SOCKS5_ADDR_TYPE_DOMAIN_NAME: // 0x03
        if(cbAddr == 0) cbAddr = strlen(szAddr);
        if(cbAddr == 0 || cbAddr > MAX_SOCKS5_NAME_STRING_LEN)
        {
            printf("domain name length should be less than 256 bytes.\n");
            goto label_errexit;
        }
        memcpy(req.dest_addr.dns, szAddr, cbAddr);
        req.dest_addr.cb_dns = cbAddr;
        break;
    case SOCKS5_ADDR_TYPE_IPV6: // 0x04
        #if defined(WIN32) && !defined(__WIN32_INET_SUPPORT_IPV6)
        printf("This version does not support ipv6.\n");
        goto label_errexit;
        #else
        rc = inet_pton(AF_INET6, szAddr, &req.dest_addr.ipv6);
        if(!rc) return FALSE;
        break;
        #endif // __WIN32_INET_SUPPORT_IPV6
    default:
        printf("unknown address type.\n");
        goto label_errexit;
    }

    req.port = htons(port);
    rc = socks5_connect(sockfd, &req, &resp);
    if(!rc) return FALSE;\

    if(resp.reply != 0x00)
    {
        printf("socks5 connect failed: %.2x", resp.reply);
        goto label_errexit;
    }
    return TRUE;
label_errexit:
    return FALSE;
}
