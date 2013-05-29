
#ifndef H_SeruroAddServerDialog
#define H_SeruroAddServerDialog

#include <wx/textctrl.h>
#include <wx/dialog.h>
#include <wx/checkbox.h>

/* Need to access default port. */
#include "../../Defs.h"

#include "../../wxJSON/wx/jsonval.h"

class AddServerForm
{
public:
    AddServerForm(wxWindow *parent_obj) : parent(parent_obj) {}
    void AddForm(wxSizer *sizer,
        const wxString &name = wxEmptyString, const wxString &host = wxEmptyString,
        const wxString &port = SERURO_DEFAULT_PORT);
    wxJSONValue GetValues();
    
protected:
    wxWindow *parent;
    
	wxTextCtrl *server_name;
	wxTextCtrl *server_host;
	wxTextCtrl *server_port;
	/* A checkbox to enable port editing. */
	wxCheckBox *checkbox;
    
	//DECLARE_EVENT_TABLE()
};

class AddServerDialog : public wxDialog, public AddServerForm
{
public:
	AddServerDialog(const wxString &name = wxEmptyString,
		const wxString &host = wxEmptyString, 
		const wxString &port = SERURO_DEFAULT_PORT);
	//wxJSONValue GetValues();

	/* Handle the single checkbox click. */
	void OnCheck(wxCommandEvent &event);

private:
	//wxTextCtrl *server_name;
	//wxTextCtrl *server_host;
	//wxTextCtrl *server_port;
	/* A checkbox to enable port editing. */
	//wxCheckBox *checkbox;

	DECLARE_EVENT_TABLE()
};

#endif