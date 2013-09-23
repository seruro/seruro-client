
#include <wx/msgdlg.h>

#include "Defs.h"
#include "frames/UIDefs.h"

#include "SeruroConfig.h"
#include "SeruroTray.h"

#include "frames/SeruroMain.h"
#include "api/SeruroStateEvents.h"
#include "SeruroClient.h"


/* OSX Hack for active focus */
#if defined(__WXMAC__) || defined(__WXOSX__)
#include <Carbon/Carbon.h>
extern "C" { void CPSEnableForegroundOperation(ProcessSerialNumber *psn); }
#endif

DECLARE_APP(SeruroClient);

BEGIN_EVENT_TABLE(SeruroTray, wxTaskBarIcon)
	EVT_TASKBAR_LEFT_DCLICK(SeruroTray::OnLeftDoubleClick)
#if SERURO_ENABLE_CRYPT_PANELS
	EVT_MENU(SERURO_PANEL_ENCRYPT_ID, SeruroTray::onEncrypt)
	EVT_MENU(SERURO_PANEL_DECRYPT_ID, SeruroTray::onDecrypt)
#endif
    EVT_MENU(SERURO_PANEL_HOME_ID, SeruroTray::OnHome)
    EVT_MENU(SERURO_PANEL_CONTACTS_ID, SeruroTray::OnContacts)
	EVT_MENU(SERURO_PANEL_SEARCH_ID, SeruroTray::onSearch)
	EVT_MENU(SERURO_PANEL_SETTINGS_ID, SeruroTray::onSettings)
    EVT_MENU(SERURO_PANEL_HELP_ID, SeruroTray::OnHelp)

	EVT_MENU(SERURO_EXIT_ID, SeruroTray::OnQuit)
END_EVENT_TABLE()

void SeruroTray::OnOptionStateChange(SeruroStateEvent &event)
{
	if (event.GetValue("option_name") == "auto_download") {
		if (event.GetValue("option_value") == "true") {
			this->menu->Delete(SERURO_PANEL_SEARCH_ID);
		} else {
			this->menu->Insert(3, SERURO_PANEL_SEARCH_ID, _("Search"));
		}
	}
	event.Skip();
}

void SeruroTray::RaiseFrame()
{
    if (! main_frame) return;

	/* Do not raise the frame if a setup is running. */
	if (main_frame->IsSetupRunning()) {
		return;
	}

	if (main_frame->IsIconized()) {
        main_frame->Iconize(false);
    }

    main_frame->SetFocus();
    main_frame->Raise();
    main_frame->Show(true);
    
    /* Hardcore */
#if defined(__WXMAC__) || defined(__WXOSX__)
    ProcessSerialNumber psn;
    GetCurrentProcess(&psn);
    TransformProcessType(&psn, kProcessTransformToForegroundApplication);
    SetFrontProcess(&psn);
#endif
}

void SeruroTray::DoIconize()
{
    main_frame->Show(false);
    
    /* Hardcore */
#if defined(__WXMAC__) || defined(__WXOSX__)
    ProcessSerialNumber psn;
    GetCurrentProcess(&psn);
    TransformProcessType(&psn, kProcessTransformToUIElementApplication);
    SetFrontProcess(&psn);
#endif
    return;
}

#if SERURO_ENABLE_CRYPT_PANELS
void SeruroTray::onEncrypt(wxCommandEvent &event)
{
    RaiseFrame();
	main_frame->ChangePanel(SERURO_PANEL_ENCRYPT_ID);
}

void SeruroTray::onDecrypt(wxCommandEvent &event)
{
    RaiseFrame();
	main_frame->ChangePanel(SERURO_PANEL_DECRYPT_ID);
}
#endif

void SeruroTray::OnHome(wxCommandEvent &event)
{
    RaiseFrame();
	main_frame->ChangePanel(SERURO_PANEL_HOME_ID);
}

void SeruroTray::OnHelp(wxCommandEvent &event)
{
    RaiseFrame();
	main_frame->ChangePanel(SERURO_PANEL_HELP_ID);
}

void SeruroTray::OnContacts(wxCommandEvent &event)
{
    RaiseFrame();
	main_frame->ChangePanel(SERURO_PANEL_CONTACTS_ID);
}

void SeruroTray::onSearch(wxCommandEvent &event)
{
    RaiseFrame();
	main_frame->ChangePanel(SERURO_PANEL_SEARCH_ID);
}

void SeruroTray::onSettings(wxCommandEvent& WXUNUSED(event))
{
    RaiseFrame();
	main_frame->ChangePanel(SERURO_PANEL_SETTINGS_ID);
}

SeruroTray::SeruroTray() : wxTaskBarIcon(wxTBI_CUSTOM_STATUSITEM) /* wxTBI_CUSTOM_STATUSITEM */
{
	this->main_frame = NULL;
    
    //wxGetApp().Bind(SERURO_STATE_CHANGE, &SeruroTray::OnOptionChange, this, STATE_TYPE_OPTION);
}

void SeruroTray::SetMainFrame(SeruroFrameMain *frame)
{
	this->main_frame = frame;
}

void SeruroTray::OnLeftDoubleClick(wxTaskBarIconEvent &event)
{
	RaiseFrame();
}

void SeruroTray::OnQuit(wxCommandEvent& WXUNUSED(event)){
	RemoveIcon();
	/* Should not be vetoed. */
	if (this->main_frame) {
        main_frame->Close(true);
    }
}

void SeruroTray::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    wxMessageBox(wxString::Format(
		wxT("Seruro Client for %s."), 
		wxVERSION_STRING, wxGetOsDescription()), 
		wxT("About Seruro Client"), wxOK | wxICON_INFORMATION);
}

wxMenu* SeruroTray::CreatePopupMenu()
{
	menu = new wxMenu;

	menu->Append(SERURO_PANEL_HOME_ID, _("Home"));
	menu->Append(SERURO_PANEL_CONTACTS_ID, _("Contacts"));

	if (theSeruroConfig::Get().GetOption("auto_download") != "true") {
		/* Position 2, before 3. */
		menu->Append(SERURO_PANEL_SEARCH_ID, wxT("&Search"));
	}

	menu->Append(SERURO_PANEL_HELP_ID, _("Help"));

#if SERURO_ENABLE_CRYPT_PANELS
	menu->Append(SERURO_PANEL_ENCRYPT_ID, wxT("&Encrypt"));
	menu->Append(SERURO_PANEL_DECRYPT_ID, wxT("&Decrypt"));
#endif
    
	menu->AppendSeparator();
	menu->Append(SERURO_PANEL_SETTINGS_ID, wxT("Settings"));
	menu->Append(SERURO_EXIT_ID, wxT("E&xit"));

	return menu;
}
