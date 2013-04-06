
#include "SeruroFrames.h"

SeruroFrameConfigure::SeruroFrameConfigure(const wxString& title) : SeruroFrame(title)
{
	wxBoxSizer *VertSizer, *HorzSizer;

	/* Whole frame sizer */
	VertSizer = new wxBoxSizer(wxVERTICAL);
	HorzSizer = new wxBoxSizer(wxHORIZONTAL);
	VertSizer->Add(HorzSizer, 1, wxEXPAND, 5);

	wxStaticText *SearchLabel;
	SearchLabel = new wxStaticText(this, wxID_ANY, wxT("Name or Email:"), 
		wxDefaultPosition, wxDefaultSize, 0);

	HorzSizer->Add(SearchLabel, wxRIGHT, 5);

	this->SetSizer(VertSizer);
}
