
#include "AddAccountDialog.h"
#include "../UIDefs.h"

#include "../../SeruroClient.h"

#include <wx/grid.h>
#include <wx/log.h>

DECLARE_APP(SeruroClient);

void AddAccountForm::AddForm(wxSizer *sizer, const wxString &address)
{
    /* Address details form. */
	wxFlexGridSizer *const grid_sizer = new wxFlexGridSizer(2, 2, 5, 10);
	grid_sizer->AddGrowableCol(1, 1);
    
	/* Address. */
	grid_sizer->Add(new Text(parent, "&Email Address:"));
    
    /* Address validator. */
	wxTextValidator address_validator(wxFILTER_EMPTY | wxFILTER_INCLUDE_CHAR_LIST);
	//name_validator.SetCharExcludes("\\\"");
	address_validator.SetCharIncludes(SERURO_INPUT_ADDRESS_WHITELIST);
    
    /* Address controller. */
	this->address = new wxTextCtrl(parent, wxID_ANY, address,
        wxDefaultPosition, wxDefaultSize, 0, address_validator);
	this->address->SetToolTip(
        "Enter the email address you wish to use.");
	this->address->SetMaxLength(SERURO_INPUT_MAX_LENGTH);
    
	grid_sizer->Add(this->address, DIALOGS_BOXSIZER_OPTIONS);
	
	/* Password. */
	grid_sizer->Add(new Text(parent, "&Password:"));
    
    /* Host controller. */
	this->password = new wxTextCtrl(parent, wxID_ANY, wxEmptyString);
	grid_sizer->Add(this->password, DIALOGS_BOXSIZER_OPTIONS);
	
    /* Add sizers together. */
	sizer->Add(grid_sizer, DIALOGS_BOXSIZER_SIZER_OPTIONS);
}

AddAccountDialog::AddAccountDialog(const wxString &address) : 
	wxDialog(wxGetApp().GetFrame(), wxID_ANY, wxString(wxT("Add Account"))),
    AddAccountForm(this)
{
	wxSizer *const vert_sizer = new wxBoxSizer(wxVERTICAL);

    /* Textual message */
	Text *msg = new Text(this, wxString(wxT(TEXT_ADD_ACCOUNT)), false);
	msg->Wrap(300);
	vert_sizer->Add(msg, DIALOGS_SIZER_OPTIONS);

	wxSizer *const info_box = new wxStaticBoxSizer(wxVERTICAL, this, "&Account Information");

    this->AddForm(info_box, address);
	
	vert_sizer->Add(info_box, DIALOGS_BOXSIZER_SIZER_OPTIONS);

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