
#ifndef H_SeruroAddAccountDialog
#define H_SeruroAddAccountDialog

#include <wx/textctrl.h>
#include <wx/dialog.h>

/* Need to access default port. */
#include "../../Defs.h"

#include "../../wxJSON/wx/jsonval.h"

class AddAccountForm
{
public:
    AddAccountForm(wxWindow *parent_obj) : parent(parent_obj) {}
    void AddForm(wxSizer *sizer, const wxString &address = wxEmptyString);
    wxJSONValue GetValues();
    
protected:
    wxWindow *parent;
    
	wxTextCtrl *address;
	wxTextCtrl *password;
};

class AddAccountDialog : public AddAccountForm, public wxDialog
{
public:
	AddAccountDialog(const wxString &account = wxEmptyString);
};

#endif