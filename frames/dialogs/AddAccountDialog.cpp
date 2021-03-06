
#include "AddAccountDialog.h"
/* Need GetServerChoice. */
#include "AddServerDialog.h"
#include "../components/PasswordCtrl.h"
#include "../UIDefs.h"

#include "../../SeruroClient.h"

#include <wx/grid.h>
#include <wx/log.h>

DECLARE_APP(SeruroClient);

void AddAccountForm::AddForm(wxSizer *sizer, const wxString &address,
	const wxString &server_name)
{
    /* Address details form. */
	wxFlexGridSizer *const grid_sizer = new wxFlexGridSizer(2, 2, 
		GRID_SIZER_WIDTH, GRID_SIZER_HEIGHT);
	grid_sizer->AddGrowableCol(1, 1);
    
	/* Address. */
	grid_sizer->Add(new Text(parent, "&Email Address: "));
    grid_sizer->SetItemMinSize((size_t) 0, SERURO_SETTINGS_FLEX_LABEL_WIDTH, -1);
    
    /* Address validator. */
	wxTextValidator address_validator(wxFILTER_EMPTY | wxFILTER_INCLUDE_CHAR_LIST);
	address_validator.SetCharIncludes(SERURO_INPUT_ADDRESS_WHITELIST);
    
    /* Address controller. */
	this->address = new wxTextCtrl(parent, SERURO_ACCOUNT_USERNAME_ID, address,
        wxDefaultPosition, wxDefaultSize, 0, address_validator);
    this->address->SetHint(_("your.email@example.com"));
	this->address->SetToolTip(
        "Your Seruro account username is your email address.");
	this->address->SetMaxLength(SERURO_INPUT_MAX_LENGTH);
    
	grid_sizer->Add(this->address, DIALOGS_BOXSIZER_OPTIONS);
	
	/* Password. */
	grid_sizer->Add(new Text(parent, "&Seruro Password: "));
    grid_sizer->SetItemMinSize((size_t) 2, SERURO_SETTINGS_FLEX_LABEL_WIDTH, -1);
    
    /* Host controller. */
	//this->password = new wxTextCtrl(parent, SERURO_ACCOUNT_PASSWORD_ID, wxEmptyString,
	//	wxDefaultPosition, wxDefaultSize,
	//	/* It's full of stars... */
	//	wxTE_PASSWORD);
    this->password = new PasswordCtrl(parent, SERURO_ACCOUNT_PASSWORD_ID);
    
	grid_sizer->Add(this->password, DIALOGS_BOXSIZER_OPTIONS);
	
    /* Add sizers together. */
	sizer->Add(grid_sizer, DEFAULT_BOXSIZER_SIZER_OPTIONS);
}

void AddAccountForm::DisableForm()
{
	this->address->Disable();
#if defined(__WXOSX__) || defined(__WXMAC__)
    password->SetWindowStyle(password->GetWindowStyle() | wxTE_READONLY);
#else
    this->password->Disable();
#endif
}

void AddAccountForm::EnableForm()
{
	this->address->Enable(true);
#if defined(__WXOSX__) || defined(__WXMAC__)
	password->SetWindowStyle(wxTE_PASSWORD);
#else
    password->Enable(true);
#endif
}

void AddAccountForm::FocusForm()
{
	this->address->SetFocus();
}

AddAccountDialog::AddAccountDialog(const wxString &address, const wxString &server_name) : 
	wxDialog(wxGetApp().GetFrame(), wxID_ANY, wxString(wxT("Add Account"))),
    AddAccountForm(this)
{
	wxSizer *const vert_sizer = new wxBoxSizer(wxVERTICAL);

    /* Textual message */
	Text *msg = new Text(this, wxString(wxT(TEXT_ADD_ADDRESS)), false);
	msg->Wrap(300);
	vert_sizer->Add(msg, DIALOGS_SIZER_OPTIONS);

	wxSizer *const server_box = new wxStaticBoxSizer(wxVERTICAL, this, "&Select Server");

	//this->server_menu = new wxChoice(this, wxID_ANY
	this->server_menu = GetServerChoice(this, server_name);
	server_box->Add(this->server_menu, DIALOGS_BOXSIZER_SIZER_OPTIONS);
	vert_sizer->Add(server_box, DIALOGS_SIZER_OPTIONS);

	wxSizer *const info_box = new wxStaticBoxSizer(wxVERTICAL, this, "&Account Information");

	/* Add the generic username/password form */
    this->AddForm(info_box, address);
	vert_sizer->Add(info_box, DIALOGS_BOXSIZER_SIZER_OPTIONS);

	Text *identity_msg = new Text(this, _(TEXT_INSTALL_IDENTITY), false);
	identity_msg->Wrap(300);
	vert_sizer->Add(identity_msg, DIALOGS_SIZER_OPTIONS);

	/* Attach an additional checkbox to allow this form to also install the identity. */
	wxSizer *const identity_box = new wxStaticBoxSizer(wxVERTICAL, this, "&Identity");

	install_identity = new wxCheckBox(this, wxID_ANY, _("Install the identity for this address."));
	identity_box->Add(install_identity, DIALOGS_BOXSIZER_OPTIONS);
	vert_sizer->Add(identity_box, DIALOGS_BOXSIZER_SIZER_OPTIONS);

	vert_sizer->Add(CreateStdDialogButtonSizer(wxOK | wxCANCEL), DIALOGS_BUTTONS_OPTIONS);
	this->SetSizerAndFit(vert_sizer);
}

wxJSONValue AddAccountForm::GetValues()
{
	wxJSONValue values;

	values["address"] = this->address->GetValue();
	values["password"] = this->password->GetValue();
	
	return values;
}

wxJSONValue AddAccountDialog::GetValues()
{
	wxJSONValue values;
	int selection;

	values = AddAccountForm::GetValues();

	selection = this->server_menu->GetSelection();
	values["server_name"] = this->server_menu->GetString(selection);
	values["install_identity"] = this->install_identity->IsChecked();

	return values;
}