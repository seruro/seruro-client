
#ifndef H_SeruroAddAccountDialog
#define H_SeruroAddAccountDialog

#include <wx/textctrl.h>
#include <wx/dialog.h>
#include <wx/choice.h>

/* Need to access default port. */
#include "../../Defs.h"

#include "../../wxJSON/wx/jsonval.h"

class AddAccountForm
{
public:
    AddAccountForm(wxWindow *parent_obj) : parent(parent_obj) {}
    void AddForm(wxSizer *sizer, const wxString &address = wxEmptyString,
		const wxString &server_name = wxEmptyString);
    wxJSONValue GetValues();
    
protected:
    wxWindow *parent;
    
	wxTextCtrl *address;
	wxTextCtrl *password;
};

class AddAccountDialog : public AddAccountForm, public wxDialog
{
public:
	/* When adding an account, the server can be explicitly defined.
	 * If not, the default behaviour will show a dropdown.
	 */
	AddAccountDialog(const wxString &account = wxEmptyString,
		const wxString &server_name = wxEmptyString);
	/* Append the server to the account form data. */
	wxJSONValue GetValues();

protected:
	wxChoice *server_menu;
};

#endif