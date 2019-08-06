/////////////////////////////////////////////////////////////////////////////
// Name:        RTSPApp.cpp
// Purpose:     
// Author:      mao
// Modified by: 
// Created:     05/08/2019 19:39:16
// RCS-ID:      
// Copyright:   mao
// Licence:     
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
////@end includes

#include "RTSPApp.h"

////@begin XPM images
////@end XPM images


/*
 * Application instance implementation
 */

////@begin implement app
IMPLEMENT_APP( RTSPApp )
////@end implement app


/*
 * RTSPApp type definition
 */

IMPLEMENT_CLASS( RTSPApp, wxApp )


/*
 * RTSPApp event table definition
 */

BEGIN_EVENT_TABLE( RTSPApp, wxApp )

////@begin RTSPApp event table entries
////@end RTSPApp event table entries

END_EVENT_TABLE()


/*
 * Constructor for RTSPApp
 */

RTSPApp::RTSPApp()
{
    Init();
}


/*
 * Member initialisation
 */

void RTSPApp::Init()
{
////@begin RTSPApp member initialisation
////@end RTSPApp member initialisation
}

/*
 * Initialisation for RTSPApp
 */

bool RTSPApp::OnInit()
{    
////@begin RTSPApp initialisation
	// Remove the comment markers above and below this block
	// to make permanent changes to the code.

#if wxUSE_XPM
	wxImage::AddHandler(new wxXPMHandler);
#endif
#if wxUSE_LIBPNG
	wxImage::AddHandler(new wxPNGHandler);
#endif
#if wxUSE_LIBJPEG
	wxImage::AddHandler(new wxJPEGHandler);
#endif
#if wxUSE_GIF
	wxImage::AddHandler(new wxGIFHandler);
#endif
	RTSPMainWnd* mainWindow = new RTSPMainWnd( NULL );
	mainWindow->Show(true);
////@end RTSPApp initialisation

    return true;
}


/*
 * Cleanup for RTSPApp
 */

int RTSPApp::OnExit()
{    
////@begin RTSPApp cleanup
	return wxApp::OnExit();
////@end RTSPApp cleanup
}

