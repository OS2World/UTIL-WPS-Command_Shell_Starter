/*-----------------------------------------------------------------------------
;                                 ***keywords*** "%l"
; LOCK STATUS       "PAULG/GO"
;
; project name
; Copyright (c) Paul Gallagher 1995
;
;                                 ***keywords*** "%n"
; Filename          "CSS.H"
; Platform          DOS/Windows (Borland C++ 3.1)
;                   OS/2 (Borland C++ 1.01)
;
; Authors           Paul Gallagher (paulpg@ibm.net)
;
; Description       
;
;                                 ***keywords*** "Version: %v  Date: %d %t"
; "Version: 1  Date: 17-Dec-95 22:03:32"
;
; Revision History
;                                 ***revision-history***
; 1 CSS.H 17-Dec-95,22:03:32,`PAULG/GO' First release
;                                 ***revision-history***
;----------------------------------------------------------------------------*/
                                  //
#ifndef __OS2__
//#ifndef _Windows
//                                ***keywords*** "%n"
#error "CSS.H" is intended for use in OS/2 applications only
//#error "" is intended for use in MS Windows applications only
#endif

#if !defined( pg__GO_H )
#define pg__GO_H

/*-----------------------------------------------------------------------------
;                   OS & C defines and includes
;----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
;                   My lib defines and includes
;----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
;                   Application defines and includes
;----------------------------------------------------------------------------*/
#define ID_APPNAME   1
#define IDS_MSG1   101
#define IDS_MSG2   102
#define IDS_MSG3   103

#define IDM_ABOUT  100

/*-----------------------------------------------------------------------------
;                   Declarations
;----------------------------------------------------------------------------*/

#endif
