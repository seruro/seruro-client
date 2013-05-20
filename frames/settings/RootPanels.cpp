
#include "SettingsPanels.h"
#include "../SeruroPanelSettings.h"
#include "../../SeruroClient.h"

#include <wx/button.h>
#include <wx/scrolwin.h>

DECLARE_APP(SeruroClient);

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
	SettingsPanelView(parent)
{

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

	wxSizer *accounts_list_sizer = new wxStaticBoxSizer(wxVERTICAL, this, "&Address List");

	for (size_t i = 0; i < all_address_list.size(); i++) {
		accounts_list_sizer->Add(new Text(this, all_address_list[i]),
			SETTINGS_PANEL_BOXSIZER_OPTIONS);
	}

	vert_sizer->Add(server_list_sizer, SETTINGS_PANEL_SIZER_OPTIONS);
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
