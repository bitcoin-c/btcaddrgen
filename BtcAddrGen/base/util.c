// util.c
// Copyright (C) 2014  chehw
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

#include "util.h"


static const char ch_util_hex[]="0123456789abcdef";
static const char ch_util_HEX[]="0123456789ABCDEF";
static const signed char ch_util_hexdigit[256] =
{ -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  0,1,2,3,4,5,6,7,8,9,-1,-1,-1,-1,-1,-1,
  -1,0xa,0xb,0xc,0xd,0xe,0xf,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,0xa,0xb,0xc,0xd,0xe,0xf,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 };




static const char* pszBase58 = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
static const int8_t b58digits[] = {
    -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
    -1, 0, 1, 2, 3, 4, 5, 6,  7, 8,-1,-1,-1,-1,-1,-1,
    -1, 9,10,11,12,13,14,15, 16,-1,17,18,19,20,21,-1,
    22,23,24,25,26,27,28,29, 30,31,32,-1,-1,-1,-1,-1,
    -1,33,34,35,36,37,38,39, 40,41,42,43,-1,44,45,46,
    47,48,49,50,51,52,53,54, 55,56,57,-1,-1,-1,-1,-1,
};

static const char pszBase64[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                                'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                'w', 'x', 'y', 'z', '0', '1', '2', '3',
                                '4', '5', '6', '7', '8', '9', '+', '/'};
static const unsigned b64Digits[256] = {
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,    // + , /
52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,    // 0-9
-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,                   // A-Z
-1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
};

void Lock()
{

}
void Unlock()
{

}

void ReverseBytes(unsigned char *begin, size_t size)
{
    assert(begin != NULL && size > 0);
    unsigned char c;
    unsigned char * end = begin + size - 1;
    while(begin < end)
    {
        c = *begin;
        *(begin++) = *end;
        *(end--) = c;
    }
}

void ReverseData(void * begin, uint32_t data_size, uint32_t count)
{
    assert(NULL != begin && data_size != 0 && count != 0);
    unsigned char * p = (unsigned char *)begin;
    unsigned char * pend = p + data_size * (count - 1);
    unsigned char * temp;

    temp = (unsigned char *)malloc(data_size);
    if(NULL == temp) return;
    while(p < pend)
    {
        memcpy(temp, p, data_size);
        memcpy(p, pend, data_size);
        memcpy(pend, temp, data_size);
        p += data_size;
        pend -= data_size;
    }
    free(temp);
}

size_t Hex2Bin(const char *begin, size_t size, unsigned char *to)
{
    size_t cb;
    char * end = (char *)begin + size;
    unsigned char * p = to;
    unsigned char c1, c2;

    if(NULL == begin || 0 == size) return 0;
    if(size % 2) return 0;

    cb = size/2;
    if(NULL == to) return cb;

    while(begin < end)
    {
        c1=ch_util_hexdigit[(int)(*begin++)];
		c2=ch_util_hexdigit[(int)(*begin++)];
		if(c1==-1 || c2==-1) return 0; // not a hex string
		*p++ =(c1 << 4)|c2;
    }
    return cb;
}

size_t Bin2Hex(const unsigned char * begin, size_t size, char *to, uint32_t fLowercase)
{
    unsigned char *end=(unsigned char *)begin+size;
	char *p;
	unsigned char c;
	size_t cb= size *2;
	const char *szHex=fLowercase?ch_util_hex:ch_util_HEX;
	if(NULL == to) return cb+1;

	p = to;

	while(begin < end)
	{
		c=*begin++;
		*p++ =szHex[(c>>4) & 0x0f];
		*p++ =szHex[c&0x0f];
	}
	*p=0;
	return cb;
}

void Dump(const void * begin, size_t size)
{
    if(NULL == begin || 0 == size) return;
    size_t cb = size * 2 + 1;
    char * to = (char *)malloc(cb);
    Bin2Hex((unsigned char *)begin, size, to, 1);
    to[cb-1] = '\n';
    fwrite(to, 1, cb, stdout);
    free(to);
}

void Dump2(const void * begin, uint32_t size, FILE * fp)
{
    if(NULL == begin || 0 == size) return;
    size_t cb = size * 2 + 1;
    char buffer[1024 + 1] = "";

    char * to = buffer;


    if(cb > 1024)
    {
        to = (char *)malloc(cb);
        if(NULL == to) return;
    }
    Bin2Hex((unsigned char *)begin, size, to, 1);
    to[cb-1] = '\n';
    fwrite(to, 1, cb, fp);
    if(to != buffer) free(to);
}


size_t HexDump(const unsigned char *data, size_t size, int fh, uint32_t nDisplayPos)
{
	unsigned char * begin=(unsigned char *)data;
	unsigned char * end= begin + size;
	int cbLine=16;
	int i;

	uint32_t nPos=nDisplayPos;
	size_t cb;
	char szLine[200]="";
	char *p;
	char szText[32];
	FILE * fp = (FILE *)fh;

	if(data == NULL || size == 0) return 0;


	while(begin<end)
	{
		p=szLine;
		cb=sprintf(p, "%.8x", nPos);

		p+=cb;
		*(p++)=' ';
		*(p++)=' ';
		if((begin+cbLine)>end) cbLine=end-begin;
		for(i=0;i<cbLine;i++)
		{
			p+=sprintf(p, "%.2x ", begin[i]);
			szText[i]=(isalnum(begin[i])|| begin[i]==0x20)?(char)begin[i]:'_';
		}
		szText[i]='\n';
		szText[i+1]=0;
		for(;i<16;i++)
		{
			*p++=' ';
			*p++=' ';
			*p++=' ';

		//	szText[i]=' ';
		}


		*(p++)=' ';
		strcpy(p, szText);
		p+=strlen(szText);
		fwrite(szLine, p-szLine, 1, fp);
		nPos+=cbLine;

		begin+=cbLine;
	}
	return begin-data;

}

size_t Hash256(const unsigned char * begin, size_t size, unsigned char to[])
{
    unsigned char  hash[HASH256_SIZE];
    if(NULL == begin || size == 0) return 0;
    SHA256(begin, size, (unsigned char *)&hash[0]);
    SHA256(&hash[0], sizeof hash, (unsigned char *)&to[0]);

    return HASH256_SIZE;
}


size_t Hash160(const unsigned char * begin, size_t size, unsigned char to[])
{
    unsigned char  hash[HASH256_SIZE];
    if(NULL == begin || size == 0) return 0;
    SHA256(begin, size, &hash[0]);
    RIPEMD160(&hash[0], sizeof hash, &to[0]);
    return HASH160_SIZE;
}


size_t Base58Encode(const unsigned char *begin, size_t size, char *to)
{
    size_t cb = 0;
	BN_CTX * ctx = NULL;

    unsigned char c;
    unsigned char *pzero = (unsigned char *)begin;
	unsigned char *pend = (unsigned char *)(begin + size);

	char *p = to;

	BIGNUM *bn = NULL, *dv = NULL, *rem = NULL, *bn58 = NULL, *bn0 = NULL;
	if((NULL == begin) || (size == 0)) return 0; // invalid parameter

	cb = size * 138 /100+1;	// sizeof output  less than (138/100 * sizeof(src))

	//** output buffer should be allocated enough memory
	if(NULL == to) return cb;


	ctx = BN_CTX_new();
	if(NULL==ctx) return 0;

	bn58 = BN_new();
	bn0 = BN_new();
	bn = BN_new();
	dv = BN_new();
	rem = BN_new();
	if(NULL == bn58 || NULL == bn0 || NULL == bn || NULL == dv || NULL == rem) goto label_exit;

//    BN_init(&bn58); BN_init(&bn0);
	BN_set_word(bn58, 58);
	BN_zero(bn0);
//	BN_init(&bn); BN_init(&dv); BN_init(&rem);

	BN_bin2bn(begin, size, bn);

	while(BN_cmp(bn, bn0) > 0)
	{
		if(!BN_div(dv, rem, bn, bn58, ctx)) break;
		BN_copy(bn, dv);
		c = BN_get_word(rem);
		*(p++) = pszBase58[c];
	}


	while(*(pzero++)==0)
	{
		*(p++) = pszBase58[0];
		if(pzero > pend) break;
	}
	*p = '\0';
	cb = p - to;

	ReverseBytes((unsigned char *)to, cb);
label_exit:
    if(NULL != ctx) BN_CTX_free(ctx);

	if(NULL!= bn) BN_clear_free(bn);
	if(NULL!= dv) BN_clear_free(dv);
	if(NULL!= rem) BN_clear_free(rem);
	if(NULL!= bn58) BN_clear_free(bn58);
	if(NULL!= bn0) BN_clear_free(bn0);

	return cb;
}

size_t Base58Decode(const char *begin, size_t size, unsigned char *to)
{
	unsigned char c;
	unsigned char *p = (unsigned char *)begin;
	unsigned char *pend = p + size;
	size_t cb;
	BIGNUM bn, bnchar;
	BIGNUM bn58, bn0;

	cb = size;
	if(NULL == to) return cb;


	BN_CTX *ctx = BN_CTX_new();
	if(NULL == ctx) return 0;

    BN_init(&bn58);
    BN_init(&bn0);
    BN_init(&bn); BN_init(&bnchar);

	BN_set_word(&bn58, 58);
	BN_zero(&bn0);


	while(p < pend)
	{
		c = *p;
		if(c & 0x80) goto label_errexit;
		if(-1 == b58digits[c]) goto label_errexit;
		BN_set_word(&bnchar, b58digits[c]);
		if(!BN_mul(&bn, &bn, &bn58, ctx)) goto label_errexit;

		BN_add(&bn, &bn, &bnchar);
		p++;
	}

	cb = BN_num_bytes(&bn);


	BN_bn2bin(&bn, to);
//	ReverseBytes(to, cb);

	BN_CTX_free(ctx);
	BN_clear_free(&bn58);
    BN_clear_free(&bn0);
    BN_clear_free(&bn); BN_clear_free(&bnchar);

	return cb;

label_errexit:
	if(NULL != ctx) BN_CTX_free(ctx);
	return 0;
}

//** Common Tools

uint64_t GetTimeMicro()
{
    uint64_t time = 0;
#ifdef WIN32
	SYSTEMTIME st0, st1;
	union
	{
	    FILETIME ft;
	    uint64_t t;
	}t0,t1;



	memset(&st0, 0, sizeof(st0));
	memset(&st1, 0, sizeof(st1));
	memset(&t0, 0, sizeof(t0));
	memset(&t1, 0, sizeof(t1));


	st0.wYear=1970; st0.wMonth=1; st0.wDay=1;
	SystemTimeToFileTime(&st0, &t0.ft);

	GetSystemTime(&st1);
	SystemTimeToFileTime(&st1, &t1.ft);

	//** assume current system is little endian
	time= (t1.t - t0.t) / 10;
#else
	struct timeval t;
    gettimeofday(&t, NULL);
    time = (int64_t) t.tv_sec * 1000000 + t.tv_usec;

#endif // WIN32
	return time;

}

uint64_t GetTime()
{
    uint64_t time = 0;
#ifdef WIN32
	SYSTEMTIME st0, st1;
	union
	{
	    FILETIME ft;
	    uint64_t t;
	}t0,t1;



	memset(&st0, 0, sizeof(st0));
	memset(&st1, 0, sizeof(st1));
	memset(&t0, 0, sizeof(t0));
	memset(&t1, 0, sizeof(t1));


	st0.wYear=1970; st0.wMonth=1; st0.wDay=1;
	SystemTimeToFileTime(&st0, &t0.ft);

	GetSystemTime(&st1);
	SystemTimeToFileTime(&st1, &t1.ft);

	//** assume current system is little endian
	time= (t1.t - t0.t) / 10000000;
#else
	struct timeval t;
    gettimeofday(&t, NULL);
    time = (int64_t) t.tv_sec;

#endif // WIN32
	return time;
}

#ifdef WIN32
BOOL UnixtimeToSystemTime(uint64_t timestamp, SYSTEMTIME * st)
{
    SYSTEMTIME st0;
	union
	{
	    FILETIME ft;
	    uint64_t t;
	}t0;
	memset(&st0, 0, sizeof(st0));
	memset(st, 0, sizeof(SYSTEMTIME));
	memset(&t0, 0, sizeof(t0));


	st0.wYear=1970; st0.wMonth=1; st0.wDay=1;
	SystemTimeToFileTime(&st0, &t0.ft);

	t0.t += timestamp * 10000000;
	FileTimeToSystemTime(&t0.ft, st);
	return TRUE;

}
#else
BOOL UnixtimeToSystemTime(uint64_t timestamp, SYSTEMTIME * st)
{
    struct tm * tm;
    BOOL rc = FALSE;
    if(NULL == st) return FALSE;

    tm = gmtime((time_t *)&timestamp);

    if(NULL != tm)
    {
        bzero(st, sizeof(SYSTEMTIME));
        st->wYear = tm->tm_year + 1900;
        st->wMonth = tm->tm_mon + 1;
        st->wDay = tm->tm_mday;
        st->wHour = tm->tm_hour;
        st->wMinute = tm->tm_min;
        st->wSecond = tm->tm_sec;
        rc = TRUE;
    }
    Unlock();
    return rc;
}


BOOL UnixtimeToGmtTime(uint64_t timestamp, struct tm *p_gmt)
{
    struct tm * tm;
    BOOL rc = FALSE;
    if(NULL == p_gmt) return FALSE;
    Lock();
    tm = gmtime((time_t *)&timestamp);
    if(NULL != tm)
    {
        memcpy(p_gmt, tm, sizeof(struct tm));
        p_gmt->tm_year += 1900;
        p_gmt->tm_mon += 1;
        rc = TRUE;
    }
    Unlock();

    return rc;
}

#endif


int64_t GetPerformanceCounter()
{
	int64_t nCounter=0;
#ifdef WIN32
    QueryPerformanceCounter((LARGE_INTEGER*)&nCounter);
#else
    struct timeval t;
    gettimeofday(&t, NULL);
    nCounter = (int64_t) t.tv_sec * 1000000 + t.tv_usec;
#endif
    return nCounter;
}

void RandAndSeed()
{
    static int64_t nLastPerfom;
	int64_t nCurPerfom;

	// Seed with CPU performance counter
	int64_t nCounter=GetPerformanceCounter();
	RAND_add(&nCounter, sizeof(nCounter), 1.5);

	// This can take up to 2 seconds, so only do it every 10 minutes
	nCurPerfom=(int64_t)GetTime()/1000000;
	if(nCurPerfom < nLastPerfom + 10*60) return;

	nLastPerfom=nCurPerfom;

//	return;

#ifdef WIN32
    // Don't need this on Linux, OpenSSL automatically uses /dev/urandom
    // Seed with the entire set of perfmon data
    unsigned char pdata[250000];
    memset(pdata, 0, sizeof(pdata));
    unsigned long nSize = sizeof(pdata);
    long ret = RegQueryValueExA(HKEY_PERFORMANCE_DATA, "Global", NULL, NULL, pdata, &nSize);
    RegCloseKey(HKEY_PERFORMANCE_DATA);
    if (ret == ERROR_SUCCESS)
    {
        RAND_add(pdata, nSize, nSize/100.0);
        OPENSSL_cleanse(pdata, nSize);
    }
#endif
}


//************************************
//** ECKEY
EC_KEY * ECKey_new()
{
    return EC_KEY_new_by_curve_name(NID_secp256k1);
}

BOOL ECKey_Free(EC_KEY * pkey)
{
    if(NULL != pkey) EC_KEY_free(pkey);
    return TRUE;
}

BOOL ECKey_Check(const unsigned char vch[HASH256_SIZE])
{
    static const unsigned char order[32] = {
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,
        0xBA,0xAE,0xDC,0xE6,0xAF,0x48,0xA0,0x3B,
        0xBF,0xD2,0x5E,0x8C,0xD0,0x36,0x41,0x40
	};
    int rc = FALSE;
	BIGNUM bn, bnOrder, bn0;
	BN_init(&bn);
	BN_init(&bnOrder);
	BN_init(&bn0);

	BN_zero(&bn0);
	BN_bin2bn(vch, HASH256_SIZE, &bn);
	BN_bin2bn(order, HASH256_SIZE, &bnOrder);

	if(BN_is_zero(&bn)) goto label_exit;
	if(BN_cmp(&bn, &bnOrder) > 0) goto label_exit;

	rc = TRUE;
label_exit:
    BN_clear_free(&bn);
    BN_clear_free(&bnOrder);
    BN_clear_free(&bn0);
    return rc;
}


uint32_t ECKey_GenKeypair(EC_KEY * pkey, unsigned char vch[HASH256_SIZE])
{
    const uint32_t key_size = HASH256_SIZE;
    const EC_GROUP * group;
    BIGNUM bn;
    BIGNUM * privkey = &bn;
    BN_CTX * ctx = NULL;
    EC_POINT * pubkey = NULL;


    if(NULL == pkey) return 0;
    BN_init(&bn);
    group = EC_KEY_get0_group(pkey);
    pubkey = EC_POINT_new(group);
    ctx = BN_CTX_new();
    if(NULL == pubkey || NULL == ctx) goto label_errexit;

    if(BN_bin2bn(vch, key_size, &bn))
    {
        if(EC_POINT_mul(group, pubkey, privkey, NULL, NULL, ctx))
        {
            EC_KEY_set_private_key(pkey, privkey);
            EC_KEY_set_public_key(pkey, pubkey);
        }
        BN_clear_free(&bn);
    }

    BN_clear_free(&bn);
    EC_POINT_free(pubkey);
    BN_CTX_free(ctx);
    return key_size;

label_errexit:
    BN_clear(&bn);
    if(NULL!=pubkey) EC_POINT_free(pubkey);
    if(NULL != ctx) BN_CTX_free(ctx);
    return 0;
}


uint32_t ECKey_GeneratePrivKey(EC_KEY * pkey, unsigned char vch[HASH256_SIZE])
{
//    RandAndSeed();
    do
    {
        RAND_bytes(vch, HASH256_SIZE);
    }while(!ECKey_Check(vch));
    if(pkey) return  ECKey_GenKeypair(pkey, vch);
    return HASH256_SIZE;
}

uint32_t ECKey_GetPubkey(EC_KEY * pkey, unsigned char * pubkey, BOOL fCompressed)
{
    unsigned char * p_pubkey = pubkey;
    uint32_t cb;

    EC_KEY_set_conv_form(pkey, fCompressed?POINT_CONVERSION_COMPRESSED:POINT_CONVERSION_UNCOMPRESSED);

    cb = i2o_ECPublicKey(pkey, NULL);
    if(0==cb || cb>65) return 0;
    if(NULL == pubkey) return cb;

    cb = i2o_ECPublicKey(pkey, &p_pubkey);
    return cb;
}

BOOL ECKey_SetPubkey(EC_KEY *pkey, const unsigned char *pubkey, size_t size)
{
    const unsigned char * p = pubkey;
    return (BOOL)o2i_ECPublicKey(&pkey, &p, size);
}


size_t ECKey_Sign(EC_KEY *pkey, const unsigned char hash[HASH256_SIZE], unsigned char **to)
{
    //** if (*to) == NULL, the caller should use OPENSSL_free(*to) to free the memory
    size_t cb = 0;
    ECDSA_SIG *sig = NULL;
    BN_CTX * ctx = NULL;

    BIGNUM order; // The order of G
    BIGNUM halforder; // get sign/unsign mark

    unsigned char *output = NULL;

    const EC_GROUP * group = EC_KEY_get0_group(pkey);   // secp256k1: G
    if(NULL == group) return 0;

    sig = ECDSA_do_sign((unsigned char *)&hash[0], HASH256_SIZE, pkey);
    if(NULL == sig) return 0;

    //** sig = (r,s) = (r,-s)
    //** s must less than order/2, otherwise, some app may parse '-s' as a large unsigned positive integer
    ctx = BN_CTX_new();
    if(NULL == ctx) goto label_exit;


    //** allocate memory for bignum
    BN_init(&order);
    BN_init(&halforder);

    // get the order of G
    EC_GROUP_get_order(group, &order, ctx); // secp256k1: n
    BN_rshift1(&halforder, &order);

    if(BN_cmp(sig->s, &halforder)>0)
    {
        // if s > order/2, then output -s. (-s = (order - s))
        BN_sub(sig->s, &order, sig->s);
    }

    BN_CTX_free(ctx);

    output = *to;
    cb = ECDSA_size(pkey);
    if(NULL == output)
    {
        output = (unsigned char *)OPENSSL_malloc(cb);
        if(NULL == output) goto label_exit;
    }

    if(NULL == *to) *to = output;

    //** i2d_ECDSA_SIG DER encode content of ECDSA_SIG object
	//** (note: this function modifies *pp (*pp += length of the DER encoded signature)).
	//** do not pass the address of 'to' directly
    cb = i2d_ECDSA_SIG(sig, &output);


label_exit:
    BN_clear_free(&order);
    BN_clear_free(&halforder);
    ECDSA_SIG_free(sig);
    return cb;
}

BOOL ECKey_Verify(EC_KEY *pkey, const unsigned char * dgst, int32_t dgst_len, const unsigned char *sig, size_t sig_len)
{
    if(ECDSA_verify(0, dgst, dgst_len, sig, sig_len, pkey) != 1) // -1 = error, 0 = bad sig, 1 = good
        return FALSE;


    return TRUE;
}


//**************************************************
//** bitcoin addr
uint32_t PubkeyToAddr(const unsigned char * pubkey, size_t size, char *to)
{
    struct
    {
        unsigned char version;
        unsigned char vch[20];
        unsigned char checksum[4];
    }ext_pubkey;
    unsigned char hash[32]={0};
//    printf("ext_pubkey size: %d\n", sizeof(ext_pubkey));

    ext_pubkey.version = 0x00;
    Hash160(pubkey, size, &ext_pubkey.vch[0]);

    Hash256(&ext_pubkey.version, 1+20, hash);
    memcpy(ext_pubkey.checksum, hash, 4);

    return Base58Encode((unsigned char *)&ext_pubkey.version, sizeof(ext_pubkey), to);
}

uint32_t PrivkeyToWIF(const unsigned char vch[HASH256_SIZE], char *to, BOOL fCompressed)
{
    struct
    {
        unsigned char version;
        unsigned char vch[HASH256_SIZE];
        unsigned char checksum[5];
    }ext_privkey;

    unsigned char hash[HASH256_SIZE];
    uint32_t offset = fCompressed?1:0;

    ext_privkey.version = 0x80;
    memcpy(ext_privkey.vch, vch, 32);

    if(offset)   ext_privkey.checksum[0] = 0x01;

    Hash256(&ext_privkey.version, 1 + HASH256_SIZE +offset, hash);
    memcpy(&ext_privkey.checksum[offset], hash, 4 );

    return Base58Encode(&ext_privkey.version, 1 + HASH256_SIZE + offset + 4, to);



}

int RecvData(int sockfd, void * payload, int cbPayload)
{
    int cb;
    int cbTotal = cbPayload;
    char * p = (char *)payload;
    while(cbTotal > 0)
    {
        cb = recv(sockfd, p, cbTotal, 0);
        if(cb <= 0) break;
        if(cb == cbTotal) return cbPayload;
        p += cb;
        cbTotal -= cb;
    };
    return 0;
}

int SendData(int sockfd, const void * payload, int cbPayload)
{
    int cb;
    int cbTotal = cbPayload;
    char * p = (char *)payload;
    while(cbTotal > 0)
    {
        cb = send(sockfd, p, cbTotal, 0);
        if(cb <= 0) break;
        if(cb == cbTotal) return cbPayload;
        p += cb;
        cbTotal -= cb;
    };
    return 0;
}

//**********************
//** cryptologic

static const unsigned char chutil_default_iv[16] = {
            0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
            0x38, 0x39, 0x43, 0x48, 0x45, 0x48, 0x57, 0x00
            };
int AES256_encrypt(const void * plaintext, uint32_t cbPlaintext,
                   const unsigned char *password, uint32_t cbPassword,
                   const unsigned char * iv, unsigned char * ciphertext)
{
    EVP_CIPHER_CTX *ctx =  NULL;
    int cb;
    int cbCiphertext = 0;
    unsigned char key[32];
    const EVP_CIPHER * cipher_method = EVP_aes_256_cbc();
    const uint32_t IV_SIZE = 16;

    assert(NULL != plaintext);
    if(0 == cbPlaintext) return 0;

    // calculate the bytes needed for the output buffer
    if(NULL == ciphertext) return ((cbPlaintext / IV_SIZE) + 1) * IV_SIZE;

    if(iv == NULL) iv = chutil_default_iv;
    if(NULL == password || cbPassword < 8)
    {
        printf("password should equal or more than 8 characters.\n");
        goto label_errexit;
    }

    // generate a 256 bits key
    Hash256(password, cbPassword, &key[0]);

    if(NULL == (ctx = EVP_CIPHER_CTX_new())) goto label_errexit;

    if(1 != EVP_EncryptInit_ex(ctx, cipher_method, NULL, &key[0], iv)) goto label_errexit;
    if(1 != EVP_EncryptUpdate(ctx, ciphertext, &cb, plaintext, cbPlaintext)) goto label_errexit;

    cbCiphertext += cb;
    if(1 != EVP_EncryptFinal_ex(ctx, ciphertext + cb, &cb)) goto label_errexit;
    cbCiphertext += cb;

    EVP_CIPHER_CTX_free(ctx);

    return cbCiphertext;
label_errexit:
    if(NULL != ctx) EVP_CIPHER_CTX_free(ctx);
    printf("crypt error.\n");
    return 0;
}

int AES256_decrypt(const unsigned char *ciphertext, int cbCiphertext,
                   const unsigned char *password, uint32_t cbPassword,
                   const unsigned char *iv,
                   unsigned char *plaintext)
{
    EVP_CIPHER_CTX *ctx = NULL;
    int cb;
    int cbPlaintext = 0;
    unsigned char key[32];
    const EVP_CIPHER * cipher_method = EVP_aes_256_cbc();

    assert(NULL != ciphertext);
    if(0 == cbCiphertext) return 0;
    if(NULL == plaintext) return cbCiphertext;

    if(NULL == password || cbPassword < 8)
    {
        printf("password should equal or more than 8 characters.\n");
        goto label_errexit;
    }

    if(NULL == iv) iv = chutil_default_iv;
    Hash256(password, cbPassword, &key[0]);

    if(NULL == (ctx = EVP_CIPHER_CTX_new())) goto label_errexit;


    if(1 != EVP_DecryptInit_ex(ctx, cipher_method, NULL, &key[0], iv)) goto label_errexit;
    if(1 != EVP_DecryptUpdate(ctx, plaintext, &cb, ciphertext, cbCiphertext)) goto label_errexit;

    cbPlaintext += cb;
    if(1 != EVP_DecryptFinal_ex(ctx, plaintext + cb, &cb)) goto label_errexit;
    cbPlaintext += cb;
    EVP_CIPHER_CTX_free(ctx);
    return cbPlaintext;
label_errexit:

    if(NULL != ctx) EVP_CIPHER_CTX_free(ctx);
    printf("decrypt error\n");
    return 0;
}


//*****************
// ** base64
uint32_t Base64Encode(const unsigned char * from, uint32_t cbFrom, char * to)
{
    uint32_t cbTo = (cbFrom + 2) * 4 / 3;
    char * p = to;
    const unsigned char * begin, * end;

    if(NULL == from || cbFrom == 0) return 0;
    if(NULL == to) return cbTo + 1;

    begin = from;
    end = from + ((cbFrom / 3) * 3);

    while(begin < end)
    {
        *p++ = pszBase64[(begin[0] >> 2) & 0x3f];
        *p++ = pszBase64[((begin[0] & 0x03) << 4) | ((begin[1] >> 4) & 0x0f)];
        *p++ = pszBase64[((begin[1] & 0x0f) << 2) | ((begin[2] >> 6) & 0x03)];
        *p++ = pszBase64[(begin[2] & 0x3f)];
        begin += 3;
    }

    switch((from + cbFrom - end))
    {
    case 1:
        *p++ = pszBase64[(begin[0] >> 2) & 0x3f];
        *p++ = pszBase64[((begin[0] & 0x03) << 4) | 0];
        *p++ = '=';
        *p++ = '=';
        break;
    case 2:
        *p++ = pszBase64[(begin[0] >> 2) & 0x3f];
        *p++ = pszBase64[((begin[0] & 0x03) << 4) | ((begin[1] >> 4) & 0x0f)];
        *p++ = pszBase64[((begin[1] & 0x0f) << 2) | 0];
        *p++ = '=';
        break;
    default: break;
    }
    *p = '\0';
    return cbTo;
}

uint32_t Base64Decode(const char * from, uint32_t cbFrom, unsigned char * to)
{
    uint32_t cbTo = 0;
    uint32_t i;
    unsigned char c0, c1, c2, c3;
    unsigned char * p = to;
    const char * begin, * end;

    if(NULL == from || cbFrom == 0 || cbFrom % 4) return 0; // error format

    cbTo = cbFrom / 4 * 3;
    begin = from;

    while(cbFrom && from[cbFrom - 1] == '=')
    {
        cbTo--;
        cbFrom--;
    };

    end = begin + cbFrom / 4 * 4;
    if(NULL == to) return cbTo;

    while(begin < end)
    {
        c0 = b64Digits[(int)begin[0]];
        c1 = b64Digits[(int)begin[1]];
        c2 = b64Digits[(int)begin[2]];
        c3 = b64Digits[(int)begin[3]];
        if(c0 == -1 || c1 == -1 || c2 == -1 || c3 == -1)
        {
            fprintf(stderr, "base64 string format error at position %d.\n", i);
            return 0;
        }
        *p++ = ((c0 & 0x3f) << 2) | ((c1 >> 4) & 0x03);
        *p++ = ((c1 & 0x0f) << 4) | ((c2 >> 2) & 0x0f);
        *p++ = ((c2 & 0x03) << 6) | (c3 & 0x3f);
        begin += 4;
    }
    switch((from + cbFrom - end))
    {
    case 2:
        c0 = b64Digits[(int)begin[0]]; c1 = b64Digits[(int)begin[1]];
        *p++ = ((c0 & 0x3f) << 2) | ((c1 >> 4) & 0x03);
        break;
    case 3:
        c0 = b64Digits[(int)begin[0]]; c1 = b64Digits[(int)begin[1]]; c2 = b64Digits[(int)begin[2]];
        *p++ = ((c0 & 0x3f) << 2) | ((c1 >> 4) & 0x03);
        *p++ = ((c1 & 0x0f) << 4) | ((c2 >> 2) & 0x0f);
        break;
    default:
        break;
    }
    return cbTo;
}
