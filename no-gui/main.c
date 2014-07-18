#include "base/compatible.h"
#include "base/util.h"

#define MAX_PREFIX_LEN (5)
const char * output_file = "output.txt";
int GenerateKeys(int nCount, const char * lpszPrefix, uint32_t cbPrefix, FILE * fp_output);

int main(int argc, char * argv[])
{
    int rc = -1;
    int nCount = 10;
    char szPrefix[32] = "";
    uint32_t cbPrefix = 0;

    uint64_t nStartTime, nEndTime;

    if(argc > 1)
    {
        if((strcmpi(argv[1], "/?") == 0) || (strcmpi(argv[1], "/help") == 0) ||
           (strcmpi(argv[1], "-h") == 0) || (strcmpi(argv[1], "--help") == 0))
        {
            printf("usage: %s [nAddrCount] [Prefix]\n", argv[0]);
            return 0;
        }
        nCount = atoi(argv[1]);
        printf("Addresses to generate: %d\n", nCount);
        if(nCount == 0) nCount = 1;
        if(argc > 2)
        {
            cbPrefix = sprintf(szPrefix, "%s", argv[2]);
            printf("prefix: %s\n", szPrefix);
            if(cbPrefix > MAX_PREFIX_LEN)
            {
                printf("Too complex to match the prefix, \n"
                       "it might take several thousand years to get such an address.\n"
                       "This app current only support prefix length equal or less than %d.\n",
                       MAX_PREFIX_LEN);
                return 0;
            }
        }
    }

    FILE * fp = fopen(output_file, "at");
    if(NULL != fp) fseek(fp, 0, SEEK_END);

    nStartTime = GetTime();
    rc = GenerateKeys(nCount, szPrefix, cbPrefix, fp);
    nEndTime = GetTime() - nStartTime;

    printf("Time ellipse: %.2d:%.2d:%.2d",
                (int32_t)(nEndTime / 3600) % 24,
                (int32_t)(nEndTime /60) % 60,
                (int32_t)(nEndTime % 60)
           );

    if(fp != NULL) fclose(fp);
    return rc;
}

//void Log(FILE * fp, const char * fmt, ...);


int GenerateKeys(int nCount, const char * lpszPrefix, uint32_t cbPrefix, FILE * fp_output)
{
 //   if(NULL == p_key) p_key = ECKey_new();
 #define BUFFER_SIZE 4096
    int i = 0;
    int nValid = 0;
    EC_KEY * p_key = NULL;
    unsigned char vch[32] ="";
    unsigned char pubkey[65] = "";
    char szAddr[100] = "";
    char szWif[100] = "";
    char * buffer = NULL, * p;
    int cb = 0;
    uint32_t cbAddr, cbWif, cbPubkey;
    int cbWrite = 0;
    if(NULL == fp_output) fp_output = stdout;

//  Initialize the seed to generate a pseudo-random number with enough security
    printf("Rand and seed ...\n...\n");
    RandAndSeed();
    printf("seed finished.\n\n");

    buffer = (char *)calloc(BUFFER_SIZE, 1);

//    fseek(fp_output, 0, SEEK_END);

    p_key = ECKey_new();
    if(NULL == p_key) goto label_exit;

    printf("Start to generate ...\n");
    while(nValid < nCount)
    {
        i++;
        if((i % 1000) == 0) printf("%d keys generated.\n", i);

        if(GetKeyState(VK_CONTROL) < 0)
        {
            // Press [Ctrl] to exit the loop
            break;
        }

        ECKey_GeneratePrivKey(p_key, vch);
        cbWif = PrivkeyToWIF(vch, szWif, TRUE); // with a flag to generate compressed public key
        if(cbWif == 0) break;

        cbPubkey = ECKey_GetPubkey(p_key, pubkey, TRUE); // compressed public key
        if(cbPubkey == 0) break;

        cbAddr = PubkeyToAddr(pubkey, cbPubkey, szAddr);
        if(cbAddr == 0) break;


        if(cbPrefix && strnicmp(&szAddr[1], lpszPrefix, cbPrefix) != 0) continue;

        // output
        p = buffer;
        p += sprintf(p, "======================================\r\n"
                        "%d:\r\n", nValid++);
        p += sprintf(p, "privkey: ");
        cb = Bin2Hex(vch, 32, p, 0);
        p += cb;

        p += sprintf(p, "\r\nwif: %s", szWif);

        p += sprintf(p, "\r\npubkey: ");
        cb = Bin2Hex(pubkey, cbPubkey, p, 0);
        p += cb;

        p += sprintf(p, "\r\naddr: %s\r\n"
                        "======================================\r\n", szAddr);

        cb = p - buffer;
        if( cb >= BUFFER_SIZE)
        {
            fprintf(stderr, "an error occured.\n");
            break;
        }

        printf("cb = %d\n", cb);

        cbWrite = fwrite(buffer, 1, cb, fp_output);
        if(cbWrite != cb) break;
        if(fp_output != stdout) fwrite(buffer, 1, cb, stdout);
    };

label_exit:
    ECKey_Free(p_key);
    if(NULL != buffer) free(buffer);
    printf("exit.\n");
    return 0;
}
