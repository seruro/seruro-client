
#ifndef H_PasswordCtrl
#define H_PasswordCtrl

#include <wx/textctrl.h>
#include <wx/event.h>

class PasswordCtrl : public wxTextCtrl
{
public:
    PasswordCtrl(wxWindow *parent, wxWindowID id);
    
private:
    void OnPaste(wxClipboardTextEvent& event);
    
    DECLARE_EVENT_TABLE();
};

class UnlockCtrl : public wxTextCtrl
{
public:
    UnlockCtrl(wxWindow *parent, wxWindowID id);
    void RefreshStyle();
    
private:
};

#endif
