
#include <wx/menu.h>
#include <wx/msgdlg.h>

#include "Defs.h"
#include "frames/UIDefs.h"

#include "SeruroTray.h"
#include "frames/SeruroFrameMain.h"

/* OSX Hack for active focus */
#if defined(__WXMAC__) || defined(__WXOSX__)
#include <Carbon/Carbon.h>
extern "C" { void CPSEnableForegroundOperation(ProcessSerialNumber *psn); }
#endif

BEGIN_EVENT_TABLE(SeruroTray, wxTaskBarIcon)
	EVT_TASKBAR_LEFT_DCLICK(SeruroTray::OnLeftDoubleClick)
#if SERURO_ENABLE_CRYPT_PANELS
	EVT_MENU(SERURO_PANEL_ENCRYPT_ID, SeruroTray::onEncrypt)
	EVT_MENU(SERURO_PANEL_DECRYPT_ID, SeruroTray::onDecrypt)
#endif
	EVT_MENU(SERURO_PANEL_SEARCH_ID, SeruroTray::onSearch)
	EVT_MENU(SERURO_PANEL_SETTINGS_ID, SeruroTray::onSettings)
	//EVT_MENU(seruroID_UPDATE, SeruroTray::onUpdate)
	EVT_MENU(SERURO_EXIT_ID, SeruroTray::OnQuit)
END_EVENT_TABLE()

void SeruroTray::RaiseFrame()
{
    if (! mainFrame)
		return;
	if (mainFrame->IsIconized())
        mainFrame->Iconize(false);
    mainFrame->SetFocus();
    mainFrame->Raise();
	//if (! mainFrame->IsShown())
        mainFrame->Show(true);
    
    /* Hardcore */
#if defined(__WXMAC__) || defined(__WXOSX__)
    //wxTheApp->SetFrontProcess();
    //wxTheApp->SetActive(true, NULL);
    //wx::MacSetFrontProcess();
    //wxTheApp->MacReopenApp();
    ProcessSerialNumber psn;
    GetCurrentProcess(&psn);
    //CPSEnableForegroundOperation(&psn);
    TransformProcessType(&psn, kProcessTransformToForegroundApplication);
    //SetFrontProcess(&psn);
#endif
}

#if SERURO_ENABLE_CRYPT_PANELS
void SeruroTray::onEncrypt(wxCommandEvent &event)
{
    RaiseFrame();
	mainFrame->ChangePanel(SERURO_PANEL_ENCRYPT_ID);
}

void SeruroTray::onDecrypt(wxCommandEvent &event)
{
    RaiseFrame();
	mainFrame->ChangePanel(SERURO_PANEL_DECRYPT_ID);
}
#endif

void SeruroTray::onSearch(wxCommandEvent &event)
{
    RaiseFrame();
	mainFrame->ChangePanel(SERURO_PANEL_SEARCH_ID);
}

void SeruroTray::onSettings(wxCommandEvent& WXUNUSED(event))
{
    RaiseFrame();
	mainFrame->ChangePanel(SERURO_PANEL_SETTINGS_ID);
}

SeruroTray::SeruroTray() : wxTaskBarIcon(wxTBI_CUSTOM_STATUSITEM) /* wxTBI_CUSTOM_STATUSITEM */
{
	mainFrame = NULL;
}

void SeruroTray::SetMainFrame(SeruroFrameMain *frame)
{
	mainFrame = frame;
}

void SeruroTray::OnLeftDoubleClick(wxTaskBarIconEvent &event)
{
	RaiseFrame();
}

void SeruroTray::OnQuit(wxCommandEvent& WXUNUSED(event)){
	RemoveIcon();
	// Should not be vetoed
	if (mainFrame) mainFrame->Close(true);
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
	//wxMenuItem *header = new wxMenuItem;

    popup->Append(SERURO_PANEL_SEARCH_ID, wxT("&Search"));
#if SERURO_ENABLE_CRYPT_PANELS
	popup->Append(SERURO_PANEL_ENCRYPT_ID, wxT("&Encrypt"));
	popup->Append(SERURO_PANEL_DECRYPT_ID, wxT("&Decrypt"));
#endif
	popup->AppendSeparator();
	//popup->AppendSeparator();
	//popup->Append(seruroID_UPDATE, wxT("Update"));
	//popup->AppendSeparator();
	popup->Append(SERURO_PANEL_SETTINGS_ID, wxT("Settings"));
	popup->Append(SERURO_EXIT_ID, wxT("E&xit"));

	return popup;
}
