
#include "SettingsPanels.h"

#include <wx/sizer.h>
#include <wx/button.h>

enum button_actions {
    BUTTON_UPDATE,
    BUTTON_REMOVE
};

SettingsPanel_Address::SettingsPanel_Address(SeruroPanelSettings *parent,
	const wxString &address, const wxString &server) :
    SettingsPanelView(parent), address(address), server_name(server)
{
	wxBoxSizer *vert_sizer = new wxBoxSizer(wxVERTICAL);

	//wxButton *address_button = new wxButton(this, wxID_ANY, address);
	//wxButton *server_button = new wxButton(this, wxID_ANY, server);
	//vert_sizer->Add(address_button, 0, wxRIGHT, 5);
	//vert_sizer->Add(server_button, 0, wxRIGHT, 5);
    
    Text *msg = new Text(this, wxString(wxT("View the details and status for your address: ") + address));
    vert_sizer->Add(msg, wxSizerFlags().Expand().Border(wxALL, 5));
    
    wxSizer *info_box = new wxStaticBoxSizer(wxVERTICAL, this, "&Address Information");
    
    /* Status information about address. */
    Text *server_name_info = new Text(this, wxString(wxT("Server Name: ") + server_name));
    Text *address_info = new Text(this, wxString(wxT("Address: ") + address));
    Text *p12_info = new Text(this, wxString(wxT("Identity last updated: Today")));
    Text *token_info = new Text(this, wxString(wxT("Token last updated: Today")));
    
    info_box->Add(server_name_info, wxSizerFlags().Expand().Border(wxBOTTOM));
    info_box->Add(address_info, wxSizerFlags().Expand().Border(wxBOTTOM));
    info_box->Add(p12_info, wxSizerFlags().Expand().Border(wxBOTTOM));
    info_box->Add(token_info, wxSizerFlags().Expand().Border(wxBOTTOM));
    
    vert_sizer->Add(info_box, wxSizerFlags().Expand().Border(wxALL, 5));
    
    Text *update_warning = new Text(this, wxT("Note: updating the address identity will send a new decryption password."));
    vert_sizer->Add(update_warning, wxSizerFlags().Expand().Border(wxALL, 5));
    
    /* Control buttons. */
    wxBoxSizer *buttons_sizer = new wxBoxSizer(wxHORIZONTAL);
    
    wxButton *update_button = new wxButton(this, BUTTON_UPDATE, wxT("Update"));
    wxButton *remove_button = new wxButton(this, BUTTON_REMOVE, wxT("Remove"));
    
    buttons_sizer->Add(update_button, wxSizerFlags().Right().Expand());
    buttons_sizer->Add(remove_button, wxSizerFlags().Right().Expand());
    
    vert_sizer->Add(buttons_sizer, wxSizerFlags().Expand().Border(wxALL, 5));

	this->SetSizer(vert_sizer);
}
