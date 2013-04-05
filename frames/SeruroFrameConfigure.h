
#include <wx/wx.h>
#include <wx/icon.h>

#include "../SeruroClient.h"

#ifndef H_SeruroFrameConfigure
#define H_SeruroFrameConfigure



// Define a new frame type: this is going to be our main frame
class SeruroFrameConfigure : public SeruroFrame
{
public:
    // ctor(s)
    SeruroFrameConfigure(const wxString& title);

	void SeruroFrameConfigure::OnClose(wxCloseEvent &event);

protected:
	SeruroTray *tray;

private:
    DECLARE_EVENT_TABLE()
};

#endif