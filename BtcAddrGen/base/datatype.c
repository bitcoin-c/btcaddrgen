// datatype.c
// Copyright (C) 2014  chehw
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

#include "compatible.h"
#include "util.h"

#include "datatype.h"

#ifndef HASH256_SIZE
#define HASH256_SIZE (32)
#endif // HASH256_SIZE
#ifndef HASH160_SIZE
#define HASH160_SIZE (20)
#endif // HASH160_SIZE
//**************************************
//** UINT256

 const struct _compact_int satoshi_difficult_1 = {{0x1d00ffff}};


int UINT256_cmp(const struct _uint256 * a, const struct _uint256 * b)
{

    int i;
    if(a == NULL && b == NULL) return 0;
    if(a == NULL) return -1;
    if(b == NULL) return 1;

    for(i = 0; i < HASH256_SIZE; i++)
    {
        if(a->vch[i] > b->vch[i]) return 1;
        else if(a->vch[i] < b->vch[i]) return -1;
    }
    return 0;
}

//** little endian compare
int UINT256_cmp_le(const struct _uint256 * a, const struct _uint256 * b)
{
    unsigned char i;
    if(a == NULL && b == NULL) return 0;
    if(a == NULL) return -1;
    if(b == NULL) return 1;

    for(i = HASH256_SIZE ; i > 0; i--)
    {
        if(a->vch[i - 1] > b->vch[i - 1]) return 1;
        if(a->vch[i - 1] < b->vch[i - 1]) return -1;
    }
    return 0;
}


//**************************************
//** UINT160
int UINT160_cmp(const struct _uint160 * a, const struct _uint160 * b)
{

    int i;
    if(a == NULL && b == NULL) return 0;
    if(a == NULL) return -1;
    if(b == NULL) return 1;

    for(i = 0; i < HASH160_SIZE; i++)
    {
        if(a->vch[i] > b->vch[i]) return 1;
        else if(a->vch[i] < b->vch[i]) return -1;
    }
    return 0;
}

//*******************************************
//** COMPACT INT
int32_t COMPACT_INT_set(struct _compact_int * p_compactint, const struct _uint256 * from)
{
    assert(NULL != from && NULL != p_compactint);

    const unsigned char * vch = &from->vch[0];
    unsigned char nTailingZeros = 0;
    unsigned char nValidBytes = 0;
    unsigned char cb = 0;
    unsigned char i;

    p_compactint->value = 0;

    //** regard the uint256 integer as a little endian integer
    //** maybe the same way that satoshi did
    for(i = 0; i < HASH256_SIZE; i++)
    {
        if(vch[HASH256_SIZE - 1 - i] != 0) break;
    }
    nTailingZeros = i;
    nValidBytes = HASH256_SIZE - nTailingZeros;

    cb = nValidBytes > 3? 3 : nValidBytes;
    vch = &from->vch[nValidBytes - cb];

    p_compactint->exp = nValidBytes;
    if(cb == 3 && (vch[2] & 0x80))
    {
        cb--;
        vch++;
        p_compactint->exp++;
    }

    memcpy(&p_compactint->mantissa[0], vch, cb);
    return p_compactint->value;
}

uint32_t COMPACT_INT_get(const struct _compact_int * p_compactint, struct _uint256 * to)
{
    assert(NULL != p_compactint && NULL != to);
    unsigned char nValidBytes = 0;
    unsigned char cb = 0;


    bzero(to, sizeof(struct _uint256));
    nValidBytes = p_compactint->exp;
    if(nValidBytes > HASH256_SIZE) return 0; // error format

    cb = nValidBytes > 3? 3 : nValidBytes;
    if(cb == 3 && p_compactint->mantissa[2] == 0 && p_compactint->mantissa[1]&0x80)
    {
        nValidBytes++;
        if(nValidBytes > HASH256_SIZE) return 0;

    }
    memcpy(&to->vch[nValidBytes - cb -1], &p_compactint->mantissa[0], cb);

    return nValidBytes;
}

int COMPACT_INT_cmp(const struct _compact_int * a, const struct _compact_int * b)
{
    assert(NULL !=a && NULL != b);
    return (a->value - b->value);
}

int COMPACT_INT_cmp_hash(const struct _compact_int * a, const struct _uint256 * hash)
{
    assert(NULL !=a && NULL != hash);
    int32_t vb;
    struct _compact_int b;
    vb = COMPACT_INT_set(&b, hash);

    return (a->value - vb);
}

double COMPACT_INT_div(const struct _compact_int * a, const struct _compact_int *b)
{
    assert(NULL !=a && NULL != b);
    double dval = 0.0;
    int32_t exp = (a->exp - b->exp) * 8;
    int32_t ia = 0, ib = 0;

    memcpy(&ia, &a->mantissa[0], 3);
    memcpy(&ib, &b->mantissa[0], 3);

//    printf("ia = %d, ib = %d\n", ia, ib);


    dval = (double)ia/(double)ib;
//    printf("dval = %f, exp = %d\n", dval, exp);

    if(exp > 0) dval *= (double)(1<<exp);
    else dval /= (double)(1<<(-exp));

    return dval;
}

double COMPACT_INT_div_bshift(const struct _compact_int * a, const struct _compact_int *b, int32_t * bshift)
{

    assert(NULL !=a && NULL != b);
    double dval = 0.0;
    int32_t exp = a->exp - b->exp;
    int32_t ia = 0, ib = 0;

    memcpy(&ia, &a->mantissa[0], 3);
    memcpy(&ib, &b->mantissa[0], 3);

    dval = (double)ia/(double)ib;
    if(NULL != bshift) *bshift = exp;
    return dval;

}

int32_t COMPACT_INT_muldiv(struct _compact_int *a, uint32_t factor, uint64_t k)
{
    BIGNUM bn_a;
    BIGNUM bn_m;
    BIGNUM bn_k;
    BIGNUM r;
    struct _uint256 hash_a;
    uint32_t cb;
    int32_t value;
    BN_CTX *ctx = NULL;
    if(k == 0) return 0; // error: div 0
    BN_init(&bn_a);
    BN_init(&bn_m);
    BN_init(&bn_k);
    BN_init(&r);




    cb = COMPACT_INT_get(a, &hash_a);
    ReverseBytes(&hash_a.vch[0], 32);
 //   Dump(&hash_a.vch[0], 32);


    ReverseBytes((unsigned char *)&k, sizeof(uint64_t));

    ctx = BN_CTX_new();
    if(NULL == ctx) return 0;
    BN_bin2bn(&hash_a.vch[0], 32, &bn_a);

    BN_bin2bn((unsigned char *)&k, sizeof(uint64_t), &bn_k);
    BN_set_word(&bn_m, factor);

    BN_mul(&bn_a, &bn_a, &bn_m, ctx);
    BN_div(&bn_a, &r, &bn_a, &bn_k, ctx);

    bzero(&hash_a.vch[0], 32);

    cb = BN_bn2bin(&bn_a, &hash_a.vch[0]);
    Dump(&hash_a, 32);
    printf("cb = %d\n", cb);

    ReverseBytes(&hash_a.vch[0], cb);


    BN_CTX_free(ctx);

    value = COMPACT_INT_set(a, &hash_a);
    return value;

}

//****************************************************
//** VARINT

uint32_t VARINT_calc_size(uint64_t value)
{
    if(value < 0xfd) return 1;
    else if(value <0xffff) return 3;
    else if(value <0xffffffff) return 5;
    else return 9;
}

uint32_t VARINT_get_size(const struct _varint * p_varint)
{
    unsigned char c = p_varint->vch[0];
    if(c < 0xfd) return 1;

    switch(c)
    {
    case 0xfd: return 3;
    case 0xfe: return 5;
    case 0xff: return 9;
    }
    return 0;

}

struct _varint * VARINT_attach(const unsigned char * begin, const unsigned char *end)
{
    assert(NULL != begin && NULL != end);
    uint32_t cb = 0;

    cb = VARINT_get_size((const struct _varint *)begin);
    if(cb == 0 || cb >9) return NULL;
    if((begin + cb)>end) return NULL;

    return (struct _varint *)begin;
}
struct _varint *VARINT_new(uint32_t size)
{
    struct _varint * p_varint = NULL;
    if(size == 0 || size > 9) size = 9;
    p_varint = (struct _varint *)malloc(size);
    if(NULL == p_varint) return NULL;

    bzero(p_varint, size);
    return p_varint;
}

struct _varint * VARINT_load(struct _varint ** pp_varint, const unsigned char * begin, const unsigned char * end)
{
    assert(NULL != begin && NULL != end);
    uint32_t cb = 0;
    struct _varint * p_varint = NULL;
    cb = VARINT_get_size((const struct _varint *)begin);
    if(cb == 0 || cb >9) return NULL;
    if((begin + cb)>end) return NULL;

    if(NULL != pp_varint) p_varint = *pp_varint;
    if(NULL != p_varint)
    {
        if(VARINT_get_size(p_varint) < cb) p_varint = (struct _varint *)realloc(p_varint, cb);
        *pp_varint = p_varint;
    }else
    {
        p_varint = (struct _varint *)malloc(cb);
    }

    if(NULL == p_varint) return NULL;

    memcpy(&p_varint->vch[0], begin, cb);
    if(NULL != pp_varint) *pp_varint = p_varint;

    return p_varint;

}
void VARINT_free(struct _varint * p_varint)
{
    free(p_varint);
}

uint32_t VARINT_set_value(struct _varint * p_varint, uint64_t value)
{
    assert(NULL != p_varint);
    uint32_t size = 1;

    size = VARINT_calc_size(value);

    if(size == 1)
    {
        p_varint->vch[0] = (unsigned char)value;
        return 1;
    }
    switch(size)
    {
        case 3: p_varint->vch[0] = 0xfd; break;
        case 5: p_varint->vch[0] = 0xfe; break;
        case 9: p_varint->vch[0] = 0xff; break;
        default: return 0;
    }
    memcpy(&p_varint->vch[1], &value, size - 1);

    return size;
}

uint64_t VARINT_get_value(const struct _varint *p_varint)
{
    uint64_t value = 0;
    uint32_t size = 0;

    if(NULL == p_varint) return 0;
    size = VARINT_get_size(p_varint);
    if(size == 0 || size > 9) return 0;


    if(size == 1)
    {
        value = (uint64_t)(p_varint->vch[0]);
        return value;
    }
    switch(size)
    {
    case 3: value = *(uint16_t *)&p_varint->vch[1]; break;
    case 5: value = *(uint32_t *)&p_varint->vch[1]; break;
    case 7: value = *(uint64_t *)&p_varint->vch[1]; break;
    default: break;
    }

    return value;

}


//****************************************************
//** VARSTR
struct _varstr * VARSTR_new(const char * str, uint32_t cbStr)
{
    struct _varstr * p_varstr = NULL;
    uint32_t cbVarint = 0;


    if(NULL == str)
    {
        cbStr = 0;
    }
    else
    {
        if(cbStr == 0) cbStr = strlen(str);
    }

    cbVarint = VARINT_calc_size(cbStr);
    if(cbVarint == 0 || cbVarint > 9) return NULL;

    p_varstr = (struct _varstr *)malloc(cbVarint + cbStr);
    if(NULL == p_varstr) return NULL;

    bzero(p_varstr->vch, cbVarint);
    VARINT_set_value((struct _varint *)&p_varstr->vch[0], cbStr);
    if(cbStr) memcpy(&p_varstr->vch[cbVarint], str, cbStr);

    return p_varstr;
}

uint32_t VARSTR_get_size(const struct _varstr * p_varstr)
{
    uint32_t cbVarint = 0;
    uint32_t cbStr = 0;


    cbVarint = VARINT_get_size((const struct _varint *)p_varstr);
    if(cbVarint == 0 || cbVarint > 9) return 0; // error format

    cbStr = VARINT_get_value((struct _varint *)p_varstr);
    return (cbVarint + cbStr);
}

uint32_t VARSTR_strlen(const struct _varstr * p_varstr)
{
    uint32_t cbVarint = 0;
    uint32_t cbStr = 0;

    cbVarint = VARINT_get_size((const struct _varint *)&p_varstr->vch[0]);
    if(cbVarint == 0 || cbVarint > 9) return 0; // error format

    cbStr = VARINT_get_value((struct _varint *)&p_varstr->vch[0]);
    return cbStr;
}

struct _varstr * VARSTR_resize(struct _varstr ** pp_varstr, uint32_t cbStr)
{
    struct _varstr * p_varstr = NULL;
    uint32_t newsize =0;
    uint32_t cbVarint = 0;


    cbVarint = VARINT_calc_size(cbStr);
    if(cbVarint == 0 || cbVarint > 9) return NULL;   // error format
    newsize = cbVarint + cbStr;

    if(NULL != pp_varstr) p_varstr = *pp_varstr;

    if(NULL != p_varstr) free(p_varstr);


    p_varstr = (struct _varstr *)malloc(newsize);
    if(NULL != p_varstr)
    {
        VARINT_set_value((struct _varint *)&p_varstr->vch[0], cbStr);
    }
    if(NULL != pp_varstr)  *pp_varstr = p_varstr;
    return p_varstr;

}

uint32_t VARSTR_set_data(struct _varstr * p_varstr, const char * str, uint32_t cbStr)
{
    uint32_t cbVarint = 0;
    if(NULL == p_varstr) return 0;
    if(NULL == str)
    {
        cbStr = 0;
    }
    else
    {
        if(cbStr == 0) cbStr = strlen(str);
    }

    cbVarint = VARINT_calc_size(cbStr);
    if(cbVarint == 0 || cbVarint > 9) return 0;
    VARINT_set_value((struct _varint *)&p_varstr->vch[0], cbStr);
    if(cbStr) memcpy(&p_varstr->vch[cbVarint], str, cbStr);

    return (cbVarint + cbStr);
}

struct _varstr * VARSTR_copy(struct _varstr ** pp_varstr, const char * str, uint32_t cbStr)
{
    uint32_t cbVarint = 0;
    struct _varstr * p_varstr = NULL;


    if(NULL != pp_varstr) p_varstr = *pp_varstr;

    if(NULL == p_varstr)
    {
        p_varstr = VARSTR_new(str, cbStr);
        if(NULL != pp_varstr) *pp_varstr = p_varstr;
        return p_varstr;
    }

    p_varstr = VARSTR_resize(pp_varstr, cbStr);
    if(NULL == p_varstr) return NULL;

    if(cbStr > 0)
    {
        cbVarint = VARINT_get_size((struct _varint *)&p_varstr->vch[0]);
        memcpy(&p_varstr->vch[0] + cbVarint, str, cbStr);
    }

    return p_varstr;
}

const char * VARSTR_get_strptr(const struct _varstr * p_varstr)
{
    uint32_t cbVarint = 0;
    cbVarint = VARINT_get_size((const struct _varint *)&p_varstr->vch[0]);
    if(cbVarint == 0 || cbVarint >9) return NULL; // error format

    return (const char *)(&p_varstr->vch[0] + cbVarint);
}

uint32_t VARSTR_to_string(const struct _varstr * p_varstr, char * to, uint32_t cbMax)
{
    uint32_t cbVarint = 0;
    uint32_t cbStr = 0;
    cbVarint = VARINT_get_size((const struct _varint *)&p_varstr->vch[0]);
    if(cbVarint == 0 || cbVarint >9) return 0; // error format
    cbStr = (uint32_t)VARINT_get_value((const struct _varint *)&p_varstr->vch[0]);
    if(NULL == to) return cbStr + 1;
    if(cbStr >= cbMax) return 0;

    memcpy(to, &p_varstr->vch[cbVarint], cbStr);
    to[cbStr] = '\0';
    return cbStr;
}

void VARSTR_free(struct _varstr * p_varstr)
{
    if(NULL != p_varstr) free(p_varstr);
}

//*******************************
//** satoshi_block_header
void SATOSHI_BLOCK_HEADER_dump(const struct _satoshi_block_header * p_hdr)
{
    SYSTEMTIME st;
    struct _uint256 hash;
    assert(NULL != p_hdr);
    Hash256((unsigned char *)p_hdr, sizeof(struct _satoshi_block_header), &hash.vch[0]);
    printf("block: ");
    Dump(&hash, 32);
    printf("version: %u\n", p_hdr->version);
    printf("prev_block_hash: \n");
    Dump(&p_hdr->prev_block_hash, 32);

    printf("merkle root hash: \n");
    Dump(&p_hdr->merkle_root_hash, 32);

    UnixtimeToSystemTime(p_hdr->timestamp, &st);
    printf("timestamp: %.4hd-%.2hd-%.2hd %.2hd:%.2hd:%.2hd UTC\n",
        st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
    printf("bits: %.8x\n", p_hdr->bits.value);

    printf("nonce: %.8x\n", p_hdr->nonce);




}


//***********************************************************
//**
struct _satoshi_txin * SATOSHI_TXIN_new(const unsigned char * p_script, uint32_t cbScript)
{
    unsigned char size;
    unsigned char cbVarint;
    struct _satoshi_txin * p_txin = NULL;
    unsigned char cb;
    unsigned char * p;


    cbVarint = VARINT_calc_size(cbScript);
    if(cbVarint == 0 || cbVarint > 9) return NULL; // error format

    size = sizeof(struct _satoshi_outpoint) + cbVarint + cbScript + sizeof(uint32_t);
    p = (unsigned char *)malloc(size);
    if(NULL == p) return NULL;

    p_txin =(struct _satoshi_txin *)p;
    bzero(p, sizeof(struct _satoshi_outpoint));

    p += sizeof(struct _satoshi_outpoint);
    cb = VARSTR_set_data(&p_txin->scriptSig, (char *)p_script, cbScript);
    if(cb != (cbVarint + cbScript))
    {
        free(p_txin);
        return NULL;
    }

    p += cb;

    *(uint32_t *)p = 0xFFFFFFFF; // default sequence

    return p_txin;

}
uint32_t SATOSHI_TXIN_load(struct _satoshi_txin ** pp_txin, const unsigned char * begin, const unsigned char * end)
{
    assert(NULL != begin && NULL != end);
    unsigned char size = 0;
    unsigned char cb;
    struct _satoshi_txin * p_txin = NULL;

    const unsigned char * p;

    p = begin + sizeof(struct _satoshi_outpoint);
    if(p >= end) return 0;

    cb = VARSTR_get_size((struct _varstr *)p);
    if(cb == 0) return 0;

    size = sizeof(struct _satoshi_outpoint) + cb + sizeof(uint32_t);
    if((begin + size) > end) return 0;

    if(NULL == pp_txin) return size;
    p_txin = *pp_txin;

    if(NULL != p_txin)
    {
        if((cb = SATOSHI_TXIN_size(p_txin)) < size)
        {
            free(p_txin);
            p_txin = NULL;
        }else
        {
            bzero(p_txin + size, cb - size);
        }
    }
    if(NULL == p_txin)
    {
        p_txin = (struct _satoshi_txin *)malloc(size);
        if(NULL == p_txin) return 0;
    }

    memcpy(p_txin, begin, size);
    return size;
}

uint32_t SATOSHI_TXIN_attach(struct _satoshi_txin ** pp_txin, const unsigned char * begin, const unsigned char * end)
{
    assert(NULL != begin && NULL != end);
    unsigned char size = 0;
    unsigned char cb;
    const unsigned char * p;

    p = begin + sizeof(struct _satoshi_outpoint);
    if(p > end) return 0;

    cb = VARSTR_get_size((struct _varstr *)p);
    if(cb == 0) return 0;

    size = sizeof(struct _satoshi_outpoint) + cb + sizeof(uint32_t);
    if((begin + size) > end) return 0;

    if(NULL == pp_txin) return size;

    *pp_txin = (struct _satoshi_txin *)begin;
    return size;

}

void SATOSHI_TXIN_free(struct _satoshi_txin * p_txin)
{
    if(NULL != p_txin) free(p_txin);
}

uint32_t SATOSHI_TXIN_size(const struct _satoshi_txin * p_txin)
{
    assert(p_txin != NULL);
    uint32_t cb;
    cb = VARSTR_get_size(&p_txin->scriptSig);
    if(cb == 0) return 0;
    return (sizeof(struct _satoshi_outpoint) + cb + sizeof(uint32_t));
}

uint32_t SATOSHI_TXIN_script_length(const struct _satoshi_txin * p_txin)
{
    assert(p_txin != NULL);
    return VARSTR_strlen(&p_txin->scriptSig);
}
uint32_t SATOSHI_TXIN_get_sequence(const struct _satoshi_txin * p_txin)
{
    assert(p_txin != NULL);
    uint32_t sizeScript = VARSTR_get_size(&p_txin->scriptSig);
    unsigned char * p = (unsigned char *)&p_txin->scriptSig;
    if(sizeScript == 0) return 0;
    p += sizeScript;
    return *(uint32_t *)p;
}
uint32_t SATOSHI_TXIN_get_script(const struct _satoshi_txin * p_txin, unsigned char * to, uint32_t cbMax)
{
    assert(p_txin != NULL);
    uint32_t cbScript = VARSTR_strlen(&p_txin->scriptSig);
    if(cbScript == 0) return 0;
    if(NULL == to) return cbScript;
    if(cbScript < cbMax) return 0;

    memcpy(to, VARSTR_get_strptr(&p_txin->scriptSig), cbScript);
    return cbScript;

}

BOOL SATOSHI_TXIN_set_script(struct _satoshi_txin ** pp_txin, const unsigned char * p_scriptSig, uint32_t cbScriptSig)
{
    assert(NULL != pp_txin);
    uint32_t size = 0;
    uint32_t newsize = 0;
    uint32_t cbVarint;
    struct _satoshi_txin * p_txin = *pp_txin;
    unsigned char * p;
    uint32_t sequence = 0xFFFFFFFF;

    if(NULL == p_txin)
    {
        *pp_txin = SATOSHI_TXIN_new(p_scriptSig, cbScriptSig);
        return size;
    }

    size = SATOSHI_TXIN_size(p_txin);

    cbVarint = VARINT_calc_size(cbScriptSig);
    newsize = sizeof(struct _satoshi_outpoint) + cbVarint + cbScriptSig + sizeof(uint32_t);
    sequence = SATOSHI_TXIN_get_sequence(p_txin);

    if(size >= newsize)
    {

        VARSTR_set_data(&p_txin->scriptSig, (char *)p_scriptSig, cbScriptSig);
        SATOSHI_TXIN_set_sequence(p_txin, sequence);
        return newsize;
    }

    p = (unsigned char *)malloc(newsize);
    memcpy(p, p_txin, sizeof(struct _satoshi_outpoint));
    free(p_txin);
    p_txin = (struct _satoshi_txin *)p;
    VARSTR_set_data(&p_txin->scriptSig, (char *)p_scriptSig, cbScriptSig);
    SATOSHI_TXIN_set_sequence(p_txin, sequence);

    return newsize;
}
BOOL SATOSHI_TXIN_set_sequence(struct _satoshi_txin * p_txin, uint32_t sequence)
{
    assert(NULL != p_txin);
    uint32_t sizeScript = 0;
    unsigned char * p = (unsigned char *)&p_txin->scriptSig.vch[0];
    sizeScript = VARSTR_get_size(&p_txin->scriptSig);
    if(sizeScript == 0) return FALSE;

    p += sizeScript;

    *(uint32_t *)p = sequence;
    return TRUE;
}

void SATOSHI_TXIN_dump(const struct _satoshi_txin * p_txin)
{
    assert(NULL != p_txin);
    uint32_t cb;
    uint32_t sequence;
    uint32_t cbScript = VARSTR_strlen(&p_txin->scriptSig);
    unsigned char * p_script = (unsigned char *)VARSTR_get_strptr(&p_txin->scriptSig);
    unsigned char * p;
    unsigned char * pend = p_script + cbScript;

    uint32_t cbSig;
    unsigned char * sig;
    struct _asn_1_tlv * p_tlv;

    printf("== txin == \n");
    printf("outpoint: prev_hash\n");
    Dump(&p_txin->outpoint.outpoint_tx_hash, 32);
    printf("outpoint: index = %d\n", p_txin->outpoint.txout_index);

    printf("scriptSig: length = %u\n", cbScript);
    Dump(p_script, cbScript);

    p = p_script;

    cbSig = (uint32_t)VARINT_get_value((struct _varint *)p);
    p += VARINT_get_size((struct _varint *)p);
    printf("sig(%u): ", cbSig);
    Dump(p, cbSig);
    //** parse signature
    sig = p;
    p += cbSig;
    if(sig[0] == 0x30) // sequence
    {
        unsigned char len;
        sig++;
        len = *sig++;
        printf("signature length = %u", len);

        p_tlv = (struct _asn_1_tlv *)sig;
        printf("type: "PRI_hhu", length: "PRI_hhu"\nr = ", p_tlv->type, p_tlv->length);
        Dump(&p_tlv->value[0], p_tlv->length);

        p_tlv = (struct _asn_1_tlv *)(&p_tlv->value[0] + p_tlv->length);
        printf("type: "PRI_hhu", length: "PRI_hhu"\ns = ", p_tlv->type, p_tlv->length);
        Dump(&p_tlv->value[0], p_tlv->length);

        sig = (&p_tlv->value[0] + p_tlv->length);
    }
    if(sig < p)
    {
        printf("hash type: ");
        Dump(sig, p - sig);
    }



    if(p < pend)
    {

    cb = (uint32_t)VARINT_get_value((struct _varint *)p);
    printf("pubkey(%u)", cb);
    p += VARINT_get_size((struct _varint *)p);
    Dump(p, pend - p);
    }
    sequence = SATOSHI_TXIN_get_sequence(p_txin);
    printf("sequence: %.8x\n", sequence);
}

//*********************************************
//** tx_out

struct _satoshi_txout * SATOSHI_TXOUT_new(int64_t value, const unsigned char * pk_script, uint32_t cbScript)
{
    struct _satoshi_txout * p_txout;
    uint32_t size = 0;
    uint32_t cbVarint = VARINT_calc_size((uint64_t)cbScript);
    if(cbVarint == 0 || cbVarint > 9) return NULL;

    size = sizeof(int64_t) + cbVarint + cbScript;
    p_txout = (struct _satoshi_txout *)malloc(size);
    if(NULL != p_txout)
    {
        p_txout->value = value;
        VARSTR_set_data(&p_txout->pk_script, (char *)pk_script, cbScript);
    }
    return p_txout;

}
uint32_t SATOSHI_TXOUT_load(struct _satoshi_txout ** pp_txout, const unsigned char *begin, const unsigned char * end)
{
    assert(NULL != begin && NULL != end);
    uint32_t size = 0;
    uint32_t cbScript = 0;
    struct _satoshi_txout * p_txout;

    const unsigned char * p = begin;

    p += sizeof(int64_t);
    if(p > end) return 0;

    cbScript = VARSTR_get_size((struct _varstr *)p);
    if(cbScript < 1) return 0;

    size = sizeof(int64_t) + cbScript;
    if(NULL == pp_txout) return size;

    p_txout = *pp_txout;

    SATOSHI_TXOUT_free(p_txout);

    p_txout = SATOSHI_TXOUT_new(*(int64_t *)begin, p, size);
    *pp_txout = p_txout;

    if(NULL == p_txout) return 0;
    return size;

}
uint32_t SATOSHI_TXOUT_attach(struct _satoshi_txout ** pp_txout, const unsigned char *begin, const unsigned char *end)
{
    assert(NULL != begin && NULL != end);
    uint32_t size = 0;
    uint32_t sizeScript = 0;


    const unsigned char * p = begin;

    p += sizeof(int64_t);
    if(p > end) return 0;

    sizeScript = VARSTR_get_size((struct _varstr *)p);
    if(sizeScript < 1) return 0;

    size = sizeof(int64_t) + sizeScript;
    if(NULL != pp_txout) *pp_txout = (struct _satoshi_txout *)begin;
    return size;
}


void SATOSHI_TXOUT_free(struct _satoshi_txout * p_txout)
{
    if(NULL != p_txout) free(p_txout);
}

uint32_t SATOSHI_TXOUT_size(const struct _satoshi_txout * p_txout)
{
    assert(NULL != p_txout);
    uint32_t sizeScript = VARSTR_get_size(&p_txout->pk_script);
    if(sizeScript < 1) return 0;
    return sizeof(int64_t) + sizeScript;
}
uint32_t SATOSHI_TXOUT_script_length(const struct _satoshi_txout * p_txout)
{
    assert(NULL != p_txout);
    return VARSTR_strlen(&p_txout->pk_script);
}
uint32_t SATOSHI_TXOUT_get_script(const struct _satoshi_txout * p_txout, unsigned char * to, uint32_t cbMax)
{
    assert(NULL != p_txout);
    uint32_t cbScript = VARSTR_strlen(&p_txout->pk_script);
    if(NULL == to) return cbScript;

    if(cbScript > cbMax) return 0;

    memcpy(to, VARSTR_get_strptr(&p_txout->pk_script), cbScript);
    return cbScript;

}

void SATOSHI_TXOUT_dump(const struct _satoshi_txout * p_txout)
{
    assert(NULL != p_txout);
    char buffer[100] = "";
    uint32_t cb;
    printf("== [txout] ==\n");
    cb = Bin2Hex((unsigned char *)&p_txout->value, 8, buffer, 0);
    printf("value = "PRI_lld"("PRI_s")\n", p_txout->value, buffer);

    cb = VARSTR_strlen(&p_txout->pk_script);
    printf("pk_script: (length = "PRI_u")\n", cb);
    Dump(VARSTR_get_strptr(&p_txout->pk_script), cb);

}


//*********************************************
//** tx

struct _satoshi_tx * SATOSHI_TX_new(const unsigned char * begin, uint32_t cbPayload)
{
    assert(NULL != begin);
    struct _satoshi_tx * p_tx;
    if(cbPayload < (4 + 1 + sizeof(struct _satoshi_outpoint) + 1 + 1 + sizeof(int64_t) + 1)) return NULL;
    p_tx = (struct _satoshi_tx *)malloc(cbPayload);
    if(NULL != p_tx)
    {
        memcpy(p_tx, begin, cbPayload);
    }
    return p_tx;
}
uint32_t SATOSHI_TX_load(struct _satoshi_tx ** pp_tx, const unsigned char *begin, const unsigned char *end)
{
    assert(NULL != begin && NULL != end);
    uint32_t size = 0;
    uint32_t txin_count;
    uint32_t txout_count;
    uint32_t cbVarint;
    uint32_t c;
    uint32_t cb;
    struct _satoshi_tx *p_tx;

    const unsigned char * p = begin;
    p += sizeof(int32_t);
    cbVarint = VARINT_get_size((struct _varint *)p);
    if((cbVarint == 0) || (cbVarint > 9)) return 0;

    txin_count = (uint32_t)VARINT_get_value((struct _varint *)p);
    p += cbVarint;
    if(txin_count < 1) return 0;

    c = txin_count;
    while(c)
    {
        cb = SATOSHI_TXIN_size((struct _satoshi_txin *)p);
        if(cb == 0) return 0;
        p += cb;
        c--;
    }

    txout_count = (uint32_t)VARINT_get_value((struct _varint *)p);
    p += cbVarint;
    if(txout_count < 1) return 0;
    c = txout_count;
    while(c)
    {
        cb = SATOSHI_TXOUT_size((struct _satoshi_txout *)p);
        if(cb == 0) return 0;
        p += cb;
        c--;
    }
    p += sizeof(uint32_t);

    if(p > end) return 0;
    size = p - begin;

    if(NULL == pp_tx) return size;

    p_tx = *pp_tx;
    if(NULL != p_tx)
    {
        SATOSHI_TX_free(p_tx);
    }

    p_tx = SATOSHI_TX_new(begin, size);
    *pp_tx = p_tx;
    if(NULL == p_tx) return 0;

    return size;
}
uint32_t SATOSHI_TX_attach(struct _satoshi_tx ** pp_tx, const unsigned char * begin, const unsigned char * end)
{
    assert(NULL != pp_tx);
    uint32_t size = 0;
    uint32_t txin_count;
    uint32_t txout_count;
    uint32_t cbVarint;
    uint32_t c;
    uint32_t cb;

    const unsigned char * p = begin;
    p += sizeof(int32_t);
    cbVarint = VARINT_get_size((struct _varint *)p);
    if((cbVarint == 0) || (cbVarint > 9)) return 0;

    txin_count = (uint32_t)VARINT_get_value((struct _varint *)p);
    p += cbVarint;
    if(txin_count < 1) return 0;

    c = txin_count;
    while(c)
    {
        cb = SATOSHI_TXIN_size((struct _satoshi_txin *)p);
        if(cb == 0) return 0;
        p += cb;
        c--;
    }

    txout_count = (uint32_t)VARINT_get_value((struct _varint *)p);
    p += cbVarint;
    if(txout_count < 1) return 0;
    c = txout_count;
    while(c)
    {
        cb = SATOSHI_TXOUT_size((struct _satoshi_txout *)p);
        if(cb == 0) return 0;
        p += cb;
        c--;
    }
    p += sizeof(uint32_t);

    if(p > end) return 0;

    size = p - begin;
    *pp_tx = (struct _satoshi_tx *)begin;
    return size;
}

uint32_t SATOSHI_TX_size(const struct _satoshi_tx * p_tx)
{
    assert(NULL != p_tx);
    uint32_t size = 0;
    uint32_t txin_count;
    uint32_t txout_count;
    uint32_t cbVarint;
    uint32_t c;
    uint32_t cb;

    const unsigned char * p = (unsigned char *)p_tx;
    p += sizeof(int32_t);
    cbVarint = VARINT_get_size((struct _varint *)p);
    if((cbVarint == 0) || (cbVarint > 9)) return 0;

    txin_count = (uint32_t)VARINT_get_value((struct _varint *)p);
    p += cbVarint;
    if(txin_count < 1) return 0;

    for(c = 0; c < txin_count; c++)
    {
        cb = SATOSHI_TXIN_size((struct _satoshi_txin *)p);
        if(cb == 0) return 0;
        p += cb;
    }


    txout_count = (uint32_t)VARINT_get_value((struct _varint *)p);
    cbVarint = VARINT_get_size((struct _varint *)p);

    p += cbVarint;
    if(txout_count < 1) return 0;
    for(c = 0; c < txout_count; c++)
    {
        cb = SATOSHI_TXOUT_size((struct _satoshi_txout *)p);
        if(cb == 0) return 0;
        p += cb;
    }
    p += sizeof(uint32_t); // lock_count
    size = p - (unsigned char *)p_tx;
    return size;
}
void SATOSHI_TX_free(struct _satoshi_tx * p_tx)
{
    free(p_tx);
}

uint32_t SATOSHI_TX_get_locktime(const struct _satoshi_tx * p_tx)
{
    assert(NULL != p_tx);
    uint32_t lock_time = 0;
    uint32_t size = SATOSHI_TX_size(p_tx);
    unsigned char * p = (unsigned char *)p_tx;

    p += size - sizeof(uint32_t);
    lock_time = *(uint32_t *)p;
    return lock_time;
}
BOOL SATOSHI_TX_set_locktime(const struct _satoshi_tx * p_tx, uint32_t lock_time)
{
    assert(NULL != p_tx);
    uint32_t size = SATOSHI_TX_size(p_tx);
    unsigned char * p = (unsigned char *)p_tx;

    p += size - sizeof(uint32_t);
    *(uint32_t *)p = lock_time;
    return TRUE;
}

uint32_t SATOSHI_TX_get_txin_list(const struct _satoshi_tx * p_tx, struct _satoshi_txin **pp_txin_list)
{
    assert(NULL != p_tx);

    uint32_t txin_count;
    uint32_t cbVarint;

    const unsigned char * p = (unsigned char *)p_tx;

    p += sizeof(int32_t);
    cbVarint = VARINT_get_size((struct _varint *)p);
    if((cbVarint == 0) || (cbVarint > 9)) return 0;

    txin_count = (uint32_t)VARINT_get_value((struct _varint *)p);
    p += cbVarint;
    if(txin_count < 1) return 0;

    if(NULL != pp_txin_list) *pp_txin_list = (struct _satoshi_txin *)p;
    return txin_count;
}
uint32_t SATOSHI_TX_get_txout_list(const struct _satoshi_tx * p_tx, struct _satoshi_txout ** pp_txout_list)
{
    assert(NULL != p_tx);

    uint32_t txin_count;
    uint32_t txout_count;
    uint32_t cbVarint;
    uint32_t c;
    uint32_t cb;
    const unsigned char * p = (unsigned char *)p_tx;
    p += sizeof(int32_t);
    cbVarint = VARINT_get_size((struct _varint *)p);
    if((cbVarint == 0) || (cbVarint > 9)) return 0;

    txin_count = (uint32_t)VARINT_get_value((struct _varint *)p);
    p += cbVarint;
    if(txin_count < 1) return 0;

    for(c = 0; c < txin_count; c++)
    {
        cb = SATOSHI_TXIN_size((struct _satoshi_txin *)p);
        if(cb == 0) return 0;
        printf("==== cb = %u\n", cb);
        p += cb;
    }

    cbVarint = VARINT_get_size((struct _varint *)p);
    if((cbVarint == 0) || (cbVarint > 9)) return 0;
    txout_count = (uint32_t)VARINT_get_value((struct _varint *)p);
    p += cbVarint;
    if(txout_count < 1) return 0;

    if(NULL != pp_txout_list) *pp_txout_list = (struct _satoshi_txout *)p;
    return txout_count;

}

void SATOSHI_TX_dump(const struct _satoshi_tx * p_tx)
{
    assert(NULL !=  p_tx);
    uint32_t txin_count = 0;
    uint32_t txout_count = 0;
    uint32_t c;
    struct _satoshi_txin * txin_list = NULL;
    struct _satoshi_txout * txout_list = NULL;
    uint32_t lock_time = 0;
    uint32_t cb;
    unsigned char * p;

    printf("==== [tx dump] ====\n");
    printf("version: %u\n", p_tx->version);

    txin_count = SATOSHI_TX_get_txin_list(p_tx, &txin_list);
    txout_count = SATOSHI_TX_get_txout_list(p_tx, &txout_list);

    printf("txin count: %u\n", txin_count);
    p = (unsigned char *)txin_list;
    for(c = 0; c < txin_count; c++)
    {
        cb = SATOSHI_TXIN_size((struct _satoshi_txin *)p);
        printf("==== txin (%.3u) ====", c + 1);
        SATOSHI_TXIN_dump((struct _satoshi_txin *)p);
        p += cb;
    }

    printf("txout count: %u\n", txout_count);
    p = (unsigned char *)txout_list;
    for(c = 0; c < txout_count; c++)
    {
        cb = SATOSHI_TXOUT_size((struct _satoshi_txout *)p);
        printf("==== txout (%.3u) ====", c + 1);
        SATOSHI_TXOUT_dump((struct _satoshi_txout *)p);
        p += cb;
    }

    lock_time = SATOSHI_TX_get_locktime(p_tx);
    printf("lock_time: %.8x\n", lock_time);


}
