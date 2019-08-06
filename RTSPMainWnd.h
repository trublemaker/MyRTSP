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
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_RTSPMAINWND 10000
#define ID_COMBOBOX 10001
#define ID_DATECTRL 10002
#define ID_DATEPICKERCTRL 10003
#define ID_PANEL 10009
#define ID_BUTTON 10004
#define ID_BUTTON1 10005
#define ID_BUTTON2 10006
#define ID_BUTTON3 10007
#define ID_BUTTON4 10008
#define SYMBOL_RTSPMAINWND_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxMINIMIZE_BOX|wxMAXIMIZE_BOX|wxCLOSE_BOX
#define SYMBOL_RTSPMAINWND_TITLE _("RTSPMainWnd")
#define SYMBOL_RTSPMAINWND_IDNAME ID_RTSPMAINWND
#define SYMBOL_RTSPMAINWND_SIZE wxSize(800, 600)
#define SYMBOL_RTSPMAINWND_POSITION wxDefaultPosition
////@end control identifiers

wxDEFINE_EVENT(MY_EVENT, wxCommandEvent);

struct IMG {
	int w;
	int h;
	int size;
	union {
		char* buf;
		int ibuf;
	};
};

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

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON
    void OnButtonClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON1
    void OnButton1Click( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON2
    void OnButton2Click( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON3
    void OnButton3Click( wxCommandEvent& event );

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
    wxPanel* m_Panel;
////@end RTSPMainWnd member variables
};

#endif
    // _RTSPMAINWND_H_
