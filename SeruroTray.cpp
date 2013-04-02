
#include <wx/menu.h>

#include "SeruroTray.h"
#include "SeruroClient.h"

BEGIN_EVENT_TABLE(SeruroTray, wxTaskBarIcon)
	EVT_TASKBAR_LEFT_DCLICK(SeruroTray::OnLeftDoubleClick)
	EVT_MENU(PopupExitID, SeruroTray::OnQuit)
END_EVENT_TABLE()

const int SeruroTray::PopupExitID = wxID_HIGHEST + 1;

SeruroTray::SeruroTray() : wxTaskBarIcon()
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

wxMenu* SeruroTray::CreatePopupMenu(){
	wxMenu *popup = new wxMenu;
   
	popup->AppendSeparator();
	popup->Append(PopupExitID, wxT("E&xit"));
	return popup;
}

void SeruroTray::OnQuit(wxCommandEvent& WXUNUSED(event)){
	RemoveIcon();
	// Should not be vetoed
	if (mainFrame) mainFrame->Close(true);
}
