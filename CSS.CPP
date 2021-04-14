/*-----------------------------------------------------------------------------
;                                 ***keywords*** "%l"
; LOCK STATUS       "PAULG/GO"
;
; CSS - Command Shell Starter
; Copyright (c) Paul Gallagher 1995
; Source code is released but remains copywrite. Derivative works or
; appropriation of code should be acknowledged by including the statement
; "portions copywrite (c) Paul Gallagher 1995" in your documentation.
;
;                                 ***keywords*** "%n"
; Filename          "CSS.CPP"
; Platform          OS/2 (Borland C++ 1.01)
;
; Authors           Paul Gallagher (paulpg@ibm.net)
;
; Description
;                   (v1.00) css - Command Shell Starter
;                   By dropping a file or folder, opens an
;                   OS/2 Command Window with working directory
;                   preset to that of the dropped file/folder.
;                   With C source. "PostcardWare" - If you get
;                   any value from this app, please send a
;                   lively or colorful postcard to my 2yo
;                   daughter Anita & brighten her day!
;                   (PO Box 5281 Wollongong 2500 Australia)
;                   Author: Paul Gallagher, paulpg@ibm.net
;
;                                 ***keywords*** "Version: %v  Date: %d %t"
; "Version: 1  Date: 17-Dec-95 23:46:48"
;
; Revision History
;                                 ***revision-history***
; 1 CSS.CPP 17-Dec-95,23:46:48,`PAULG/GO' First release
;                                 ***revision-history***
;----------------------------------------------------------------------------*/
                                  //
/*-----------------------------------------------------------------------------
;                   OS & C defines and includes
;----------------------------------------------------------------------------*/
#define INCL_WIN
#define INCL_GPI
#define INCL_DOSSESMGR
#define INCL_DOSFILEMGR
#include <os2.h>
#include <dir.h>
#include <ctype.h>
#include <process.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

/*-----------------------------------------------------------------------------
;                   My lib defines and includes
;----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
;                   Application defines and includes
;----------------------------------------------------------------------------*/
#include "css.h"
#define  UM_DROP WM_USER
#define WX     10
#define WY     10
#define WXSIZE 200
#define WYSIZE 33

/*-----------------------------------------------------------------------------
;                   Declarations
;----------------------------------------------------------------------------*/
MRESULT EXPENTRY ClientWndProc (HWND,ULONG,MPARAM,MPARAM);
void             ReSize(HWND hwnd);
VOID             WorkerThread (void);
MRESULT EXPENTRY WorkWndProc (HWND, ULONG, MPARAM, MPARAM);
void             Msg( PSZ szFormat,... );
void             SetCDtoFilePath(PSZ path);
void             RunCmd(PSZ path);

/*-----------------------------------------------------------------------------
;                   Global variables
;----------------------------------------------------------------------------*/
HAB   hab;
HWND  hWndFrame,
      hWndClient;
HWND  hwndWorker;

CHAR  szTitle[64];
CHAR  szMsg1[60],szMsg2[60],szMsg3[60];

/*-----------------------------------------------------------------------------
;                   Function definitions
;----------------------------------------------------------------------------*/

int
main( int argc, char *argv[] )
{
                                  // if params passed, runcmd then exit using
                                  // 1st param as path
    if ( argc>1 ) {
        PSZ path;
        path=(PSZ)malloc(strlen((PSZ)argv[1]));
        strcpy(path, (PSZ)argv[1]);
        SetCDtoFilePath(path);
        RunCmd(path);
        free(path);
        return (0);
    }

    HMQ   hmq;
    QMSG  qmsg;
    ULONG flFrameFlags    = FCF_TITLEBAR | FCF_SYSMENU  | FCF_SIZEBORDER |
                            FCF_MINMAX   | FCF_TASKLIST | FCF_ICON;
    CHAR  szClientClass[] = "CLIENT";

    hab = WinInitialize (0);
    hmq = WinCreateMsgQueue (hab, 0);

    WinRegisterClass (hab, szClientClass, (PFNWP)ClientWndProc, 0, 0);
    WinLoadString (hab, 0, ID_APPNAME, sizeof(szTitle), szTitle);
    WinLoadString (hab, 0, IDS_MSG1, sizeof(szMsg1), szMsg1);
    WinLoadString (hab, 0, IDS_MSG2, sizeof(szMsg2), szMsg2);
    WinLoadString (hab, 0, IDS_MSG3, sizeof(szMsg3), szMsg3);

    hWndFrame = WinCreateStdWindow (HWND_DESKTOP, 0,
        &flFrameFlags, szClientClass, szTitle, 0, 0, ID_APPNAME, &hWndClient);
    ReSize(hWndFrame);
    WinSetWindowPos(hWndFrame, HWND_TOP,0,0,0,0, SWP_SHOW | SWP_ACTIVATE | SWP_ZORDER);

    while (WinGetMsg (hab, &qmsg, 0, 0, 0))
        WinDispatchMsg (hab, &qmsg);

    WinDestroyWindow (hWndFrame);
    WinDestroyMsgQueue (hmq);
    WinTerminate (hab);
    return (0);
}

/*-----------------------------------------------------------------------------
;                   ReSize
;----------------------------------------------------------------------------*/
void
ReSize(HWND hwnd)
{
   LONG menuHeight,captionHeight,borderHeight,borderWidth;
   LONG cx,cy;

   captionHeight = WinQuerySysValue(HWND_DESKTOP, SV_CYTITLEBAR);
   borderHeight = WinQuerySysValue(HWND_DESKTOP, SV_CYDLGFRAME) * 2;
   borderWidth = WinQuerySysValue(HWND_DESKTOP, SV_CXDLGFRAME) * 2;
   menuHeight = WinQuerySysValue(HWND_DESKTOP, SV_CYMENU);

   cx = borderWidth + WXSIZE;
   cy = menuHeight + captionHeight + borderHeight + WYSIZE;

   WinSetWindowPos(hwnd, 0, WX, WY, cx, cy, SWP_MOVE | SWP_SIZE);
   return;
}

/*-----------------------------------------------------------------------------
;                   ClientWndProc
;----------------------------------------------------------------------------*/
MRESULT EXPENTRY
ClientWndProc (HWND hWnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{

    HPS     hps;
    BOOL    bHandled = TRUE;
    MRESULT mReturn  = 0;

    switch (msg)
    {

        case DM_DRAGOVER:
            mReturn = MRFROM2SHORT(DOR_DROP, DO_UNKNOWN);
            break;

        case DM_DROP:
            {
#           define TEMPSTRLEN 255
                                  // DRAGINFO struct ptr
            PDRAGINFO pDInfo;
                                  // DRAGITEM struct ptr
            PDRAGITEM pDItem;
                                  // buffers
            PSZ     pContainer;
            PSZ     pSource;

            pContainer=(PSZ)malloc(TEMPSTRLEN);
            pSource=(PSZ)malloc(TEMPSTRLEN);

            if ((pContainer) && (pSource))  {

                                  // Get DRAGINFO pointer
               pDInfo = (PDRAGINFO)mp1;
                                  // Access DRAGINFO
               DrgAccessDraginfo(pDInfo);
                                  // Access DRAGITEM, index 0 (first)
               pDItem = DrgQueryDragitemPtr(pDInfo, 0);
               DrgQueryStrName(pDItem->hstrContainerName,TEMPSTRLEN,pContainer);
               DrgQueryStrName(pDItem->hstrSourceName,TEMPSTRLEN,pSource);
               WinPostMsg(hwndWorker, UM_DROP, (PVOID)pContainer, (PVOID)pSource);
               DrgDeleteDraginfoStrHandles(pDInfo);
               DrgFreeDraginfo(pDInfo);
            }
            }
            break;

        case WM_PAINT:
                                  // update region
            RECTL  rcl;

            hps = WinBeginPaint (hWnd,0,0);
                                  // get window dimensions
            WinQueryWindowRect(hWnd, &rcl);

                                  // clear entire window
            WinFillRect(hps, &rcl, CLR_PINK);

            WinDrawText(hps, strlen(szMsg1), szMsg1, &rcl, 0L, 0L,
              DT_TOP | DT_LEFT | DT_TEXTATTRS);
            rcl.yTop -= 15;
            WinDrawText(hps, strlen(szMsg2), szMsg2, &rcl, 0L, 0L,
              DT_TOP | DT_LEFT | DT_TEXTATTRS);
            rcl.yTop -= 15;
            WinDrawText(hps, strlen(szMsg3), szMsg3, &rcl, 0L, 0L,
              DT_TOP | DT_LEFT | DT_TEXTATTRS);

            WinEndPaint (hps);
            break;

        case WM_ERASEBACKGROUND:
            mReturn = MRFROMLONG(1L);
            break;

        case WM_CREATE:
            if (_beginthread ( (void (*)(void *)) WorkerThread, 8192, NULL) == -1)
            {
	       WinMessageBox (HWND_DESKTOP, HWND_DESKTOP,
	           "Creation of second thread failed!", "Step 2",
                   0, MB_OK | MB_CUACRITICAL);
            }
            break;

        default:
            bHandled = FALSE;
            break;
    }

    if (!bHandled)
        mReturn = WinDefWindowProc (hWnd,msg,mp1,mp2);

    return (mReturn);
}

/*-----------------------------------------------------------------------------
; background thread
;----------------------------------------------------------------------------*/
void WorkerThread (void)
{
   HAB  habWork;
   HMQ  hmqWork;
   QMSG qmsg;

   habWork = WinInitialize (0);

   hmqWork = WinCreateMsgQueue(habWork, 0);

   WinRegisterClass(habWork, "WATCH", WorkWndProc, 0, 0);

   hwndWorker = WinCreateWindow( HWND_OBJECT, "WATCH", "",
     0, 0, 0, 0, 0, HWND_OBJECT, HWND_BOTTOM, 0, NULL, NULL );

   while( WinGetMsg ( habWork, &qmsg, 0, 0, 0 ))
     WinDispatchMsg ( habWork, &qmsg );

   WinPostMsg( hWndClient, WM_QUIT, 0, 0 );

   WinDestroyWindow( hwndWorker );
   WinDestroyMsgQueue( hmqWork );
   WinTerminate (habWork);
   _endthread ();
}

/*-----------------------------------------------------------------------------
; background thread window procedure
;----------------------------------------------------------------------------*/
MRESULT EXPENTRY
WorkWndProc(HWND hWnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   BOOL    bHandled = TRUE;
   MRESULT mReturn  = 0;
   PSZ     path;

   switch (msg)
   {
      case UM_DROP:
         path=(PSZ)malloc(strlen((PSZ)mp1)+strlen((PSZ)mp2));
         if (path) {
           strcpy(path,(PSZ)mp1);
           strcat(path,(PSZ)mp2);
           SetCDtoFilePath(path);
           RunCmd(path);
           free(path);
         }
         if (mp1) free(mp1);
         if (mp2) free(mp2);
         break;
      case WM_DESTROY:
         break;
   }
   if (!bHandled)
      mReturn = WinDefWindowProc (hWnd,msg,mp1,mp2);

   return (mReturn);
}

/*-----------------------------------------------------------------------------
; change current directory to that of the dir/file passed in param
;----------------------------------------------------------------------------*/
void
SetCDtoFilePath(PSZ path)
{
   PSZ s;
   ULONG        PathInfoLevel;   /* Data required */
   FILESTATUS3  PathInfoBuf;     /* File info buffer */
   ULONG        PathInfoBufSize; /* Data buffer size */
   APIRET       rc;              /* Return code */

   if (path[strlen(path)-1]=='\\') path[strlen(path)-1]='\0';

   PathInfoLevel = 1;    /* Indicate that Level 1 information */
                          /*   is desired                      */

   PathInfoBufSize = sizeof(FILESTATUS3);
                          /* Size of the buffer that will      */
                          /*   receive the Level 1 information */

   rc = DosQueryPathInfo(path, PathInfoLevel, &PathInfoBuf, PathInfoBufSize);
                          /* On successful return, the Level 1 */
                          /*   directory information is in the */
                          /*   PathInfoBuf buffer              */

   if (rc != 0) {
      Msg("DosQueryPathInfo error: return code = %ld", rc);
      return;
   }

   char drive[MAXDRIVE];
   char dir[MAXDIR];
   char file[MAXFILE];
   char ext[MAXEXT];
   int flags;

   if (PathInfoBuf.attrFile & FILE_DIRECTORY) {
      setdisk(toupper(path[0])-(int)'A');
   } else {
      flags = fnsplit(path,drive,dir,file,ext);
      path[0]='\0';
      if(flags & DRIVE)
        strcat(path,drive);
      if(flags & DIRECTORY)
        strcat(path,dir);
      if (path[strlen(path)-1]=='\\') path[strlen(path)-1]='\0';
      setdisk(toupper(drive[0])-(int)'A');
   }
   chdir(path);
   return;
}

/*-----------------------------------------------------------------------------
; start command shell
;----------------------------------------------------------------------------*/
void
RunCmd(PSZ path)
{
   STARTDATA   StartData;         // Start session data structure
   ULONG       SessID;            // Session ID (returned)
   PID         PID;               // Process ID (returned)
   UCHAR       ObjBuf[100];       // Object buffer
   PSZ         PgmTitle;

                                  //  Specify the various session start parameters

                                  // Length of STARTDATA structure
   StartData.Length = sizeof(STARTDATA);
                                  // unrelated session
   StartData.Related = SSF_RELATED_INDEPENDENT;
                                  // Start child session in foreground
   StartData.FgBg = SSF_FGBG_FORE;
                                  // Don't trace session
   StartData.TraceOpt = SSF_TRACEOPT_NONE;

                                  // Session Title string
   PgmTitle=(PSZ)malloc(strlen(path)+18);
   strcpy(PgmTitle,"Command Prompt - ");
   strcat(PgmTitle,path);
   StartData.PgmTitle = PgmTitle;

                                  // Program path-name string
   StartData.PgmName = NULL;
                                  // Assume no input arguments need be passed
                                  // to the program
   StartData.PgmInputs = 0;
                                  // Assume no environment string
   StartData.Environment = 0;
                                  // Inherit environment and open file handles
                                  // from parent
   StartData.InheritOpt = SSF_INHERTOPT_PARENT;
                                  // Allow the Shell to establish the session type
   StartData.SessionType = SSF_TYPE_WINDOWABLEVIO;
                                  // Assume no specific icon file is provided
   StartData.IconFile = 0;
                                  // Do not use the installation file
   StartData.PgmHandle = 0;
                                  // Start the program as visible and maximized
   StartData.PgmControl = SSF_CONTROL_VISIBLE;
                                  // Initial window coordinates and size
   StartData.InitXPos = 30;
   StartData.InitYPos = 40;
   StartData.InitXSize = 200;
   StartData.InitYSize = 140;
                                  // Reserved, must be zero
   StartData.Reserved = 0;
                                  // Object buffer to hold DosExecPgm
                                  // failure causes
   StartData.ObjectBuffer = ObjBuf;
                                  // Size of object buffer
   StartData.ObjectBuffLen = 100;
                                  // Start session
   DosStartSession(&StartData, &SessID, &PID);
// freeing PgmTitle causes problems - we seem to have lost 'ownership', but I
// haven't found this documented anywhere. Can anyone clarify?
//   free(PgmTitle);
   return;
}

/*-----------------------------------------------------------------------------
  DISPLAY A MESSAGE TO THE USER.

  PARMS: a message in printf format with its parms

  NOTES:

  RETURNS: nothing
;----------------------------------------------------------------------------*/
#define MESSAGE_SIZE 200
void
Msg( PSZ szFormat,... )
{
    PSZ     szMsg;
    va_list argptr;

    szMsg = (PSZ) malloc( MESSAGE_SIZE );
    if( szMsg )
    {
        va_start( argptr, szFormat );
        vsprintf( szMsg, szFormat, argptr );
        va_end( argptr );

        szMsg[ MESSAGE_SIZE - 1 ] = 0;

        WinAlarm( HWND_DESKTOP, WA_WARNING );
        WinMessageBox(  HWND_DESKTOP, HWND_DESKTOP, szMsg,
                        szTitle, 1,
                        MB_OK | MB_MOVEABLE );
        free( szMsg );
    }
    else
    {
        DosBeep( 1000, 1000 );
        return;
    }
}


