
#include "SeruroHelp.h"
#include "UIDefs.h"

SeruroPanelHelp::SeruroPanelHelp(wxBookCtrlBase *book) : SeruroPanel(book, wxT("Help"))
{
    wxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    
    sizer->Add(new Text(this, _("No help yet...")), 1, wxALL, 10);
    
    this->SetSizer(sizer);
    this->Layout();
}