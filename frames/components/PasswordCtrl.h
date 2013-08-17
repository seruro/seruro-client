
#include <wx/textctrl.h>
#include <wx/event.h>

class PasswordCtrl : public wxTextCtrl
{
public:
    PasswordCtrl(wxWindow *parent, wxWindowID id);
    
private:
    void OnPaste(wxClipboardTextEvent& event);
    void OnKeyDown(wxKeyEvent &event);
    
    DECLARE_EVENT_TABLE();
};