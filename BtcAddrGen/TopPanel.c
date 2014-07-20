// toppanel.c
// Copyright (C) 2014  chehw
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

#include "resource.h"
#include "TopPanel.h"
#include "MainFrame.h"

#include <pthread.h>

typedef enum _TOPPANEL_ID
{
    IDC_GENERATE = 200,
    IDC_NUM_KEYS = 201,
    IDC_PREFIX = 202,
    IDC_THREADS_COUNT = 203,
    IDC_GENERATE_STOP = 299
}TOPPANEL_ID;

//#define IDC_GENERATE     200
//#define IDC_NUM_ADDRESS  201
//#define IDC_PREFIX       202
//#define IDC_GENERATE_STOP 299
//
//#define IDC_DO_SEARCH     300
//#define IDC_SEARCH_BOX    301

extern CMainFrame * cf;


static LONG nAddrCount = 0;
static LONG nKeysGen;

BOOL fGenerate = FALSE;

#define MAX_THREADS 16
static pthread_t hGenThreads[MAX_THREADS];
static pthread_mutex_t hMutex;

static int nThreads;
static uint64_t nStartTime; // seconds


static BOOL TopPanel_OnCreate(CTopPanel * pWnd, LPCREATESTRUCT pcs);
static BOOL TopPanel_Destroy(CTopPanel * pWnd);
static LRESULT TopPanel_OnSize(CTopPanel * pWnd, UINT uState, int cx, int cy);
static LRESULT TopPanel_OnNotify(CTopPanel * pWnd, LPNMHDR p_nmhdr);
static LRESULT TopPanel_OnCommand(CTopPanel * pWnd, UINT uID, HWND hWndCtrl, UINT uNotifyCode);
static void TopPanel_OnPaint(CTopPanel * pWnd, HDC hdc, LPPAINTSTRUCT p_ps);

LRESULT TopPanel_WindowProc(ChWinObj * p_obj, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CTopPanel * pWnd = (CTopPanel *)p_obj;
    HWND hWnd = p_obj->hWnd;
    switch(uMsg)
    {
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            TopPanel_OnPaint(pWnd, hdc, &ps);
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_SIZE:
        if(TopPanel_OnSize(pWnd, wParam, (short)LOWORD(lParam), (short)HIWORD(lParam))) break;
        return 0;
    case WM_NOTIFY:
        return TopPanel_OnNotify(pWnd, (LPNMHDR)lParam);
        break;
    case WM_COMMAND:
        if(TopPanel_OnCommand(pWnd, LOWORD(wParam), (HWND)lParam, HIWORD(wParam))) break;
        return 0;
    case WM_DESTROY:
        TopPanel_Destroy(pWnd);
        return 0;
    case WM_CREATE:
        if(!TopPanel_OnCreate(pWnd, (LPCREATESTRUCT)lParam)) return -1;
        return 0;
    default:
        break;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

static BOOL TopPanel_OnCreate(CTopPanel * pWnd, LPCREATESTRUCT pcs)
{
//    printf("top panel create.\n");
    HWND hEdit;
    HWND hButton;
    HWND hWnd;


    pthread_mutex_init(&hMutex, NULL);


    hWnd = CreateWindowEx(0, TEXT("STATIC"), TEXT("keys count"),
                           WS_CHILD|WS_CLIPSIBLINGS|SS_LEFT,
                           5, 8, 80, 22, CHWND(pWnd), (HMENU)(-1), pcs->hInstance, NULL);
    if(NULL != hWnd)
    {
        SendMessage(hWnd, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);
        ShowWindow(hWnd, SW_SHOW);
    }

    hEdit = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT("100"),
                           WS_CHILD|WS_CLIPSIBLINGS|ES_CENTER|ES_AUTOHSCROLL|WS_TABSTOP|ES_NUMBER,
                           90, 5, 60, 22, CHWND(pWnd), (HMENU)(IDC_NUM_KEYS), pcs->hInstance, NULL);
    if(NULL != hEdit)
    {
        SendMessage(hEdit, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);
        ShowWindow(hEdit, SW_SHOW);
    }

    hWnd = CreateWindowEx(0, TEXT("STATIC"), TEXT("addr prefix"),
                           WS_CHILD|WS_CLIPSIBLINGS|SS_LEFT,
                           160, 8, 80, 22, CHWND(pWnd), (HMENU)(-1), pcs->hInstance, NULL);
    if(NULL != hWnd)
    {
        SendMessage(hWnd, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);
        ShowWindow(hWnd, SW_SHOW);
    }


    hEdit = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT(""),
                           WS_CHILD|WS_CLIPSIBLINGS|ES_LEFT|ES_AUTOHSCROLL|WS_TABSTOP,
                           245, 5, 120, 22, CHWND(pWnd), (HMENU)(IDC_PREFIX), pcs->hInstance, NULL);
    if(NULL != hEdit)
    {
        SendMessage(hEdit, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);
        ShowWindow(hEdit, SW_SHOW);
    }


    hButton = CreateWindowEx(0, TEXT("BUTTON"), TEXT("Generate"),
                           WS_CHILD|WS_CLIPSIBLINGS|BS_PUSHLIKE|BS_VCENTER|WS_TABSTOP,
                           370 , 5, 80, 24, CHWND(pWnd), (HMENU)(IDC_GENERATE), pcs->hInstance, NULL);
    if(NULL != hButton)
    {
        pWnd->hSubmit = hButton;
        SendMessage(hButton, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);
        ShowWindow(hButton, SW_SHOW);
    }

    hButton = CreateWindowEx(0, TEXT("BUTTON"), TEXT("Stop"),
                           WS_CHILD|WS_CLIPSIBLINGS|BS_PUSHLIKE|BS_VCENTER|WS_TABSTOP|WS_DISABLED,
                           455 , 5, 80, 24, CHWND(pWnd), (HMENU)(IDC_GENERATE_STOP), pcs->hInstance, NULL);
    if(NULL != hButton)
    {
        pWnd->hSubmit = hButton;
        SendMessage(hButton, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);
        ShowWindow(hButton, SW_SHOW);
    }


    hWnd = CreateWindowEx(0, TEXT("STATIC"), TEXT("Threads count: "),
                           WS_CHILD|WS_CLIPSIBLINGS|SS_LEFT,
                           540, 8, 80, 22, CHWND(pWnd), (HMENU)(-1), pcs->hInstance, NULL);
    if(NULL != hWnd)
    {
        SendMessage(hWnd, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);
        ShowWindow(hWnd, SW_SHOW);
    }


    hEdit = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT("1"),
                           WS_CHILD|WS_CLIPSIBLINGS|ES_LEFT|ES_AUTOHSCROLL|WS_TABSTOP|ES_NUMBER,
                           625, 5, 40, 22, CHWND(pWnd), (HMENU)(IDC_THREADS_COUNT), pcs->hInstance, NULL);
    if(NULL != hEdit)
    {
        SendMessage(hEdit, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);
        ShowWindow(hEdit, SW_SHOW);
    }

//    hButton = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("BUTTON"), TEXT("GO"),
//                           WS_CHILD|WS_CLIPSIBLINGS|BS_PUSHLIKE|BS_VCENTER|WS_TABSTOP,
//                           0, 0, 80, 24, CHWND(pWnd), (HMENU)(300), pcs->hInstance, NULL);
//    if(NULL != hButton)
//    {
//        pWnd->hSubmit = hButton;
//        SendMessage(hButton, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);
//        ShowWindow(hButton, SW_SHOW);
//    }

    return TRUE;
}
static BOOL TopPanel_Destroy(CTopPanel * pWnd)
{
    fGenerate = FALSE;
    sleep(1);

    if(NULL != hMutex)
    {
        pthread_mutex_destroy(&hMutex);
    }
//    for(i = 0; i < nThreads; i++)
//    {
//        pthread_join(hGenThreads[i], NULL);
//    }
    return TRUE;
}

static LRESULT TopPanel_OnSize(CTopPanel * pWnd, UINT uState, int cx, int cy)
{
    if(SIZE_MAXHIDE == uState || SIZE_MINIMIZED == uState) return 1;
//    HWND hEdit = pWnd->hInputBox;
//    HWND hButton = pWnd->hSubmit;
//
//    SetWindowPos(hEdit, CHWND(pWnd), 0, 0, cx - 180, cy - 10, SWP_NOZORDER|SWP_NOMOVE);
//    SetWindowPos(hButton, CHWND(pWnd), cx - 90, 5, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

    return 0;
}
static LRESULT TopPanel_OnNotify(CTopPanel * pWnd, LPNMHDR p_nmhdr)
{
    return 0;
}
static LRESULT TopPanel_OnCommand(CTopPanel * pWnd, UINT uID, HWND hWndCtrl, UINT uNotifyCode)
{

    switch((TOPPANEL_ID)uID)
    {
    case IDC_GENERATE:
    case IDC_GENERATE_STOP:
        if(BN_CLICKED == uNotifyCode)
        {
            void TopPanel_OnButtonClicked(CTopPanel *, UINT);
            TopPanel_OnButtonClicked(pWnd, uID);
        }
        return 0;
    case IDC_NUM_KEYS:
    case IDC_PREFIX:
    case IDC_THREADS_COUNT:
        if(EN_CHANGE == uNotifyCode)
        {
            void TopPanel_OnEditChange(CTopPanel *, UINT);
            TopPanel_OnEditChange(pWnd, uID);
        }else if(EN_KILLFOCUS == uNotifyCode)
        {
            if(uID == IDC_THREADS_COUNT)
            {
                int nCount = GetDlgItemInt(CHWND(pWnd), uID, NULL, TRUE);
                if(nCount < 1 || nCount > MAX_THREADS)
                {
                    MessageBox(CHWND(pWnd), TEXT("Threads count should between 1 and 16"), "Invalid Parameter", 0);
                    SetFocus(hWndCtrl);
                    SendMessage(hWndCtrl, EM_SETSEL, 0, -1);
                    return 0;
                }

            }
        }
        return 0;
    default:
        return 1;
    }
    return 0;
}
static void TopPanel_OnPaint(CTopPanel * pWnd, HDC hdc, LPPAINTSTRUCT p_ps)
{
    HFONT hFont = GetStockObject(DEFAULT_GUI_FONT);
    int bkmode;
    static TCHAR szText[] = TEXT("Hashes: ");
    hFont = (HFONT)SelectObject(hdc, hFont);
    bkmode = SetBkMode(hdc, TRANSPARENT);

    TextOut(hdc, 5, 8, szText, sizeof(szText)/sizeof(szText[0]) - 1);
    SetBkMode(hdc, bkmode);
    SelectObject(hdc, hFont);
}


void TopPanel_OnButtonClicked(CTopPanel * pWnd, UINT uID)
{
    HWND hWnd = CHWND(pWnd);
    int nCount = 0;
    BOOL rc = FALSE;
    TCHAR szPrefix[100 + 1] = TEXT("");
    DWORD cbPrefix;

    if(uID == IDC_GENERATE)
    {
        nCount = GetDlgItemInt(hWnd, IDC_NUM_KEYS, &rc, 0);
        if(!rc) return;

        printf("nCount = %d\n", nCount);

        if(nCount <= 0 || nCount >1000000)
        {
            MessageBox(hWnd, TEXT("keys count should between 0 and 10000!"), TEXT("Invalid Parameters"), 0);
            SetFocus(GetDlgItem(hWnd, IDC_NUM_KEYS));
            return;
        }

        EnableWindow(GetDlgItem(hWnd, IDC_GENERATE_STOP), TRUE);
        EnableWindow(GetDlgItem(hWnd, IDC_GENERATE), FALSE);

        cbPrefix = GetDlgItemText(hWnd, IDC_PREFIX, szPrefix, 100);

        BOOL GenerateKeysAndAddresses(CTopPanel *, int, LPCTSTR, DWORD);
        if(!GenerateKeysAndAddresses(pWnd, nCount, szPrefix, cbPrefix))
        {

        }



    }else if(uID == IDC_GENERATE_STOP)
    {

        pthread_mutex_lock(&hMutex);
        fGenerate = FALSE;
        pthread_mutex_unlock(&hMutex);


        EnableWindow(GetDlgItem(hWnd, IDC_GENERATE_STOP), FALSE);
        EnableWindow(GetDlgItem(hWnd, IDC_GENERATE), TRUE);
    }
}

void TopPanel_OnEditChange(CTopPanel * pWnd, UINT uID)
{

}


struct _GKAA_PARAM
{
    CTopPanel * pWnd;
    int nCount;
    TCHAR szPrefix[100];
    DWORD cbPrefix;
};

void * GenerateKeysAndAddresses_thread(void * pvParam)
{
    struct _GKAA_PARAM * param = (struct _GKAA_PARAM *)pvParam;


    int i = 0;
    unsigned char vch[32] = {0};
    unsigned char pubkey[65] = {0};
    char szAddr[100] = "";

    HWND hOutput = cf->hOutput;
    HWND hWnd = CHWND(param->pWnd);
    HWND hStatusBar = cf->base_obj.hStatusBar;

//    char * szText = (char *)calloc(1024, 1);
//    char * p;
//    char * sz = (char *)calloc(100, 1);
//    char * szWif = (char *)calloc(100, 1);

    char szText[1024] = "";
    char * p, * p_end;
    char sz[100] = "";
    char szWif[100] = "";

    DWORD dwWait;
    BOOL fRun = TRUE;
    EC_KEY * pkey = NULL;

    int nValidCount = 0;

    if(NULL == param) goto label_exit;
    if(param->nCount <= 0)
    {
        printf("error: param->nCount = %u\n", param->nCount);
        CHUTIL_FreeMemory(param);
        return 0;
    }

    uint32_t cb, cbPubkey, cbAddr, cbWif;

//    WaitForSingleObject(hMutex, INFINITE);
//
//    ReleaseCapture();

    pkey = ECKey_new();
    p_end = szText + sizeof(szText);
    if(NULL == pkey) goto label_exit;
    while(fGenerate)
    {
        pthread_mutex_lock(&hMutex);
        fRun = fGenerate;
        pthread_mutex_unlock(&hMutex);
        if(!fRun) goto label_exit;

        cb = ECKey_GeneratePrivKey(pkey, vch);
        szWif[0] = '\0';
        cbWif = PrivkeyToWIF(vch, szWif, TRUE);
        if(!cbWif)
        {
            printf("PrivkeyToWIF error.\n");
            break;
        }

        cbPubkey= ECKey_GetPubkey(pkey, pubkey, TRUE);
        if(cbPubkey == 0)
        {
            printf("ECKey_GetPubkey error.\n");
            break;
        }
        cbAddr = PubkeyToAddr(pubkey, cbPubkey, szAddr);
        if(cbAddr == 0)
        {
            printf("PubkeyToAddr error.\n");
            break;
        }



        i = InterlockedIncrement(&nKeysGen);


        if((i % 100) == 0)
        {
            uint64_t nCurTime;
            nCurTime = GetTime();
            nCurTime -= nStartTime;


            sprintf(sz, "generated %d keys", i);
            SendMessage(hStatusBar, SB_SETTEXT, 2, (LPARAM)sz);
            sprintf(sz, "%.2u:%.2u:%.2u",
                (uint32_t)(nCurTime/3600)%24, (uint32_t)(nCurTime/60)%60, (uint32_t)(nCurTime%60) );
            SendMessage(hStatusBar, SB_SETTEXT, 3, (LPARAM)sz);

        }

        if(param->cbPrefix)
        {
            if(strnicmp(&szAddr[1], param->szPrefix, param->cbPrefix) != 0)
            {
                continue;
            }
        }




        pthread_mutex_lock(&hMutex);
        nAddrCount++;
        nValidCount = nAddrCount;

        SendMessage(hOutput, EM_SETSEL, -1, 0);


        p = szText;
        p += sprintf(p, "======================================\r\n%d:\r\n", nValidCount);
        p += sprintf(p, "privkey: ");
        cb = Bin2Hex(vch, 32, p, 0);
        p += cb;

        p += sprintf(p, "\r\nwif: %s", szWif);
        if(p > p_end) break;

        p += sprintf(p, "\r\npubkey: ");
        cb = Bin2Hex(pubkey, cbPubkey, p, 0);
        p += cb;
        if(p > p_end) break;

        p += sprintf(p, "\r\naddr: %s\r\n======================================\r\n", szAddr);
        if(p > p_end) break;
        cb = p - szText;

//        SendMessage(hOutput, EM_REPLACESEL, 0, (LPARAM)szText);

        cf->Log(cf, szText);

        pthread_mutex_unlock(&hMutex);

        if(nValidCount >= param->nCount) break;


    };



label_exit:
    ECKey_Free(pkey);
    printf("thread exit.\n");
//    if(NULL != szText) free(szText);
//    if(NULL != sz) free(sz);
//    if(NULL != szWif) free(szWif);

    CHUTIL_FreeMemory(param);

//    SendMessage(hStatusBar, SB_SETTEXT, 0, (LPARAM)"");
//    SendMessage(hStatusBar, SB_SETTEXT, 1, (LPARAM)"");
//    SendMessage(hStatusBar, SB_SETTEXT, 2, (LPARAM)"");
    SendMessage(hStatusBar, SB_SETTEXT, 1, (LPARAM)"stopped");

    EnableWindow(GetDlgItem(hWnd, IDC_GENERATE_STOP), FALSE);
    EnableWindow(GetDlgItem(hWnd, IDC_GENERATE), TRUE);

    return 0;
}

BOOL GenerateKeysAndAddresses(CTopPanel * pWnd, int nCount, LPCTSTR lpszPrefix, DWORD cbPrefix)
{
    HWND hStatusBar = cf->base_obj.hStatusBar;

    struct _GKAA_PARAM Param;
    struct _GKAA_PARAM * param;
    int i;
    int rc = 0;
    nAddrCount = 0;
    nKeysGen = 0;
    uint64_t nCurTime = 0;
    char sz[100] = "";

    bzero(&Param, sizeof(Param));
    param = &Param;



    nThreads = GetDlgItemInt(CHWND(pWnd), IDC_THREADS_COUNT, NULL, TRUE);
    if(nThreads < 1 || nThreads >= MAX_THREADS) goto label_errexit;

    param->pWnd = pWnd;
    param->nCount = nCount;


    if(cbPrefix >= sizeof(param->szPrefix))
    {
//        LogError("GenerateKeysAndAddresses", "Invalid parameter [cbPrefix]");
        CHUTIL_FreeMemory(param);
        goto label_errexit;
    }
    param->cbPrefix = cbPrefix;
    if(cbPrefix)
    {
        lstrcpyn(param->szPrefix, lpszPrefix, sizeof(param->szPrefix));
        param->szPrefix[cbPrefix] = '\0';
    }

     //**
    SendMessage(hStatusBar, SB_SETTEXT, 0, (LPARAM)"Rand and seed:");
    SendMessage(hStatusBar, SB_SETTEXT, 1, (LPARAM)"");
    SendMessage(hStatusBar, SB_SETTEXT, 2, (LPARAM)"Please wait about 10 seconds for initialize the seed.");

    nStartTime = GetTime();
    RandAndSeed();

    nCurTime = GetTime();
    nCurTime -= nStartTime;
    sprintf(sz, "%.2u:%.2u:%.2u",
            (uint32_t)(nCurTime/3600)%24, (uint32_t)(nCurTime/60)%60, (uint32_t)(nCurTime%60) );

    SendMessage(hStatusBar, SB_SETTEXT, 1, (LPARAM)"Start");
    SendMessage(hStatusBar, SB_SETTEXT, 2, (LPARAM)"");
    SendMessage(hStatusBar, SB_SETTEXT, 3, (LPARAM)sz);

    fGenerate = TRUE;
    nAddrCount = 0;
    nKeysGen = 0;

    SendMessage(hStatusBar, SB_SETTEXT, 0, (LPARAM)"Generating:");
    if(param->cbPrefix)
    {
        sprintf(sz, "1%s", Param.szPrefix);
        SendMessage(hStatusBar, SB_SETTEXT, 1, (LPARAM)sz);
    }

    cf->Log(cf, NULL);

    for(i = 0; i < nThreads; i++)
    {
        param = CHUTIL_AllocMemory(struct _GKAA_PARAM, 1);
        if(NULL == param) goto label_errexit;
        memcpy(param, &Param, sizeof(Param));
        rc = pthread_create(&hGenThreads[i], NULL, GenerateKeysAndAddresses_thread, param);

        if(rc != 0)
        {
            ErrorMsgBox(CHWND(pWnd), GetLastError(), TEXT("GenerateKeysAndAddresses Error"), 0);
            goto label_errexit;
        }
    }





    return TRUE;
label_errexit:
    if(NULL != param && param != &Param) CHUTIL_FreeMemory(param);
    return FALSE;
}
