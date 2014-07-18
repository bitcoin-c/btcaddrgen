#include "hWinObj.h"
#include <winuser.h>

const struct _ChWinObj * __ch_winobj;
const struct _ChWinObj * __ch_form;
const struct _ChFrame  * __ch_frame;
static BOOL __chwinobj_init = FALSE;

static LRESULT ChFrame_WindowProc(ChWinObj * pWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static LRESULT ChWinObj_WindowProc(ChWinObj * pWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int GetErrorMsg(int nErrCode, LPTSTR lpszOutput, DWORD dwSize)
{
    int cb = 0;
    if(nErrCode == 0) nErrCode = GetLastError();
    cb = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL,
                       nErrCode,
                       MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                       lpszOutput, dwSize, NULL);
    return cb;
}

int ErrorMsgBox(HWND hWnd, int nErrCode, LPCTSTR lpszCaption, UINT uType)
{
    int rc = IDCANCEL;
    TCHAR szText[MAX_PATH + 100];
    TCHAR szErrMsg[MAX_PATH + 1] = "";
    if(GetErrorMsg(nErrCode, szErrMsg, MAX_PATH) > 0)
    {
        wsprintf(szText, TEXT("Error %ld(%0x): %s"), nErrCode, nErrCode, szErrMsg);
        rc = MessageBox(hWnd, szText, lpszCaption, uType);
    }
    return rc;
}

struct _ChWinObj * ChWinObj_AllocMemory(size_t size)
{
    if(size < sizeof(ChWinObj)) size = sizeof(ChWinObj);
//  return (ChWinObj *)calloc(size, 1);
    return (ChWinObj *)LocalAlloc(LPTR, size);
}
void ChWinObj_FreeMemory(const void * p)
{
    ChWinObj * pWnd = (ChWinObj *)p;
    if(NULL != pWnd)
    {
        if(!pWnd->fIsTemplate) LocalFree((HLOCAL)pWnd);
    }
}

ATOM ChWinObj_Register(HINSTANCE hInstance, LPCTSTR lpszClassName, LPCTSTR lpszMenuName, WNDPROC wndProc, BOOL fIsMainFrame)
{
    ATOM rc = 0;
    WNDCLASSEX w;
    if(NULL == lpszClassName) lpszClassName = CHWINOBJ_CLASSNAME;
    if(NULL == wndProc) wndProc = fIsMainFrame ? ChFrame_WndProc : ChWinObj_WndProc;

    w.cbSize = sizeof(WNDCLASSEX);
    rc = GetClassInfoEx(hInstance, lpszClassName, &w);
    if(rc) // if the class has already been registered, then unregister it.
    {
        return rc;
    }

    w.cbSize = sizeof(WNDCLASSEX);
    w.style = 0;
    w.lpfnWndProc = wndProc;
    w.cbClsExtra = 0;
    w.cbWndExtra = chwinobj_wnd_extra;
    w.hInstance = GetModuleHandle(NULL);
    w.hIcon = LoadIcon(NULL, IDI_WINLOGO);
    w.hCursor = LoadCursor(NULL, IDC_ARROW);
    w.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    w.lpszMenuName = lpszMenuName;
    w.lpszClassName = lpszClassName;
    w.hIconSm = LoadIcon(NULL, IDI_WINLOGO);


    rc = RegisterClassEx(&w);
    return rc;
}

void ChWinObj_GetDefaultWndClass(LPWNDCLASSEX pwcex)
{
    assert(NULL != pwcex);
    pwcex->cbSize = sizeof(WNDCLASSEX);
    pwcex->style = 0;
    pwcex->lpfnWndProc = NULL; //ChWinObj_WndProc;
    pwcex->cbClsExtra = 0;
    pwcex->cbWndExtra = chwinobj_wnd_extra;
    pwcex->hInstance = GetModuleHandle(NULL);
    pwcex->hIcon = LoadIcon(NULL, IDI_WINLOGO);
    pwcex->hCursor = LoadCursor(NULL, IDC_ARROW);
    pwcex->hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    pwcex->lpszClassName = NULL; // CHWINOBJ_CLASSNAME;
    pwcex->lpszMenuName = NULL;
    pwcex->hIconSm = LoadIcon(NULL, IDI_WINLOGO);
}

const ChWinObj * ChWinObj_NewTemplate
    (
    size_t size,
    LPWNDCLASSEX pwcex, // use for register class
    CHWINOBJ_WINDOWPROC ch_windowproc // for class specialized window_proc
    )
{
    ATOM a = 0;
    WNDCLASSEX w ={sizeof(WNDCLASSEX)};
    struct _ChWinObj * p_obj = NULL;

    if(size < sizeof(ChWinObj)) size = sizeof(ChWinObj);

    if(NULL == pwcex)
    {
        ChWinObj_GetDefaultWndClass(&w);
    }else
    {
        memcpy(&w, pwcex, sizeof(WNDCLASSEX));
    }

    if(NULL == w.lpfnWndProc) w.lpfnWndProc = ChWinObj_WndProc;
    if(NULL == ch_windowproc) ch_windowproc = ChWinObj_WindowProc;

    a = RegisterClassEx(&w);
    if(a == 0) return NULL;

    p_obj = ChWinObj_AllocMemory(size);
    if(NULL == p_obj) return NULL;

    p_obj->cbSize = size;
    p_obj->lpszClassName = (LPCTSTR)((uint32_t)a);
    p_obj->hInstance = w.hInstance;
    p_obj->fIsTemplate = TRUE;

    p_obj->WindowProc = ch_windowproc;
    p_obj->CreateEx = ChWinObj_NewWindow;

    return p_obj;
};

void ChWinObj_ReplaceTemplate(ChWinObj ** pp_objTempl, const ChWinObj * p_newTempl)
{
    assert(NULL != pp_objTempl);
    if(NULL == p_newTempl) return;

    ChWinObj * p_oldTempl = (ChWinObj *)* pp_objTempl;
    if(NULL != p_oldTempl)
    {
        UnregisterClass(MAKEINTRESOURCE(p_oldTempl->lpszClassName), p_oldTempl->hInstance);
        p_oldTempl->fIsTemplate = FALSE;
        ChWinObj_FreeMemory(p_oldTempl);
    }
    *pp_objTempl = (ChWinObj *)p_newTempl;
}

BOOL InitChWinObjClasses(HINSTANCE hInstance)
{
    WNDCLASSEX w;
    if(__chwinobj_init) return TRUE;

    w.cbSize = sizeof(WNDCLASSEX);
    w.style = 0;
    w.lpfnWndProc = ChWinObj_WndProc;
    w.cbClsExtra = 0;
    w.cbWndExtra = chwinobj_wnd_extra;
    w.hInstance = GetModuleHandle(NULL);
    w.hIcon = LoadIcon(NULL, IDI_WINLOGO);
    w.hCursor = LoadCursor(NULL, IDC_ARROW);
    w.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    w.lpszClassName = CHWINOBJ_CLASSNAME;
    w.lpszMenuName = NULL;
    w.hIconSm = LoadIcon(NULL, IDI_WINLOGO);

    __ch_winobj = ChWinObj_NewTemplate(sizeof(ChWinObj), &w, ChWinObj_WindowProc);
    if(NULL == __ch_winobj) return FALSE;

    w.hbrBackground = (HBRUSH)(COLOR_APPWORKSPACE + 1);
    w.lpszClassName = CHFRAME_CLASSNAME;
#ifdef IDR_MAINFRAME
    w.lpszMenuName = (LPCTSTR)IDR_MAINFRAME;
#endif // IDR_MAINFRAME
    w.lpfnWndProc = ChFrame_WndProc;
    __ch_frame = ChFrame_NewTemplate(sizeof(ChFrame), &w, ChFrame_WindowProc);
    if(NULL == __ch_frame) return FALSE;

    w.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    w.lpszClassName = CHFORM_CLASSNAME;
    w.lpfnWndProc = ChWinObj_WndProc;
    w.lpszMenuName = NULL;
    __ch_form = ChWinObj_NewTemplate(sizeof(ChWinObj), &w, ChWinObj_WindowProc);
    if(NULL == __ch_form) return FALSE;

    __chwinobj_init = TRUE;
    return TRUE;
}
BOOL ReleaseChWinObjClasses()
{
    if(NULL != __ch_frame)
    {
        UnregisterClass(MAKEINTRESOURCE(__ch_frame->base_obj.lpszClassName), __ch_frame->base_obj.hInstance);
        LocalFree((HLOCAL)__ch_frame);
        __ch_frame = NULL;
    }
    if(NULL != __ch_winobj)
    {
        UnregisterClass(MAKEINTRESOURCE(__ch_winobj->lpszClassName), __ch_winobj->hInstance);
        LocalFree((HLOCAL)__ch_winobj);
        __ch_winobj = NULL;
    }
    if(NULL != __ch_form)
    {
        UnregisterClass(MAKEINTRESOURCE(__ch_form->lpszClassName), __ch_form->hInstance);
        LocalFree((HLOCAL)__ch_form);
        __ch_form = NULL;
    }

    __chwinobj_init = FALSE;
    return TRUE;
}

static LRESULT ChWinObj_WindowProc(ChWinObj * pWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if(uMsg == WM_DESTROY)
    {

        return 0;
    }else if(uMsg == WM_CREATE)
    {

        return 0;
    }
    return DefWindowProc(ChWinObj_GetHandle(pWnd), uMsg, wParam, lParam);
}

LRESULT CALLBACK ChWinObj_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    struct _ChWinObj * pWnd = (struct _ChWinObj *)GetWindowLong(hWnd, GWL_THIS);
    switch(uMsg)
    {
    case WM_NCCREATE:
    case WM_CREATE:
        if(NULL == pWnd)
        {
            LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
            LPWNDPARAM pWndParam = (LPWNDPARAM)pcs->lpCreateParams;
            if(NULL != pWndParam)
            {
                pWnd = pWndParam->pWnd;
                if(NULL != pWnd)
                {
                    pWnd->hWnd = hWnd;
                    SetWindowLong(hWnd, GWL_THIS, (LONG)pWnd);
                    if(NULL == pWnd->WindowProc) pWnd->WindowProc = ChWinObj_WindowProc;
                }
                pcs->lpCreateParams = pWndParam->pvParam;
                free(pWndParam);
                pWndParam = NULL;
            }
        }
        if(uMsg == WM_NCCREATE) break; //return TRUE;
        break;
    case WM_DESTROY:
        if(NULL != pWnd)
        {
            pWnd->WindowProc(pWnd, uMsg, wParam, lParam);
            SetWindowLong(hWnd, GWL_THIS, 0);
            pWnd->hWnd = NULL;
            ChWinObj_FreeMemory(pWnd);
            pWnd = NULL;
            return 0;
        }
        break;
    default:
        break;
    }
    if(NULL != pWnd) return pWnd->WindowProc(pWnd, uMsg, wParam, lParam);
    return DefWindowProc(hWnd, uMsg, wParam, lParam);

}

struct _ChWinObj * ChWinObj_NewWindow(const struct _ChWinObj * p_ObjTempl, DWORD dwExStyle, LPCTSTR lpszWindowName, DWORD dwStyle,
                     int x, int y, int cx, int cy, HWND hParent, UINT uID, HINSTANCE hInstance, LPVOID lpParam)
{
    assert(__chwinobj_init);
    HWND hWnd;
    LPWNDPARAM pWndParam = NULL;
    struct _ChWinObj * pWnd =  NULL;

    if(NULL == p_ObjTempl) p_ObjTempl = __ch_winobj;

    pWndParam = (LPWNDPARAM)calloc(sizeof(WNDPARAM), 1);
    if(NULL == pWndParam) return NULL;

    pWnd = (ChWinObj *)p_ObjTempl;
    if(p_ObjTempl->fIsTemplate)
    {
        pWnd = ChWinObj_AllocMemory(p_ObjTempl->cbSize);
        if(NULL == pWnd)
        {
            free(pWndParam);
            return NULL;
        }
        memcpy(pWnd, p_ObjTempl, p_ObjTempl->cbSize);
        pWnd->fIsTemplate = FALSE;
    }

    pWndParam->pWnd = pWnd;
    pWndParam->pvParam = lpParam;

    if(NULL == hInstance) hInstance = pWnd->hInstance;



    hWnd = CreateWindowEx(dwExStyle, (LPCTSTR)MAKEINTRESOURCE(pWnd->lpszClassName), lpszWindowName, dwStyle,
                          x, y, cx, cy,
                          hParent, (HMENU)uID, hInstance, pWndParam);
    if(NULL == hWnd)
    {
        free(pWndParam);
        ChWinObj_FreeMemory(pWnd);
        return NULL;
    }
    return pWnd;
}

ChWinObj * ChWinObj_new(size_t size, const ChWinObj * p_objTempl, CHWINOBJ_WINDOWPROC ch_windowproc)
{
    assert(__chwinobj_init);
    ChWinObj * pWnd = NULL;

    if(NULL == p_objTempl) p_objTempl = __ch_winobj;
    if(size < p_objTempl->cbSize) size = p_objTempl->cbSize;

    pWnd = ChWinObj_AllocMemory(size);
    if(NULL == pWnd) return NULL;

    memcpy(pWnd, p_objTempl, p_objTempl->cbSize);
    pWnd->fIsTemplate = FALSE;
    if(NULL != ch_windowproc) pWnd->WindowProc = ch_windowproc;
    return pWnd;
}

void ChWinObj_free(struct _ChWinObj * pWnd)
{
    if(NULL != pWnd)
    {
        if(NULL != pWnd->hWnd)
        {
            DestroyWindow(pWnd->hWnd);
        }else
        {
            ChWinObj_FreeMemory(pWnd);
        }
    }
}


//***************************************************
//** ChFrame

static LRESULT ChFrame_WindowProc(ChWinObj * pWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
//    ChFrame * p_cf = (ChFrame *)pWnd;
    HWND hWnd = ChWinObj_GetHandle(pWnd);

    if(uMsg == WM_DESTROY)
    {
        return 0;
    }else if(uMsg == WM_CREATE)
    {
        return 0;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK ChFrame_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    struct _ChWinObj * pWnd = (struct _ChWinObj *)GetWindowLong(hWnd, GWL_THIS);
    switch(uMsg)
    {
    case WM_NCCREATE:
    case WM_CREATE:
        if(NULL == pWnd)
        {
            LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
            LPWNDPARAM pWndParam = (LPWNDPARAM)pcs->lpCreateParams;
            if(NULL != pWndParam)
            {
                pWnd = pWndParam->pWnd;
                if(NULL != pWnd)
                {
                    pWnd->hWnd = hWnd;
                    SetWindowLong(hWnd, GWL_THIS, (LONG)pWnd);
                    if(NULL == pWnd->WindowProc) pWnd->WindowProc = (CHWINOBJ_WINDOWPROC)ChFrame_WindowProc;
                }
                pcs->lpCreateParams = pWndParam->pvParam;
                free(pWndParam);
                pWndParam = NULL;
            }
        }

        if(uMsg == WM_NCCREATE) break;
        break;
    case WM_DESTROY:
        if(NULL != pWnd)
        {
            pWnd->WindowProc(pWnd, uMsg, wParam, lParam);
            SetWindowLong(hWnd, GWL_THIS, 0);
            pWnd->hWnd = NULL;
            ChWinObj_FreeMemory(pWnd);
            pWnd = NULL;
            PostQuitMessage(0);
            printf("post quit message.\n");
            return 0;
        }
        break;
    default:
        break;
    }
    if(NULL != pWnd) return pWnd->WindowProc(pWnd, uMsg, wParam, lParam);
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}



int ChFrame_Run(const ChFrame * p_cf)
{
    MSG msg;
    int rc = -1;
    while((rc = GetMessage(&msg, NULL, 0, 0 ) != 0))
    {
        if(rc == -1) return rc;
        if(!p_cf->PreTranslateMessage(p_cf, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

    }
    return msg.wParam;
}

BOOL ChFrame_PreTranslateMessage(const ChFrame * p_cf, LPMSG pmsg)
{
    if(NULL != p_cf->hAccel)
    {
        if(TranslateAccelerator(p_cf->base_obj.hWnd, p_cf->hAccel, pmsg)) return TRUE;
    }
    if(NULL != p_cf->hActiveDlg)
    {
        if(IsDialogMessage(p_cf->hActiveDlg, pmsg)) return TRUE;
    }
    return FALSE;
}


const ChFrame * ChFrame_NewTemplate(size_t size,LPWNDCLASSEX pwcex,CHWINOBJ_WINDOWPROC ch_windowproc)
{
    ChFrame * p_frame = NULL;
    WNDCLASSEX w;
    if(NULL == pwcex)
    {
        ChWinObj_GetDefaultWndClass(&w);
    }else
    {
        memcpy(&w, pwcex, sizeof(WNDCLASSEX));

    }
    if(NULL == w.lpfnWndProc) w.lpfnWndProc = ChFrame_WndProc;

    if(NULL == ch_windowproc) ch_windowproc = (CHWINOBJ_WINDOWPROC)ChFrame_WindowProc;
    if(size < sizeof(ChFrame)) size = sizeof(ChFrame);

    p_frame = (ChFrame *)ChWinObj_NewTemplate(size, &w, ch_windowproc);
    if(NULL == p_frame) return NULL;

    p_frame->CreateEx = ChFrame_CreateEx;
    p_frame->PreTranslateMessage = ChFrame_PreTranslateMessage;
    p_frame->Run = ChFrame_Run;
    return p_frame;

}

struct _ChFrame * ChFrame_NewWindow(const ChFrame * p_objTempl, DWORD dwExStyle, LPCTSTR lpszWindowName, DWORD dwStyle,
                     int x, int y, int cx, int cy, HWND hParent, UINT uID, HINSTANCE hInstance, LPVOID lpParam)
{
    if(!__chwinobj_init)
        if(!InitChWinObjClasses(hInstance)) return NULL;
    if(NULL == p_objTempl) p_objTempl = __ch_frame;
    return (ChFrame *)ChWinObj_NewWindow((ChWinObj *)p_objTempl, dwExStyle, lpszWindowName, dwStyle,
                                   x, y, cx, cy, hParent, uID, hInstance, lpParam);
}

ChFrame * ChFrame_new(size_t size, const ChFrame * p_objTempl, CHWINOBJ_WINDOWPROC ch_windowproc)
{
    assert(__chwinobj_init);
    ChFrame * pWnd = NULL;

    if(NULL == p_objTempl) p_objTempl = (ChFrame *)__ch_frame;
    if(size < p_objTempl->base_obj.cbSize) size = p_objTempl->base_obj.cbSize;

    pWnd = (ChFrame *)ChWinObj_new(size, (ChWinObj *)p_objTempl, ch_windowproc);
    return pWnd;
}
