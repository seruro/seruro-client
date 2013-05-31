
#ifndef H_SeruroAddServerDialog
#define H_SeruroAddServerDialog

#include <wx/textctrl.h>
#include <wx/dialog.h>
#include <wx/checkbox.h>

/* Need to access default port. */
#include "../../Defs.h"

#include "../../wxJSON/wx/jsonval.h"

class AddServerForm : public wxEvtHandler
{
public:
    AddServerForm(wxWindow *parent_obj) : parent(parent_obj) {}
    void AddForm(wxSizer *sizer,
        const wxString &name = wxEmptyString, const wxString &host = wxEmptyString,
        const wxString &port = SERURO_DEFAULT_PORT);
    wxJSONValue GetValues();
    
    /* Handle the single checkbox click. */
	void OnCustomPort(wxCommandEvent &event);
    
protected:
    wxWindow *parent;
    
	wxTextCtrl *server_name;
	wxTextCtrl *server_host;
	wxTextCtrl *server_port;
	/* A checkbox to enable port editing. */
	wxCheckBox *checkbox;
    
	//DECLARE_EVENT_TABLE()
};

class AddServerDialog : public AddServerForm, public wxDialog
{
public:
	AddServerDialog(const wxString &name = wxEmptyString,
		const wxString &host = wxEmptyString, 
		const wxString &port = SERURO_DEFAULT_PORT);
	//wxJSONValue GetValues();

private:
	//wxTextCtrl *server_name;
	//wxTextCtrl *server_host;
	//wxTextCtrl *server_port;
	/* A checkbox to enable port editing. */
	//wxCheckBox *checkbox;

	DECLARE_EVENT_TABLE()
};

#endif