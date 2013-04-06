
#include <wx/icon.h>
#include <wx/notebook.h>
#include <wx/artprov.h>

#include "SeruroFrameMain.h"
#include "../SeruroTray.h"
#include "../SeruroSetup.h"

BEGIN_EVENT_TABLE(SeruroFrameMain, wxFrame)
	/* Events for Window interaction */
	EVT_MENU	(Event_Quit,	SeruroFrameMain::OnQuit)
    //EVT_MENU	(Event_About,	SeruroFrame::OnAbout)
	EVT_ICONIZE	(				SeruroFrameMain::OnIconize)
	EVT_CLOSE	(				SeruroFrameMain::OnClose)
	/* Events for optional setup wizard */
	EVT_MENU    (seruroID_SETUP_ALERT,  SeruroFrameMain::OnSetupRun)
    EVT_WIZARD_CANCEL(wxID_ANY,   SeruroFrameMain::OnSetupCancel)
    EVT_WIZARD_FINISHED(wxID_ANY, SeruroFrameMain::OnSetupFinished)	
END_EVENT_TABLE()

SeruroFrameMain::SeruroFrameMain(const wxString& title) : SeruroFrame(title)
{
	/* Set tray icon data */
	tray = new SeruroTray();
	tray->SetMainFrame(this);

	/* Todo: replace icon */
	//#if defined(__WXMSW__)
    	SetIcon(wxIcon(icon_good));
	//	tray->SetIcon(wxICON(main), wxT("Seruro Client"));
	//#endif
    //#if defined(__WXMAC__)
        tray->SetIcon(wxIcon(icon_good), wxT("Seruro Client"));
    //#endif

	/* There is an optional setup wizard. */
	if (false) {
		SeruroSetup setup(this);
		setup.RunWizard(setup.GetFirstPage());
	}

	/* Testing IMGCTRL */
	const wxSize imageSize(32, 32);
	wxImageList *list = new wxImageList(imageSize.GetWidth(), imageSize.GetHeight());
	list->Add(wxArtProvider::GetIcon(wxART_INFORMATION, wxART_OTHER, imageSize));

	/* Add singular panel */
	wxPanel *panel = new wxPanel(this, wxID_ANY);
	wxNotebook *book = new wxNotebook(panel, wxID_ANY, wxPoint(-1, -1), wxSize(-1, -1), wxNB_TOP);
	book->SetImageList(list);

	/* Add content */
	SeruroPanel *configure = new SeruroPanel(book, wxT("Configure"));

	//wxPanel *page1 = new wxPanel(book, wxID_ANY);
	//wxStaticText *page1_t = new wxStaticText(page1, wxID_ANY, wxT("This is a page1."));

	//book->AddPage(page1, wxT("Page 1"), false, 0);
	//book->AddPage(page1, wxT("Page 2"), false, 0);

	/* Footer sizer */
	wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
	mainSizer->Add(book, 1, wxEXPAND);
	panel->SetSizer(mainSizer);
	panel->Layout();


	/*
	SeruroFrameConfigure	*configure	= new SeruroFrameConfigure(wxT("Seruro Client: Configure"));
	SeruroFrameSearch		*search		= new SeruroFrameSearch(wxT("Seruro Client: Search"));
	*/
}

void SeruroFrameMain::OnClose(wxCloseEvent &event)
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

void SeruroFrameMain::OnIconize(wxIconizeEvent& WXUNUSED(event))
{
	// Remove program from taskbar
	Hide();
}

void SeruroFrameMain::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    // true is to force the frame to close
    Close(true);
}

void SeruroFrameMain::OnSetupRun(wxCommandEvent &event)
{

}

void SeruroFrameMain::OnSetupCancel(wxWizardEvent& event)
{

}

void SeruroFrameMain::OnSetupFinished(wxWizardEvent& event)
{

}
