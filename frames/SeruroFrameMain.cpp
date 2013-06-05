
#include <wx/icon.h>
#include <wx/notebook.h>
#include <wx/artprov.h>

#include "SeruroFrameMain.h"
#include "SeruroPanelSettings.h"
#include "SeruroPanelSearch.h"
#include "../setup/SeruroSetup.h"

#if SERURO_ENABLE_CRYPT_PANELS
#include "SeruroPanelDecrypt.h"
#include "SeruroPanelEncrypt.h"
#endif

#if SERURO_ENABLE_DEBUG_PANELS
#include "SeruroPanelTest.h"
#endif

int seruro_panels_ids[SERURO_MAX_PANELS];
int seruro_panels_size;

BEGIN_EVENT_TABLE(SeruroFrameMain, wxFrame)
	/* Events for Window interaction */
	EVT_MENU	(wxID_EXIT,     SeruroFrameMain::OnQuit)
    //EVT_MENU	(wxID_ABOUT,	SeruroFrame::OnAbout)
	EVT_ICONIZE	(				SeruroFrameMain::OnIconize)
	EVT_CLOSE	(				SeruroFrameMain::OnClose)
	/* Events for optional setup wizard */
	//EVT_MENU    (seruroID_SETUP_ALERT,  SeruroFrameMain::OnSetupRun)
    EVT_WIZARD_CANCEL(wxID_ANY,   SeruroFrameMain::OnSetupCancel)
    EVT_WIZARD_FINISHED(wxID_ANY, SeruroFrameMain::OnSetupFinished)	
END_EVENT_TABLE()

SeruroFrameMain::SeruroFrameMain(const wxString& title, int width, int height) : SeruroFrame(title)
{
	/* Set the default size of the entire application. */
	this->SetSize(width, height);

	/* Set tray icon data */
	tray = new SeruroTray();
	tray->SetMainFrame(this);

	/* Todo: replace icon */
	//#if defined(__WXMSW__)
    	SetIcon(wxIcon(icon_good));
	//	tray->SetIcon(wxICON(main), wxT("Seruro Client"));
	//#endif
    //#if defined(__WXMAC__)
        tray->SetIcon(wxIcon(icon_good), wxT(SERURO_APP_NAME));
    //#endif

	/* Testing IMGCTRL */
	//const wxSize imageSize(32, 32);
	//wxImageList *list = new wxImageList(imageSize.GetWidth(), imageSize.GetHeight());
	//list->Add(wxArtProvider::GetIcon(wxART_INFORMATION, wxART_OTHER, imageSize));

	/* Add singular panel */
	wxPanel *panel = new wxPanel(this, wxID_ANY);
	book = new wxNotebook(panel, wxID_ANY, wxPoint(-1, -1), wxSize(-1, -1), wxNB_TOP);
	//book->SetImageList(list);

	//wxPanel *page1 = new wxPanel(book, wxID_ANY);
	//wxStaticText *page1_t = new wxStaticText(page1, wxID_ANY, wxT("This is a page1."));

	//book->AddPage(page1, wxT("Page 1"), false, 0);
	//book->AddPage(page1, wxT("Page 2"), false, 0);

	/* Footer sizer */
	wxBoxSizer *panelSizer = new wxBoxSizer(wxVERTICAL);
	panelSizer->Add(book, 1, wxEXPAND);
	panel->SetSizer(panelSizer);
	panel->Layout();

	this->mainSizer->Add(panel, 1, wxEXPAND, 5);
	/*
	SeruroFrameConfigure	*configure	= new SeruroFrameConfigure(wxT("Seruro Client: Configure"));
	SeruroFrameSearch		*search		= new SeruroFrameSearch(wxT("Seruro Client: Search"));
	*/
}

/* The frame's (book's) panels may depend on objects created after the first frame,
 * for example: the configuration and loggin facilities.
 */
void SeruroFrameMain::AddPanels()
{
	/* Add content */
	search_panel = new SeruroPanelSearch(book);
	/* Must define the order of panels. */
	seruro_panels_ids[seruro_panels_size++] = SERURO_PANEL_SEARCH_ID;
#if SERURO_ENABLE_CRYPT_PANELS
	SeruroPanelEncrypt	 *encrypt	= new SeruroPanelEncrypt(book);
	seruro_panels_ids[seruro_panels_size++] = SERURO_PANEL_ENCRYPT_ID;
	SeruroPanelDecrypt	 *decrypt	= new SeruroPanelDecrypt(book);
	seruro_panels_ids[seruro_panels_size++] = SERURO_PANEL_DECRYPT_ID;
#endif
	settings_panel = new SeruroPanelSettings(book);
	seruro_panels_ids[seruro_panels_size++] = SERURO_PANEL_SETTINGS_ID;
	
#if SERURO_ENABLE_DEBUG_PANELS
	test_panel = new SeruroPanelTest(book);
	seruro_panels_ids[seruro_panels_size++] = 0; /* Test is not controllable. */
#endif
}

/* The tray menu generates events based on IDs, these IDs correspond to pages. 
 * The mainFrame should change the notebook selection to the given page.
 */
void SeruroFrameMain::ChangePanel(int panel_id)
{
	/* Iterate through the vector of panel ids, if a match is found, set selection. */
	for (int i = 0; i < seruro_panels_size; i++) {
		if (panel_id == seruro_panels_ids[i]) {
			this->book->SetSelection(i);
		}
	}
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
