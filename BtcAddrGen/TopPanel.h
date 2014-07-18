#ifndef TOPPANEL_H_INCLUDED
#define TOPPANEL_H_INCLUDED

#include "base/compatible.h"
#include "base/datatype.h"
#include "base/hWinObj.h"

typedef struct _CTopPanel
{
    struct _ChWinObj base_obj;
    HWND hInputBox;
    HWND hSubmit;
}CTopPanel;
LRESULT TopPanel_WindowProc(ChWinObj * p_obj, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif // TOPPANEL_H_INCLUDED
