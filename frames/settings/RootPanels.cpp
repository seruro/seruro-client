
#include "SettingsPanels.h"
#include "../SeruroPanelSettings.h"
#include "../../SeruroClient.h"

#include "../dialogs/AddServerDialog.h"
#include "../dialogs/AddAccountDialog.h"

#include <wx/button.h>
#include <wx/scrolwin.h>

DECLARE_APP(SeruroClient);

enum button_actions
{
	BUTTON_ADD_SERVER,
	BUTTON_ADD_ACCOUNT
};

BEGIN_EVENT_TABLE(SettingsPanel_RootAccounts, SettingsPanel)
	EVT_BUTTON(BUTTON_ADD_SERVER, SettingsPanel_RootAccounts::OnAddServer)
END_EVENT_TABLE()

/* Create the 'AddServer' dialog, and if the user does not cancle, return the values
 * from the dialog's form.
 */
wxJSONValue AddServer()
{
	wxJSONValue server_info;

	AddServerDialog *dialog = new AddServerDialog();
	if (dialog->ShowModal() == wxID_OK) {
		wxLogMessage(wxT("RootPanels::AddServer> OK"));
		server_info = dialog->GetValues();
	}
	delete dialog;

	return server_info;
}

wxJSONValue AddAddress()
{
	wxJSONValue address_info;

	return address_info;
}

SettingsPanel::SettingsPanel(SeruroPanelSettings *instance_panel) : 
	wxScrolledWindow(instance_panel->GetViewer(), wxID_ANY), 
	main_panel(instance_panel) {}

SettingsPanelView::SettingsPanelView(SeruroPanelSettings *instance_panel) :
	SettingsPanel(instance_panel)
{
	this->SetWindowStyle(wxBORDER_SIMPLE);

	/* Initialize the child-view using an "unchecked" call to render. 
	 * The constructor should have set the data used by render (which will be checked 
	 * and possibly refreshed by 'Changed'.
	 */
	//this->Render();
}

SettingsPanel_RootAccounts::SettingsPanel_RootAccounts(SeruroPanelSettings *parent) :
	SettingsPanelView(parent) {}

void SettingsPanel_RootAccounts::OnAddAccount(wxCommandEvent &event)
{

}

void SettingsPanel_RootAccounts::OnAddServer(wxCommandEvent &event)
{
    /* Todo: Get all users (emails) for given server. */
    wxJSONValue server_info;
	wxJSONValue account_info;
    
	do {
		/* Loop until the user enters valid information into the 'AddServer' dialog.
		 * If no fields are populated then the user canceled the form. 
		 */
		server_info = AddServer();
		if (! server_info.HasMember("server_name")) {
			/* Canceled. */
			return;
		}
	} while (server_info["server_name"].Size() == 0 || 
		server_info["server_address"].Size() == 0);
}

bool SettingsPanel_RootAccounts::Changed()
{
	return true;
}

void SettingsPanel_RootAccounts::Render()
{
    wxBoxSizer *vert_sizer = new wxBoxSizer(wxVERTICAL);
    
    Text *msg = new Text(this, wxT("The Seruro Client can be configured to support multiple Seruro Servers."));
    vert_sizer->Add(msg, SETTINGS_PANEL_SIZER_OPTIONS);
    
	/* The Servers list box will show all the available servers. 
	 * Each server will be a child tree item under the root accounts, and will show additional
	 * details about the server such as the last time the CA/CRL was fetched. 
	 */
	wxSizer *server_list_sizer = new wxStaticBoxSizer(wxVERTICAL, this, "&Servers List");

	wxString server_string;//, server_name;
	wxArrayString servers_list = wxGetApp().config->GetServerList();
	wxArrayString all_address_list;
	wxArrayString address_list;

	/* Iterate through the list of servers, adding each to list display. */
	for (size_t i = 0; i < servers_list.size(); i++) {
		server_string =  wxGetApp().config->GetServerString(servers_list[i]);
		server_list_sizer->Add(new Text(this, server_string), 
			SETTINGS_PANEL_BOXSIZER_OPTIONS);
		address_list = wxGetApp().config->GetAddressList(servers_list[i]);
		for (size_t j = 0; j < address_list.size(); j++) {
			all_address_list.Add(address_list[i] + wxT(" (") + servers_list[i] + wxT(")"));
		}
	}

	vert_sizer->Add(server_list_sizer, SETTINGS_PANEL_SIZER_OPTIONS);
	
	/* Add server button */
	wxBoxSizer *servers_buttons_sizer = new wxBoxSizer(wxHORIZONTAL);
	wxButton *add_server_button = new wxButton(this, BUTTON_ADD_SERVER, wxT("Add Server"));

	/* Add spacer, and button to horz sizer, then horz sizer to vert sizer. */
	//servers_buttons_sizer->AddStretchSpacer();
	servers_buttons_sizer->Add(add_server_button, SETTINGS_PANEL_BUTTONS_OPTIONS);
	vert_sizer->Add(servers_buttons_sizer, SETTINGS_PANEL_SIZER_OPTIONS);

	wxSizer *accounts_list_sizer = new wxStaticBoxSizer(wxVERTICAL, this, "&Address List");

	for (size_t i = 0; i < all_address_list.size(); i++) {
		accounts_list_sizer->Add(new Text(this, all_address_list[i]),
			SETTINGS_PANEL_BOXSIZER_OPTIONS);
	}

	vert_sizer->Add(accounts_list_sizer, SETTINGS_PANEL_SIZER_OPTIONS);

    this->SetSizer(vert_sizer);
}

void SettingsPanel_RootGeneral::Render()
{
	wxBoxSizer *vert_sizer = new wxBoxSizer(wxVERTICAL);

	//wxButton *test_button = new wxButton(this, wxID_ANY, wxT("Root General Button"));
	//vert_sizer->Add(test_button, 0, wxRIGHT, 5);
    
    Text *msg = new Text(this, 
		wxT("The Seruro Client has many optional features which can be controlled below.")
        wxT("Use the navigation tree on the left to add additional servers and accounts, ")
		wxT("check the status of servers and accounts, configuration application settings, ")
		wxT("and control Seruro extensions."));
    vert_sizer->Add(msg, SETTINGS_PANEL_SIZER_OPTIONS);

	this->SetSizer(vert_sizer);
}
