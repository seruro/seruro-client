
#include "SeruroPanelSearch.h"

#include <wx/stattext.h>

SeruroPanelSearch::SeruroPanelSearch(wxBookCtrlBase *book) : SeruroPanel(book, wxT("Search"))
{
	wxStaticText *SearchLabel;
	SearchLabel = new wxStaticText(this, wxID_ANY, wxT("Name or Email:"), 
		wxDefaultPosition, wxDefaultSize, 0);

	this->mainSizer->Add(SearchLabel, wxRIGHT, 5);
	//SearchControl = new wxSearchCtrl
}
