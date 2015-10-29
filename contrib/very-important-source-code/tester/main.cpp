#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include "resource.h"
#include "rest.h"
#include "npifc.h"

HINSTANCE hInst;
UINT_PTR timer = 0;

VOID CALLBACK TimerProcedure(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
  (void) uMsg;
  (void) idEvent;
  (void) dwTime;
  tir_data_t td;
  npifc_getdata(&td);
  SetDlgItemInt(hwnd, IDC_PITCH, td.pitch, true);
  SetDlgItemInt(hwnd, IDC_ROLL, td.roll, true);
  SetDlgItemInt(hwnd, IDC_YAW, td.yaw, true);

  SetDlgItemInt(hwnd, IDC_X1, td.tx, true);
  SetDlgItemInt(hwnd, IDC_Y1, td.ty, true);
  SetDlgItemInt(hwnd, IDC_Z1, td.tz, true);

  SetDlgItemInt(hwnd, IDC_X2, td.padding[0], true);
  SetDlgItemInt(hwnd, IDC_Y2, td.padding[1], true);
  SetDlgItemInt(hwnd, IDC_Z2, td.padding[2], true);
  SetDlgItemInt(hwnd, IDC_X3, td.padding[3], true);
  SetDlgItemInt(hwnd, IDC_Y3, td.padding[4], true);
  SetDlgItemInt(hwnd, IDC_Z3, td.padding[5], true);
  SetDlgItemInt(hwnd, IDC_S, td.status, true);
  SetDlgItemInt(hwnd, IDC_F, td.frame, true);
}

BOOL CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    (void) lParam;
    switch(uMsg)
    {
        case WM_INITDIALOG:
            SetDlgItemInt(hwndDlg, IDC_APPID, 2307, true);
            return TRUE;

        case WM_CLOSE:
            EndDialog(hwndDlg, 0);
            return TRUE;

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                /*
                 * TODO: Add more control ID's, when needed.
                 */
                case IDQUIT:
                    npifc_close();
                    EndDialog(hwndDlg, 0);
                    return TRUE;
                case IDSTART:
                  int ok;
                  int num = GetDlgItemInt(hwndDlg, IDC_APPID, (BOOL*)&ok, false);
                  if(!ok){
                    num = 2307;
                  }
                  game_desc_t gd;
                  if(timer != 0){
                    KillTimer(hwndDlg, timer);
                    timer = 0;
                  }
                  if(game_data_get_desc(num, &gd)){
                    printf("Application ID: %d - %s\n", num, gd.name);
                    if(npifc_init(hwndDlg, num)){
                      timer = SetTimer(hwndDlg, 0, 50, TimerProcedure);
                    }
                  }else{
                    printf("Unknown Application ID: %d\n", num);
                  }
                  break;

            }
    }

    return FALSE;
}


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
  (void) hPrevInstance;
  (void) lpCmdLine;
  (void) nShowCmd;
  hInst = hInstance;
  
  // The user interface is a modal dialog box
  return DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, (DLGPROC)DialogProc);
}


