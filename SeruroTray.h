
#include <wx/wxprec.h>

#include <wx/taskbar.h>

class MainFrame;

const int seruroID_SEARCH		= wxID_HIGHEST + 10;
const int seruroID_CONFIGURE	= wxID_HIGHEST + 11;
const int seruroID_UPDATE		= wxID_HIGHEST + 12;
const int seruroID_DECRYPT		= wxID_HIGHEST + 13;
const int seruroID_ENCRYPT		= wxID_HIGHEST + 14;
const int seruroID_EXIT			= wxID_HIGHEST + 1;

class SeruroTray: public wxTaskBarIcon
{
public:
	SeruroTray();
	~SeruroTray() {}

	void SetMainFrame(MainFrame *frame);
	virtual wxMenu *CreatePopupMenu();

	void onEncrypt(wxCommandEvent &event);
	void onDecrypt(wxCommandEvent &event);
	void onUpdate(wxCommandEvent &event);
	void onConfigure(wxCommandEvent &event);
	void onSearch(wxCommandEvent &event);

	void OnLeftDoubleClick(wxTaskBarIconEvent &event);
	void OnQuit(wxCommandEvent &event);

protected:
	MainFrame *mainFrame;

private:
	DECLARE_EVENT_TABLE()
};