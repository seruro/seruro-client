
#include "AddServerDialog.h"
#include "../UIDefs.h"

#include "../../SeruroClient.h"

#include <wx/grid.h>
#include <wx/checkbox.h>
#include <wx/log.h>

DECLARE_APP(SeruroClient);

BEGIN_EVENT_TABLE(AddServerDialog, wxDialog)
	EVT_CHECKBOX(SERURO_ADD_SERVER_PORT_ID, AddServerDialog::OnForm_OnCustomPort)
END_EVENT_TABLE()

wxChoice* GetServerChoice(wxWindow *parent, const wxString &server_name)
{
	wxChoice *server_menu;
	wxArrayString servers_list;
	bool disable_menu = false;

	if (server_name.compare(wxEmptyString) == 0) {
		/* Add all servers. */
		servers_list = theSeruroConfig::Get().GetServerNames();
	} else {
		/* Add just this server. */
		servers_list.Add(server_name);
		disable_menu = true;
	}

	server_menu = new wxChoice(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, servers_list);
	server_menu->SetSelection(0);
	server_menu->Enable(! disable_menu);

	return server_menu;
}

void AddServerForm::AddForm(wxSizer *sizer, 
	const wxString &host, const wxString &port)
{
    /* Server details form. */
	wxFlexGridSizer *const grid_sizer = new wxFlexGridSizer(3, 2,
        GRID_SIZER_WIDTH, GRID_SIZER_HEIGHT);
	grid_sizer->AddGrowableCol(1, 1);
    
	/* Host (hostname). */
	grid_sizer->Add(new Text(parent, "&Hostname: "));
    grid_sizer->SetItemMinSize((size_t) 0, SERURO_SETTINGS_FLEX_LABEL_WIDTH, -1);
    
    /* Server host validator. */
	wxTextValidator host_validator(wxFILTER_EMPTY | wxFILTER_INCLUDE_CHAR_LIST);
	host_validator.SetCharIncludes(SERURO_INPUT_HOSTNAME_WHITELIST);
    
    /* Host controller. */
	this->server_host = new wxTextCtrl(parent, wxID_ANY, host, wxDefaultPosition, wxDefaultSize, 0, host_validator);
    this->server_host->SetHint(_("alpha.seruro.com"));
	grid_sizer->Add(this->server_host, DIALOGS_BOXSIZER_OPTIONS);
	
	/* Optional server port (validator included). */
    if (SERURO_ALLOW_CUSTOM_PORT) {
        /* Even though the custom port is optional, the client may not allow the option... */
        grid_sizer->Add(new Text(parent, "&Port: "));
        grid_sizer->SetItemMinSize((size_t) 2, SERURO_SETTINGS_FLEX_LABEL_WIDTH, -1);
        this->server_port = new wxTextCtrl(parent, wxID_ANY, port,
            wxDefaultPosition, wxDefaultSize, 0, wxTextValidator(wxFILTER_EMPTY | wxFILTER_DIGITS));
        this->server_port->SetMaxLength(5);
        this->server_port->Enable(false);
    
        /* Show some text and a checkbox to enable editing of the port. */
        this->checkbox = new wxCheckBox(parent, SERURO_ADD_SERVER_PORT_ID, wxT("Use non-standard port."));
        this->checkbox->Enable(true);
        
        wxSizer *const port_sizer = new wxBoxSizer(wxHORIZONTAL);
        port_sizer->Add(this->server_port);
        port_sizer->AddSpacer(20);
        port_sizer->Add(this->checkbox, DIALOGS_BOXSIZER_OPTIONS);
        grid_sizer->Add(port_sizer, DIALOGS_BOXSIZER_OPTIONS);
    }
        
    /* Add sizers together. */
	sizer->Add(grid_sizer, DIALOGS_BOXSIZER_SIZER_OPTIONS);
}

/* This event handler function will be duplicated (defined) for each implmentor. 
 * To prevent code duplication, simply call these event handlers from the implementing
 * class using a wrapper handler. 
 */
void AddServerForm::OnCustomPort()
{
    /* The client may have disabled custom ports (at compile time). */
    if (! SERURO_ALLOW_CUSTOM_PORT) return;
    
    wxLogMessage(wxT("checkbox clicked."));
	this->server_port->Enable(this->checkbox->IsChecked());
	if (! this->checkbox->IsChecked()) {
		/* If the checkbox becomes un-checked, reset the port value. */
		this->server_port->SetValue(SERURO_DEFAULT_PORT);
	}
}

void AddServerDialog::OnForm_OnCustomPort(wxCommandEvent &event)
{
	this->OnCustomPort();
}

AddServerDialog::AddServerDialog(
	//const wxString &name, 
	const wxString &host, const wxString &port) : 
	wxDialog(wxGetApp().GetFrame(), wxID_ANY, wxString(wxT("Add Server"))),
    AddServerForm(this)
{
	wxSizer *const vert_sizer = new wxBoxSizer(wxVERTICAL);

    /* Textual message */
	Text *msg = new Text(this, wxString(wxT(TEXT_ADD_SERVER)), false);
	msg->Wrap(300);
	vert_sizer->Add(msg, DIALOGS_SIZER_OPTIONS);

	wxSizer *const info_box = new wxStaticBoxSizer(wxVERTICAL, this, "&Server Information");

    this->AddForm(info_box, 
		//name, 
		host, port);
	
	vert_sizer->Add(info_box, DIALOGS_BOXSIZER_SIZER_OPTIONS);

	vert_sizer->Add(CreateStdDialogButtonSizer(wxOK | wxCANCEL), DIALOGS_BUTTONS_OPTIONS);
	this->SetSizerAndFit(vert_sizer);
}

wxJSONValue AddServerForm::GetValues()
{
	wxJSONValue values;

	values["host"] = this->server_host->GetValue();
    
    if (SERURO_ALLOW_CUSTOM_PORT) {
        values["port"] = this->server_port->GetValue();
    } else {
        values["port"] = _(SERURO_DEFAULT_PORT);
    }

	return values;
}