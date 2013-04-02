
#include <wx/wxprec.h>

#include <wx/taskbar.h>

class MainFrame;

class SeruroTray: public wxTaskBarIcon
{
public:
	SeruroTray();
	~SeruroTray() {}

	void OnLeftDoubleClick(wxTaskBarIconEvent &event);

	void SetMainFrame(MainFrame *frame);
	void OnQuit(wxCommandEvent &event);
	virtual wxMenu *CreatePopupMenu();

protected:
	static const int PopupExitID;

	MainFrame *mainFrame;

private:
	DECLARE_EVENT_TABLE()
};