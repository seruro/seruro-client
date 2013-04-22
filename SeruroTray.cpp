
#include <wx/menu.h>
#include <wx/msgdlg.h>

#include "SeruroTray.h"
#include "frames/SeruroFrameMain.h"

/* OSX Hack for active focus */
#if defined(__WXMAC__)
#include <Carbon/Carbon.h>
extern "C" { void CPSEnableForegroundOperation(ProcessSerialNumber *psn); }
#endif

BEGIN_EVENT_TABLE(SeruroTray, wxTaskBarIcon)
	EVT_TASKBAR_LEFT_DCLICK(SeruroTray::OnLeftDoubleClick)
	EVT_MENU(seruroID_ENCRYPT, SeruroTray::onEncrypt)
	EVT_MENU(seruroID_DECRYPT, SeruroTray::onDecrypt)
	EVT_MENU(seruroID_SEARCH, SeruroTray::onSearch)
	EVT_MENU(seruroID_CONFIGURE, SeruroTray::onConfigure)
	EVT_MENU(seruroID_UPDATE, SeruroTray::onUpdate)
	EVT_MENU(seruroID_EXIT, SeruroTray::OnQuit)
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
#if defined(__WXMAC__)
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

void SeruroTray::onEncrypt(wxCommandEvent &event)
{
    RaiseFrame();
	mainFrame->ChangePanel(seruroID_ENCRYPT);
}

void SeruroTray::onDecrypt(wxCommandEvent &event)
{
    RaiseFrame();
	mainFrame->ChangePanel(seruroID_DECRYPT);
}

void SeruroTray::onSearch(wxCommandEvent &event)
{
    RaiseFrame();
	mainFrame->ChangePanel(seruroID_SEARCH);
}

void SeruroTray::onConfigure(wxCommandEvent& WXUNUSED(event))
{
    RaiseFrame();
	mainFrame->ChangePanel(seruroID_CONFIGURE);
}

void SeruroTray::onUpdate(wxCommandEvent &event)
{
    RaiseFrame();
	mainFrame->ChangePanel(seruroID_UPDATE);
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

	popup->Append(seruroID_ENCRYPT, wxT("Encrypt"));
	popup->Append(seruroID_DECRYPT, wxT("Decrypt"));
	popup->AppendSeparator();
	popup->AppendSeparator();
	popup->Append(seruroID_SEARCH, wxT("Search"));
	popup->Append(seruroID_UPDATE, wxT("Update"));
	popup->AppendSeparator();
	popup->Append(seruroID_CONFIGURE, wxT("Configure"));
	popup->Append(seruroID_EXIT, wxT("E&xit"));

	return popup;
}
