/////////////////////////////////////////////////////////////////////////////
// Name:        RTSPApp.h
// Purpose:     
// Author:      mao
// Modified by: 
// Created:     05/08/2019 19:39:16
// RCS-ID:      
// Copyright:   mao
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#ifndef _RTSPAPP_H_
#define _RTSPAPP_H_


/*!
 * Includes
 */

////@begin includes
#include "wx/image.h"
#include "RTSPMainWnd.h"
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
////@end control identifiers

/*!
 * RTSPApp class declaration
 */

class RTSPApp: public wxApp
{    
    DECLARE_CLASS( RTSPApp )
    DECLARE_EVENT_TABLE()

public:
    /// Constructor
    RTSPApp();

    void Init();

    /// Initialises the application
    virtual bool OnInit();

    /// Called on exit
    virtual int OnExit();

////@begin RTSPApp event handler declarations

////@end RTSPApp event handler declarations

////@begin RTSPApp member function declarations

////@end RTSPApp member function declarations

////@begin RTSPApp member variables
////@end RTSPApp member variables
};

/*!
 * Application instance declaration 
 */

////@begin declare app
DECLARE_APP(RTSPApp)
////@end declare app

#endif
    // _RTSPAPP_H_
