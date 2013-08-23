
#include "AlertDialog.h"
#include "../UIDefs.h"

#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/region.h>
#include <wx/bitmap.h>

#include <wx/memory.h>
#include <wx/dcmemory.h>

/* Higher = more opaque. */
#define INITIAL_TRANSPARENCY   175
#define ROUNDED_CORENER_RADIUS 25
#define FADE_STEP 5
#define FADE_DELAY 500

#define TEST_STRING "This is a test alert"

AlertDialog::AlertDialog() : wxDialog(NULL, wxID_ANY, _("Testing"), wxDefaultPosition, wxDefaultSize, wxSTAY_ON_TOP | wxFRAME_SHAPED)
{
    this->SetSize(300, 75);
    
    wxSizer *const sizer = new wxBoxSizer(wxVERTICAL);
    
    /* Consider styles from wxWindow: wxBORDER_NONE, wxTRANSPARENT_WINDOW, wxCLIP_CHILDREN */
    /* Capture the moving of the main_frame from the client, and move this alert window too: 
     * http://docs.wxwidgets.org/trunk/classwx_move_event.html */
    
    //wxButton *button = new wxButton(this, wxID_ANY, _("Close"));
    //sizer->Add(button);
    
    wxStaticText *alert_text = new wxStaticText(this, wxID_ANY, _(TEST_STRING), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL);
    sizer->Add(alert_text, wxSizerFlags().Expand().Proportion(1).Border(wxTOP | wxRIGHT | wxLEFT, 5));
    
    this->SetSizer(sizer);
    this->SetTransparent(INITIAL_TRANSPARENCY);
    this->SetBackgroundColour(wxColour(0, 0, 0));
    this->CreateRoundedCorners();
    
    this->Layout();
    this->Center();
}

void AlertDialog::CreateRoundedCorners()
{
    int width, height;
    wxMemoryDC dc;
    
    //wxSize best_size;
    //best_size = this->GetBest
    
    //this->Fit();
    //this->GetBestSize(&width, &height);
    //height += 30;
    this->GetSize(&width, &height);
        width = width - 100;
    
    wxBitmap bmp(width+50, height+50);

    dc.SelectObject(bmp);
    dc.SetBackground(*wxBLACK);
    dc.Clear();
    dc.SetBrush(*wxWHITE_BRUSH);
    dc.DrawRoundedRectangle(0, 0, width, height, ROUNDED_CORENER_RADIUS);
    dc.SelectObject(wxNullBitmap);
    
    wxRegion rgn(bmp, *wxBLACK);
    this->SetShape(rgn);
}