
#include "SeruroPanelTest.h"

#include "../wxJSON/wx/jsonval.h"

#include <wx/button.h>

BEGIN_EVENT_TABLE(SeruroPanelTest, wxPanel)
	EVT_BUTTON(BUTTON_GET_CA, SeruroPanelTest::OnGetCA)

	/* Request events */
	EVT_COMMAND(wxID_ANY, SERURO_API_RESULT, SeruroPanelTest::OnResult)
END_EVENT_TABLE()

SeruroPanelTest::SeruroPanelTest(wxBookCtrlBase *book) : SeruroPanel(book, wxT("API Test"))
{
	api = new SeruroServerAPI(this->GetEventHandler());
	//wxStaticText *SearchLabel;
	//SearchLabel = new wxStaticText(this, wxID_ANY, wxT("Name or Email:"), 
	//	wxDefaultPosition, wxDefaultSize, 0);
	//this->mainSizer->Add(SearchLabel, wxRIGHT, 5);
	//SearchControl = new wxSearchCtrl
	wxButton *get_ca = new wxButton(this, BUTTON_GET_CA, wxT("GET CA"), 
		wxDefaultPosition, wxDefaultSize, 0);
	//this->mainSizer->Add(get_ca, wxRIGHT, 5);
}

void SeruroPanelTest::OnGetCA(wxCommandEvent &event)
{
	wxJSONValue params; /* no params */
	//SeruroServerAPI *api = new SeruroServerAPI();
	//SeruroRequest *api = new SeruroRequest(SERURO_API_GET_CA, params);

	SeruroRequest *request = api->CreateRequest(SERURO_API_GET_CA, params);
	request->Run();
}

void SeruroPanelTest::OnResult(wxCommandEvent &event)
{
	wxJSONValue *response = (wxJSONValue *) event.GetClientData();

	bool hasMember = response->HasMember("data");
	if (hasMember)
		wxLogMessage(wxT("SeruroPanelTest> API valid response."));

	/* Controller owns client data? */
	delete[] response;
}
