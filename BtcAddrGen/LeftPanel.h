#ifndef LEFTPANEL_H_INCLUDED
#define LEFTPANEL_H_INCLUDED

#include "base/hWinObj.h"

typedef struct _CLeftPanel
{
    ChWinObj base_obj;
    HWND hTree;
    HTREEITEM hItemCur;
}CLeftPanel;
LRESULT LeftPanel_WindowProc(ChWinObj * p_obj, UINT uMsg, WPARAM wParam, LPARAM lParam);

#endif // LEFTPANEL_H_INCLUDED
