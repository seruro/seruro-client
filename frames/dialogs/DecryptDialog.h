
#ifndef H_SeruroDecryptDialog
#define H_SeruroDecryptDialog

#include <wx/textctrl.h>
#include <wx/dialog.h>

/* DecryptDialog will accept a "method", meaning the method the decryption key
 * should have been communicated to the user.
 *
 * See Defs.h for TEXT_DECRYPT_METHOD_... for various textual messages.
 */
class DecryptDialog : public wxDialog
{
public:
	DecryptDialog(const wxString &method);
    
	/* Return the entered password. */
	wxString GetValue();
private:
	wxTextCtrl *password_control;
};

#endif
