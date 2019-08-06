/////////////////////////////////////////////////////////////////////////////
// Name:        RTSPMainWnd.cpp
// Purpose:     
// Author:      mao
// Modified by: 
// Created:     05/08/2019 19:39:40
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

#include "RTSPMainWnd.h"
#include "videoplayer/videoplayer.h"

////@begin XPM images
////@end XPM images


/*
 * RTSPMainWnd type definition
 */

IMPLEMENT_CLASS( RTSPMainWnd, wxFrame )

/*
 * RTSPMainWnd event table definition
 */

BEGIN_EVENT_TABLE( RTSPMainWnd, wxFrame )

////@begin RTSPMainWnd event table entries
    EVT_BUTTON( ID_BUTTON, RTSPMainWnd::OnButtonClick )
    EVT_BUTTON( ID_BUTTON1, RTSPMainWnd::OnButton1Click )
    EVT_BUTTON( ID_BUTTON2, RTSPMainWnd::OnButton2Click )
    EVT_BUTTON( ID_BUTTON3, RTSPMainWnd::OnButton3Click )
////@end RTSPMainWnd event table entries
	EVT_COMMAND(10086, wxEVT_NULL, RTSPMainWnd::OnFreshEvent)
END_EVENT_TABLE()


/*
 * RTSPMainWnd constructors
 */

RTSPMainWnd::RTSPMainWnd()
{
    Init();
}

RTSPMainWnd::RTSPMainWnd( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create( parent, id, caption, pos, size, style );
}


/*
 * RTSPMainWnd creator
 */

bool RTSPMainWnd::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin RTSPMainWnd creation
    wxFrame::Create( parent, id, caption, pos, size, style );

    CreateControls();
    Centre();
////@end RTSPMainWnd creation
    return true;
}


/*
 * RTSPMainWnd destructor
 */

RTSPMainWnd::~RTSPMainWnd()
{
////@begin RTSPMainWnd destruction
////@end RTSPMainWnd destruction
}


/*
 * Member initialisation
 */

void RTSPMainWnd::Init()
{
////@begin RTSPMainWnd member initialisation
    m_Panel = NULL;
////@end RTSPMainWnd member initialisation
}


/*
 * Control creation for RTSPMainWnd
 */

void RTSPMainWnd::CreateControls()
{    
////@begin RTSPMainWnd content construction
    RTSPMainWnd* itemFrame1 = this;

    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxVERTICAL);
    itemFrame1->SetSizer(itemBoxSizer1);

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer1->Add(itemBoxSizer2, 0, wxGROW|wxALL, 5);

    wxArrayString itemComboBox1Strings;
    wxComboBox* itemComboBox1 = new wxComboBox( itemFrame1, ID_COMBOBOX, wxEmptyString, wxDefaultPosition, wxDefaultSize, itemComboBox1Strings, wxCB_DROPDOWN );
    itemBoxSizer2->Add(itemComboBox1, 1, wxGROW|wxALL, 5);

    itemBoxSizer2->Add(5, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxDatePickerCtrl* itemDatePickerCtrl2 = new wxDatePickerCtrl( itemFrame1, ID_DATECTRL, wxDateTime(), wxDefaultPosition, wxDefaultSize, wxDP_DEFAULT );
    itemBoxSizer2->Add(itemDatePickerCtrl2, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxDatePickerCtrl* itemDatePickerCtrl3 = new wxDatePickerCtrl( itemFrame1, ID_DATEPICKERCTRL, wxDateTime(), wxDefaultPosition, wxDefaultSize, wxDP_DEFAULT );
    itemBoxSizer2->Add(itemDatePickerCtrl3, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer1->Add(itemBoxSizer3, 1, wxGROW|wxALL, 5);

    m_Panel = new wxPanel( itemFrame1, ID_PANEL, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
    m_Panel->SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    itemBoxSizer3->Add(m_Panel, 1, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer1->Add(itemBoxSizer4, 0, wxGROW|wxALL, 5);

    wxButton* itemButton5 = new wxButton( itemFrame1, ID_BUTTON, _("Play"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemButton5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton6 = new wxButton( itemFrame1, ID_BUTTON1, _("Stop"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemButton6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton7 = new wxButton( itemFrame1, ID_BUTTON2, _("Pause"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemButton7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton8 = new wxButton( itemFrame1, ID_BUTTON3, _("Resume"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemButton8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton9 = new wxButton( itemFrame1, ID_BUTTON4, _("Button"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemButton9, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end RTSPMainWnd content construction
}


/*
 * Should we show tooltips?
 */

bool RTSPMainWnd::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap RTSPMainWnd::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin RTSPMainWnd bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end RTSPMainWnd bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon RTSPMainWnd::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin RTSPMainWnd icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end RTSPMainWnd icon retrieval
}


/*
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON
 */

void RTSPMainWnd::OnButtonClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON in RTSPMainWnd.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON in RTSPMainWnd. 

	//rtsp://182.139.226.78/PLTV/88888893/224/3221227219/10000100000000060000000001366244_0.smil?playseek=20190801100000-20190801113000

	VideoPlayer *vp = new VideoPlayer(this);

	vp->Run();

}


/*
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON1
 */

void RTSPMainWnd::OnButton1Click( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON1 in RTSPMainWnd.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON1 in RTSPMainWnd. 
}


/*
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON2
 */

void RTSPMainWnd::OnButton2Click( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON2 in RTSPMainWnd.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON2 in RTSPMainWnd. 
}


/*
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON3
 */

void RTSPMainWnd::OnButton3Click( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON3 in RTSPMainWnd.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON3 in RTSPMainWnd. 
}

#include <wx/mstream.h>
#include <wx/rawbmp.h>

void RTSPMainWnd::OnFreshEvent(wxCommandEvent& event) {
	wxLogDebug("OnFreshEvent");
	long ptr = event.GetExtraLong();

	IMG img = *(IMG*)ptr;

	if (1) return;

	//wxMemoryOutputStream outs;
	//wxMemoryInputStream ins(img.buf, img.size);

	//wxImage ximg; 
	
	//ximg.LoadFile(ins, wxBITMAP_TYPE_BMP_RESOURCE);

	//wxBitmap bmp(img.buf, img.w, img.h, 1);
	//wxBitmap bmp(ximg);


	wxMemoryDC memDC;

	wxBitmap bitmap(img.w, img.h,24);

	wxNativePixelData data(bitmap);
	wxNativePixelData::Iterator p(data);

	// we draw a (10, 10)-(20, 20) rect manually using the given r, g, b
	p.Offset(data, 0, 0);
	for (int y = 0; y < img.h/4; ++y)
	{
		wxNativePixelData::Iterator rowStart = p;
		for (int x = 0; x < img.w/4; ++x, ++p)
		{
			
			//p.Data() = &img.ibuf[y*img.w + x];
			p.Red() = img.buf[4*(y*img.w+x)];
			p.Green() = img.buf[4*(y*img.w + x) +1];
			p.Blue() = img.buf[4 * (y*img.w + x) +2];

			//p++;
		}
		p = rowStart;
		p.OffsetY(data, 1);
	}

	memDC.SelectObject(bitmap);

	// Create a memory DC
	wxMemoryDC temp_dc;
	temp_dc.SelectObject(bitmap);

	wxClientDC dc(m_Panel);
	wxSize s = m_Panel->GetSize();
	dc.StretchBlit(0, 0, s.GetWidth(), s.GetHeight(), &temp_dc, 0, 0, img.w, img.h);
}