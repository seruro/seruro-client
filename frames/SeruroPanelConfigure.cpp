
#include "SeruroPanelConfigure.h"
#include "../api/SeruroServerAPI.h"

#include <wx/stattext.h>

/* Todo: catch some custom SeruroRequestEvents (error, timeout, data) */

SeruroPanelConfigure::SeruroPanelConfigure(wxBookCtrlBase *book) : SeruroPanel(book, wxT("Configure"))
{
	wxStaticText *SearchLabel;
	SearchLabel = new wxStaticText(this, wxID_ANY, wxT("Name or Email:"), 
		wxDefaultPosition, wxDefaultSize, 0);

	this->mainSizer->Add(SearchLabel);

	/* Button */
	//SeruroServerAPI *api = new SeruroServerAPI();
	//api->CreateRequest("api_name");
	//api->Run();
}
