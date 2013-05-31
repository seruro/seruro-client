
#include "AddServerDialog.h"
#include "../UIDefs.h"

#include "../../SeruroClient.h"

#include <wx/grid.h>
#include <wx/checkbox.h>
#include <wx/log.h>

DECLARE_APP(SeruroClient);

BEGIN_EVENT_TABLE(AddServerDialog, wxDialog)
	EVT_CHECKBOX(SERURO_ADD_SERVER_PORT_ID, AddServerDialog::OnCustomPort)
END_EVENT_TABLE()

void AddServerForm::AddForm(wxSizer *sizer,
    const wxString &name, const wxString &host, const wxString &port)
{
    /* Server details form. */
	wxFlexGridSizer *const grid_sizer = new wxFlexGridSizer(4, 2, 5, 10);
	grid_sizer->AddGrowableCol(1, 1);
    
	/* Name. */
	grid_sizer->Add(new Text(parent, "&Name:"));
    
    /* Server name validator. */
	wxTextValidator name_validator(wxFILTER_EMPTY | wxFILTER_INCLUDE_CHAR_LIST);
	//name_validator.SetCharExcludes("\\\"");
	name_validator.SetCharIncludes(SERURO_INPUT_WHITELIST);
    
    /* Name controller. */
	this->server_name = new wxTextCtrl(parent, wxID_ANY, name,
        wxDefaultPosition, wxDefaultSize, 0, name_validator);
	this->server_name->SetToolTip(
        "Enter a custom name for this server, use letters, numbers, spaces, underscores or dashes.");
	this->server_name->SetMaxLength(SERURO_INPUT_MAX_LENGTH);
    
	grid_sizer->Add(this->server_name, DIALOGS_BOXSIZER_OPTIONS);
	
	/* Host (hostname). */
	grid_sizer->Add(new Text(parent, "&Hostname:"));
    
    /* Server host validator. */
	wxTextValidator host_validator(wxFILTER_EMPTY | wxFILTER_INCLUDE_CHAR_LIST);
	//name_validator.SetCharExcludes("_() ");
	host_validator.SetCharIncludes(SERURO_INPUT_HOSTNAME_WHITELIST);
    
    /* Host controller. */
	this->server_host = new wxTextCtrl(parent, wxID_ANY, host,
        wxDefaultPosition, wxDefaultSize, 0, host_validator);
	grid_sizer->Add(this->server_host, DIALOGS_BOXSIZER_OPTIONS);
	
	/* Optional server port (validator included). */
	grid_sizer->Add(new Text(parent, "&Port:"));
	this->server_port = new wxTextCtrl(parent, wxID_ANY, port,
        wxDefaultPosition, wxDefaultSize, 0,
        wxTextValidator(wxFILTER_EMPTY | wxFILTER_DIGITS));
	this->server_port->SetMaxLength(5);
	this->server_port->Enable(false);
    
	wxSizer *const port_sizer = new wxBoxSizer(wxVERTICAL);
	port_sizer->Add(this->server_port);
	grid_sizer->Add(port_sizer, DIALOGS_BOXSIZER_OPTIONS);
    
	/* Show some text and a checkbox to enable editing of the port. */
	grid_sizer->AddSpacer(0);
	this->checkbox = new wxCheckBox(parent, SERURO_ADD_SERVER_PORT_ID,
        wxT("Use non-standard server port."));
	this->checkbox->Enable(true);
	grid_sizer->Add(this->checkbox, DIALOGS_BOXSIZER_OPTIONS);
    
    /* Add sizers together. */
	sizer->Add(grid_sizer, DIALOGS_BOXSIZER_SIZER_OPTIONS);
}

/* This event handler function will be duplicated (defined) for each implmentor. */
void AddServerDialog::OnCustomPort(wxCommandEvent &event)
{
    wxLogMessage(wxT("checkbox clicked."));
	this->server_port->Enable(this->checkbox->IsChecked());
	if (! this->checkbox->IsChecked()) {
		/* If the checkbox becomes un-checked, reset the port value. */
		this->server_port->SetValue(SERURO_DEFAULT_PORT);
	}
}

AddServerDialog::AddServerDialog(const wxString &name, const wxString &host, const wxString &port) : 
	wxDialog(wxGetApp().GetFrame(), wxID_ANY, wxString(wxT("Add Server"))),
    AddServerForm(this)
{
	wxSizer *const vert_sizer = new wxBoxSizer(wxVERTICAL);

    /* Textual message */
	Text *msg = new Text(this, wxString(wxT(TEXT_ADD_SERVER)), false);
	msg->Wrap(300);
	vert_sizer->Add(msg, DIALOGS_SIZER_OPTIONS);

	wxSizer *const info_box = new wxStaticBoxSizer(wxVERTICAL, this, "&Server Information");

    this->AddForm(info_box, name, host, port);
	
	vert_sizer->Add(info_box, DIALOGS_BOXSIZER_SIZER_OPTIONS);

	vert_sizer->Add(CreateStdDialogButtonSizer(wxOK | wxCANCEL), DIALOGS_BUTTONS_OPTIONS);
	this->SetSizerAndFit(vert_sizer);
}

wxJSONValue AddServerForm::GetValues()
{
	wxJSONValue values;

	values["name"] = this->server_name->GetValue();
	values["host"] = this->server_host->GetValue();
	values["port"] = this->server_port->GetValue();

	return values;
}