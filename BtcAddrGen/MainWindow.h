#ifndef MAINWINDOW_H_INCLUDED
#define MAINWINDOW_H_INCLUDED

#include "base/hWinObj.h"

typedef struct _CMainWindow
{
    ChWinObj base_obj;
    HWND hTxList;
    int nListCount;
}CMainWindow;

LRESULT MainWindow_WindowProc(ChWinObj * p_obj, UINT uMsg, WPARAM wParam, LPARAM lParam);

#endif // MAINWINDOW_H_INCLUDED
