// leftpanel.c
// Copyright (C) 2014  chehw
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

#include "resource.h"
#include "base/compatible.h"
#include "base/datatype.h"

#include "LeftPanel.h"




static BOOL LeftPanel_OnCreate(CLeftPanel * pWnd, LPCREATESTRUCT pcs);
static BOOL LeftPanel_Destroy(CLeftPanel * pWnd);
static LRESULT LeftPanel_OnSize(CLeftPanel * pWnd, UINT uState, int cx, int cy);
static LRESULT LeftPanel_OnNotify(CLeftPanel * pWnd, LPNMHDR p_nmhdr);
static LRESULT LeftPanel_OnCommand(CLeftPanel * pWnd, UINT uID, HWND hWndCtrl, UINT uNotifyCode);

LRESULT LeftPanel_WindowProc(ChWinObj * p_obj, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CLeftPanel * pWnd = (CLeftPanel *)p_obj;
    HWND hWnd = p_obj->hWnd;
    switch(uMsg)
    {
    case WM_SIZE:
        if(LeftPanel_OnSize(pWnd, wParam, (short)LOWORD(lParam), (short)HIWORD(lParam))) break;
        return 0;
    case WM_NOTIFY:
        return LeftPanel_OnNotify(pWnd, (LPNMHDR)lParam);
        break;
    case WM_COMMAND:
        if(LeftPanel_OnCommand(pWnd, LOWORD(wParam), (HWND)lParam, HIWORD(wParam))) break;
        return 0;
    case WM_DESTROY:
        LeftPanel_Destroy(pWnd);
        return 0;
    case WM_CREATE:
        if(!LeftPanel_OnCreate(pWnd, (LPCREATESTRUCT)lParam)) return -1;
        return 0;
    default:
        break;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

static BOOL LeftPanel_OnCreate(CLeftPanel * pWnd, LPCREATESTRUCT pcs)
{
    printf("left panel create.\n");
    return TRUE;
}
static BOOL LeftPanel_Destroy(CLeftPanel * pWnd)
{
    printf("left panel destoryed.\n");
    return TRUE;
}

static LRESULT LeftPanel_OnSize(CLeftPanel * pWnd, UINT uState, int cx, int cy)
{
    return 0;
}
static LRESULT LeftPanel_OnNotify(CLeftPanel * pWnd, LPNMHDR p_nmhdr)
{
    return 0;
}
static LRESULT LeftPanel_OnCommand(CLeftPanel * pWnd, UINT uID, HWND hWndCtrl, UINT uNotifyCode)
{
    return 0;
}
