#ifndef SAMPLE_H_INCLUDED
#define SAMPLE_H_INCLUDED


#include "compatible.h"

#include <openssl/evp.h>
#include <openssl/conf.h>
#include <openssl/err.h>


#include <time.h>

//** openssl headers
#include <openssl/bn.h>
#include <openssl/ecdsa.h>
#include <openssl/sha.h>
#include <openssl/ripemd.h>
#include <openssl/rand.h>
#include <openssl/obj_mac.h>


#ifndef HASH256_SIZE
#define HASH256_SIZE (32)
#endif // HASH256_SIZE
#ifndef HASH160_SIZE
#define HASH160_SIZE (20)
#endif // HASH160_SIZE


#ifndef WIN32
typedef struct _SYSTEMTIME
{
    unsigned short  wYear;
    unsigned short  wMonth;
    unsigned short  wDayOfWeek;
    unsigned short  wDay;
    unsigned short  wHour;
    unsigned short  wMinute;
    unsigned short  wSecond;
    unsigned short  wMilliseconds;
}SYSTEMTIME;
#endif

void ReverseBytes(unsigned char *begin, size_t size);
void ReverseData(void * begin, uint32_t data_size, uint32_t count);
size_t Hex2Bin(const char *begin, size_t size, unsigned char *to);
size_t Bin2Hex(const unsigned char * begin, size_t size, char *to, uint32_t fLowercase);

size_t Hash256(const unsigned char * begin, size_t size, unsigned char to[32]);
size_t Hash160(const unsigned char * begin, size_t size, unsigned char to[20]);

size_t Base58Encode(const unsigned char *begin, size_t size, char *to);
size_t Base58Decode(const char *begin, size_t size, unsigned char *to);

void Dump(const void * begin, size_t size);
void Dump2(const void * begin, size_t size, FILE * fp);
size_t HexDump(const unsigned char *data, size_t size, int fp, uint32_t nDisplayPos);

uint64_t GetTimeMicro();
uint64_t GetTime();
BOOL UnixtimeToGmtTime(uint64_t time, struct tm * p_gmt);
BOOL UnixtimeToSystemTime(uint64_t time, SYSTEMTIME * st);
int64_t GetPerformanceCounter();
void RandAndSeed();

void Lock();
void Unlock();

int RecvData(int sockfd, void * payload, int cbPayload);
int SendData(int sockfd, const void * payload, int cbPayload);


uint32_t Base64Encode(const unsigned char * from, uint32_t cbFrom, char * to);
uint32_t Base64Decode(const char * from, uint32_t cbFrom, unsigned char * to);


//************************************
//** ECKEY
EC_KEY * ECKey_new();
BOOL ECKey_Free(EC_KEY * pkey);
BOOL ECKey_Check(const unsigned char vch[32]);
uint32_t ECKey_GenKeypair(EC_KEY * pkey, unsigned char vch[32]);
uint32_t ECKey_GeneratePrivKey(EC_KEY * pkey, unsigned char vch[32]);
uint32_t ECKey_GetPubkey(EC_KEY * pkey, unsigned char * pubkey, BOOL fCompressed);

BOOL ECKey_SetPubkey(EC_KEY *pkey, const unsigned char *pubkey, size_t size);
size_t ECKey_Sign(EC_KEY *pkey, const unsigned char hash[HASH256_SIZE], unsigned char **to);
BOOL ECKey_Verify(EC_KEY *pkey, const unsigned char * dgst, int32_t dgst_len, const unsigned char *sig, size_t sig_len);

uint32_t PubkeyToAddr(const unsigned char * pubkey, size_t size, char *to);
uint32_t PrivkeyToWIF(const unsigned char vch[32], char *to, BOOL fCompressed);


int AES256_encrypt(const void * plaintext, uint32_t cbPlaintext,
                   const unsigned char *password, uint32_t cbPassword,
                   const unsigned char * iv, unsigned char * ciphertext);

int AES256_decrypt(const unsigned char *ciphertext, int cbCiphertext,
                   const unsigned char *password, uint32_t cbPassword,
                   const unsigned char *iv,
                   unsigned char *plaintext);

#endif // SAMPLE_H_INCLUDED
