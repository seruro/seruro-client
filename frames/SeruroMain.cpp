
#include <wx/icon.h>
#include <wx/artprov.h>

#include "SeruroMain.h"

#include "../setup/SeruroSetup.h"
#include "../SeruroClient.h"
#include "UIDefs.h"

#include "SeruroSettings.h"
#include "SeruroSearch.h"
#include "SeruroContacts.h"

#if SERURO_ENABLE_CRYPT_PANELS
#include "SeruroPanelDecrypt.h"
#include "SeruroPanelEncrypt.h"
#endif

#if SERURO_ENABLE_DEBUG_PANELS
#include "SeruroPanelTest.h"
#endif

/* Potential MSW icons. */
#ifdef __WXMSW__
#include "../resources/images/logo_block_256_flat.png.h"
#include "../resources/images/logo_block_128_flat.png.h"
#include "../resources/images/logo_block_64_flat.png.h"
#include "../resources/images/logo_block_32_flat.png.h"
#include "../resources/images/logo_block_16_flat.png.h"
#endif
 
/* Potential OSX icons. */
#if defined(__WXOSX__) || defined(__WXMAC__)
#include "../resources/images/tray_osx_hard_black.png.h"
#endif

/* OSX Hack for active focus */
#if defined(__WXMAC__) || defined(__WXOSX__)
#include <Carbon/Carbon.h>
extern "C" { void CPSEnableForegroundOperation(ProcessSerialNumber *psn); }
#endif

#include <wx/iconbndl.h>

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

	EVT_NOTEBOOK_PAGE_CHANGED(SERURO_NOTEBOOK_ID, SeruroFrameMain::OnChange)
	//EVT_NOTEBOOK_PAGE_CHANGING(SERURO_NOTEBOOK_ID, SeruroFrameMain::OnChange)
END_EVENT_TABLE()

DECLARE_APP(SeruroClient);

SeruroFrameMain::SeruroFrameMain(const wxString& title, int width, int height) : SeruroFrame(title)
{
	/* Set the default size of the entire application. */
	this->SetSize(width, height);

	/* Set tray icon data */
	tray = new SeruroTray();
	tray->SetMainFrame(this);

	wxIconBundle icon_bundle;
	wxIcon icon;

#ifdef __WXMSW__
	icon.CopyFromBitmap(wxGetBitmapFromMemory(logo_block_256_flat));
	icon_bundle.AddIcon(icon);
	icon.CopyFromBitmap(wxGetBitmapFromMemory(logo_block_128_flat));
	icon_bundle.AddIcon(icon);
	icon.CopyFromBitmap(wxGetBitmapFromMemory(logo_block_64_flat));
	icon_bundle.AddIcon(icon);
	icon.CopyFromBitmap(wxGetBitmapFromMemory(logo_block_32_flat));
	icon_bundle.AddIcon(icon);
	icon.CopyFromBitmap(wxGetBitmapFromMemory(logo_block_16_flat));
	icon_bundle.AddIcon(icon);
#endif
    
#if defined(__WXOSX__) || defined(__WXMAC__)
    icon.CopyFromBitmap(wxGetBitmapFromMemory(tray_osx_hard_black));
#endif

	tray->SetIcon(icon, _(SERURO_APP_NAME));
	this->SetIcons(icon_bundle);

    /* No setup started. */
    this->setup = 0;

	/* Add singular panel */
	book = new wxNotebook(this, SERURO_NOTEBOOK_ID, wxDefaultPosition, wxDefaultSize, 
		wxNB_FIXEDWIDTH);
	this->mainSizer->Add(book, 1, wxEXPAND, 5);
}

/* The frame's (book's) panels may depend on objects created after the first frame,
 * for example: the configuration and loggin facilities.
 */
void SeruroFrameMain::AddPanels()
{
	/* Add content */
    contacts_panel = new SeruroPanelContacts(book);
    seruro_panels_ids[seruro_panels_size++] = SERURO_PANEL_CONTACTS_ID;
    
    /* Optional search panel (if certificates/contacts are not automatically downloaded). */
	search_panel = new SeruroPanelSearch(book);
	seruro_panels_ids[seruro_panels_size++] = SERURO_PANEL_SEARCH_ID;
    
    /* Always create the search, but remove if it will not be used. */
    if (theSeruroConfig::Get().GetOption("auto_download") == "true") {
        book->RemovePage(seruro_panels_size-1);
    }
    
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
    
    wxGetApp().Bind(SERURO_STATE_CHANGE, &SeruroFrameMain::OnOptionChange, this, STATE_TYPE_OPTION);
    
    this->Show();
    this->Layout();
    this->Center();
}

void SeruroFrameMain::OnOptionChange(SeruroStateEvent &event)
{
    size_t search_panel_id = -1;
    
    /* Only handle "auto_download" option changes. */
    if (event.GetValue("option_name") != "auto_download") {
        event.Skip();
        return;
    }

    for (size_t i = 0; i < book->GetPageCount(); ++i) {
        if (this->search_panel == book->GetPage(i)) {
            search_panel_id = i;
            break;
        }
    }
    
    /* Perform the appropriate UI action. */
    if (event.GetValue("option_value") == "true") {
        book->RemovePage(search_panel_id);
    } else {
        book->InsertPage(1, this->search_panel, _("Search"));
    }
    
    event.Skip();
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

void SeruroFrameMain::OnChange(wxBookCtrlEvent &event)
{
	/* The search panel may need to update it's UI if servers were added/removed. */
	wxLogMessage(_("Notebook panel changed (%d)."), event.GetSelection());
	if (seruro_panels_ids[event.GetSelection()] == SERURO_PANEL_SEARCH_ID) {
		//((SeruroPanelSearch*) this->search_panel)->DoFocus();
	}
}

void SeruroFrameMain::OnClose(wxCloseEvent &event)
{
    /* The close is not a "QUIT". */
	if (event.CanVeto()) {
        tray->DoIconize();
        event.Veto();
        
        return;
	}
	
	/* If there is a running wizard, cancel it. */
	this->StopSetup();

	/* Delete the tray icon/controller. */
	if (tray) {
		tray->RemoveIcon();
		tray->Destroy();
	}

	/* Using the "QUIT" */
	Destroy();
}

void SeruroFrameMain::OnIconize(wxIconizeEvent& WXUNUSED(event))
{
	this->Hide();
}

void SeruroFrameMain::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    // true is to force the frame to close
    this->Close(true);
}

void SeruroFrameMain::StartSetup()
{
    SeruroSetup *initial_setup;
    
    /* There is a "first-launch" setup wizard. */
    this->setup = 0;
	if (theSeruroConfig::Get().HasConfig()) {
        /* No setup needed. */
        return;
    }
    
    /* The panels and "book view" should be hidden while the wizard is running. */
    this->Hide();
        
    initial_setup = new SeruroSetup(this);
    initial_setup->RunWizard(initial_setup->GetInitialPage());
    
    /* Save pointer to setup. */
    this->setup = initial_setup;
}

void SeruroFrameMain::StopSetup()
{
    /* Ask the setup to stop, the application is closing. */
    if (this->setup == 0) {
        return;
    }
    
    ((SeruroSetup *) this->setup)->EndModal(0);
    this->setup->Close();
    this->setup = 0;
}

void SeruroFrameMain::OnSetupRun(wxCommandEvent &event)
{

}

void SeruroFrameMain::OnSetupCancel(wxWizardEvent& event)
{
    /* Is this correct? */
    this->setup = 0;
    
    if (theSeruroConfig::Get().GetServerNames().size() == 0) {
        wxLogMessage(_("SeruroFrameMain> (OnSetupCancel) the initial setup was cancled."));
		if (! SERURO_ALLOW_NO_ACCOUNTS) {
			this->Close(true);
            return;
		}
    }
    
    this->Show();
}

void SeruroFrameMain::OnSetupFinished(wxWizardEvent& event)
{
    /* Is this correct? */
    this->setup = 0;
    
    if (theSeruroConfig::Get().GetServerNames().size() == 0) {
        wxLogMessage(_("SeruroFrameMain> (OnSetupFinished) there were no servers added?"));
		if (! SERURO_ALLOW_NO_ACCOUNTS) {
			this->Close(true);
            return;
		}
    }
    
    this->Show();
}
