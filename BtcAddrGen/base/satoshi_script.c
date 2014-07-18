#include "satoshi_script.h"

void SSO_clean(struct _satoshi_script_obj * p_obj)
{
    if(NULL == p_obj) return;
    if(p_obj->type == SSOT_POINTER && p_obj->fNeedFree && NULL != p_obj->p)
    {
        free(p_obj->p);
        p_obj->p = NULL;
        p_obj->fNeedFree = FALSE;
    }
}

uint32_t SSO_copy(struct _satoshi_script_obj ** pp_dest, const struct _satoshi_script_obj * p_src)
{
    assert(NULL != pp_dest);
    uint32_t size;
    struct _satoshi_script_obj * p_obj;
    *pp_dest = NULL;
    if(NULL == p_src)
    {

        return 0;
    }

    size = SSO_get_size(p_src);
    if(size < 16) return 0;

    p_obj = (struct _satoshi_script_obj *)malloc(size);
    if(NULL == p_obj) return 0;
    memcpy(p_obj, p_src, size);
    *pp_dest = p_obj;
    return size;
}

static struct _satoshi_script_obj * SSO_resize(struct _satoshi_script_obj ** pp_obj, uint16_t type, uint32_t cbData)
{
    uint32_t size = 16;
    struct _satoshi_script_obj * p_obj = NULL;
    if(NULL != pp_obj) p_obj = *pp_obj;
    switch(type)
    {
    case SSOT_BINARY: case SSOT_HASH: case SSOT_STRING:
        size = cbData < sizeof(uint64_t)?16: 8 + cbData;
    default: break;
    }

    if(NULL == p_obj)
    {
        p_obj = (struct _satoshi_script_obj *)calloc(1, size);
        if(NULL != p_obj)
        {
            p_obj->type = type;
            p_obj->length = cbData;
        }
        if(NULL != pp_obj) *pp_obj = p_obj;
        return p_obj;
    }

    switch(p_obj->type)
    {
    case SSOT_BINARY: case SSOT_HASH: case SSOT_STRING:
        if((p_obj->length >= 8) && (p_obj->length < cbData))
        {
            free(p_obj);
            p_obj = (struct _satoshi_script_obj *)calloc(1, size);
            if(NULL != p_obj)
            {
                p_obj->type = type;
                p_obj->length = cbData;
            }
            if(NULL != pp_obj) *pp_obj = p_obj;
            return p_obj;
        }
        break;
    case SSOT_POINTER:
        if(p_obj->fNeedFree && NULL != p_obj->p)
        {
            free(p_obj->p);
            p_obj->p = NULL;
            p_obj->fNeedFree = 0;
        }
        break;
    default: break;
    }

    p_obj->type = type;
    p_obj->length = cbData;
    return p_obj;
}

uint32_t SSO_set_data(struct _satoshi_script_obj ** pp_obj, uint16_t type, const void * p_data, uint32_t cbData, BOOL fNeedFree)
{
    assert(NULL != pp_obj);
    struct _satoshi_script_obj * p_obj = NULL;

    p_obj = SSO_resize(pp_obj, type, cbData);
    if(NULL == p_obj) return 0;

    if(type != SSOT_POINTER)
    {
        p_obj->fNeedFree = 0;
        memcpy(&p_obj->vch[0], p_data, cbData);
    }else
    {
        p_obj->p = (void *)p_data;
    }

    if(NULL == *pp_obj) *pp_obj = p_obj;
    return SSO_get_size(p_obj);
}

void SSO_free(struct _satoshi_script_obj * p_obj)
{
    if(NULL == p_obj) return;
    SSO_clean(p_obj);
    free(p_obj);
}

uint32_t SSO_get_size(const struct _satoshi_script_obj * p_obj)
{
    assert(NULL != p_obj);
    switch(p_obj->type)
    {
    case SSOT_INTEGER: case SSOT_BOOLEAN: case SSOT_POINTER: return 16;
    default: break;
    }
    return (p_obj->length < 8 ? 16: 8 + p_obj->length);
}
uint32_t SSO_get_data(const struct _satoshi_script_obj * p_obj, unsigned char * to)
{
    assert(NULL != p_obj);
    if(NULL == to) return p_obj->length;
    if(p_obj->type == SSOT_POINTER)
    {
        if(p_obj->length) memcpy(to, p_obj->p, p_obj->length);
    }
    else memcpy(to, &p_obj->vch[0], p_obj->length);
    return p_obj->length;
}
/*
uint32_t SSO_set_integer(struct _satoshi_script_obj ** pp_obj, const void * p_data, uint32_t cbData)
{
   return SSO_set_data(pp_obj, SSOT_INTEGER, p_data, cbData, 0);
}

uint32_t SSO_set_hash(struct _satoshi_script_obj ** pp_obj, const void * p_data, uint32_t cbData)
{
    return SSO_set_data(pp_obj, SSOT_HASH, p_data, cbData, 0);
}
uint32_t SSO_set_string(struct _satoshi_script_obj ** pp_obj, const void * p_data, uint32_t cbData)
{
    return SSO_set_data(pp_obj, SSOT_STRING, p_data, cbData, 0);
}
uint32_t SSO_set_binary(struct _satoshi_script_obj ** pp_obj, const void * p_data, uint32_t cbData)
{
    return SSO_set_data(pp_obj, SSOT_BINARY, p_data, cbData, 0);
}
uint32_t SSO_set_pointer(struct _satoshi_script_obj ** pp_obj, const void * p_data, uint32_t cbData, BOOL fNeedFree)
{
    return SSO_set_data(pp_obj, SSOT_POINTER, p_data, cbData, fNeedFree);
}
uint32_t SSO_set_boolean(struct _satoshi_script_obj ** pp_obj, const void * p_data, uint32_t cbData)
{
    return SSO_set_data(pp_obj, SSOT_BOOLEAN, p_data, cbData, 0);
}
*/


//*******************************************************
//** SATOSHI_SCRIPT_PROCESS

struct _satoshi_script_stack * SSS_new()
{
    struct _satoshi_script_stack * p_stack = calloc(sizeof(struct _satoshi_script_stack), 1);
    return p_stack;
}
uint32_t SSS_push(struct _satoshi_script_stack * p_stack, struct _satoshi_script_obj * p_obj)
{
    assert(NULL != p_stack);
    struct _sss_node * node = NULL;
    node = (struct _sss_node *)calloc(sizeof(struct _sss_node), 1);
    if(NULL == node) return FALSE;

    node->p_obj = p_obj;
    node->next = p_stack->top;

    p_stack->top = node;
    p_stack->size ++;
    return p_stack->size;
}
struct _satoshi_script_obj * SSS_pop(struct _satoshi_script_stack * p_stack)
{
    assert(NULL != p_stack);
    struct _sss_node * node = p_stack->top;
    struct _satoshi_script_obj * p_obj;
    if(p_stack->size == 0) return NULL;
    if(NULL == node) return NULL;
    p_stack->top = p_stack->top->next;
    p_stack->size--;

    p_obj = node->p_obj;
    free(node);
    return p_obj;
}
struct _satoshi_script_obj * SSS_peek(struct _satoshi_script_stack * p_stack, uint32_t depth)
{
    assert(NULL != p_stack);
    struct _sss_node * node = p_stack->top;
    if(p_stack->size == 0) return NULL;
    if(depth >= p_stack->size) return NULL;

    while(depth)
    {
        node = node->next;
        if(node == NULL) return NULL;
        depth--;
    };

    return node->p_obj;
}


uint32_t SSS_size(const struct _satoshi_script_stack * p_stack)
{
    if (NULL == p_stack) return 0;
    return p_stack->size;
}
void SSS_clean(struct _satoshi_script_stack * p_stack)
{
    if(NULL == p_stack) return;
    struct _satoshi_script_obj * obj = SSS_pop(p_stack);

    while(obj != NULL)
    {
        if(!p_stack->fUserAlloc) SSO_free(obj);
        obj = SSS_pop(p_stack);
    }
}

void SSS_free_node(struct _satoshi_script_stack * p_stack, struct _sss_node * p_node)
{
    assert(p_stack != NULL);
    if(p_node != NULL)
    {
        if(!p_stack->fUserAlloc)
        {
            SSO_free(p_node->p_obj);
        }
        free(p_node);
    }
}

void SSS_free(struct _satoshi_script_stack * p_stack)
{
    if(NULL == p_stack) return;
    SSS_clean(p_stack);
    free(p_stack);
}


uint32_t SSS_load(struct _satoshi_script_stack * p_stack, const struct _varstr * p_script)
{
    if(NULL == p_stack || NULL == p_script) return 0;

    uint32_t cbScript;
    unsigned char * p;
    unsigned char * pend ;
    unsigned char op;
    BOOL fInvalid = FALSE;

    struct _satoshi_script_stack * p_altstack = NULL;

    cbScript = VARSTR_strlen(p_script);
    if(0 ==  cbScript) return 0;
    p = (unsigned char *)VARSTR_get_strptr(p_script);
    pend = p + cbScript;




    while(p < pend)
    {
        op = *p++;
        p = SATOSHI_SCRIPT_op(p_stack, op, p, pend, p_altstack, &fInvalid);
        if(NULL == p || fInvalid) break;
    }
    return p - (unsigned char *)VARSTR_get_strptr(p_script);
}


static BOOL SATOSHI_SCRIPT_OP_is_valid(unsigned char op)
{
    if(op >= 0xba && op < 0xfd ) return FALSE;
    switch(op)
    {
    case 0xff:
    case OP_CAT: case OP_SUBSTR: case OP_LEFT: case OP_RIGHT:
    case OP_INVERT: case OP_AND: case OP_OR: case OP_XOR:
    case OP_2MUL: case OP_2DIV: case OP_MUL: case OP_DIV: case OP_MOD: case OP_LSHIFT: case OP_RSHIFT:
    case OP_RESERVED: case OP_RESERVED1: case OP_RESERVED2:
    case OP_VER: case OP_VERIF: case OP_VERNOTIF:
        return FALSE;
    }
    return TRUE;
}


unsigned char * SATOSHI_SCRIPT_op(struct _satoshi_script_stack *p_stack,
    unsigned char op,
    const unsigned char * p_params, const unsigned char * end,
    struct _satoshi_script_stack *p_altstack,
    BOOL * p_fInvalid)
{
    unsigned char * p = (unsigned char *)p_params;
    struct _satoshi_script_obj * p_obj = NULL;
    struct _satoshi_script_obj * p_dup_obj;
    struct _sss_node * p_node = NULL;
    uint32_t cb;
    int32_t iValue;
    uint32_t uValue;


    assert(p_fInvalid != NULL && NULL != p_stack && NULL != p_params && NULL != end);
    if(!SATOSHI_SCRIPT_OP_is_valid(op)) goto label_errexit;

    if(op == OP_FALSE)
    {
        SSS_push(p_stack, NULL);
        return p;
    }
    if(op > 0 && op < 0x4b)
    {
        cb = op;
        if((p + cb) > end) goto label_errexit;
        if(0 == SSO_set_binary(&p_obj, p, op)) goto label_errexit;
        if(!SSS_push(p_stack, p_obj)) goto label_errexit;
        p += cb;
        return p;
    }


    switch(op)
    {
    case OP_PUSHDATA1:
    case OP_PUSHDATA2:
    case OP_PUSHDATA4:
        if(op == OP_PUSHDATA1) cb = 1;
        else if(op == OP_PUSHDATA2) cb = 2;
        else cb = 4;
        p_obj = NULL;
        if((p + cb) > end) goto label_errexit;
        if(0 == SSO_set_integer(&p_obj, p, cb)) goto label_errexit;
        if(!SSS_push(p_stack, p_obj)) goto label_errexit;
        p += cb;
        return p;
    }
    if(op == OP_1NEGATE)
    {
        iValue = -1;
        p_obj = NULL;
        if(0 == SSO_set_integer(&p_obj, &iValue, sizeof(iValue))) goto label_errexit;
        if(!SSS_push(p_stack, p_obj)) goto label_errexit;
        return p;
    }

    if(op >= OP_1 && op <= 16)
    {
        iValue = op - OP_1 + 1;
        p_obj = NULL;
        if(0 == SSO_set_integer(&p_obj, &iValue, sizeof(iValue))) goto label_errexit;
        if(!SSS_push(p_stack, p_obj)) goto label_errexit;
        return p;
    }

    if(op == OP_NOP) return p;

    if(op == OP_IF)
    {
        unsigned char * p_else = NULL, * p_endif = NULL;
        unsigned char next_op;
        unsigned char * p_block_end;
        p_endif = p;
        while(p_endif < end)
        {
            next_op = *p_endif++;
            if(next_op == OP_ENDIF) break;
            if(NULL == p_else && next_op == OP_ELSE) p_else = p_endif;
        }
        if(NULL == p_endif) goto label_errexit;

        p_block_end = (p_else == NULL) ? p_endif: p_else;


        p_obj = SSS_pop(p_stack);
        if(NULL != p_obj && p_obj->llu != 0)
        {
            next_op = *p++;
            p = SATOSHI_SCRIPT_op(p_stack, next_op, p, p_block_end, p_altstack, p_fInvalid);
            if(NULL == p) goto label_errexit;
            return p_endif;
        }else
        {
            if(NULL != p_else)
            {
                p = p_else;
                next_op = *p++;
                p = SATOSHI_SCRIPT_op(p_stack, next_op, p, p_block_end, p_altstack, p_fInvalid);
                if(NULL == p) goto label_errexit;
                return p_endif;
            }
        }
        return p_endif;
    }

    if(op == OP_ELSE || op == OP_ENDIF) goto label_errexit;

    if(op == OP_VERIFY)
    {
        p_obj = SSS_pop(p_stack);
        if(NULL == p_obj) goto label_errexit;
        if(p_obj->llu == 0) *p_fInvalid = TRUE;
        return p;
    }

    if(op == OP_RETURN)
    {
        *p_fInvalid = TRUE;
        return p;
    }

//**** Stack ****
    if(op == OP_TOALTSTACK)
    {
        if(p_stack->size == 0 || NULL == p_altstack) goto label_errexit;
        p_obj = SSS_pop(p_stack);
        if( 0 == SSS_push(p_altstack, p_obj)) goto label_errexit;
        return p;
    }else if(op == OP_FROMALTSTACK)
    {
        if(NULL == p_altstack || p_altstack->size == 0) goto label_errexit;
        p_obj = SSS_pop(p_altstack);
        if(0 == SSS_push(p_stack, p_obj)) goto label_errexit;
        return p;
    }else if(op == OP_IFDUP)
    {
        p_obj = SSS_peek(p_stack, 0);
        if(NULL == p_obj || p_obj->llu == 0) return p;

        cb = SSO_get_size(p_obj);

        p_dup_obj = (struct _satoshi_script_obj *)malloc(cb);
        if(NULL == p_dup_obj) goto label_errexit;
        memcpy(p_dup_obj, p_obj, cb);

        if(0 == SSS_push(p_stack, p_dup_obj)) goto label_errexit;
        return p;
    }else if(op == OP_DEPTH)
    {
        cb = SSS_size(p_stack);
        p_obj = NULL;
        SSO_set_integer(&p_obj, &cb, sizeof(cb));
        if(NULL == p_obj) goto label_errexit;
        if(0 == SSS_push(p_stack, p_obj)) goto label_errexit;
        return p;
    }else if(op == OP_DROP)
    {
        p_obj = SSS_pop(p_stack);
        if(!p_stack->fUserAlloc) SSO_free(p_obj);
        return p;
    }else if(op == OP_DUP)
    {
        if(p_stack->size == 0) goto label_errexit;

        p_obj = SSS_peek(p_stack, 0);
        cb = SSO_get_size(p_obj);

        p_dup_obj = (struct _satoshi_script_obj *)malloc(cb);
        if(NULL == p_dup_obj) goto label_errexit;
        memcpy(p_dup_obj, p_obj, cb);
        if(0 == SSS_push(p_stack, p_dup_obj)) goto label_errexit;
        return p;
    }else if(op == OP_NIP)
    {
        if(p_stack->size < 2) goto label_errexit;
        p_node = p_stack->top->next;
        p_stack->top->next = p_node->next;
        SSS_free_node(p_stack, p_node);
        return p;
    }else if(op == OP_OVER)
    {
        if(p_stack->size < 2) goto label_errexit;
        p_node = p_stack->top->next;

        p_dup_obj = NULL;
        p_obj = p_node->p_obj;
        SSO_copy(&p_dup_obj, p_node->p_obj);
        if(NULL != p_obj && NULL == p_dup_obj) goto label_errexit;
        if(0 == SSS_push(p_stack, p_dup_obj)) goto label_errexit;
        return p;
    }else if(op == OP_PICK)
    {
        if(p_stack->size == 0 || p_stack->top == NULL) goto label_errexit;
        p_obj = SSS_pop(p_stack);
        if(NULL == p_obj) goto label_errexit;
        uValue = p_obj->lu;
        if(uValue >= p_stack->size) goto label_errexit;

        p_node = p_stack->top;
        while(uValue)
        {
            p_node = p_node->next;
            if(p_node == NULL) goto label_errexit;
            uValue--;
        }

        p_dup_obj = NULL;
        p_obj = p_node->p_obj;
        SSO_copy(&p_dup_obj, p_obj);
        if(NULL != p_obj && NULL == p_dup_obj) goto label_errexit;
        if(0 == SSS_push(p_stack, p_dup_obj)) goto label_errexit;
        return p;

    }else if(op == OP_ROLL)
    {
        struct _sss_node * p_prior_node = NULL;
        if(p_stack->size == 0 || p_stack->top == NULL) goto label_errexit;
        p_obj = SSS_pop(p_stack);
        if(NULL == p_obj) goto label_errexit;
        uValue = p_obj->lu;
        if(uValue == 0) return p;
        if(uValue >= p_stack->size) goto label_errexit;

        p_node = p_stack->top;
        while(uValue > 1)
        {
            p_node = p_node->next;
            if(p_node == NULL) goto label_errexit;
            uValue--;
        }
        p_prior_node = p_node;
        p_node = p_prior_node->next;
        p_prior_node->next = p_node->next;

        p_node->next = p_stack->top;
        p_stack->top = p_node;
        return p;
    }











    switch(op)
    {


    default:
        if(NULL != p_fInvalid) *p_fInvalid = TRUE;
        return p;
    }



    return p;
label_errexit:
    *p_fInvalid = TRUE;
    return NULL;

}
