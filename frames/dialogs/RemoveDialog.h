
#ifndef H_SeruroRemoveDialog
#define H_SeruroRemoveDialog

#include <wx/dialog.h>
#include <wx/checkbox.h>

/* Need to access default port. */
#include "../../Defs.h"

#include "../../wxJSON/wx/jsonval.h"

/*
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
*/

//#define SERURO_REMOVE_CA_ID 8000
//#define SERURO_REMOVE_CERTS_CA 8001
//#define SERURO_REMOVE_IDENTITY_BASE 8002

class RemoveDialog : public wxDialog
{
public:
	/* If address is provided, only provide a selection to remove
	 * that specific address (not the CA, or any certs).
	 */
	RemoveDialog(const wxString &server_name,
		const wxString &address = wxEmptyString);
	/* Based on the selection from the user, perform the action. */
	void DoRemove();

protected:
	bool remove_server;
	/* Checkboxes specific to a server removal. */
	wxCheckBox *remove_ca;
	wxCheckBox *remove_certs;
	/* A list of all identitied to remove. */
	wxCheckBox **remove_identities;
};

#endif