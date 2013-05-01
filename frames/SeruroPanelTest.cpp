
#include "SeruroPanelTest.h"
#include "../crypto/SeruroCrypto.h"

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

	/* The getP12 API call should respond with up to 3 P12s. */
	if (! response.HasMember("p12"))
	{
		wxLogMessage(wxT("SeruroPanelTest> (GetP12) Response does not include P12 data."));
		return;
	}

	/* Get password from user for p12 containers. */
	wxString p12_key;
	wxPasswordEntryDialog *get_key = new wxPasswordEntryDialog(this, wxT("Enter decryption key"));
	if (get_key->ShowModal() == wxID_OK) {
		p12_key = get_key->GetValue();
	} else {
		return;
	}
	/* Remove the modal from memory. */
	get_key->Destroy();

	SeruroCrypto *cryptoHelper = new SeruroCrypto();

	/* Install all P12 b64 blobs. */
	wxArrayString p12_blobs = response["p12"].GetMemberNames();
	wxString p12_encoded;
	size_t p12_decode_error;
	wxMemoryBuffer p12_decoded;

	for (size_t i = 0; i < p12_blobs.size(); i++) {
		p12_encoded = response["p12"][p12_blobs[i]].AsString();
		p12_decode_error = 0;
		p12_decoded = wxBase64Decode(p12_encoded, wxBase64DecodeMode_Relaxed, &p12_decode_error);
		
		if (p12_decode_error != 0) {
			/* posErr (the argument name) is the position in the encoded string of the non-decodable object. */
			wxLogMessage(wxT("SeruroPanelTest> (GetP12) Could not decode position %d in p12 blob '%s'."), 
				p12_decode_error, p12_blobs[i]);
			continue;
		}

		cryptoHelper->InstallP12(p12_decoded, p12_key);
	}

	/* Cleanup. */
	delete cryptoHelper;
	p12_key.Empty();

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
