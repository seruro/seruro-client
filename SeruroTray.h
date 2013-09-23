
#ifndef H_SeruroTray
#define H_SeruroTray

#include "api/SeruroStateEvents.h"

#include <wx/taskbar.h>
#include <wx/menu.h>

class SeruroStateEvent;
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
    void DoIconize();
    
	virtual wxMenu *CreatePopupMenu();

#if SERURO_ENABLE_CRYPT_PANELS
	void onEncrypt(wxCommandEvent &event);
	void onDecrypt(wxCommandEvent &event);
#endif

	//void onUpdate(wxCommandEvent &event);
	void onSettings(wxCommandEvent &event);
	void onSearch(wxCommandEvent &event);

	/* new events */
	void OnHome(wxCommandEvent &event);
	void OnHelp(wxCommandEvent &event);
	void OnContacts(wxCommandEvent &event);

	void OnLeftDoubleClick(wxTaskBarIconEvent &event);
	void OnQuit(wxCommandEvent &event);
	void OnAbout(wxCommandEvent& event);

	/* state monitors. */
	void OnOptionStateChange(SeruroStateEvent &event);

protected:
    /* May have to remove the "search" option. */
    //void OnOptionChange(SeruroStateEvent &event);
    
	SeruroFrameMain *main_frame;

private:
	DECLARE_EVENT_TABLE()

	wxMenu *menu;
};

#endif