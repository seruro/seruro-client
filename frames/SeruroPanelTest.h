
#ifndef H_SeruroPanelTest
#define H_SeruroPanelTest

#include "SeruroFrame.h"

#include "../api/SeruroServerAPI.h"

enum api_actions_t
{
	BUTTON_GET_CA,
	BUTTON_GET_P12,

	BUTTON_WRITE_TOKEN,
	BUTTON_SEARCH
};

enum api_result_handlers_t
{
	CALLBACK_GET_CA,
	CALLBACK_GET_P12,
	CALLBACK_GET_CERT,
	CALLBACK_GET_CRL,
	CALLBACK_SEARCH
};

// Define a new frame type: this is going to be our main frame
class SeruroPanelTest : public SeruroPanel
{
public:
    // ctor(s)
    SeruroPanelTest(wxBookCtrlBase *book);
	~SeruroPanelTest() {
		delete [] api;
	}

	void OnGetCA(wxCommandEvent &event);
	void OnGetCAResult(wxCommandEvent &event);

	void OnGetP12(wxCommandEvent &event);
	void OnGetP12Result(wxCommandEvent &event);

	void OnWriteToken(wxCommandEvent &event);

	void OnSearch(wxCommandEvent &event);
	void OnSearchResult(wxCommandEvent &event);

private:
	SeruroServerAPI *api;

	wxTextCtrl *m_server_box;
	wxTextCtrl *m_user_box;
	wxTextCtrl *m_search_box;
	
	DECLARE_EVENT_TABLE();
};

#endif