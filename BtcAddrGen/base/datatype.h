#ifndef DATATYPE_H_INCLUDED
#define DATATYPE_H_INCLUDED

#include "compatible.h"



#pragma pack(push)
#pragma pack(1)
typedef struct _asn_1_tlv
{
    unsigned char type;
    unsigned char length;
    unsigned char value[0];
}asn_1_tlv;
#pragma pack(pop)


typedef struct _uint256
{
    unsigned char vch[32];
}uint256_t, HASH256;

int UINT256_cmp(const struct _uint256 * a, const struct _uint256 *b);
int UINT256_cmp_le(const struct _uint256 * a, const struct _uint256 * b);

typedef struct _uint160
{
    unsigned char vch[20];
}uint160_t, HASH160;
int UINT160_cmp(const struct _uint160 * a, const struct _uint160 *b);


#pragma pack(push)
#pragma pack(1)
typedef struct _compact_int
{
    union
    {
        int32_t value;
        struct
        {
            unsigned char mantissa[3];
            unsigned char exp;
        };
    };
}compact_int;
#pragma pack(pop)

int32_t COMPACT_INT_set(struct _compact_int * p_compactint, const struct _uint256 * from);
uint32_t COMPACT_INT_get(const struct _compact_int * p_compactint, struct _uint256 * to);
int COMPACT_INT_cmp(const struct _compact_int * a, const struct _compact_int * b);
int COMPACT_INT_cmp_hash(const struct _compact_int * p_compactint, const struct _uint256 * hash);
double COMPACT_INT_div(const struct _compact_int * a, const struct _compact_int *b);
double COMPACT_INT_div_bshift(const struct _compact_int * a, const struct _compact_int *b, int32_t * bshift);
int32_t COMPACT_INT_muldiv(struct _compact_int *a, uint32_t m, uint64_t k);

#pragma pack(push)
#pragma pack(1)
typedef struct _varint
{
    unsigned char vch[1];
}varint_t;
#pragma pack(pop)

uint32_t VARINT_calc_size(uint64_t value);
uint32_t VARINT_get_size(const struct _varint * p_varint);
struct _varint * VARINT_attach(const unsigned char * begin, const unsigned char *end);
struct _varint *VARINT_new(uint32_t size);
struct _varint * VARINT_load(struct _varint ** pp_varint, const unsigned char * begin, const unsigned char * end);
void VARINT_free(struct _varint *p_varint);
uint32_t VARINT_set_value(struct _varint * p_varint, uint64_t value);
uint64_t VARINT_get_value(const struct _varint *p_varint);



//*****************************************
//** VARSTR
#pragma pack(push)
#pragma pack(1)
typedef struct _varstr
{
    unsigned char vch[1];
}varstr_t;
#pragma pack(pop)

struct _varstr * VARSTR_new(const char * str, uint32_t cbStr);
uint32_t VARSTR_get_size(const struct _varstr * p_varstr);
uint32_t VARSTR_strlen(const struct _varstr * p_varstr);
uint32_t VARSTR_set_data(struct _varstr * p_varstr, const char * str, uint32_t cbStr);
struct _varstr * VARSTR_resize(struct _varstr ** pp_varstr, uint32_t cbStr);
struct _varstr * VARSTR_copy(struct _varstr ** pp_varstr, const char * str, uint32_t cbStr);
const char * VARSTR_get_strptr(const struct _varstr * p_varstr);
uint32_t VARSTR_to_string(const struct _varstr * p_varstr, char * to, uint32_t cbMax);
void VARSTR_free(struct _varstr * p_varstr);



typedef struct _satoshi_block_header
{
    uint32_t version;
    struct _uint256 prev_block_hash;
    struct _uint256 merkle_root_hash;
    uint32_t timestamp;
    struct _compact_int bits;
    uint32_t nonce;
}satoshi_block_header;
void SATOSHI_BLOCK_HEADER_dump(const struct _satoshi_block_header * p_hdr);

//*************
// ** tx

typedef struct _satoshi_outpoint
{
    struct _uint256 outpoint_tx_hash;
    int32_t txout_index;
}satoshi_outpoint;

#pragma pack(push)
#pragma pack(1)
typedef struct _satoshi_txin
{
    struct _satoshi_outpoint outpoint;
    struct _varstr scriptSig;
    // uint32_t sequence;p
}satoshi_txin;
#pragma pack(pop)

struct _satoshi_txin * SATOSHI_TXIN_new(const unsigned char * p_script, uint32_t cbScript);   // should free after used
uint32_t SATOSHI_TXIN_load(struct _satoshi_txin ** pp_txin, const unsigned char * begin, const unsigned char * end);   // should free after used
uint32_t SATOSHI_TXIN_attach(struct _satoshi_txin ** pp_txin, const unsigned char * begin, const unsigned char * end);
void SATOSHI_TXIN_free(struct _satoshi_txin * p_txin);
uint32_t SATOSHI_TXIN_size(const struct _satoshi_txin * p_txin);
uint32_t SATOSHI_TXIN_script_length(const struct _satoshi_txin * p_txin);
uint32_t SATOSHI_TXIN_get_sequence(const struct _satoshi_txin * p_txin);
uint32_t SATOSHI_TXIN_get_script(const struct _satoshi_txin * p_txin, unsigned char * to, uint32_t cbMax);

BOOL SATOSHI_TXIN_set_script(struct _satoshi_txin ** p_txin, const unsigned char * p_scriptSig, uint32_t cbScriptSig);
BOOL SATOSHI_TXIN_set_sequence(struct _satoshi_txin * p_txin, uint32_t sequence);
void SATOSHI_TXIN_dump(const struct _satoshi_txin * p_txin);


#pragma pack(push)
#pragma pack(1)
typedef struct _satoshi_txout
{
    int64_t value;
    struct _varstr pk_script;
}satoshi_txout;
#pragma pack(pop)

struct _satoshi_txout * SATOSHI_TXOUT_new(int64_t value, const unsigned char * pk_script, uint32_t cbScript);
uint32_t SATOSHI_TXOUT_load(struct _satoshi_txout ** pp_txout, const unsigned char *begin, const unsigned char * end);
uint32_t SATOSHI_TXOUT_attach(struct _satoshi_txout ** pp_txout, const unsigned char *begin, const unsigned char *end);
void SATOSHI_TXOUT_free(struct _satoshi_txout * p_txout);

uint32_t SATOSHI_TXOUT_size(const struct _satoshi_txout * p_txout);
uint32_t SATOSHI_TXOUT_script_length(const struct _satoshi_txout * p_txout);
uint32_t SATOSHI_TXOUT_get_script(const struct _satoshi_txout * p_txout, unsigned char * to, uint32_t cbMax);
void SATOSHI_TXOUT_dump(const struct _satoshi_txout * p_txout);

#pragma pack(push)
#pragma pack(1)
typedef struct _satoshi_tx
{
    uint32_t version;
    unsigned char vch[1];
}satoshi_tx;
#pragma pack(pop)

struct _satoshi_tx * SATOSHI_TX_new(const unsigned char * begin, uint32_t cbPayload);
uint32_t SATOSHI_TX_load(struct _satoshi_tx ** pp_tx, const unsigned char *begin, const unsigned char *end);
uint32_t SATOSHI_TX_attach(struct _satoshi_tx ** pp_tx, const unsigned char * begin, const unsigned char * end);
uint32_t SATOSHI_TX_size(const struct _satoshi_tx * p_tx);
void SATOSHI_TX_free(struct _satoshi_tx * p_tx);

uint32_t SATOSHI_TX_get_locktime(const struct _satoshi_tx * p_tx);
BOOL SATOSHI_TX_set_locktime(const struct _satoshi_tx * p_tx, uint32_t lock_time);
uint32_t SATOSHI_TX_get_txin_list(const struct _satoshi_tx * p_tx, struct _satoshi_txin **pp_txin_list);
uint32_t SATOSHI_TX_get_txout_list(const struct _satoshi_tx * p_tx, struct _satoshi_txout **pp_txout_list);
void SATOSHI_TX_dump(const struct _satoshi_tx * p_tx);



typedef struct _satoshi_tx_helpler
{
    const struct _satoshi_tx * p_tx;
    uint32_t version;
    struct _varint *txin_count;
    struct _satoshi_txin ** txin_list;
    struct _varint *txout_count;
    struct _satoshi_txout ** txout_list;
    uint32_t lock_time;
}satoshi_tx_helpler;

BOOL SATOSHI_TX_HELPLER_init(struct _satoshi_tx_helpler * p_txhlp, const struct _satoshi_tx * p_tx);
void SATOSHI_TX_HELPER_clean(struct _satoshi_tx_helpler * p_txhlp);
void SATOSHI_TX_HELPER_free(struct _satoshi_tx_helpler * p_txhlp);






struct _VARINT
{
    struct _varint * (*attach)(const unsigned char * begin, const unsigned char *end);
    struct _varint * (*load)(struct _varint ** pp_varint, const unsigned char * begin, const unsigned char * end);
};
extern struct _VARINT VARINT;


extern const struct _compact_int satoshi_difficult_1;
extern const int32_t SATOSHI_DEFAULT_TX_VERSION;

#endif // DATATYPE_H_INCLUDED
