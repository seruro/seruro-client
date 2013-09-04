
#include "SettingsWindows.h"
#include "../SeruroSettings.h"
#include "../UIDefs.h"

#include "../../SeruroConfig.h"
#include "../../SeruroClient.h"

#include <wx/textctrl.h>

#define TEXT_GENERAL_SETTINGS "You may change general options, which control how Seruro behaves. \
The it is best to enable all options. \
If you use multiple Seruro servers you may select the a 'default' for searching."

#define TEXT_OPTION_AUTO_DOWNLOAD "Download Seruro contacts automatically."
#define TEXT_OPTION_DEFAULT_SERVER "Set the default Seruro server for searches."
#define TEXT_OPTION_SAVE_ENCIPHERMENT "Never delete Encryption certificates (when removing accounts)."
#define TEXT_OPTION_POLL_REVOCATIONS "Continue to monitor Seruro certificate revocations."
#define TEXT_OPTION_POLL_CERTSTORE "Monitor the operating system for certificate changes."

#define OPTION_SIZER_OPTIONS wxSizerFlags().Expand().Border(wxALL, 5).Border(wxLEFT | wxBOTTOM, 10)
#define SERVER_SIZER_OPTIONS wxSizerFlags().Border(wxLEFT, 5)

enum setting_options_ids
{
	OPTION_AUTO_DOWNLOAD_ID,
	OPTION_DEFAULT_SERVER_ID,
	OPTION_POLL_REVOCATIONS_ID,
	OPTION_POLL_CERTSTORE_ID,
	OPTION_SAVE_ENCIPHERMENT_ID
};

SettingsView::SettingsView(SeruroPanelSettings *window) : parent(window),
    wxWindow(window, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME) {
        SetBackgroundColour(_("white"));
}

BEGIN_EVENT_TABLE(GeneralWindow, SettingsView)
	EVT_CHOICE(OPTION_DEFAULT_SERVER_ID, GeneralWindow::OnDefaultServer)

	EVT_CHECKBOX(OPTION_AUTO_DOWNLOAD_ID, GeneralWindow::OnAutoDownload)
	EVT_CHECKBOX(OPTION_SAVE_ENCIPHERMENT_ID, GeneralWindow::OnSaveEncipherment)
	EVT_CHECKBOX(OPTION_POLL_REVOCATIONS_ID, GeneralWindow::OnPollRevocations)
	EVT_CHECKBOX(OPTION_POLL_CERTSTORE_ID, GeneralWindow::OnPollCertstore)
END_EVENT_TABLE()

DECLARE_APP(SeruroClient);

void GeneralWindow::OnServerStateEvent(SeruroStateEvent &event)
{
    if (event.GetAction() == STATE_ACTION_ADD || event.GetAction() == STATE_ACTION_REMOVE) {
        //this->GenerateServersList();
    }
	event.Skip();
}

void GeneralWindow::OnOptionStateChange(SeruroStateEvent &event)
{
    wxCheckBox *affected_option;
    wxString option_name;
    wxString option_value;
    
    event.Skip();
    
    option_name = event.GetValue("option_name");
    option_value = event.GetValue("option_value");
    
    /* Set the server selection. */
    if (option_name == "default_server") {
        //this->GenerateServersList();
        return;
    }
    
    /* Switch over possible boolean controls. */
    if (option_name == "auto_download") {
        affected_option = this->option_auto_download;
    } else if (option_name == "save_encipherment") {
        affected_option = this->option_save_encipherment;
    } else if (option_name == "poll_revocations") {
        affected_option = this->option_poll_revocations;
    } else if (option_name == "poll_certstore") {
        affected_option = this->option_poll_certstore;
    } else {
        return;
    }
    
    /* This works event if this control set the value. */
    if (option_value == "true") {
        affected_option->SetValue(true);
    } else {
        affected_option->SetValue(false);
    }
}

void GeneralWindow::GenerateServersList()
{
	wxArrayString server_names;
	wxString default_server_uuid;
	wxString server_uuid;

	/* Regenerate the names. */
	server_names = theSeruroConfig::Get().GetServerNames();
	this->option_default_server->Clear();
	this->option_default_server->Append(server_names);

	/* Disable the default server option if there's no choice. */
	if (this->option_default_server->GetCount() <= 1) {
		this->option_default_server->Disable();
	} else {
		this->option_default_server->Enable();
	}

	/* Set the appropriate selection. */
	default_server_uuid = theSeruroConfig::Get().GetOption("default_server");
	if (default_server_uuid == wxEmptyString) {
		this->option_default_server->SetSelection(0);
		return;
	}

	for (size_t i = 0; i < this->option_default_server->GetCount(); i++) {
		server_uuid = theSeruroConfig::Get().GetServerUUID(this->option_default_server->GetString(i));
		if (default_server_uuid == server_uuid) {
			/* Show the default server as the selected server. */
			this->option_default_server->SetSelection(i);
		}
	}
	return;
}

void GeneralWindow::OnAutoDownload(wxCommandEvent &event)
{
	this->SetBoolean("auto_download", this->option_auto_download->IsChecked());
}

void GeneralWindow::OnDefaultServer(wxCommandEvent &event)
{
	wxString server_uuid;
	int index;

	index = this->option_default_server->GetSelection();
	server_uuid = theSeruroConfig::Get().GetServerUUID(this->option_default_server->GetString(index));
	
	this->SetString("default_server", server_uuid);
}

void GeneralWindow::OnSaveEncipherment(wxCommandEvent &event)
{
	this->SetBoolean("save_encipherment", this->option_save_encipherment->IsChecked());
}

void GeneralWindow::OnPollRevocations(wxCommandEvent &event)
{
	this->SetBoolean("poll_revocations", this->option_poll_revocations->IsChecked());
}

void GeneralWindow::OnPollCertstore(wxCommandEvent &event)
{
	this->SetBoolean("poll_certstore", this->option_poll_certstore->IsChecked());
}

bool GeneralWindow::GetBoolean(wxString key)
{
	return (theSeruroConfig::Get().GetOption(key) == "true");
}

void GeneralWindow::SetBoolean(wxString key, bool value)
{
	theSeruroConfig::Get().SetOption(key, (value) ? "true" : "false", true);
}

void GeneralWindow::SetString(wxString key, wxString value)
{
	theSeruroConfig::Get().SetOption(key, value, true);
}

GeneralWindow::GeneralWindow(SeruroPanelSettings *window) : SettingsView(window)
{
	wxSizer *const sizer = new wxBoxSizer(wxVERTICAL);
    
	//wxButton *button = new wxButton(this, wxID_ANY, _("General"));
    //sizer->Add(button, DIALOGS_SIZER_OPTIONS);

	/* A simple message about general settings. */
	sizer->Add(new Text(this, _(TEXT_GENERAL_SETTINGS)), DIALOGS_SIZER_OPTIONS);

	option_auto_download = new wxCheckBox(this, OPTION_AUTO_DOWNLOAD_ID, TEXT_OPTION_AUTO_DOWNLOAD);
	option_auto_download->SetValue(this->GetBoolean("auto_download"));
	sizer->Add(option_auto_download, OPTION_SIZER_OPTIONS);
	sizer->AddSpacer(10);

	/* Put default server selector in it's own horizontal sizer. */
	//wxSizer *const server_sizer = new wxBoxSizer(wxHORIZONTAL);
	//server_sizer->Add(new Text(this, _(TEXT_OPTION_DEFAULT_SERVER)), OPTION_SIZER_OPTIONS);

	//option_default_server = new wxChoice(this, OPTION_DEFAULT_SERVER_ID);
	//this->GenerateServersList();

	//server_sizer->Add(option_default_server, SERVER_SIZER_OPTIONS);
	//sizer->Add(server_sizer, DIALOGS_BOXSIZER_SIZER_OPTIONS);
	//sizer->AddStretchSpacer();

	/* Add the less-commonly-used options. */
	option_save_encipherment = new wxCheckBox(this, OPTION_SAVE_ENCIPHERMENT_ID, TEXT_OPTION_SAVE_ENCIPHERMENT);
	option_save_encipherment->SetValue(this->GetBoolean("save_encipherment"));
	option_save_encipherment->Disable();
	sizer->Add(option_save_encipherment, OPTION_SIZER_OPTIONS);

	option_poll_revocations = new wxCheckBox(this, OPTION_POLL_REVOCATIONS_ID, TEXT_OPTION_POLL_REVOCATIONS);
	option_poll_revocations->SetValue(this->GetBoolean("poll_revocations"));
	option_poll_revocations->Disable();
	sizer->Add(option_poll_revocations, OPTION_SIZER_OPTIONS);

	option_poll_certstore = new wxCheckBox(this, OPTION_POLL_CERTSTORE_ID, TEXT_OPTION_POLL_CERTSTORE);
	option_poll_certstore->SetValue(this->GetBoolean("poll_certstore"));
	option_poll_certstore->Disable();
	sizer->Add(option_poll_certstore, OPTION_SIZER_OPTIONS);

	/* Monitor server changes. */
	wxGetApp().Bind(SERURO_STATE_CHANGE, &GeneralWindow::OnServerStateEvent, this, STATE_TYPE_SERVER);
    wxGetApp().Bind(SERURO_STATE_CHANGE, &GeneralWindow::OnOptionStateChange, this, STATE_TYPE_OPTION);

    this->SetSizer(sizer);
	this->Layout();
}

