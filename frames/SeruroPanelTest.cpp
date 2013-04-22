
#include "SeruroPanelTest.h"

#include "../wxJSON/wx/jsonval.h"
#include "../wxJSON/wx/jsonreader.h"

#include <wx/button.h>
#include <wx/log.h>

/* Base64 decoding. */
#include <wx/base64.h>
#include <wx/buffer.h>

/* Username and password, encryption keys. */
#include <wx/textdlg.h>

BEGIN_EVENT_TABLE(SeruroPanelTest, wxPanel)
	EVT_BUTTON(BUTTON_GET_CA, SeruroPanelTest::OnGetCA)
	EVT_BUTTON(BUTTON_GET_P12, SeruroPanelTest::OnGetP12)

	/* Request events */
	EVT_COMMAND(CALLBACK_GET_CA, SERURO_API_RESULT, SeruroPanelTest::OnGetCAResult)
	EVT_COMMAND(CALLBACK_GET_P12, SERURO_API_RESULT, SeruroPanelTest::OnGetP12Result)
END_EVENT_TABLE()

SeruroPanelTest::SeruroPanelTest(wxBookCtrlBase *book) : SeruroPanel(book, wxT("API Test"))
{
	api = new SeruroServerAPI(this->GetEventHandler());

	wxButton *get_ca = new wxButton(this, BUTTON_GET_CA, wxT("GET CA"), wxDefaultPosition, wxDefaultSize, 0);
	wxButton *get_p12 = new wxButton(this, BUTTON_GET_P12, wxT("GET P12"), wxDefaultPosition, wxDefaultSize, 0);
	this->mainSizer->Add(get_ca, wxRIGHT, 5);
	this->mainSizer->Add(get_p12, wxRIGHT, 5);

	//this->SetSizer(mainSizer);
}

void SeruroPanelTest::OnGetP12(wxCommandEvent &event)
{
	wxJSONValue params; /* no params */
	params[wxT("server")] = wxT("open.seruro.com");

	SeruroRequest *request = api->CreateRequest(SERURO_API_GET_P12, params, CALLBACK_GET_P12);
	request->Run();
	/* Todo: Cannot delete the request because the thread still exists, who cleans up this memory? */
	//delete [] request;
}

void SeruroPanelTest::OnGetP12Result(wxCommandEvent &event)
{
	//wxJSONValue *response = (wxJSONValue *) event.GetClientData();
	wxJSONReader reader;
	wxJSONValue response;
	wxString responseString = event.GetString();
	reader.Parse(responseString, &response);

	if (! response.HasMember("result") || ! response["result"].AsBool()) {
		wxLogMessage(wxT("SeruroPanelTest> (GetP12) Bad Result."));
		return;
	}

	wxLogMessage(wxT("SeruroPanelTest> (GetP12) API valid response."));

	/* The getP12 API call should respond with two P12s, a TLS and SMIME container. */
	if (! response.HasMember("p12") || 
		(! response["p12"].HasMember("tls") || ! response["p12"].HasMember("smime"))) {
		wxLogMessage(wxT("SeruroPanelTest> (GetP12) Response does not include P12 TLS/SMIME data."));
		return;
	}

	/* Both P12s will be base64 encoded for HTTP transfer. */
	wxMemoryBuffer tls_p12 = wxBase64Decode(response["p12"]["tls"].AsString());
	wxMemoryBuffer smime_p12 = wxBase64Decode(response["p12"]["smime"].AsString());
	/* Todo: error check the decode. */

	/* Get password from user for p12 containers. */
	wxPasswordEntryDialog *get_key = new wxPasswordEntryDialog(this, wxT("Enter decryption key"));
	if (get_key->ShowModal() == wxID_OK) {

	}
	
	get_key->Destroy();
	return;
}

void SeruroPanelTest::OnGetCA(wxCommandEvent &event)
{
	wxJSONValue params; /* no params */
	params[wxT("server")] = wxT("open.seruro.com");

	SeruroRequest *request = api->CreateRequest(SERURO_API_GET_CA, params, CALLBACK_GET_CA);
	request->Run();
	//delete [] request;
}

void SeruroPanelTest::OnGetCAResult(wxCommandEvent &event)
{
	//wxJSONValue *response = (wxJSONValue *) event.GetClientData();

	//if (response->HasMember("result"))
	//	wxLogMessage(wxT("SeruroPanelTest> (GetCA) API valid response."));

	/* Controller owns client data? */
    //if (response)
    //    delete[] response;
}
