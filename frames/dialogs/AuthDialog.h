
#ifndef H_SeruroAuthDialog
#define H_SeruroAuthDialog

#include "../../wxJSON/wx/jsonval.h"

#include <wx/textctrl.h>
#include <wx/choice.h>
#include <wx/dialog.h>

/* The auth dialog is a beefed up password dialog with an added selection for
 * username (or email address). Since a single server may have multiple addresses
 * configured.
 */
class AuthDialog : public wxDialog
{
public:
    AuthDialog(const wxString &server,
        const wxString &address = wxEmptyString, int selected = 0);
    
	/* Return the address/password pair. */
	wxJSONValue GetValues();
    
	/* Note: there are no events registered, since the auth dialog should
	 * only be used as a modal.
	 */
    
private:
	wxChoice *address_control;
	wxTextCtrl *password_control;
};

#endif 
