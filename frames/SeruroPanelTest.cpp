
#include "SeruroPanelTest.h"

#include "../crypto/SeruroCrypto.h"
#include "../SeruroClient.h"
#include "dialogs/DecryptDialog.h"

#include "../wxJSON/wx/jsonval.h"
#include "../wxJSON/wx/jsonreader.h"

#include <wx/button.h>
#include <wx/log.h>

/* Base64 decoding. */
#include <wx/base64.h>
#include <wx/buffer.h>

/* Username and password, encryption keys. */
#include <wx/textdlg.h>

DECLARE_APP(SeruroClient);

BEGIN_EVENT_TABLE(SeruroPanelTest, wxPanel)
	EVT_BUTTON(BUTTON_GET_CA, SeruroPanelTest::OnGetCA)
	EVT_BUTTON(BUTTON_GET_P12, SeruroPanelTest::OnGetP12)
	EVT_BUTTON(BUTTON_WRITE_TOKEN, SeruroPanelTest::OnWriteToken)
	EVT_BUTTON(BUTTON_SEARCH, SeruroPanelTest::OnSearch)

	/* Request events */
	EVT_COMMAND(SERURO_API_CALLBACK_GET_CA, SERURO_API_RESULT, SeruroPanelTest::OnGetCAResult)
	EVT_COMMAND(SERURO_API_CALLBACK_GET_P12, SERURO_API_RESULT, SeruroPanelTest::OnGetP12Result)
	EVT_COMMAND(SERURO_API_CALLBACK_SEARCH, SERURO_API_RESULT, SeruroPanelTest::OnSearchResult)
END_EVENT_TABLE()

SeruroPanelTest::SeruroPanelTest(wxBookCtrlBase *book) : SeruroPanel(book, wxT("API Test"))
{
	api = new SeruroServerAPI(this->GetEventHandler());

	wxBoxSizer *info = new wxBoxSizer(wxHORIZONTAL);
	m_server_box = new wxTextCtrl(this, wxID_ANY, wxEmptyString);
	m_user_box = new wxTextCtrl(this, wxID_ANY, wxEmptyString);
	info->Add(m_server_box, wxRIGHT, 5);
	info->Add(m_user_box, wxRIGHT, 5);

	wxBoxSizer *hSizer = new wxBoxSizer(wxHORIZONTAL);
	wxButton *get_ca = new wxButton(this, BUTTON_GET_CA, wxT("API: getCA"), wxDefaultPosition, wxDefaultSize, 0);
	wxButton *get_p12 = new wxButton(this, BUTTON_GET_P12, wxT("API: getP12s"), wxDefaultPosition, wxDefaultSize, 0);
	hSizer->Add(get_ca, wxRIGHT, 5);
	hSizer->Add(get_p12, wxRIGHT, 5);

	wxBoxSizer *hSizer2 = new wxBoxSizer(wxHORIZONTAL);
	wxButton *write_token = new wxButton(this, BUTTON_WRITE_TOKEN, wxT("Write Token"), 
		wxDefaultPosition, wxDefaultSize, 0);
	hSizer2->Add(write_token, wxRIGHT, 5);

	wxBoxSizer *hSizer3 = new wxBoxSizer(wxHORIZONTAL);
	m_search_box = new wxTextCtrl(this, wxID_ANY, wxEmptyString);
	wxButton *search = new wxButton(this, BUTTON_SEARCH, wxT("Search"));
	hSizer3->Add(m_search_box, wxRIGHT, 5);
	hSizer3->Add(search, wxRight, 5);

	wxBoxSizer *vSizer = new wxBoxSizer(wxVERTICAL);
	vSizer->Add(info, wxRIGHT, 5);
	vSizer->Add(hSizer, wxRIGHT, 5);
	vSizer->Add(hSizer2, wxRIGHT, 5);
	vSizer->Add(hSizer3, wxRIGHT, 5);

	this->mainSizer->Add(vSizer, wxRIGHT, 5);

	//this->SetSizer(mainSizer);
}

void SeruroPanelTest::OnSearch(wxCommandEvent &event)
{
    wxString server_name = m_server_box->GetValue();
	wxJSONValue server = api->GetServer(server_name);
	wxString address = m_user_box->GetValue();

	wxString query = m_search_box->GetValue();

	wxJSONValue params;
	params["query"] = query;
	params["server"] = server;
	//params["address"] = address;

	SeruroRequest *request = api->CreateRequest(SERURO_API_SEARCH, params, SERURO_API_CALLBACK_SEARCH);
	request->Run();
}

void SeruroPanelTest::OnSearchResult(wxCommandEvent &event)
{
	/* Do nothing */
}

void SeruroPanelTest::OnWriteToken(wxCommandEvent &event)
{
	wxString server_name = m_server_box->GetValue();
	wxString address = m_user_box->GetValue();

	wxString current_token = wxGetApp().config->GetToken(server_name, address);
	wxLogMessage(wxT("Token for open.seruro.com, ted@valdrea.com: %s"), current_token);

	wxString token_string;
	wxTextEntryDialog *get_token = new wxTextEntryDialog(this, wxT("Enter token"));
	if (get_token->ShowModal() == wxID_OK) {
		token_string = get_token->GetValue();
	} else {
		return;
	}
	get_token->Destroy();

	bool results = wxGetApp().config->WriteToken(server_name, address, 
		token_string);

}

void SeruroPanelTest::OnGetP12(wxCommandEvent &event)
{
	wxJSONValue params; /* no params */
	//wxString server_name = wxT("Open Seruro");
	//wxString server_name = m_server_box->GetValue();
    wxString server_name = m_server_box->GetValue();
	params["server"] = api->GetServer(server_name);
	/* Todo, revisit an explicit auth. */
	params["address"] = m_user_box->GetValue();

	SeruroRequest *request = api->CreateRequest(SERURO_API_GET_P12, params, SERURO_API_CALLBACK_GET_P12);
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

	api->InstallP12(response);

#if 0
	if (! response.HasMember("success") || ! response["success"].AsBool()) {
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
	DecryptDialog *decrypt_dialog = new DecryptDialog(response["method"].AsString());
	if (decrypt_dialog->ShowModal() == wxID_OK) {
		p12_key = decrypt_dialog->GetValue();
	} else {
		return;
	}
	/* Remove the modal from memory. */
	decrypt_dialog->Destroy();

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
#endif

	return;
}

void SeruroPanelTest::OnGetCA(wxCommandEvent &event)
{
	wxJSONValue params; /* no params */
	wxString server_name = wxT("Open Seruro");
	params["server"] = api->GetServer(server_name);//wxT("open.seruro.com");

	SeruroRequest *request = api->CreateRequest(SERURO_API_GET_CA, params, SERURO_API_CALLBACK_GET_CA);
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
