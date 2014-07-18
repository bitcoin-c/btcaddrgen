// mainframe.c
// Copyright (C) 2014  chehw
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
#include "base/compatible.h"
#include "base/util.h"

#include "resource.h"
#include "MainFrame.h"


#define NUM_STATUS_PANES (5)
static BOOL MainFrame_OnCreate(CMainFrame * p_obj, LPCREATESTRUCT pcs);
static BOOL MainFrame_Destroy(CMainFrame * p_obj);
static LRESULT MainFrame_OnSize(CMainFrame * p_obj, UINT uState, int cx, int cy);
static LRESULT MainFrame_OnNotify(CMainFrame * p_obj, LPNMHDR p_nmhdr);
static LRESULT MainFrame_OnCommand(CMainFrame * p_obj, UINT uID, HWND hWndCtrl, UINT uNotifyCode);

static BOOL MainFrame_Init(CMainFrame *pWnd);
static int MainFrame_LogFmt(CMainFrame * pWnd, const char * fmt, ...);
static int MainFrame_Log(CMainFrame * pWnd, const char * szLog);

static BOOL MainFrame_PreTranslateMessage(const ChFrame * pWnd, LPMSG pmsg)
{
    if(pmsg->message == WM_KEYDOWN)
    {

        if((GetKeyState(VK_CONTROL) & 0x8000) && pmsg->wParam == 'A' )
        {
            printf("%c", (TCHAR)pmsg->wParam);
            HWND hWnd = GetFocus();
            TCHAR szClass[100 + 1] = "";
            if(GetClassName(hWnd, szClass, 100))
            {
                if(stricmp(szClass, TEXT("EDIT")) == 0)
                {
                    SendMessage(hWnd, EM_SETSEL, 0, -1);
                    return TRUE;
                }
            }
        }
    }

    if(NULL != pWnd->hAccel)
    {
        if(TranslateAccelerator(pWnd->base_obj.hWnd, pWnd->hAccel, pmsg)) return TRUE;
    }
    if(NULL != pWnd->hActiveDlg)
    {
        if(IsDialogMessage(pWnd->hActiveDlg, pmsg)) return TRUE;
    }

    return FALSE;
}


LRESULT MainFrame_WindowProc(ChWinObj * p_obj, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

    HWND hWnd = CHWND(p_obj);
    CMainFrame * pWnd = (CMainFrame *)p_obj;

    switch(uMsg)
    {
    case WM_SIZE:
        if(MainFrame_OnSize(pWnd, wParam, (short)LOWORD(lParam), (short)HIWORD(lParam))) break;
        return 0;
    case WM_NOTIFY:
        return MainFrame_OnNotify(pWnd, (LPNMHDR)lParam);
        break;
    case WM_COMMAND:
        if(MainFrame_OnCommand(pWnd, LOWORD(wParam), (HWND)lParam, HIWORD(wParam))) break;
        return 0;
    case WM_DESTROY:
        MainFrame_Destroy(pWnd);
        printf("mainframe destroyed\n");
        return 0;
    case WM_CREATE:
        if(!MainFrame_OnCreate(pWnd, (LPCREATESTRUCT)lParam)) return -1;
        return 0;
    default:
        break;
    }
    if(NULL == hWnd) return 0;
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

static BOOL MainFrame_OnCreate(CMainFrame * pWnd, LPCREATESTRUCT pcs)
{
    HWND hWnd;
    const ChWinObj * p_templ;
    ChWinObj * p_obj = (ChWinObj *)pWnd;
    HWND hParent = CHWND(pWnd);
    printf("wm_create\n");
    pWnd->Log = MainFrame_Log;
    pWnd->base_obj.PreTranslateMessage = MainFrame_PreTranslateMessage;



    hWnd = pWnd->base_obj.hStatusBar = CreateWindowEx(0, STATUSCLASSNAME, NULL,
                    WS_CHILD|WS_CLIPCHILDREN, 0,0,0,0,
                    hParent, (HMENU)33, p_obj->hInstance, NULL);
    if(NULL != hWnd)
    {
        RECT rt;
        SendMessage(hWnd, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0 );
        GetWindowRect(hWnd, &rt);
        p_obj->rtMargin.bottom = rt.bottom - rt.top;
        ShowWindow(hWnd, SW_SHOW);
    }

    //** output box
    hWnd = pWnd->hOutput = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), NULL,
                        WS_CHILD|WS_CLIPSIBLINGS|ES_MULTILINE|ES_AUTOVSCROLL|WS_TABSTOP|WS_VSCROLL,
                        0,0,0,0,
                        hParent, (HMENU)66, pcs->hInstance, NULL);
    if(NULL != hWnd)
    {
        HLOCAL hOldMem;
        SendMessage(hWnd, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0 );
        pWnd->cbMem = DEFAULT_ALLOC_SIZE;
        pWnd->hMem = LocalAlloc(LPTR, pWnd->cbMem);
        hOldMem = (HLOCAL)SendMessage(hWnd, EM_GETHANDLE, 0, 0);
        if(NULL != hOldMem) LocalFree(hOldMem);
        SendMessage(hWnd, EM_SETHANDLE, (WPARAM)pWnd->hMem, 0);
        SendMessage(hWnd, EM_SETLIMITTEXT, pWnd->cbMem, 0);
        ShowWindow(hWnd, SW_SHOW);
    }


//** Top Panel
    p_templ = (ChWinObj *)ChWinObj_new(sizeof(CTopPanel), __ch_form, TopPanel_WindowProc);
    pWnd->p_TopPanel = (CTopPanel *)ChWinObj_CreateEx(p_templ, WS_EX_CLIENTEDGE, TEXT("top panel"),
                            WS_CHILD|WS_CLIPSIBLINGS|WS_CLIPCHILDREN,
                            0,0,0,0,
                            hParent, 102, pcs->hInstance, NULL);
    if(NULL == pWnd->p_TopPanel) return FALSE;
    p_obj->rtMargin.top = 36;
    ShowWindow(CHWND(pWnd->p_TopPanel), SW_SHOW);


//** Left Panel
    p_templ = (ChWinObj *)ChWinObj_new(sizeof(CLeftPanel), NULL, LeftPanel_WindowProc);
    pWnd->p_LeftPanel = (CLeftPanel *)ChWinObj_CreateEx(p_templ, WS_EX_CLIENTEDGE, TEXT("left panel"),
                            WS_CHILD|WS_CLIPSIBLINGS|WS_CLIPCHILDREN,
                            0,0,0,0,
                            hParent, 101, p_obj->hInstance, NULL);
    if(NULL == pWnd->p_LeftPanel) return FALSE;
    p_obj->rtMargin.left = 180;
    ShowWindow(CHWND(pWnd->p_LeftPanel), SW_SHOW);

//**MainWindow
    p_templ = (ChWinObj *)ChWinObj_new(sizeof(CMainWindow), NULL, MainWindow_WindowProc);
    pWnd->p_MainWindow = (CMainWindow *)ChWinObj_CreateEx(p_templ, WS_EX_CLIENTEDGE, TEXT("main window"),
                            WS_CHILD|WS_CLIPSIBLINGS|WS_CLIPCHILDREN,
                            0,0,0,0,
                            hParent, 100, p_obj->hInstance, NULL);
    if(NULL == pWnd->p_MainWindow) return FALSE;

    ShowWindow(CHWND(pWnd->p_MainWindow), SW_SHOW);


    void CALLBACK MainFrame_TimerProc(HWND hwnd,UINT uMsg,UINT_PTR idEvent,DWORD dwTime);
    SetTimer(hParent, 1, 1000, MainFrame_TimerProc);



    MainFrame_Init(pWnd);
    return TRUE;
}
static BOOL MainFrame_Destroy(CMainFrame * pWnd)
{
    KillTimer(CHWND(pWnd), 1);
    ChWinObj_free((ChWinObj *)pWnd->p_LeftPanel);
    ChWinObj_free((ChWinObj *)pWnd->p_MainWindow);
    return TRUE;
}
static LRESULT MainFrame_OnSize(CMainFrame * pWnd, UINT uState, int cx, int cy)
{
    static int aWidth[NUM_STATUS_PANES] = {180, 280};
    HWND hParent = CHWND(pWnd);
    ChWinObj * p_obj = (ChWinObj *)pWnd;
    LPRECT p_rtMargin = &p_obj->rtMargin;
    int cyOutput = 360;

    if(SIZE_MAXHIDE == uState || SIZE_MINIMIZED == uState) return 1;
    if(NULL == pWnd->p_LeftPanel || NULL == pWnd->p_MainWindow) return 1;

    aWidth[NUM_STATUS_PANES - 1] = cx;
    aWidth[NUM_STATUS_PANES - 2] = cx - 100;
    aWidth[NUM_STATUS_PANES - 3] = cx - 200;

    SendMessage(pWnd->base_obj.hStatusBar, SB_SETPARTS, (WPARAM)NUM_STATUS_PANES, (LPARAM)aWidth);

    SetWindowPos(pWnd->base_obj.hStatusBar, hParent, 0, cy - p_obj->rtMargin.bottom, cx, p_obj->rtMargin.bottom, SWP_NOZORDER );

    SetWindowPos(CHWND(pWnd->p_TopPanel), hParent, 0, 0, cx, p_rtMargin->top, SWP_NOZORDER);

    cy -= p_obj->rtMargin.top + p_obj->rtMargin.bottom;
    cx -= p_obj->rtMargin.left +p_obj->rtMargin.right;
    SetWindowPos(CHWND(pWnd->p_LeftPanel), hParent, 0, p_rtMargin->top, p_rtMargin->left, cy -1, SWP_NOZORDER);
    SetWindowPos(pWnd->hOutput, hParent,
                 p_rtMargin->left + 1, p_rtMargin->top, cx, cyOutput, SWP_NOZORDER);
    SetWindowPos(CHWND(pWnd->p_MainWindow), hParent,
                 p_rtMargin->left + 1, p_rtMargin->top + cyOutput, cx, cy - cyOutput - 1, SWP_NOZORDER);
    return 0;
}
static LRESULT MainFrame_OnNotify(CMainFrame * pWnd, LPNMHDR p_nmhdr)
{

    return 0;
}
static LRESULT MainFrame_OnCommand(CMainFrame * pWnd, UINT uID, HWND hWndCtrl, UINT uNotifyCode)
{
    pWnd->base_obj.hActiveDlg = CHWND(pWnd);
    if(uID == IDM_EXIT)
    {
        DestroyWindow(CHWND(pWnd));
    }
    return 0;
}


static BOOL MainFrame_Init(CMainFrame *pWnd)
{
    BOOL rc = FALSE;



    return rc;
//label_errexit:
    return rc;
}

void CALLBACK MainFrame_TimerProc(HWND hWnd,UINT uMsg,UINT_PTR idEvent,DWORD dwTime)
{
    HWND hStatusBar;
    SYSTEMTIME st;
    TCHAR szTime[100] = TEXT("");
    CMainFrame * pWnd = (CMainFrame *)GetWindowLong(hWnd, GWL_THIS);
    if(idEvent != 1) return;
    if(NULL == pWnd) return;
    hStatusBar = pWnd->base_obj.hStatusBar;

    GetLocalTime(&st);
    sprintf(szTime, "%.2d:%.2d:%.2d", st.wHour, st.wMinute, st.wSecond);
    SendMessage(hStatusBar, SB_SETTEXT, 4, (LPARAM)szTime);
    return;
}

static int MainFrame_LogFmt(CMainFrame * pWnd, const char * fmt, ...)
{
    va_list marker;
    int cb = 0;
    char * buffer = CHUTIL_AllocMemory(char, DEFAULT_ALLOC_SIZE + 1);
    if(NULL == buffer) return 0;

    va_start(marker, fmt);
    cb = vsnprintf(buffer, DEFAULT_ALLOC_SIZE, fmt, marker);
    va_end(marker);
    if(cb > 0)
    {
        buffer[cb] = '\0';
        cb = MainFrame_Log(pWnd, buffer);
    }
    free(buffer);
    return cb;
}
static int MainFrame_Log(CMainFrame * pWnd, const char * szLog)
{
    HLOCAL hMem = pWnd->hMem;
    HWND hOutput =  pWnd->hOutput;
    HLOCAL hOldMem;
    uint32_t cbMem = pWnd->cbMem;
    uint32_t cbCurr, cbLog = 0;

    if(NULL == hMem) return 0;

    cbCurr = SendMessage(pWnd->hOutput, WM_GETTEXTLENGTH, 0, 0);
    //cbLog = strlen(szLog);

    if(NULL == szLog || (cbLog = strlen(szLog))== 0)
    {
        SendMessage(hOutput, WM_SETTEXT, 0, 0);
        return 0;
    }




    cbMem = cbLog + cbCurr;
    if(cbMem > pWnd->cbMem)
    {
//        printf("cbCurr = %d, cbLog = %d\n", cbCurr, cbLog);

        cbMem = (cbMem /DEFAULT_ALLOC_SIZE + 1) * DEFAULT_ALLOC_SIZE;


        hOldMem = (HLOCAL)SendMessage(hOutput, EM_GETHANDLE, 0, 0);
        hMem = LocalAlloc(LPTR, cbMem);
        if(NULL == hMem) {
            printf("new mem size: %d\n", cbMem);
            ErrorMsgBox(hOutput, GetLastError(), "local re alloc", 0);
            return 0;

        }
        memcpy(hMem, hOldMem, cbCurr);
        SendMessage(hOutput, EM_SETHANDLE, (WPARAM)hMem, 0);
        LocalFree(hOldMem);

        SendMessage(hOutput, EM_SETLIMITTEXT, cbMem, 0);

        pWnd->hMem = hMem;
        pWnd->cbMem = cbMem;
    }

//    buffer = (char *)hMem;
//    strncpy(buffer + cbCurr, szLog, cbLog + 1);
//    SendMessage(hOutput, EM_SETMODIFY, TRUE, 0);
    SendMessage(hOutput, EM_SETSEL, cbCurr, -1);
    SendMessage(hOutput, EM_REPLACESEL, 0, (LPARAM)szLog);

    return cbLog;
}
