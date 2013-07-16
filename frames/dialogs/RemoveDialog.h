
#ifndef H_SeruroRemoveDialog
#define H_SeruroRemoveDialog

#include <wx/dialog.h>
#include <wx/checkbox.h>

/* Need to access default port. */
#include "../../Defs.h"

#include "../../wxJSON/wx/jsonval.h"

class RemoveDialog : public wxDialog
{
public:
	/* If address is provided, only provide a selection to remove
	 * that specific address (not the CA, or any certs).
	 */
	RemoveDialog(const wxString &server_name,
		const wxString &address = wxEmptyString);
    //~RemoveDialog();
    
	/* Based on the selection from the user, perform the action. */
	void DoRemove();

protected:
    void RemoveServer();
    void RemoveAddress();
    
	bool remove_server;

	wxString server_name;
	wxString address;

	/* Checkboxes specific to a server removal. */
	wxCheckBox *remove_ca;
	wxCheckBox *remove_certs;
	
    /* A list of all identitied to remove. */
    size_t identity_count;
	wxCheckBox **remove_identities;
};

#endif