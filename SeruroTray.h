
#ifndef H_SeruroTray
#define H_SeruroTray

#include <wx/taskbar.h>

#include "SeruroClient.h"
#include "frames/SeruroFrames.h"

class SeruroMainFrame;

enum
{
	seruroID_SEARCH		= wxID_HIGHEST + 10,
	seruroID_CONFIGURE	= wxID_HIGHEST + 11,
	seruroID_UPDATE		= wxID_HIGHEST + 12,
	seruroID_DECRYPT	= wxID_HIGHEST + 13,
	seruroID_ENCRYPT	= wxID_HIGHEST + 14,
	seruroID_EXIT		= wxID_HIGHEST + 1
};

class SeruroTray: public wxTaskBarIcon
{
public:
	SeruroTray();
	//~SeruroTray() {}

	void SetMainFrame(SeruroFrameMain *frame);
    void RaiseFrame();
	virtual wxMenu *CreatePopupMenu();

	void onEncrypt(wxCommandEvent &event);
	void onDecrypt(wxCommandEvent &event);
	void onUpdate(wxCommandEvent &event);
	void onConfigure(wxCommandEvent &event);
	void onSearch(wxCommandEvent &event);

	void OnLeftDoubleClick(wxTaskBarIconEvent &event);
	void OnQuit(wxCommandEvent &event);
	void OnAbout(wxCommandEvent& event);

protected:
	SeruroFrameMain *mainFrame;

private:
	DECLARE_EVENT_TABLE()
};

#endif