
#include <wx/menu.h>
#include <wx/msgdlg.h>

#include "Defs.h"
#include "frames/UIDefs.h"

#include "SeruroTray.h"
#include "frames/SeruroFrameMain.h"
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
	EVT_MENU(SERURO_PANEL_SEARCH_ID, SeruroTray::onSearch)
	EVT_MENU(SERURO_PANEL_SETTINGS_ID, SeruroTray::onSettings)
	EVT_MENU(SERURO_EXIT_ID, SeruroTray::OnQuit)
END_EVENT_TABLE()

void SeruroTray::RaiseFrame()
{
    if (! main_frame) return;
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
		wxT("Welcome to %s!\n\nThis is the minimal wxWidgets sample\nrunning under %s."), 
		wxVERSION_STRING, wxGetOsDescription()), 
		wxT("About wxWidgets minimal sample"), wxOK | wxICON_INFORMATION);
}

wxMenu* SeruroTray::CreatePopupMenu()
{
	wxMenu *popup = new wxMenu;

    popup->Append(SERURO_PANEL_SEARCH_ID, wxT("&Search"));
#if SERURO_ENABLE_CRYPT_PANELS
	popup->Append(SERURO_PANEL_ENCRYPT_ID, wxT("&Encrypt"));
	popup->Append(SERURO_PANEL_DECRYPT_ID, wxT("&Decrypt"));
#endif
    
	popup->AppendSeparator();
	popup->Append(SERURO_PANEL_SETTINGS_ID, wxT("Settings"));
	popup->Append(SERURO_EXIT_ID, wxT("E&xit"));

	return popup;
}
