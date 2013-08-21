
#ifndef H_SeruroDecryptDialog
#define H_SeruroDecryptDialog

#include "../../wxJSON/wx/jsonval.h"
#include "../components/PasswordCtrl.h"

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
    wxJSONValue GetValues();
    /* deprecated. */ wxString GetValue();
    
	/* Disallow/Allow form actions. */
	void DisableForm();
	void EnableForm();
    void DisableAuthentication();
    void DisableEncipherment();
    
    void SetAuthenticationHint(wxString hint) {
        authentication_control->SetHint(hint);
    }
    void SetEnciphermentHint(wxString hint) {
        encipherment_control->SetHint(hint);
    }
    
	/* Start the focus at the first form element. */
	void FocusForm();
    
protected:
    wxWindow *parent;
    
	//wxTextCtrl *password_control;
    
    UnlockCtrl *encipherment_control;
    UnlockCtrl *authentication_control;
};

class DecryptDialog : public wxDialog, public DecryptForm
{
public:
	DecryptDialog(const wxString &method);
};

#endif
