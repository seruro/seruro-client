/* Remember to set enable_prev to false. */

#include "SeruroSetup.h"
#include "../SeruroClient.h"
#include "../api/SeruroStateEvents.h"

#include "../frames/SeruroFrame.h"

#include "../resources/images/setup_full_step_4_flat.png.h"
#include "../resources/images/setup_identity_step_3_flat.png.h"

DECLARE_APP(SeruroClient);

BEGIN_EVENT_TABLE(SettingsPage, SetupPage)
END_EVENT_TABLE()

#define TEXT_FINISH_SETUP "The setup is finished! \
You are now ready to use Seruo to secure your email. \
Seruro will continue to run in the background to monitor your email applications and certificates. \
If you install new email applications Seruro may ask to configure them for you. \
If you choose to download contacts automatically, Seruro will continue to check the server for you. \
You may change any of these options, at any time, using the Setting menu."

#define TEXT_DOWNLOAD_CERTS "Download Seruro contacts automatically."
#define TEXT_DEFAULT_SERVER "Use %s as the default Seruro server."
#define TEXT_FINISH_READY   "You're now ready to use %s."

void SettingsPage::OnCertsOption(wxCommandEvent &event)
{
	/* Direct update. */
	theSeruroConfig::Get().SetOption("auto_download", 
		(this->certs_option->IsChecked()) ? "true" : "false", true);
	return;
}

void SettingsPage::OnDefaultOption(wxCommandEvent &event)
{
	wxJSONValue server_info;

	server_info = this->wizard->GetServerInfo();
	if (this->default_option->IsChecked()) {
		/* Set the default server to this uuid. */
		theSeruroConfig::Get().SetOption("default_server", server_info["uuid"].AsString(), true);
		return;
	}

	/* Otherwise, if there was an existing default server, replace. */
	if (this->existing_default_uuid != wxEmptyString) {
		theSeruroConfig::Get().SetOption("default_server", this->existing_default_uuid, true);
	}
}

void SettingsPage::DoFocus()
{
    wxJSONValue server_info;

    if (wizard->GetSetupType() == SERURO_SETUP_IDENTITY) {
        wizard->SetBitmap(wxGetBitmapFromMemory(setup_identity_step_3_flat));
    } else {
        wizard->SetBitmap(wxGetBitmapFromMemory(setup_full_step_4_flat));
    }
    
    server_info = this->wizard->GetServerInfo();
    this->default_option->SetLabel(wxString::Format(_(TEXT_DEFAULT_SERVER), server_info["name"].AsString()));
    this->server_text->SetLabel(wxString::Format(_(TEXT_FINISH_READY), server_info["name"].AsString()));
    
    if (theSeruroConfig::Get().GetServerList().size() > 1) {
        /* Only allow them to set the default server if there are more than one servers configured. */
        this->default_option->Enable();
    }
    
    /* Check if there were previous settings. */
    if (theSeruroConfig::Get().GetOption("auto_download") == "false") {
        this->certs_option->SetValue(false);
    } else if (theSeruroConfig::Get().GetOption("auto_download") != "true") {
        theSeruroConfig::Get().SetOption("auto_download", "true", true);
    }
    
	this->existing_default_uuid = theSeruroConfig::Get().GetOption("default_server");
	if (this->existing_default_uuid != wxEmptyString) {
        this->default_option->SetValue(false);
    } else {
        theSeruroConfig::Get().SetOption("default_server", server_info["uuid"].AsString(), true);
    }
}

SettingsPage::SettingsPage(SeruroSetup *parent) : SetupPage(parent)
{
	/* Allow them to finish. */
	this->enable_next = true;

    wxSizer *const vert_sizer = new wxBoxSizer(wxVERTICAL);
    
    /* Generic explaination. */
    Text *msg = new Text(this, TEXT_FINISH_SETUP);
    vert_sizer->Add(msg, DIALOGS_SIZER_OPTIONS);
    
    this->server_text = new Text(this, "");
    vert_sizer->Add(this->server_text, DIALOGS_SIZER_OPTIONS);
    
	/* Auto-download contacts. */
    this->certs_option = new wxCheckBox(this, wxID_ANY, TEXT_DOWNLOAD_CERTS);
    this->certs_option->SetValue(true);
    
	/* Use this server for API actions and as the default when searching. */
    this->default_option = new wxCheckBox(this, wxID_ANY, wxEmptyString);
    this->default_option->SetValue(true);
    this->default_option->Disable();

	vert_sizer->Add(this->certs_option, 0, wxEXPAND | wxTOP | wxLEFT, 10);
	vert_sizer->Add(this->default_option, 0, wxEXPAND | wxLEFT | wxTOP, 10);
    
    this->SetSizer(vert_sizer);
}



