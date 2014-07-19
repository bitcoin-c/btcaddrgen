// main.c
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
#include "base/hWinObj.h"
#include "base/util.h"

//#include <setjmp.h>

#include "MainFrame.h"


CMainFrame * cf;

BOOL InitInstance(HINSTANCE hInstance);
void Release();

//jmp_buf jmp_flags[2];

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPSTR lpszCmdLine, int nCmdShow)
{
    HWND hWnd;
    ChFrame * mainframe;
//    my user defined Structured Exception Handling
//    if(setjmp(jmp_flags[0]) == IDM_EXIT)
//    {
//        PostQuitMessage((0));
//        goto label_exit;
//    }
    if(!InitInstance(hInstance)) return -1;

    mainframe = ChFrame_CreateEx((ChFrame *)cf, 0, TEXT("Bitcoin-c"),
                          WS_OVERLAPPEDWINDOW|WS_CLIPSIBLINGS|WS_CLIPCHILDREN,
                          CW_USEDEFAULT, 0,
                          800, 600,
                          NULL, 0, hInstance, NULL);
    if(NULL == mainframe) return -1;
    printf("cf = %p\nf = %p\n", cf, mainframe);

    hWnd = CHWND(cf);
    if(NULL != hWnd)
    {
        // do some initialize works
        ShowWindow(hWnd, nCmdShow);
        UpdateWindow(hWnd);
        mainframe->Run(mainframe);
    }


//label_exit:
    Release();

    return 0;
}

BOOL InitInstance(HINSTANCE hInstance)
{
    WNDCLASSEX w;
    const ChFrame * templ;
//    WSADATA wsaData;
//    WSAStartup(MAKEWORD(2,2), &wsaData);

    if(!InitChWinObjClasses(hInstance)) return FALSE;

    // WinXP might failed to display statusbar if not call InitCommonControls
    InitCommonControls();

    ChWinObj_GetDefaultWndClass(&w);
    w.lpszMenuName = MAKEINTRESOURCE(IDR_MAINFRAME);
    w.lpszClassName = TEXT("ch_MainFrame");
 //   w.lpfnWndProc = ChFrame_WndProc;


    templ = ChFrame_NewTemplate(0, &w, MainFrame_WindowProc);
    cf = (CMainFrame *)ChFrame_new(sizeof(CMainFrame), templ, NULL);

    return NULL != cf;
}

void Release()
{
    ReleaseChWinObjClasses();
//    WSACleanup();
}


