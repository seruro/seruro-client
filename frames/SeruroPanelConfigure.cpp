
#include "SeruroPanelConfigure.h"

#include <wx/stattext.h>

SeruroPanelConfigure::SeruroPanelConfigure(wxBookCtrlBase *book) : SeruroPanel(book, wxT("Configure"))
{
	wxStaticText *SearchLabel;
	SearchLabel = new wxStaticText(this, wxID_ANY, wxT("Name or Email:"), 
		wxDefaultPosition, wxDefaultSize, 0);

	this->mainSizer->Add(SearchLabel);
}
