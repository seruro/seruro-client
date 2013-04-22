
#include "SeruroPanelTest.h"

#include "../wxJSON/wx/jsonval.h"

#include <wx/button.h>
#include <wx/log.h>

BEGIN_EVENT_TABLE(SeruroPanelTest, wxPanel)
	EVT_BUTTON(BUTTON_GET_CA, SeruroPanelTest::OnGetCA)

	/* Request events */
	EVT_COMMAND(CALLBACK_GET_CA, SERURO_API_RESULT, SeruroPanelTest::OnGetCAResult)
END_EVENT_TABLE()

SeruroPanelTest::SeruroPanelTest(wxBookCtrlBase *book) : SeruroPanel(book, wxT("API Test"))
{
	api = new SeruroServerAPI(this->GetEventHandler());

	wxButton *get_ca = new wxButton(this, BUTTON_GET_CA, wxT("GET P12"), 
		wxDefaultPosition, wxDefaultSize, 0);
	this->mainSizer->Add(get_ca, wxRIGHT, 5);
}

void SeruroPanelTest::OnGetCA(wxCommandEvent &event)
{
	wxJSONValue params; /* no params */
	params[wxT("server")] = wxT("open.seruro.com");

	SeruroRequest *request = api->CreateRequest(SERURO_API_GET_P12, params, CALLBACK_GET_CA);
	//SeruroRequest *request = api->CreateRequest(SERURO_API_GET_P12, params, GET_CA_CALLBACK);
	request->Run();
	//delete [] request;
}

void SeruroPanelTest::OnGetCAResult(wxCommandEvent &event)
{
	wxJSONValue *response = (wxJSONValue *) event.GetClientData();

	bool hasMember = response->HasMember("result");
	if (hasMember)
		wxLogMessage(wxT("SeruroPanelTest> API valid response."));

	/* Controller owns client data? */
    if (response)
        delete[] response;
}
