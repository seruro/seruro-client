
#include "../../SeruroClient.h"
#include "PasswordCtrl.h"

#include <wx/clipbrd.h>
#include <wx/dataobj.h>

BEGIN_EVENT_TABLE(PasswordCtrl, wxTextCtrl)
    EVT_TEXT_PASTE(wxID_ANY, PasswordCtrl::OnPaste)
END_EVENT_TABLE()

DECLARE_APP(SeruroClient);

UnlockCtrl::UnlockCtrl(wxWindow *parent, wxWindowID id)
  : wxTextCtrl(parent, id, wxEmptyString, wxDefaultPosition)
{
    wxTextAttr style;
    
    /* Give it a code-like look. */
    style.SetFontFamily(wxFONTFAMILY_TELETYPE);
    this->SetDefaultStyle(style);
}

PasswordCtrl::PasswordCtrl(wxWindow *parent, wxWindowID id)
  : wxTextCtrl(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD) {}

void PasswordCtrl::OnPaste(wxClipboardTextEvent& event)
{
#if defined(__WXMSW__)
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
#endif
}
