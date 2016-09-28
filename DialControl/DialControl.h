// ****************************************************************************
// By Antonino Perricone
// http://amicoperry.altervista.org/luce/
//
// This work is licensed under the Creative Commons 
// Attribution-NonCommercial-ShareAlike 3.0 Unported License. 
// To view a copy of this license, visit 
// http://creativecommons.org/licenses/by-nc-sa/3.0/.
// ****************************************************************************

#pragma once
#include <windows.h>

#define DIALCTRLCLASSNAME __TEXT("DialControl")

// MESSAGES

//! Set the angle value
//! \param wParam Redraw flag. If set to TRUE, the message redraws the dial 
//!				  after the range is set. If set to FALSE, the message sets the 
//!				  range but does not redraw the dial.
//! \param lParam A pointer to a double with the angle in radiants
//! \return TRUE
#define DCM_SETVALUE	WM_USER
/* TODO
//! Set the minimum angle
//! \param wParam Redraw flag. If set to TRUE, the message redraws the dial 
//!				  after the range is set. If set to FALSE, the message sets the 
//!				  range but does not redraw the dial.
//! \param lParam A pointer to a double with the angle in radiants
//! \return TRUE
#define DCM_SETRANGEMIN	WM_USER+1
//! Set the maximum angle
//! \param wParam Redraw flag. If set to TRUE, the message redraws the dial 
//!				  after the range is set. If set to FALSE, the message sets the 
//!				  range but does not redraw the dial.
//! \param lParam A pointer to a double with the angle in radiants
//! \return TRUE
#define DCM_SETRANGEMAX	WM_USER+2
*/
//! Set the step, delta angle to apply when the use press right,left up or down
//! \param wParam Redraw flag. If set to TRUE, the message redraws the dial 
//!				  after the range is set. If set to FALSE, the message sets the 
//!				  range but does not redraw the dial.
//! \param lParam A pointer to a double with the angle in radiants
//! \return TRUE
#define DCM_SETSTEP	WM_USER+3
//! Set the page, delta angle to apply when the use press pgup, or pgdown
//! \param wParam Redraw flag. If set to TRUE, the message redraws the dial 
//!				  after the range is set. If set to FALSE, the message sets the 
//!				  range but does not redraw the dial.
//! \param lParam A pointer to a double with the angle in radiants
//! \return TRUE
#define DCM_SETPAGE	WM_USER+4

//! Get the value
//! \param wParam This parameter is not used.
//! \param lParam A pointer to a double that is to receive the angle in radiants
//! \return TRUE
#define DCM_GETVALUE	WM_USER+5

// NOTIFICATIONS

//! The user is changing the angle
//! \param wParam The LOWORD contains the identifier of the dial control. 
//|				  The HIWORD specifies the notification code.
//! \param lParam A handle to the dial control.
#define DCN_CHANGING		0
//! The user changed the angle
//! \param wParam The LOWORD contains the identifier of the dial control. 
//|				  The HIWORD specifies the notification code.
//! \param lParam A handle to the dial control.
#define DCN_CHANGED		1

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	BOOL RegisterDialControl(HINSTANCE hInstance);
	void UnregisterDialControl(HINSTANCE hInstance);

#ifdef __cplusplus
}
#endif  /* __cplusplus */
