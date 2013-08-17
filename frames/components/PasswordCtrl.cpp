
#include "SeruroClient.h"
#include "PasswordCtrl.h"

#include <wx/clipbrd.h>
#include <wx/dataobj.h>

BEGIN_EVENT_TABLE(PasswordCtrl, wxTextCtrl)
    EVT_TEXT_PASTE(wxID_ANY, PasswordCtrl::OnPaste)

    EVT_KEY_DOWN(PasswordCtrl::OnKeyDown)
END_EVENT_TABLE()

DECLARE_APP(SeruroClient);

PasswordCtrl::PasswordCtrl(wxWindow *parent, wxWindowID id)
  : wxTextCtrl(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize)
{
    SetWindowStyle(wxTE_PASSWORD);
    wxGetApp().Bind(wxEVT_KEY_DOWN, &PasswordCtrl::OnKeyDown, this);
}

void PasswordCtrl::OnKeyDown(wxKeyEvent &event)
{
    if (! this->HasFocus()) {
        event.Skip();
        return;
    }
    
    if (! event.GetModifiers() == wxMOD_CONTROL) {
        event.Skip();
        return;
    }
    
    event.Skip();
}

void PasswordCtrl::OnPaste(wxClipboardTextEvent& event)
{
    wxTextDataObject clipboard_data;
	wxTheClipboard->Open();
	
	if (wxTheClipboard->IsSupported(wxDF_TEXT)) {
		/* Only text can go into the password field. */
		wxTheClipboard->GetData(clipboard_data);
	}
    
	/* If there was non-text in the clipboard, clear the password field. */
	wxTheClipboard->Close();
	
	this->Clear();
	this->SetValue(clipboard_data.GetText());
	this->SetInsertionPointEnd();
}
