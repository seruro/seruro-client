
#ifndef H_SeruroPanelTest
#define H_SeruroPanelTest

#include "SeruroFrame.h"

#include "../api/SeruroServerAPI.h"

enum api_actions_t
{
	BUTTON_GET_CA
};

// Define a new frame type: this is going to be our main frame
class SeruroPanelTest : public SeruroPanel
{
public:
    // ctor(s)
    SeruroPanelTest(wxBookCtrlBase *book);

	void OnGetCA(wxCommandEvent &event);
	void OnResult(wxCommandEvent &event);

private:
	SeruroServerAPI *api;
	
	DECLARE_EVENT_TABLE();
};

#endif