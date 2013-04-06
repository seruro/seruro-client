
#include "SeruroFrames.h"

BEGIN_EVENT_TABLE(SeruroFrameSearch, SeruroFrame)
END_EVENT_TABLE()

SeruroFrameSearch::SeruroFrameSearch(const wxString& title) : SeruroFrame(title)
{
	wxBoxSizer *VertSizer, *HorzSizer;
	wxPanel *panel;

	panel = new wxPanel(this);

	/* Whole frame sizer */
	VertSizer = new wxBoxSizer(wxVERTICAL);
	HorzSizer = new wxBoxSizer(wxHORIZONTAL);
	VertSizer->Add(HorzSizer, 1, wxEXPAND, 5);

	wxStaticText *SearchLabel;
	SearchLabel = new wxStaticText(this, wxID_ANY, wxT("Name or Email:"), 
		wxDefaultPosition, wxDefaultSize, 0);

	HorzSizer->Add(SearchLabel, wxRIGHT, 5);

	panel->SetSizer(VertSizer);
	this->Layout();
	//SearchControl = new wxSearchCtrl
}
