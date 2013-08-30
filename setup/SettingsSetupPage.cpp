/* Remember to set enable_prev to false. */

#include "SeruroSetup.h"
#include "../SeruroClient.h"


#include "../frames/SeruroFrame.h"

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
    
}

void SettingsPage::OnDefaultOption(wxCommandEvent &event)
{
    
}

void SettingsPage::DoFocus()
{
    wxJSONValue server_info;
    
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
    } else {
        theSeruroConfig::Get().SetOption("auto_download", "true");
    }
    
    if (theSeruroConfig::Get().GetOption("default_server") != wxEmptyString) {
        this->default_option->SetValue(false);
    } else {
        theSeruroConfig::Get().SetOption("default_server", server_info["uuid"].AsString());
    }
}

SettingsPage::SettingsPage(SeruroSetup *parent) : SetupPage(parent)
{
    wxSizer *const vert_sizer = new wxBoxSizer(wxVERTICAL);
    
    /* Generic explaination. */
    Text *msg = new Text(this, TEXT_FINISH_SETUP);
    vert_sizer->Add(msg, DIALOGS_SIZER_OPTIONS);
    
    this->server_text = new Text(this, "");
    vert_sizer->Add(this->server_text, DIALOGS_SIZER_OPTIONS);
    
    this->certs_option = new wxCheckBox(this, wxID_ANY, TEXT_DOWNLOAD_CERTS);
    this->certs_option->SetValue(true);
    
    this->default_option = new wxCheckBox(this, wxID_ANY, wxEmptyString);
    this->default_option->SetValue(true);
    this->default_option->Disable();
    
    this->SetSizer(vert_sizer);
}



