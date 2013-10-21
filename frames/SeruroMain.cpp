
#include <wx/icon.h>
#include <wx/artprov.h>

#include "SeruroMain.h"

#include "../setup/CancelSetupDialog.h"
#include "../setup/SeruroSetup.h"
#include "../SeruroClient.h"
#include "UIDefs.h"

#include "SeruroSettings.h"
#include "SeruroSearch.h"
#include "SeruroContacts.h"

#include "SeruroHome.h"
#include "SeruroHelp.h"

/* For downloading contacts. */
#include "../api/ServerMonitor.h"

/* Close the app if the wizard is canceled. */
#define SERURO_ALLOW_NO_ACCOUNTS 0

#if SERURO_ENABLE_CRYPT_PANELS
#include "SeruroPanelDecrypt.h"
#include "SeruroPanelEncrypt.h"
#endif

#if SERURO_ENABLE_DEBUG_PANELS
#include "SeruroPanelTest.h"
#endif

/* Potential MSW icons. */
#ifdef __WXMSW__
#include "../resources/images/logo_new_256_flat.png.h"
#include "../resources/images/logo_new_128_flat.png.h"
#include "../resources/images/logo_new_64_flat.png.h"
#include "../resources/images/logo_new_32_flat.png.h"
#include "../resources/images/logo_new_16_flat.png.h"
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

enum {
    MENU_RECHECK_CONTACTS_ID,
};

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
	this->app_ending = false;

	wxIconBundle icon_bundle;
	wxIcon icon;

#ifdef __WXMSW__
	icon.CopyFromBitmap(wxGetBitmapFromMemory(logo_new_256_flat));
	icon_bundle.AddIcon(icon);
	icon.CopyFromBitmap(wxGetBitmapFromMemory(logo_new_128_flat));
	icon_bundle.AddIcon(icon);
	icon.CopyFromBitmap(wxGetBitmapFromMemory(logo_new_64_flat));
	icon_bundle.AddIcon(icon);
	icon.CopyFromBitmap(wxGetBitmapFromMemory(logo_new_32_flat));
	icon_bundle.AddIcon(icon);
	icon.CopyFromBitmap(wxGetBitmapFromMemory(logo_new_16_flat));
	icon_bundle.AddIcon(icon);
#endif
    
#if defined(__WXOSX__) || defined(__WXMAC__)
    /* Set monochromatic icon for OSX's status bar. */
    icon.CopyFromBitmap(wxGetBitmapFromMemory(tray_osx_hard_black));
    
    /* Create menubar for OSX. */
    this->AddOSXMenu();
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

void SeruroFrameMain::AddOSXMenu()
{
    //wxApp::s_macPreferencesMenuItemId = 1;
    wxMenuBar *menubar = new wxMenuBar();
    wxMenu *file_menu = new wxMenu();
    wxMenu *help_menu = new wxMenu();
    wxMenu *window_menu = new wxMenu();
    
    /* Set up standard menu items. */
    help_menu->Append(SERURO_PANEL_SETTINGS_ID, _("Preferences...") + "\tCtrl+,");
    help_menu->Append(wxID_ABOUT, _("About Seruro Client"));
    //help_menu->Append(SERURO_EXIT_ID, _(SERURO_MENU_EXIT));
    
    /* Set up file menu. */
    file_menu->Append(wxID_ANY, _("Add New Account") + "\tCtrl+N");
    file_menu->Append(wxID_ANY, _("Add New Server") + "\tCtrl+S");
    file_menu->AppendSeparator();
    file_menu->Append(wxID_ANY, _("Save Log..."));
    file_menu->Append(wxID_ANY, _("Check for New Contacts..."));
    menubar->Append(file_menu, _("File"));
    
    /* Set up window menu. */
    window_menu->Append(wxID_ANY, _("Hide Window"));
    window_menu->AppendSeparator();
    window_menu->Append(wxID_ANY, _("Show Seruro Home") + "\tShift+Ctrl+1");
    window_menu->Append(wxID_ANY, _("Show Contacts") + "\tShift+Ctrl+2");
    window_menu->Append(wxID_ANY, _("Show Accounts") + "\tShift+Ctrl+3");
    window_menu->Append(wxID_ANY, _("Show Applications") + "\tShift+Ctrl+4");
    window_menu->Append(wxID_ANY, _("Show Log") + "\tShift+Ctrl+5");
    window_menu->AppendSeparator();
    window_menu->Append(wxID_ANY, _("Bring All to Front"));
    menubar->Append(window_menu, _("Window"));
    
    /* Set up help items. */
    help_menu->Append(wxID_ANY, _("Report and issue..."));
    help_menu->AppendSeparator();
    help_menu->Append(wxID_ANY, _("View online help..."));
    help_menu->Append(SERURO_PANEL_HELP_ID, _("Seruro Help"));
    menubar->Append(help_menu, _("Help"));
    
    this->SetMenuBar(menubar);
}

/* The frame's (book's) panels may depend on objects created after the first frame,
 * for example: the configuration and loggin facilities.
 */
void SeruroFrameMain::AddPanels()
{
	/* Add content */
    home_panel = new SeruroPanelHome(book);
    seruro_panels_ids[seruro_panels_size++] = SERURO_PANEL_HOME_ID;
    
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
    
    help_panel = new SeruroPanelHelp(book);
    seruro_panels_ids[seruro_panels_size++] = SERURO_PANEL_HELP_ID;
	
#if SERURO_ENABLE_DEBUG_PANELS
	test_panel = new SeruroPanelTest(book);
	seruro_panels_ids[seruro_panels_size++] = 0; /* Test is not controllable. */
#endif
    
	/* No setup at construction. */
	this->setup_running = false;

    wxGetApp().Bind(SERURO_STATE_CHANGE, &SeruroFrameMain::OnOptionChange, this, STATE_TYPE_OPTION);
	wxGetApp().Bind(SERURO_STATE_CHANGE, &SeruroFrameMain::OnServerStateChange, this, STATE_TYPE_SERVER);
    
    this->Show();
    this->Layout();
    this->Center();
}

void SeruroFrameMain::OnServerStateChange(SeruroStateEvent &event)
{
	event.Skip();

	/* If there are no servers left after a remove action, open the setup. */
	if (event.GetAction() == STATE_ACTION_REMOVE) {
		if (theSeruroConfig::Get().GetServerList().size() == 0) {
			this->StartSetup();
		}
	}

}

void SeruroFrameMain::OnOptionChange(SeruroStateEvent &event)
{
    size_t search_panel_id = -1;
    
    /* Only handle "auto_download" option changes. */
    event.Skip();
	if (event.GetValue("option_name") != "auto_download") {
        return;
    }

    for (size_t i = 0; i < book->GetPageCount(); ++i) {
        if (this->search_panel == book->GetPage(i)) {
            search_panel_id = i;
            break;
        }
    }

	/* The search panel was never added. */
	if (search_panel_id < 0) {
		return;
	}
    
    /* Perform the appropriate UI action. */
    if (event.GetValue("option_value") == "true") {
        book->RemovePage(search_panel_id);
    } else {
        book->InsertPage(2, this->search_panel, _("Search"));
    }
}

/* The tray menu generates events based on IDs, these IDs correspond to pages. 
 * The mainFrame should change the notebook selection to the given page.
 */
void SeruroFrameMain::ChangePanel(int panel_id)
{
    int offset;
    
	if (this->IsSetupRunning()) {
		return;
	}
    
    /* Search may be removed. */
    offset = 0;
    if (theSeruroConfig::Get().GetOption("auto_download") == "true") {
        if (panel_id != SERURO_PANEL_HOME_ID && panel_id != SERURO_PANEL_CONTACTS_ID) {
            offset -= 1;
        }
    }

	/* Iterate through the vector of panel ids, if a match is found, set selection. */
	for (int i = 0; i < seruro_panels_size; i++) {
		if (panel_id == seruro_panels_ids[i]) {
			this->book->SetSelection(i + offset);
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
	
	/* Signal to potential lazy-event handlers that the application is ending. */
	this->app_ending = true;

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

wxWindow *SeruroFrameMain::GetTop()
{
    if (this->setup_running) {
        return setup;
    } else {
        return this;
    }
}

void SeruroFrameMain::SetSetup(SeruroSetup *setup)
{
    if (this->setup_running) {
        /* There should never be more than one setup running. */
        setup->Destroy();
        return;
    }
    
    this->setup = setup;
    this->setup_running = true;
    setup->RunWizard(setup->GetInitialPage());
}

void SeruroFrameMain::StartSetup(bool force)
{
    SeruroSetup *initial_setup;
    
    if (this->setup_running) {
        /* Something asked for a new setup, but one was already running. */
        return;
    }
    
    /* There is a "first-launch" setup wizard. */
    this->setup = 0;
	if (! force && theSeruroConfig::Get().HasConfig()) {
        /* No setup needed. */
        return;
    }
    
    /* The panels and "book view" should be hidden while the wizard is running. */
    this->Hide();
    
    initial_setup = new SeruroSetup(this);
	this->setup_running = true;
    /* Save pointer to setup. */
    this->setup = initial_setup;
    
    /* Execute the wizard. */
    initial_setup->RunWizard(initial_setup->GetInitialPage());
}

void SeruroFrameMain::StopSetup()
{
    /* Ask the setup to stop, the application is closing. */
    if (! this->setup_running || this->setup == 0) {
        return;
    }
    
    ((SeruroSetup *) this->setup)->EndModal(0);
    this->setup->Close();
	this->setup->Destroy();

	this->setup_running = false;
    this->setup = 0;
}

void SeruroFrameMain::OnFinishSetup()
{
    /* Is this correct? */
	this->setup_running = false;

	if (this->app_ending) {
		return;
	}
    
    if (theSeruroConfig::Get().GetServerNames().size() == 0) {
        wxLogMessage(_("SeruroFrameMain> (OnSetupCancel) the initial setup was cancled."));
		if (! SERURO_ALLOW_NO_ACCOUNTS) {
			this->Close(true);
            return;
		}
    }
    
    /* Restart the monitor thread (to allow certificates/contacts to download). */
    //wxGetApp().StopMonitor();
    //wxGetApp().StartMonitor();
    
    /* Restart the monitor (server) using a transient call to the server monitor. */
    ServerMonitor *transient_monitor = new ServerMonitor(true);
    transient_monitor->Monitor();
    delete transient_monitor;
    
    this->Show();
	//this->ChangePanel(SERURO_PANEL_HOME_ID);
}

void SeruroFrameMain::OnSetupCancel(wxWizardEvent& event)
{
    CancelSetupDialog *dialog;
    bool should_cancel;
    
    should_cancel = true;
    if (theSeruroConfig::Get().GetServerNames().size() == 0) {
        if (! SERURO_ALLOW_NO_ACCOUNTS) {
            /* Warn about canceling. */
            dialog = new CancelSetupDialog(this);
            should_cancel = (dialog->ShowModal() == wxID_YES);
            delete dialog;
        }
    }
    
    if (! should_cancel) {
        /* The wizard should not cancel. */
        event.Veto();
        return;
    }
    
    this->OnFinishSetup();
}

void SeruroFrameMain::OnSetupFinished(wxWizardEvent& event)
{
    this->OnFinishSetup();
}
