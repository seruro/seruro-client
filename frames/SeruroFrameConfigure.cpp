
#include "SeruroFrameConfigure.h"
#include "../SeruroTray.h"

BEGIN_EVENT_TABLE(SeruroFrameConfigure, SeruroFrame)
		EVT_CLOSE	(SeruroFrameConfigure::OnClose)
END_EVENT_TABLE()

SeruroFrameConfigure::SeruroFrameConfigure(const wxString& title) : SeruroFrame(title)
{
	tray = new SeruroTray();
	tray->SetMainFrame(this);

	#if defined(__WXMSW__)
        tray->SetIcon(wxICON(main), wxT("Seruro Client"));
	#endif
    #if defined(__WXMAC__)
        tray->SetIcon(wxIcon(icon_good), wxT("Seruro Client"));
    #endif

	wxBoxSizer *VertSizer, *HorzSizer;
	VertSizer = new wxBoxSizer(wxVERTICAL);
	HorzSizer = new wxBoxSizer(wxHORIZONTAL);

	wxStaticText *SearchLabel;
	SearchLabel = new wxStaticText(this, wxID_ANY, wxT("Name or Email:"), 
		wxDefaultPosition, wxDefaultSize, 0);
	HorzSizer->Add(SearchLabel, wxRIGHT, 5);

	//SearchControl = new wxSearchCtrl
}

void SeruroFrameConfigure::OnClose(wxCloseEvent &event)
{
	if (event.CanVeto()) {
		Show(false);
		event.Veto();
		return;
	}
	
	if (tray) {
		tray->RemoveIcon();
		tray->Destroy();
	}
	/* Using the "QUIT" */
	Destroy();
}
