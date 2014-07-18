#ifndef SATOSHI_SCRIPT_H_INCLUDED
#define SATOSHI_SCRIPT_H_INCLUDED

#include "compatible.h"
#include "util.h"
#include "datatype.h"

#pragma pack(push)
#pragma pack(1)

typedef enum
{
    SSOT_INTEGER = 0,
    SSOT_HASH = 1,
    SSOT_STRING = 2,
    SSOT_BINARY = 3,
    SSOT_POINTER = 4,
    SSOT_BOOLEAN = 9
}enum_script_obj_type;

typedef struct _satoshi_script_obj
{
    struct
    {
        uint16_t type;
        uint16_t fNeedFree;
    };
    uint32_t length;
    union
    {
        uint64_t llu;
        uint32_t lu;
        int64_t lld;
        int32_t ld;
        BOOL fok;
        unsigned char vch[1];
        char sz[1];
        void * p;
    };
}satoshi_script_obj;
#pragma pack(pop)
uint32_t SSO_set_data(struct _satoshi_script_obj ** pp_obj, uint16_t type, const void * p_data, uint32_t cbData, BOOL fNeedFree);
void SSO_free(struct _satoshi_script_obj * p_obj);
uint32_t SSO_get_size(const struct _satoshi_script_obj * p_obj);
uint32_t SSO_get_data(const struct _satoshi_script_obj * p_obj, unsigned char * to);

inline static uint32_t SSO_set_integer(struct _satoshi_script_obj ** pp_obj, const void * p_data, uint32_t cbData)
{
   return SSO_set_data(pp_obj, SSOT_INTEGER, p_data, cbData, 0);
}

inline static uint32_t SSO_set_hash(struct _satoshi_script_obj ** pp_obj, const void * p_data, uint32_t cbData)
{
    return SSO_set_data(pp_obj, SSOT_HASH, p_data, cbData, 0);
}
inline static uint32_t SSO_set_string(struct _satoshi_script_obj ** pp_obj, const void * p_data, uint32_t cbData)
{
    return SSO_set_data(pp_obj, SSOT_STRING, p_data, cbData, 0);
}
inline static uint32_t SSO_set_binary(struct _satoshi_script_obj ** pp_obj, const void * p_data, uint32_t cbData)
{
    return SSO_set_data(pp_obj, SSOT_BINARY, p_data, cbData, 0);
}
inline static uint32_t SSO_set_pointer(struct _satoshi_script_obj ** pp_obj, const void * p_data, uint32_t cbData, BOOL fNeedFree)
{
    return SSO_set_data(pp_obj, SSOT_POINTER, p_data, cbData, fNeedFree);
}
inline static uint32_t SSO_set_boolean(struct _satoshi_script_obj ** pp_obj, const void * p_data, uint32_t cbData)
{
    return SSO_set_data(pp_obj, SSOT_BOOLEAN, p_data, cbData, 0);
}



typedef struct _sss_node
{
    struct _satoshi_script_obj * p_obj;
//    struct _sss_node * prior;
    struct _sss_node * next;
}sss_node;

typedef struct _satoshi_script_stack
{
    struct _sss_node * top;
    uint32_t size;
    BOOL fUserAlloc;
}satoshi_script_stack;

struct _satoshi_script_stack * SSS_new();
uint32_t SSS_push(struct _satoshi_script_stack * p_stack, struct _satoshi_script_obj * obj);
struct _satoshi_script_obj * SSS_pop(struct _satoshi_script_stack * p_stack);
struct _satoshi_script_obj * SSS_peek(struct _satoshi_script_stack * p_stack, uint32_t depth);
uint32_t SSS_size(const struct _satoshi_script_stack * p_stack);
void SSS_clean(struct _satoshi_script_stack * p_stack);
void SSS_free(struct _satoshi_script_stack * p_stack);


uint32_t SSS_load(struct _satoshi_script_stack * p_stack, const struct _varstr * p_script);


typedef enum
{
// Constants
    OP_O = 0,
    OP_FALSE = 0,
    OP_PUSHDATA1 = 0x4c, //77
    OP_PUSHDATA2 = 0x4d, //78
    OP_PUSHDATA4 = 0x4e,
    OP_1NEGATE = 0x4f,
    OP_1 = 0x51,
    OP_TRUE = 0x51,
    OP_2 = 0x52,
    OP_3 = 0x53,
    OP_4 = 0x54,
    OP_5 = 0x55,
    OP_6 = 0x56,
    OP_7 = 0x57,
    OP_8 = 0x58,
    OP_9 = 0x59,
    OP_10 = 0x5a,
    OP_11 = 0x5b,
    OP_12 = 0x5c,
    OP_13 = 0x5d,
    OP_14 = 0x5e,
    OP_15 = 0x5f,
    OP_16 = 0x60,
// Flow control
    OP_NOP = 0x61, //	97		Nothing	Nothing	Does nothing.
    OP_IF = 0x63, //	99		<expression> if [statements] [else [statements]]* endif	If the top stack value is not 0, the statements are executed. The top stack value is removed.
    OP_NOTIF = 0x64, //	100		<expression> if [statements] [else [statements]]* endif	If the top stack value is 0, the statements are executed. The top stack value is removed.
    OP_ELSE = 0x67, //	103		<expression> if [statements] [else [statements]]* endif	If the preceding OP_IF or OP_NOTIF or OP_ELSE was not executed then these statements are and if the preceding OP_IF or OP_NOTIF or OP_ELSE was executed then these statements are not.
    OP_ENDIF = 0x68, //	104		<expression> if [statements] [else [statements]]* endif	Ends an if/else block. All blocks must end, or the transaction is invalid. An OP_ENDIF without OP_IF earlier is also invalid.
    OP_VERIFY = 0x69, //	105		True / false	Nothing / False	Marks transaction as invalid if top stack value is not true.
    OP_RETURN = 0x6a, //	106		Nothing	Nothing	Marks transaction as invalid.
// Stack
    OP_TOALTSTACK = 0x6b, //	107		x1	(alt)x1	Puts the input onto the top of the alt stack. Removes it from the main stack.
    OP_FROMALTSTACK = 0x6c, //	108		(alt)x1	x1	Puts the input onto the top of the main stack. Removes it from the alt stack.
    OP_IFDUP = 0x73, //	115		x	x / x x	If the top stack value is not 0, duplicate it.
    OP_DEPTH = 0x74, //	116		Nothing	<Stack size>	Puts the number of stack items onto the stack.
    OP_DROP = 0x75, //	117		x	Nothing	Removes the top stack item.
    OP_DUP = 0x76, //	118		x	x x	Duplicates the top stack item.
    OP_NIP = 0x77, //	119		x1 x2	x2	Removes the second-to-top stack item.
    OP_OVER = 0x78, //	120		x1 x2	x1 x2 x1	Copies the second-to-top stack item to the top.
    OP_PICK = 0x79, //	121		xn ... x2 x1 x0 <n>	xn ... x2 x1 x0 xn	The item n back in the stack is copied to the top.
    OP_ROLL = 0x7a, //	122		xn ... x2 x1 x0 <n>	... x2 x1 x0 xn	The item n back in the stack is moved to the top.
    OP_ROT = 0x7b, //	123		x1 x2 x3	x2 x3 x1	The top three items on the stack are rotated to the left.
    OP_SWAP = 0x7c, //	124		x1 x2	x2 x1	The top two items on the stack are swapped.
    OP_TUCK = 0x7d, //	125		x1 x2	x2 x1 x2	The item at the top of the stack is copied and inserted before the second-to-top item.
    OP_2DROP = 0x6d, //	109		x1 x2	Nothing	Removes the top two stack items.
    OP_2DUP = 0x6e, //	110		x1 x2	x1 x2 x1 x2	Duplicates the top two stack items.
    OP_3DUP = 0x6f, //	111		x1 x2 x3	x1 x2 x3 x1 x2 x3	Duplicates the top three stack items.
    OP_2OVER = 0x70, //	112		x1 x2 x3 x4	x1 x2 x3 x4 x1 x2	Copies the pair of items two spaces back in the stack to the front.
    OP_2ROT = 0x71, //	113		x1 x2 x3 x4 x5 x6	x3 x4 x5 x6 x1 x2	The fifth and sixth items back are moved to the top of the stack.
    OP_2SWAP = 0x72, //	114		x1 x2 x3 x4	x3 x4 x1 x2	Swaps the top two pairs of items.
// Splice
// If any opcode marked as disabled is present in a script, it must abort and fail.
    OP_CAT = 0x7e, //	126		x1 x2	out	Concatenates two strings. disabled.
    OP_SUBSTR = 0x7f, //127		in begin size	out	Returns a section of a string. disabled.
    OP_LEFT = 0x80, //	128		in size	out	Keeps only characters left of the specified point in a string. disabled.
    OP_RIGHT = 0x81, //	129		in size	out	Keeps only characters right of the specified point in a string. disabled.
    OP_SIZE = 0x82, //	130		in	in size	Pushes the string length of the top element of the stack (without popping it).
// Bitwise logic
    OP_INVERT = 0x83, //	131		in	out	Flips all of the bits in the input. disabled.
    OP_AND = 0x84, //	132		x1 x2	out	Boolean and between each bit in the inputs. disabled.
    OP_OR = 0x85, //	133		x1 x2	out	Boolean or between each bit in the inputs. disabled.
    OP_XOR = 0x86, //	134		x1 x2	out	Boolean exclusive or between each bit in the inputs. disabled.
    OP_EQUAL = 0x87, //	135		x1 x2	True / false	Returns 1 if the inputs are exactly equal, 0 otherwise.
    OP_EQUALVERIFY = 0x88, //	136		x1 x2	True / false	Same as OP_EQUAL, but runs OP_VERIFY afterward.
//Arithmetic
//Note: Arithmetic inputs are limited to signed 32-bit integers, but may overflow their output.
//
//If any input value for any of these commands is longer than 4 bytes, the script must abort and fail. If any opcode marked as disabled is present in a script - it must also abort and fail.
    OP_1ADD = 0x8b, //	139		in	out	1 is added to the input.
    OP_1SUB = 0x8c, //	140		in	out	1 is subtracted from the input.
    OP_2MUL = 0x8d, //	141		in	out	The input is multiplied by 2. disabled.
    OP_2DIV = 0x8e, //	142		in	out	The input is divided by 2. disabled.
    OP_NEGATE = 0x8f, //	143		in	out	The sign of the input is flipped.
    OP_ABS = 0x90, //	144		in	out	The input is made positive.
    OP_NOT = 0x91, //	145		in	out	If the input is 0 or 1, it is flipped. Otherwise the output will be 0.
    OP_0NOTEQUAL = 0x92, //	146		in	out	Returns 0 if the input is 0. 1 otherwise.
    OP_ADD = 0x93, //	147		a b	out	a is added to b.
    OP_SUB = 0x94, //	148		a b	out	b is subtracted from a.
    OP_MUL = 0x95, //	149		a b	out	a is multiplied by b. disabled.
    OP_DIV = 0x96, //	\150		a b	out	a is divided by b. disabled.
    OP_MOD = 0x97, //	151		a b	out	Returns the remainder after dividing a by b. disabled.
    OP_LSHIFT = 0x98, //	152		a b	out	Shifts a left b bits, preserving sign. disabled.
    OP_RSHIFT = 0x99, //	153		a b	out	Shifts a right b bits, preserving sign. disabled.
    OP_BOOLAND = 0x9a, //	154		a b	out	If both a and b are not 0, the output is 1. Otherwise 0.
    OP_BOOLOR = 0x9b, //	155		a b	out	If a or b is not 0, the output is 1. Otherwise 0.
    OP_NUMEQUAL = 0x9c, //	156		a b	out	Returns 1 if the numbers are equal, 0 otherwise.
    OP_NUMEQUALVERIFY = 0x9d, //	157		a b	out	Same as OP_NUMEQUAL, but runs OP_VERIFY afterward.
    OP_NUMNOTEQUAL = 0x9e, //	158		a b	out	Returns 1 if the numbers are not equal, 0 otherwise.
    OP_LESSTHAN = 0x9f, //	159		a b	out	Returns 1 if a is less than b, 0 otherwise.
    OP_GREATERTHAN = 0xa0, //	160		a b	out	Returns 1 if a is greater than b, 0 otherwise.
    OP_LESSTHANOREQUAL = 0xa1, //	161		a b	out	Returns 1 if a is less than or equal to b, 0 otherwise.
    OP_GREATERTHANOREQUAL = 0xa2, //	162		a b	out	Returns 1 if a is greater than or equal to b, 0 otherwise.
    OP_MIN = 0xa3, //	163		a b	out	Returns the smaller of a and b.
    OP_MAX = 0xa4, //	164		a b	out	Returns the larger of a and b.
    OP_WITHIN = 0xa5, //	165		x min max	out	Returns 1 if x is within the specified range (left-inclusive), 0 otherwise.
// Crypto

    OP_RIPEMD160 = 0xa6, //	166		in	hash	The input is hashed using RIPEMD-160.
    OP_SHA1 = 0xa7, //	167		in	hash	The input is hashed using SHA-1.
    OP_SHA256 = 0xa8, //	168		in	hash	The input is hashed using SHA-256.
    OP_HASH160 = 0xa9, //	169		in	hash	The input is hashed twice: first with SHA-256 and then with RIPEMD-160.
    OP_HASH256 = 0xaa, //	170		in	hash	The input is hashed two times with SHA-256.
    OP_CODESEPARATOR = 0xab, //	171		Nothing	Nothing	All of the signature checking words will only match signatures to the data after the most recently-executed OP_CODESEPARATOR.
    OP_CHECKSIG = 0xac, //	172		sig pubkey	True / false	The entire transaction's outputs, inputs, and script (from the most recently-executed OP_CODESEPARATOR to the end) are hashed. The signature used by OP_CHECKSIG must be a valid signature for this hash and public key. If it is, 1 is returned, 0 otherwise.
    OP_CHECKSIGVERIFY = 0xad, //	173		sig pubkey	True / false	Same as OP_CHECKSIG, but OP_VERIFY is executed afterward.
    OP_CHECKMULTISIG = 0xae, //	174		x sig1 sig2 ... <number of signatures> pub1 pub2 <number of public keys>	True / False	For each signature and public key pair, OP_CHECKSIG is executed. If more public keys than signatures are listed, some key/sig pairs can fail. All signatures need to match a public key. If all signatures are valid, 1 is returned, 0 otherwise. Due to a bug, one extra unused value is removed from the stack.
    OP_CHECKMULTISIGVERIFY = 0xaf, //	175		x sig1 sig2 ... <number of signatures> pub1 pub2 ... <number of public keys>	True / False	Same as OP_CHECKMULTISIG, but OP_VERIFY is executed afterward.
//Pseudo-words
//These words are used internally for assisting with transaction matching. They are invalid if used in actual scripts.
    OP_PUBKEYHASH = 0xfd, //	253		Represents a public key hashed with OP_HASH160.
    OP_PUBKEY = 0xfe, //	254		Represents a public key compatible with OP_CHECKSIG.
    OP_INVALIDOPCODE = 0xff, //	255		Matches any opcode that is not yet assigned.
//Reserved words
//Any opcode not assigned is also reserved. Using an unassigned opcode makes the transaction invalid.
    OP_RESERVED = 0x50, //	80		Transaction is invalid unless occuring in an unexecuted OP_IF branch
    OP_VER = 0x62, //	98		Transaction is invalid unless occuring in an unexecuted OP_IF branch
    OP_VERIF = 0x65, //	101		Transaction is invalid even when occuring in an unexecuted OP_IF branch
    OP_VERNOTIF = 0x66, //	102		Transaction is invalid even when occuring in an unexecuted OP_IF branch
    OP_RESERVED1 = 0x89, //	137		Transaction is invalid unless occuring in an unexecuted OP_IF branch
    OP_RESERVED2 = 0x8a, //	138	Transaction is invalid unless occuring in an unexecuted OP_IF branch
    OP_NOP1 = 0xb0, // 176-185		The word is ignored. Does not mark transaction as invalid.
    OP_NOP2 = 0xb1,
    OP_NOP3 = 0xb2,
    OP_NOP4 = 0xb3,
    OP_NOP5 = 0xb4,
    OP_NOP6 = 0xb5,
    OP_NOP7 = 0xb6,
    OP_NOP8 = 0xb7,
    OP_NOP9 = 0xb8,
    OP_NOP10 = 0xb9
}satoshi_script_opcode;


unsigned char * SATOSHI_SCRIPT_op(struct _satoshi_script_stack *p_stack,
    unsigned char opcode,
    const unsigned char * p_params, const unsigned char * end,
    struct _satoshi_script_stack *p_altstack,
    BOOL * p_fInvalid);

#endif // SATOSHI_SCRIPT_H_INCLUDED
