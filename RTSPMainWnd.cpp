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
    EVT_CLOSE( RTSPMainWnd::OnCloseWindow )
    EVT_DATE_CHANGED( ID_DATECTRL, RTSPMainWnd::OnDatectrlDateChanged )
    EVT_DATE_CHANGED( ID_DATEPICKERCTRL, RTSPMainWnd::OnDatepickerctrlDateChanged )
    EVT_BUTTON( ID_BTN_PLAY, RTSPMainWnd::OnBtnPlayClick )
    EVT_BUTTON( ID_BTN_STOP, RTSPMainWnd::OnBtnStopClick )
    EVT_BUTTON( ID_BTN_PAUSE, RTSPMainWnd::OnBtnPauseClick )
    EVT_BUTTON( ID_BTN_RESUME, RTSPMainWnd::OnBtnResumeClick )
////@end RTSPMainWnd event table entries
	EVT_COMMAND(10086, wxEVT_NULL, RTSPMainWnd::OnFreshEvent)
	EVT_TIME_CHANGED(ID_DATEPICKERCTRL, RTSPMainWnd::OnDatepickerctrlDateChanged)
	EVT_TIME_CHANGED(ID_DATEPICKERCTRL1, RTSPMainWnd::OnDatepickerctrlDateChanged)
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
    m_cmb_RTSP = NULL;
    m_Date = NULL;
    m_StartTime = NULL;
    m_EndTime = NULL;
    m_Panel = NULL;
////@end RTSPMainWnd member initialisation
	vp = NULL;
}


/*
 * Control creation for RTSPMainWnd
 */

void RTSPMainWnd::CreateControls()
{    
////@begin RTSPMainWnd content construction
    RTSPMainWnd* itemFrame1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemFrame1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxGROW|wxALL, 1);

    wxArrayString m_cmb_RTSPStrings;
    m_cmb_RTSPStrings.Add(_("rtsp://182.139.226.78/PLTV/88888893/224/3221227219/10000100000000060000000001366244_0.smil?playseek=20190805101000-20190805113000"));
    m_cmb_RTSP = new wxComboBox( itemFrame1, ID_COMBOBOX, _("rtsp://182.139.226.78/PLTV/88888893/224/3221227219/10000100000000060000000001366244_0.smil?playseek=20190805101000-20190805113000"), wxDefaultPosition, wxDefaultSize, m_cmb_RTSPStrings, wxCB_DROPDOWN );
    m_cmb_RTSP->SetStringSelection(_("rtsp://182.139.226.78/PLTV/88888893/224/3221227219/10000100000000060000000001366244_0.smil?playseek=20190805101000-20190805113000"));
    itemBoxSizer3->Add(m_cmb_RTSP, 1, wxGROW|wxALL, 5);

    itemBoxSizer3->Add(5, 5, 0, wxGROW|wxALL, 5);

    m_Date = new wxDatePickerCtrl( itemFrame1, ID_DATECTRL, wxDateTime(), wxDefaultPosition, wxDefaultSize, wxDP_DROPDOWN );
    itemBoxSizer3->Add(m_Date, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticLine* itemStaticLine7 = new wxStaticLine( itemFrame1, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL );
    itemBoxSizer3->Add(itemStaticLine7, 0, wxGROW|wxALL, 5);

    m_StartTime = new wxTimePickerCtrl( itemFrame1, ID_DATEPICKERCTRL, wxDateTime(), wxDefaultPosition, wxDefaultSize, wxDP_DEFAULT );
    itemBoxSizer3->Add(m_StartTime, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_EndTime = new wxTimePickerCtrl( itemFrame1, ID_DATEPICKERCTRL1, wxDateTime(), wxDefaultPosition, wxDefaultSize, wxDP_DEFAULT );
    itemBoxSizer3->Add(m_EndTime, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer10 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer10, 1, wxGROW|wxALL, 1);

    m_Panel = new wxPanel( itemFrame1, ID_PANEL, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
    m_Panel->SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    m_Panel->SetForegroundColour(wxColour(0, 0, 64));
    m_Panel->SetBackgroundColour(wxColour(0, 0, 64));
    itemBoxSizer10->Add(m_Panel, 1, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer12, 0, wxGROW|wxALL, 1);

    wxButton* itemButton13 = new wxButton( itemFrame1, ID_BTN_PLAY, _("Play"), wxDefaultPosition, wxSize(50, -1), 0 );
    itemBoxSizer12->Add(itemButton13, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton14 = new wxButton( itemFrame1, ID_BTN_STOP, _("Stop"), wxDefaultPosition, wxSize(50, -1), 0 );
    itemBoxSizer12->Add(itemButton14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton15 = new wxButton( itemFrame1, ID_BTN_PAUSE, _("Pause"), wxDefaultPosition, wxSize(50, -1), 0 );
    itemBoxSizer12->Add(itemButton15, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton16 = new wxButton( itemFrame1, ID_BTN_RESUME, _("Resume"), wxDefaultPosition, wxSize(50, -1), 0 );
    itemBoxSizer12->Add(itemButton16, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemBoxSizer12->Add(5, 5, 0, wxGROW|wxALL, 5);

    wxStaticLine* itemStaticLine18 = new wxStaticLine( itemFrame1, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL );
    itemBoxSizer12->Add(itemStaticLine18, 0, wxGROW|wxALL, 5);

    wxSlider* itemSlider19 = new wxSlider( itemFrame1, ID_SLIDER, 0, 0, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL );
    itemBoxSizer12->Add(itemSlider19, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // Connect events and objects
    m_Panel->Connect(ID_PANEL, wxEVT_SIZE, wxSizeEventHandler(RTSPMainWnd::OnSize), NULL, this);
////@end RTSPMainWnd content construction

	wxDateSpan ds; ds.SetDays(0);
	wxDateTime dt = m_Date->GetValue();
	dt = dt - ds;
	m_Date->SetValue(dt);
	ds.SetDays(10);	
	dt = dt - ds;
	m_Date->SetRange(dt, wxDateTime::Now());

	//wxTimePickerCtrl* itemTimePickerCtrl11 = new wxTimePickerCtrl(itemFrame1, ID_DATECTRL, wxDateTime(), wxDefaultPosition, wxDefaultSize);
	//itemBoxSizer2->Add(itemTimePickerCtrl11, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
	m_StartTime->SetTime(12, 0, 0);
	m_EndTime->SetTime(13, 0, 0);

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

void RTSPMainWnd::OnBtnPlayClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON in RTSPMainWnd.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON in RTSPMainWnd. 

	//rtsp://182.139.226.78/PLTV/88888893/224/3221227219/10000100000000060000000001366244_0.smil?playseek=20190801100000-20190801113000

	vp = new VideoPlayer(this);
	vp->Run();

}


/*
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON1
 */

void RTSPMainWnd::OnBtnStopClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON1 in RTSPMainWnd.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON1 in RTSPMainWnd. 

	vp->Delete();
}


/*
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON2
 */

void RTSPMainWnd::OnBtnPauseClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON2 in RTSPMainWnd.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON2 in RTSPMainWnd. 
}


/*
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON3
 */

void RTSPMainWnd::OnBtnResumeClick( wxCommandEvent& event )
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

/*
 * wxEVT_DATE_CHANGED event handler for ID_DATECTRL
 */

void RTSPMainWnd::OnDatectrlDateChanged( wxDateEvent& event )
{
////@begin wxEVT_DATE_CHANGED event handler for ID_DATECTRL in RTSPMainWnd.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_DATE_CHANGED event handler for ID_DATECTRL in RTSPMainWnd. 
}


/*
 * wxEVT_SIZE event handler for ID_PANEL
 */

void RTSPMainWnd::OnSize( wxSizeEvent& event )
{
////@begin wxEVT_SIZE event handler for ID_PANEL in RTSPMainWnd.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_SIZE event handler for ID_PANEL in RTSPMainWnd. 
}


/*
 * wxEVT_DATE_CHANGED event handler for ID_DATEPICKERCTRL
 */

void RTSPMainWnd::OnDatepickerctrlDateChanged( wxDateEvent& event )
{
////@begin wxEVT_DATE_CHANGED event handler for ID_DATEPICKERCTRL in RTSPMainWnd.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_DATE_CHANGED event handler for ID_DATEPICKERCTRL in RTSPMainWnd. 
	wxDateTime dt = event.GetDate();
	 dt = event.GetDate();

	 //EVT_TIME_CHANGED(ID_DATEPICKERCTRL, RTSPMainWnd::OnDatepickerctrlDateChanged)
	 //EVT_TIME_CHANGED(ID_DATEPICKERCTRL1, RTSPMainWnd::OnDatepickerctrlDateChanged)
	 if (event.GetId() == ID_DATEPICKERCTRL) {

	 }
	 wxLogDebug("%d-%s", event.GetId(),dt.FormatTime());

	 int sh, sm, ss, eh, em, es;
	 m_StartTime->GetTime(&sh, &sm, &ss);
	 m_EndTime->GetTime(&eh, &em, &es);

	 wxTimeSpan sts(sh, sm, ss);
	 wxTimeSpan ets(eh, em, es);
	 wxTimeSpan delta(1, 0, 0);
	 if (sts >= ets) {
		 sts = ets -delta;
	 }
	 int h = sts.GetHours();
	 if (h < 1) h = 1;
	 m_StartTime->SetTime(h, 0, 0);

	 m_StartTime->GetTime(&sh, &sm, &ss);
	 m_EndTime->GetTime(&eh, &em, &es);
}

/*
 * wxEVT_CLOSE_WINDOW event handler for ID_RTSPMAINWND
 */
extern int quit;
void RTSPMainWnd::OnCloseWindow( wxCloseEvent& event )
{
////@begin wxEVT_CLOSE_WINDOW event handler for ID_RTSPMAINWND in RTSPMainWnd.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_CLOSE_WINDOW event handler for ID_RTSPMAINWND in RTSPMainWnd. 
	quit = 1;

	if (vp) {
		vp->Delete();
		//vp->Wait();
	}
	
}

