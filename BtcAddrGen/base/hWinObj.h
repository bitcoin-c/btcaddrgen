#ifndef HWINOBJ_H_INCLUDED
#define HWINOBJ_H_INCLUDED



#include "compatible.h"
#include "util.h"
#include <commctrl.h>


BOOL InitChWinObjClasses(HINSTANCE hInstance);
BOOL ReleaseChWinObjClasses();

struct _ChWinObj;
typedef enum
{
    GWL_THIS = 0,
    GWL_DATA = 8
}enum_chwinobj;

#define chwinobj_wnd_extra (16)
#define CHWINOBJ_CLASSNAME TEXT("CHWINOBJ_CLASSNAME")
#define CHFRAME_CLASSNAME TEXT("CHFRAME_CLASSNAME")
#define CHFORM_CLASSNAME TEXT("CHFORM_CLASSNAME")

#define ChWinObj_GetHandle(pWnd) ((ChWinObj *)pWnd)->hWnd
#define CHWND(p) (NULL == p)?NULL:(*(HWND *)(p))
typedef LRESULT (* CHWINOBJ_WINDOWPROC)(struct _ChWinObj * pWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


int GetErrorMsg(int nErrCode, LPTSTR lpszOutput, DWORD dwSize);
int ErrorMsgBox(HWND hWnd, int nErrCode, LPCTSTR lpszCaption, UINT uType);

struct _ChWinObj * ChWinObj_AllocMemory(size_t size);
void ChWinObj_FreeMemory(const void * pWnd);

void ChWinObj_GetDefaultWndClass(LPWNDCLASSEX pwcex);




typedef struct _WNDPARAM
{
    struct _ChWinObj * pWnd;
    LPVOID  pvParam;
}WNDPARAM, *LPWNDPARAM;

typedef struct _ChWinObj
{
    HWND hWnd;
    DWORD cbSize;
    ATOM lpszClassName;
    HINSTANCE hInstance;
    BOOL fIsTemplate;
    RECT rtMargin;
    LRESULT (* WindowProc)(struct _ChWinObj * pWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    struct _ChWinObj * (* CreateEx)(const struct _ChWinObj * pWnd, DWORD dwExStyle, LPCTSTR lpszWindowName, DWORD dwStyle, int x, int y, int cx, int cy, HWND hParent, UINT uID, HINSTANCE hInstance, LPVOID lpParam);
}ChWinObj;

const ChWinObj * ChWinObj_NewTemplate(size_t size, LPWNDCLASSEX pwcex, CHWINOBJ_WINDOWPROC ch_windowproc);
void ChWinObj_ReplaceTemplate(struct _ChWinObj ** pp_objTempl, const ChWinObj * p_newTempl);
ChWinObj * ChWinObj_new(size_t size, const ChWinObj * p_objTempl, CHWINOBJ_WINDOWPROC ch_windowproc);
void ChWinObj_free(struct _ChWinObj * pWnd);
struct _ChWinObj * ChWinObj_NewWindow(const struct _ChWinObj * p_objTempl, DWORD dwExStyle, LPCTSTR lpszWindowName, DWORD dwStyle,
                     int x, int y, int cx, int cy, HWND hParent, UINT uID, HINSTANCE hInstance, LPVOID lpParam);
#define ChWinObj_CreateEx ChWinObj_NewWindow
LRESULT CALLBACK ChWinObj_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);




//**********************
typedef struct _ChFrame
{
    struct _ChWinObj base_obj;
    HWND hActiveDlg;
    HACCEL hAccel;
    HWND hStatusBar;

    struct _ChFrame * (*CreateEx)(const struct _ChFrame * p_objTempl, DWORD dwExStyle, LPCTSTR lpszWindowName, DWORD dwStyle, int x, int y, int cx, int cy, HWND hParent, UINT uID, HINSTANCE hInstance, LPVOID lpParam);

    int (*Run)(const struct _ChFrame * p_cf);
    BOOL (*PreTranslateMessage)(const struct _ChFrame * p_cf, LPMSG pmsg);
}ChFrame;


const ChFrame * ChFrame_NewTemplate(size_t size, LPWNDCLASSEX pwcex, CHWINOBJ_WINDOWPROC ch_windowproc);
struct _ChFrame * ChFrame_NewWindow(const ChFrame * p_objTempl, DWORD dwExStyle, LPCTSTR lpszWindowName, DWORD dwStyle,
                     int x, int y, int cx, int cy, HWND hParent, UINT uID, HINSTANCE hInstance, LPVOID lpParam);
#define ChFrame_CreateEx ChFrame_NewWindow
LRESULT CALLBACK ChFrame_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
ChFrame * ChFrame_new(size_t size, const ChFrame * p_objTempl, CHWINOBJ_WINDOWPROC ch_windowproc);
#define ChFrame_free(p) ChWinObj_free((ChWinObj *)p)

extern const struct _ChWinObj * __ch_winobj;
extern const struct _ChWinObj * __ch_form;
extern const struct _ChFrame * __ch_frame;

#endif // HWINOBJ_H_INCLUDED
