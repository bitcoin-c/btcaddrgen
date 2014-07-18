#ifndef MAINFRAME_H_INCLUDED
#define MAINFRAME_H_INCLUDED

#include "base/hWinObj.h"
#include "MainWindow.h"
#include "LeftPanel.h"
#include "TopPanel.h"

#define DEFAULT_ALLOC_SIZE  (128 * 1024)  //(4 * 1024 * 1024)


typedef struct _CMainFrame
{
    struct _ChFrame base_obj;
    CTopPanel * p_TopPanel;
    CLeftPanel * p_LeftPanel;
    CMainWindow * p_MainWindow;
    HWND hOutput;
    HLOCAL hMem;
    uint32_t cbMem;

    int (* Log)(struct _CMainFrame *, const char *);
}CMainFrame;

LRESULT MainFrame_WindowProc(ChWinObj * p_obj, UINT uMsg, WPARAM wParam, LPARAM lParam);

#endif // MAINFRAME_H_INCLUDED
