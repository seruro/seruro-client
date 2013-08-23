
#include "AlertDialog.h"
#include "../UIDefs.h"

#include "../../SeruroClient.h"

#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/region.h>
#include <wx/bitmap.h>

#include <wx/memory.h>
#include <wx/dcmemory.h>
#include <wx/dcclient.h>
#include <wx/graphics.h>
#include <wx/dcgraph.h>

/* Higher = more opaque. */
#define INITIAL_TRANSPARENCY   125
#define ROUNDED_CORENER_RADIUS 15.0
#define FADE_STEP 5
#define FADE_DELAY 500

#define TEST_STRING "This is a test alert"

DECLARE_APP(SeruroClient);

AlertDialog::AlertDialog()
{
    Create(NULL, wxID_ANY, _("Testing"), wxDefaultPosition, wxDefaultSize, wxSTAY_ON_TOP | wxFRAME_SHAPED);
    
    this->SetSize(150, 30);
    
    wxSizer *const sizer = new wxBoxSizer(wxVERTICAL);
    
    /* Consider styles from wxWindow: wxBORDER_NONE, wxTRANSPARENT_WINDOW, wxCLIP_CHILDREN */
    /* Capture the moving of the main_frame from the client, and move this alert window too: 
     * http://docs.wxwidgets.org/trunk/classwx_move_event.html */
    
    //wxButton *button = new wxButton(this, wxID_ANY, _("Close"));
    //sizer->Add(button);
    
    wxStaticText *alert_text = new wxStaticText(this, wxID_ANY, _(TEST_STRING), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL);
    sizer->Add(alert_text, wxSizerFlags().Expand().Proportion(1).Border(wxTOP | wxRIGHT | wxLEFT, 5));

    
    //wxFont my_font;
    //alert_text->SetFont(my_font);
    alert_text->SetForegroundColour(_("WHITE"));
    
    this->SetSizer(sizer);
    this->SetTransparent(INITIAL_TRANSPARENCY);
    this->SetBackgroundColour(wxColour(50, 50, 50));
    
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
        //width = width -;
    
    wxBitmap bmp(width+50, height+50);
    bmp.UseAlpha();

    dc.SelectObject(bmp);
    dc.SetBackground(*wxBLACK);
    dc.Clear();

    wxGraphicsContext *gc = wxGraphicsContext::Create(dc);
    
    //dc.SetBrush(*wxWHITE_BRUSH);
    //dc.DrawRoundedRectangle(0, 0, width, height, ROUNDED_CORENER_RADIUS);
    //dc.SelectObject(wxNullBitmap);
    
    //gc->SetBrush(*wxWHITE_BRUSH);
    gc->SetBrush(wxBrush(*wxLIGHT_GREY));
    gc->SetPen( *wxTRANSPARENT_PEN );
    gc->DrawRoundedRectangle(0, 0, width, height, ROUNDED_CORENER_RADIUS);
    gc->SetAntialiasMode(wxANTIALIAS_DEFAULT);
    gc->SetInterpolationQuality(wxINTERPOLATION_BEST);
    
    //this->SetBackgroundBitmap(bmp);
    
    //wxPaintDC pdc(this);
    //wxGCDC gdc(pdc);
    //wxDC &dc2 = gdc;
    
    //dc2.SelectObject(bmp)
    //dc2.SetBrush(wxBrush(*wxLIGHT_GREY));
    //dc2.SetPen(wxPen(*wxBLACK));
    
    //dc2.DrawRoundedRectangle(wxPoint(0, 0), width, height, 10.0);
    
    //wxImage original = bmp.ConvertToImage();
    //wxImage anti( 150, 150 );
    
    /* This is quite slow, but safe. Use wxImage::GetData() for speed instead. */
    //anti.SetData(original.GetData());
    //original.GetData();
    
    /*for (int y = 1; y < 149; y++)
        for (int x = 1; x < 149; x++)
        {
            int red = original.GetRed( x*2, y*2 ) +
            original.GetRed( x*2-1, y*2 ) +
            original.GetRed( x*2, y*2+1 ) +
            original.GetRed( x*2+1, y*2+1 );
            red = red/4;
            
            int green = original.GetGreen( x*2, y*2 ) +
            original.GetGreen( x*2-1, y*2 ) +
            original.GetGreen( x*2, y*2+1 ) +
            original.GetGreen( x*2+1, y*2+1 );
            green = green/4;
            
            int blue = original.GetBlue( x*2, y*2 ) +
            original.GetBlue( x*2-1, y*2 ) +
            original.GetBlue( x*2, y*2+1 ) +
            original.GetBlue( x*2+1, y*2+1 );
            blue = blue/4;
            anti.SetRGB( x, y, (unsigned char)red, (unsigned char)green, (unsigned char)blue );
        }*/
    
    //wxBitmap my_anti;
    //my_anti = wxBitmap(anti);
    
    wxRegion rgn(bmp, *wxBLACK);
    this->SetShape(rgn);
}

