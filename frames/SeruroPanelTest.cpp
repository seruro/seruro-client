
#include "SeruroPanelTest.h"

#include "../wxJSON/wx/jsonval.h"

#include <wx/button.h>
#include <wx/log.h>

BEGIN_EVENT_TABLE(SeruroPanelTest, wxPanel)
	EVT_BUTTON(BUTTON_GET_CA, SeruroPanelTest::OnGetCA)

	/* Request events */
	EVT_COMMAND(GET_CA_CALLBACK, SERURO_API_RESULT, SeruroPanelTest::OnResult)
END_EVENT_TABLE()

SeruroPanelTest::SeruroPanelTest(wxBookCtrlBase *book) : SeruroPanel(book, wxT("API Test"))
{
	api = new SeruroServerAPI(this->GetEventHandler());

	wxButton *get_ca = new wxButton(this, BUTTON_GET_CA, wxT("GET CA"), 
		wxDefaultPosition, wxDefaultSize, 0);
	this->mainSizer->Add(get_ca, wxRIGHT, 5);
}

void SeruroPanelTest::OnGetCA(wxCommandEvent &event)
{
	wxJSONValue params; /* no params */
	params[wxT("server")] = wxT("google.com");

	SeruroRequest *request = api->CreateRequest(SERURO_API_GET_CA, params, GET_CA_CALLBACK);
	request->Run();
}

void SeruroPanelTest::OnResult(wxCommandEvent &event)
{
	wxJSONValue *response = (wxJSONValue *) event.GetClientData();

	bool hasMember = response->HasMember("result");
	if (hasMember)
		wxLogMessage(wxT("SeruroPanelTest> API valid response."));

	/* Controller owns client data? */
    if (response)
        delete[] response;
}
