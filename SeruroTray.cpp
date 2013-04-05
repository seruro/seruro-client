
#include <wx/menu.h>

#include "SeruroTray.h"
#include "SeruroClient.h"

BEGIN_EVENT_TABLE(SeruroTray, wxTaskBarIcon)
	EVT_TASKBAR_LEFT_DCLICK(SeruroTray::OnLeftDoubleClick)
	EVT_MENU(seruroID_ENCRYPT, SeruroTray::onEncrypt)
	EVT_MENU(seruroID_ENCRYPT, SeruroTray::onDecrypt)
	EVT_MENU(seruroID_ENCRYPT, SeruroTray::onSearch)
	EVT_MENU(seruroID_ENCRYPT, SeruroTray::onConfigure)
	EVT_MENU(seruroID_ENCRYPT, SeruroTray::onUpdate)
	EVT_MENU(seruroID_EXIT, SeruroTray::OnQuit)
END_EVENT_TABLE()

void SeruroTray::onEncrypt(wxCommandEvent &event)
{

}

void SeruroTray::onDecrypt(wxCommandEvent &event)
{

}

void SeruroTray::onSearch(wxCommandEvent &event)
{

}

void SeruroTray::onConfigure(wxCommandEvent &event)
{

}

void SeruroTray::onUpdate(wxCommandEvent &event)
{

}

SeruroTray::SeruroTray() : wxTaskBarIcon(wxTBI_CUSTOM_STATUSITEM)
{
	mainFrame = NULL;
}

void SeruroTray::SetMainFrame(MainFrame *frame)
{
	mainFrame = frame;
}

void SeruroTray::OnLeftDoubleClick(wxTaskBarIconEvent &event)
{
	if (! mainFrame)
		return;
	if (mainFrame->IsIconized()) mainFrame->Iconize(false);
	if (! mainFrame->IsShown()) mainFrame->Show();
	mainFrame->Raise();
}

void SeruroTray::OnQuit(wxCommandEvent& WXUNUSED(event)){
	RemoveIcon();
	// Should not be vetoed
	if (mainFrame) mainFrame->Close(true);
}

wxMenu* SeruroTray::CreatePopupMenu()
{
	wxMenu *popup = new wxMenu;
	wxMenuItem *header = new wxMenuItem;

	header->SetItemLabel(wxT("Active Sessions"));
	header->SetTextColour(wxT("gray"));

	popup->Append(seruroID_ENCRYPT, wxT("Encrypt"));
	popup->Append(seruroID_DECRYPT, wxT("Decrypt"));
	popup->AppendSeparator();
	popup->Append(header);
	popup->AppendSeparator();
	popup->Append(seruroID_SEARCH, wxT("Search"));
	popup->Append(seruroID_UPDATE, wxT("Update"));
	popup->AppendSeparator();
	popup->Append(seruroID_CONFIGURE, wxT("Configure"));
	popup->Append(seruroID_EXIT, wxT("E&xit"));
	return popup;
}
