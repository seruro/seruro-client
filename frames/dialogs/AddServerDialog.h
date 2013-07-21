
#ifndef H_SeruroAddServerDialog
#define H_SeruroAddServerDialog

#include <wx/textctrl.h>
#include <wx/dialog.h>
#include <wx/checkbox.h>

/* Need to access default port. */
#include "../../Defs.h"

#include "../../wxJSON/wx/jsonval.h"

#include <wx/choice.h>

/* Helper function to show a list of servers. */
wxChoice* GetServerChoice(wxWindow *parent, 
	const wxString &server_name = wxEmptyString
	);

class AddServerForm
{
public:
    AddServerForm(wxWindow *parent_obj) : parent(parent_obj) {}
    void AddForm(wxSizer *sizer,
        //const wxString &name = wxEmptyString, 
		const wxString &host = wxEmptyString,
        const wxString &port = SERURO_DEFAULT_PORT);
    wxJSONValue GetValues();
    
	/* Handle the single checkbox click. */
	/* Todo: find a better way to implement this event handler,
	 * within the base abstract class would be great, tried inheriting
	 * from wxEvtHandler, but this results in invalid access if the event
	 * table is defined for the implementor, and no action if the event 
	 * table is defined for the abstract.
	 */
	void OnCustomPort();

protected:
    wxWindow *parent;
    
	//wxTextCtrl *server_name;
	wxTextCtrl *server_host;
	wxTextCtrl *server_port;
	/* A checkbox to enable port editing. */
	wxCheckBox *checkbox;
    
	//DECLARE_EVENT_TABLE()
};

class AddServerDialog : public AddServerForm, public wxDialog
{
public:
	AddServerDialog(const wxString &host = wxEmptyString, 
		const wxString &port = SERURO_DEFAULT_PORT);

	void OnForm_OnCustomPort(wxCommandEvent &event);

private:
	DECLARE_EVENT_TABLE()
};

#endif