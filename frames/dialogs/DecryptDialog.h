
#ifndef H_SeruroDecryptDialog
#define H_SeruroDecryptDialog

#include <wx/textctrl.h>
#include <wx/dialog.h>

/* DecryptDialog will accept a "method", meaning the method the decryption key
 * should have been communicated to the user.
 *
 * See Defs.h for TEXT_DECRYPT_METHOD_... for various textual messages.
 */

class DecryptForm
{
public:
    DecryptForm(wxWindow *parent_obj) : parent(parent_obj) {}

    void AddForms(wxSizer *sizer);
    wxString GetValue();
    
	/* Disallow/Allow form actions. */
	void DisableForm();
	void EnableForm();
    
	/* Start the focus at the first form element. */
	void FocusForm();
    
protected:
    wxWindow *parent;
    
	wxTextCtrl *password_control;
};

class DecryptDialog : public wxDialog, public DecryptForm
{
public:
	DecryptDialog(const wxString &method);
};

#endif
