
#ifndef H_SeruroTray
#define H_SeruroTray

#include <wx/taskbar.h>

class SeruroFrameMain;

/* Store the WX_IDs of the panels. */
//extern int seruro_panels_ids[];
//extern int seruro_panels_size;

class SeruroTray: public wxTaskBarIcon
{
public:
	SeruroTray();
	//~SeruroTray() {}

	void SetMainFrame(SeruroFrameMain *frame);
    void RaiseFrame();
	virtual wxMenu *CreatePopupMenu();
#if SERURO_ENABLE_CRYPT_PANELS
	void onEncrypt(wxCommandEvent &event);
	void onDecrypt(wxCommandEvent &event);
#endif
	//void onUpdate(wxCommandEvent &event);
	void onSettings(wxCommandEvent &event);
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