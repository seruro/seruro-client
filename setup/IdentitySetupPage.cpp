
//#include "../frames/dialogs/DecryptDialog.h"
#include "../crypto/SeruroCrypto.h"
#include "../api/SeruroStateEvents.h"

#include "SeruroSetup.h"
//#include "../frames/UIDefs.h"

enum {
    BUTTON_DOWNLOAD_IDENTITY
};

BEGIN_EVENT_TABLE(IdentityPage, SetupPage)
	//EVT_CHECKBOX(SERURO_INSTALL_IDENTITY_ID, IdentityPage::OnToggleInstall)
    EVT_BUTTON(BUTTON_DOWNLOAD_IDENTITY, IdentityPage::OnDownloadIdentity)
    EVT_SERURO_REQUEST(SERURO_API_CALLBACK_P12S, IdentityPage::OnP12sResponse)
END_EVENT_TABLE()

void IdentityPage::DownloadIdentity()
{
    wxJSONValue params; /* no params */
    wxJSONValue server_info = wizard->GetServerInfo();
    wxString account = wizard->GetAccount();
    
	/* Disable interaction while thread is running. */
	this->DisablePage();
    this->SetIdentityStatus(_("Downloading..."));

    /* Reset the ability to install P12s (until the response SKIDs are checked). */
    install_encipherment = false;
    install_authentication = false;
    
    SeruroServerAPI *api = new SeruroServerAPI(this);
    
	params["server"] = server_info;
    params["address"] = account;
    params["no_prompt"] = true;
    
	api->CreateRequest(SERURO_API_P12S, params, SERURO_API_CALLBACK_P12S)->Run();
	delete api;
}

void IdentityPage::OnP12sResponse(SeruroRequestEvent &event)
{
	wxJSONValue response = event.GetResponse();
    wxString server_uuid, address;

	/* Make sure the request worked, and enable the page. */
	if (! response["success"].AsBool()) {
        if (response.HasMember("error")) {
            this->SetIdentityStatus(response["error"].AsString());
        } else {
            this->SetIdentityStatus(_("Unable to download."));
        }
		this->EnablePage();
		/* The page will work, but the key entry will not. */
		this->DisableForm();
		return;
	}
    
    server_uuid = response["uuid"].AsString();
    address = response["address"].AsString();
    
    /* Compare the P12 response SKIDs to installed SKIDs (if any). These may (again) disable the form. */
    SeruroCrypto crypto;
    if (crypto.HaveIdentity(server_uuid, address, response["p12"]["encipherment"][0].AsString())) {
        this->SetEnciphermentHint(_("Encryption already installed."));
    } else {
        /* The install command should attempt to decrypt and install the encipherment P12. */
        this->SetEnciphermentHint(wxEmptyString);
        install_encipherment = true;
    }
    
    if (crypto.HaveIdentity(server_uuid, address, response["p12"]["authentication"][0].AsString())) {
        this->SetAuthenticationHint(_("Digital Identity already installed."));
    } else {
        /* Likewise for the authentication P12. */
        this->SetAuthenticationHint(wxEmptyString);
        install_authentication = true;
    }

    /* Save the P12 information. */
	this->download_response = response;
	this->identity_downloaded = true;
	this->SetIdentityStatus(_("Downloaded."));
    
    this->EnablePage();
    //this->download_button->SetLabelText(_("Re-Download"));
}

void IdentityPage::OnDownloadIdentity(wxCommandEvent &event)
{
	this->DownloadIdentity();
}

void IdentityPage::DoFocus()
{
	/* This is only focued using success response. */
	if (! this->identity_downloaded) {
		this->DisableForm();
	}

	/* Check to see if this page should automatically download. */
	if (this->force_download && SERURO_ALLOW_AUTO_DOWNLOAD) {
        /* The client may not allow forced downloads. */
		this->DisablePage();
		this->DownloadIdentity();
		/* Only perform this action once. */
		this->force_download = false;
	}
}

IdentityPage::IdentityPage(SeruroSetup *parent, bool force_download)
	: SetupPage(parent), DecryptForm(this),
	identity_downloaded(false), identity_installed(false), 
	/* The wizard can instruct the identity page to force a download. */
	force_download(force_download)
{    
    wxSizer *const vert_sizer = new wxBoxSizer(wxVERTICAL);
    
	/* The identity page does not allow the user to go backward. */
	this->enable_prev = false;
    this->enable_next = false;
	/* The user may not proceeded unless the P12 is download (may happen automatically). */
	this->next_button = _("&Install");

    /* Textual notice about the use of an identity. */
    wxString msg_text = _(TEXT_INSTALL_IDENTITY);
    Text *msg = new Text(this, msg_text);
    vert_sizer->Add(msg, DIALOGS_SIZER_OPTIONS);
    
    /* Download form if the P12 is not retreived automatically. */
    wxSizer *const identity_form = new wxStaticBoxSizer(wxHORIZONTAL, this, "&Download Identity and Encryption Containers");
    identity_form->Add(new Text(this, _("I trust this machine: ")), DIALOGS_SIZER_OPTIONS);
    identity_form->AddStretchSpacer();
    download_button = new wxButton(this, BUTTON_DOWNLOAD_IDENTITY, _("Download"));
    identity_form->Add(download_button, DIALOGS_SIZER_OPTIONS);
    vert_sizer->Add(identity_form, DIALOGS_BOXSIZER_SIZER_OPTIONS);

	/* Decrypt form. */
	wxSizer *const decrypt_form = new wxStaticBoxSizer(wxVERTICAL, this, "&Unlock Identity and Encryption Containers");

	/* Add decryption input. */
	this->AddForms(decrypt_form);
    this->DisableForm();

    /* Add status message. */
    wxSizer *const status_sizer = new wxBoxSizer(wxHORIZONTAL);
	status_sizer->Add(new Text(this, _("Download status: ")), DIALOGS_SIZER_OPTIONS);
    status_sizer->SetItemMinSize((size_t) 0, SERURO_SETTINGS_FLEX_LABEL_WIDTH, -1);
    
	identity_status = new Text(this, _("Not downloaded."));
	status_sizer->Add(this->identity_status, DIALOGS_SIZER_OPTIONS);
	decrypt_form->Add(status_sizer, DIALOGS_BOXSIZER_OPTIONS);
    
	vert_sizer->Add(decrypt_form, DIALOGS_BOXSIZER_SIZER_OPTIONS);

    this->SetSizer(vert_sizer);
}

void IdentityPage::EnablePage()
{
    if (this->identity_downloaded) {
		this->wizard->EnableNext(true);
	}
    
    download_button->Enable();
	this->EnableForm();
    if (! install_authentication) {
        this->DisableAuthentication();
    }
    if (! install_encipherment) {
        this->DisableEncipherment();
    }
    
	this->FocusForm();
}

void IdentityPage::DisablePage()
{
	if (! this->identity_downloaded) {
		download_button->Enable(false);
	}
	this->DisableForm();
}

bool IdentityPage::GoNext(bool from_callback)
{
    wxJSONValue unlock_codes;
    wxString server_uuid, address;
    
	/* Either a subsequent click or a callback success. */
	if (this->identity_installed) {
		if (from_callback) {
			this->wizard->ForceNext();
		}
		return true;
	}

	/* If the install failed, do not allow callbacks to loop. */
	if (from_callback) return false;
	/* The identity must have been downloaded. */
	if (! identity_downloaded) return false;

	/* About to do some security-related work, which may block for a while. */
    unlock_codes = this->GetValues();
	this->DisablePage();

	SeruroServerAPI *api = new SeruroServerAPI(this);
    
    server_uuid = this->download_response["server_uuid"].AsString();
    address = this->download_response["address"].AsString();
    
	/* Try to install with the saved response, and the input key, force the install incase there is an empty input. */
    bool try_install = true;
    if (install_authentication) {
        try_install &= api->InstallP12(server_uuid, address, _("authentication"),
            this->download_response["p12"]["authentication"][1].AsString(),
            unlock_codes["authentication"].AsString(), true);
    }
    
    if (install_encipherment) {
        try_install &= api->InstallP12(server_uuid, address, _("encipherment"),
            this->download_response["p12"]["encipherment"][1].AsString(),
            unlock_codes["encipherment"].AsString(), true);
    }
    
    delete api;
    
	if (! try_install) {
        /* Enable page is responsible for checking each p12/skid from download_response. */
		this->EnablePage();
		this->SetIdentityStatus(_("Unable to install (incorrect unlock code?)."));
		return false;
	}
    
    /* Create identity installed event. */
    SeruroStateEvent event(STATE_TYPE_ACCOUNT, STATE_ACTION_UPDATE);
    event.SetServerUUID(download_response["server_uuid"].AsString());
    event.SetAccount(download_response["address"].AsString());
    this->ProcessWindowEvent(event);

	/* Proceed to the next page. */
	this->SetIdentityStatus(_("Success."));
	identity_installed = true;
    
	return true;
}