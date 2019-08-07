/////////////////////////////////////////////////////////////////////////////
// Name:        RTSPMainWnd.h
// Purpose:     
// Author:      mao
// Modified by: 
// Created:     05/08/2019 19:39:40
// RCS-ID:      
// Copyright:   mao
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#ifndef _RTSPMAINWND_H_
#define _RTSPMAINWND_H_


/*!
 * Includes
 */

////@begin includes
#include "wx/frame.h"
#include "wx/datectrl.h"
#include "wx/dateevt.h"
#include "wx/statline.h"
////@end includes

#include <wx/timectrl.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxTimePickerCtrl;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_RTSPMAINWND 10000
#define ID_COMBOBOX 10001
#define ID_DATECTRL 10002
#define ID_DATEPICKERCTRL 10003
#define ID_DATEPICKERCTRL1 10010
#define ID_PANEL 10009
#define ID_BTN_PLAY 10004
#define ID_BTN_STOP 10005
#define ID_BTN_PAUSE 10006
#define ID_BTN_RESUME 10007
#define ID_SLIDER 10011
#define SYMBOL_RTSPMAINWND_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxMINIMIZE_BOX|wxMAXIMIZE_BOX|wxCLOSE_BOX
#define SYMBOL_RTSPMAINWND_TITLE wxGetTranslation(wxString(wxT("RTSP")) + (wxChar) 0x6D41 + (wxChar) 0x5A92 + (wxChar) 0x4F53 + (wxChar) 0x64AD + (wxChar) 0x653E + (wxChar) 0x5668)
#define SYMBOL_RTSPMAINWND_IDNAME ID_RTSPMAINWND
#define SYMBOL_RTSPMAINWND_SIZE wxSize(800, 600)
#define SYMBOL_RTSPMAINWND_POSITION wxDefaultPosition
////@end control identifiers

struct IMG {
	int w;
	int h;
	int size;
	union {
		char* buf;
		int ibuf;
	};
};

class VideoPlayer;
/*!
 * RTSPMainWnd class declaration
 */

class RTSPMainWnd: public wxFrame
{    
    DECLARE_CLASS( RTSPMainWnd )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    RTSPMainWnd();
    RTSPMainWnd( wxWindow* parent, wxWindowID id = SYMBOL_RTSPMAINWND_IDNAME, const wxString& caption = SYMBOL_RTSPMAINWND_TITLE, const wxPoint& pos = SYMBOL_RTSPMAINWND_POSITION, const wxSize& size = SYMBOL_RTSPMAINWND_SIZE, long style = SYMBOL_RTSPMAINWND_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_RTSPMAINWND_IDNAME, const wxString& caption = SYMBOL_RTSPMAINWND_TITLE, const wxPoint& pos = SYMBOL_RTSPMAINWND_POSITION, const wxSize& size = SYMBOL_RTSPMAINWND_SIZE, long style = SYMBOL_RTSPMAINWND_STYLE );

    /// Destructor
    ~RTSPMainWnd();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin RTSPMainWnd event handler declarations

    /// wxEVT_CLOSE_WINDOW event handler for ID_RTSPMAINWND
    void OnCloseWindow( wxCloseEvent& event );

    /// wxEVT_DATE_CHANGED event handler for ID_DATECTRL
    void OnDatectrlDateChanged( wxDateEvent& event );

    /// wxEVT_DATE_CHANGED event handler for ID_DATEPICKERCTRL
    void OnDatepickerctrlDateChanged( wxDateEvent& event );

    /// wxEVT_SIZE event handler for ID_PANEL
    void OnSize( wxSizeEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BTN_PLAY
    void OnBtnPlayClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BTN_STOP
    void OnBtnStopClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BTN_PAUSE
    void OnBtnPauseClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BTN_RESUME
    void OnBtnResumeClick( wxCommandEvent& event );

////@end RTSPMainWnd event handler declarations

////@begin RTSPMainWnd member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end RTSPMainWnd member function declarations

	void OnFreshEvent(wxCommandEvent& event);

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin RTSPMainWnd member variables
    wxTimePickerCtrl* m_StartTime;
    wxTimePickerCtrl* m_EndTime;
    wxPanel* m_Panel;
////@end RTSPMainWnd member variables

	VideoPlayer *vp;
};

#endif
    // _RTSPMAINWND_H_
