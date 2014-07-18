// mainwindow.c
// Copyright (C) 2014  chehw
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.


#include "MainWindow.h"

static BOOL MainWindow_OnCreate(CMainWindow * pWnd, LPCREATESTRUCT pcs);
static BOOL MainWindow_Destroy(CMainWindow * pWnd);
static LRESULT MainWindow_OnSize(CMainWindow * pWnd, UINT uState, int cx, int cy);
static LRESULT MainWindow_OnNotify(CMainWindow * pWnd, LPNMHDR p_nmhdr);
static LRESULT MainWindow_OnCommand(CMainWindow * pWnd, UINT uID, HWND hWndCtrl, UINT uNotifyCode);

LRESULT MainWindow_WindowProc(ChWinObj * p_obj, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CMainWindow * pWnd = (CMainWindow *)p_obj;
    HWND hWnd = p_obj->hWnd;
    switch(uMsg)
    {
    case WM_SIZE:
        if(MainWindow_OnSize(pWnd, wParam, (short)LOWORD(lParam), (short)HIWORD(lParam))) break;
        return 0;
    case WM_NOTIFY:
        return MainWindow_OnNotify(pWnd, (LPNMHDR)lParam);
        break;
    case WM_COMMAND:
        if(MainWindow_OnCommand(pWnd, LOWORD(wParam), (HWND)lParam, HIWORD(wParam))) break;
        return 0;
    case WM_DESTROY:
        MainWindow_Destroy(pWnd);
        return 0;
    case WM_CREATE:
        if(!MainWindow_OnCreate(pWnd, (LPCREATESTRUCT)lParam)) return -1;
        return 0;
    case WM_LBUTTONDOWN:
        SetFocus(pWnd->base_obj.hWnd);
        return 0;
    default:
        break;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

static BOOL MainWindow_OnCreate(CMainWindow * pWnd, LPCREATESTRUCT pcs)
{
    printf("main window created.\n");
    return TRUE;
}
static BOOL MainWindow_Destroy(CMainWindow * pWnd)
{
    printf("main window destroyed.\n");
    return TRUE;
}
static LRESULT MainWindow_OnSize(CMainWindow * pWnd, UINT uState, int cx, int cy)
{
    return 0;
}
static LRESULT MainWindow_OnNotify(CMainWindow * pWnd, LPNMHDR p_nmhdr)
{
    return 0;
}
static LRESULT MainWindow_OnCommand(CMainWindow * pWnd, UINT uID, HWND hWndCtrl, UINT uNotifyCode)
{
    return 0;
}
