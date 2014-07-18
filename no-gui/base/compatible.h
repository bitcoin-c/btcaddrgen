#ifndef COMPATIBLE_H_INCLUDED
#define COMPATIBLE_H_INCLUDED

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include <assert.h>
#include <pthread.h>



#ifdef WIN32
#include <io.h>
#include <ws2tcpip.h>
#include <winsock2.h>
#include <mswsock.h>
#include <windows.h>
#include <commctrl.h>



#if _WIN32_WINNT >= 0x600

#if defined(_need_inet_pton)
#define __WIN32_INET_SUPPORT_IPV6
#define inet_pton InetPton
#define inet_ntop InetNtop
#endif /* _need_inet_pton : support for ipv6 */
#endif // _WIN32_WINNT

#define bool int8_t

#define sleep(sec) Sleep((sec)*1000)

#else
#include <stdbool.h>
#include <sys/socket.h> /* for socket(), bind(), connect(), recv() and send() */
#include <arpa/inet.h> /* for sockaddr_inand inet_ntoa() */
#include <unistd.h> /* for close() */
#include <netinet/ip.h>
#include <netinet/ip6.h>

#include <iconv.h>  // for convert from character sets

#define closesocket(s) close(s)

#ifndef SOCKET
#define SOCKET int
#endif // SOCKET

#ifndef INVALID_SOCKET
#define INVALID_SOCKET (-1)
#endif


#endif /* wiN32 */


#ifndef BOOL
#define BOOL int32_t
#endif /* BOOL */


#ifndef FALSE
#define FALSE (0)
#define TRUE (!FALSE)
#endif

#ifndef bzero
#define bzero(s, n) memset((s), 0, (n))
#endif

#ifndef CHUTIL_AllocMemory
#ifdef WIN32
#define CHUTIL_AllocMemory(type, count) (type *)LocalAlloc(LPTR, sizeof(type) * count)
#define CHUTIL_ReallocMemory(type, p, count) (type *)LocalReAlloc(p, sizeof(type) * count)
#define CHUTIL_FreeMemory(p) {if(NULL != (p)) {LocalFree((p)); p = NULL;}}
#else
#define CHUTIL_AllocMemory(type, count) (type *)calloc(size, 1)
#define CHUTIL_ReallocMemory(type, p, count) (type *)realloc(p, sizeof(type) * count)
#define CHUTIL_FreeMemory(p) {if(NULL != (p)) {free((p)); p = NULL;}}
#endif
#endif


//*
#define PRI_x "%x"
#define PRI_s "%s"
#define PRI_p "%p"
#define PRI_f "%f"
//*/
//*
#ifdef _WIN32
#define PRI_llu "%I64u"
#define PRI_lu "%lu"
#define PRI_u "%u"
#define PRI_hu "%u"
#define PRI_hhu "%hu"
#define PRI_lld "%I64d"
#define PRI_ld "%ld"
#define PRI_d "%d"
#define PRI_hd "%d"
#define PRI_hhd "%hd"
#else
#define PRI_llu "%llu"
#define PRI_lu "%u"
#define PRI_u "%u"
#define PRI_hu "%hu"
#define PRI_hhu "%hhu"
#define PRI_lld "%lld"
#define PRI_ld "%d"
#define PRI_d "%d"
#define PRI_hd "%hd"
#define PRI_hhd "%hhd"
#endif
//*/

#endif /* COMPATIBLE_H_INCLUDED */
