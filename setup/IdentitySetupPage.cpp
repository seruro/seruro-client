
#include "../frames/dialogs/DecryptDialog.h"
#include "../crypto/SeruroCrypto.h"

#include "SeruroSetup.h"
#include "../frames/UIDefs.h"

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
	wxJSONValue server_info = ((ServerPage *) wizard->GetServerPage())->GetValues();
	wxJSONValue account_info = ((AccountPage *) wizard->GetAccountPage())->GetValues();
    
	/* Disable interaction while thread is running. */
	this->DisablePage();

    SeruroServerAPI *api = new SeruroServerAPI(this->GetEventHandler());
    
	//params["server"] = api->GetServer(this->server_name);
	params["server"] = api->GetServer(server_info["name"].AsString());
	//params["address"] = this->address;
	params["address"] = account_info["address"].AsString();
    
	api->CreateRequest(SERURO_API_P12S, params, SERURO_API_CALLBACK_P12S)->Run();
	delete api;
}

void IdentityPage::OnP12sResponse(SeruroRequestEvent &event)
{
	wxJSONValue response = event.GetResponse();

	/* Make sure the request worked, and enable the page. */
	if (! response["success"].AsBool()) {
		this->SetIdentityStatus(_("Unable to download."));
		this->EnablePage();
		return;
	}

	/* Save the P12 information. */
	this->download_response = response;
	this->identity_downloaded = true;
	this->SetIdentityStatus(_("Downloaded."));

	this->EnablePage();
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
	if (this->force_download) {
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
    wxSizer *const identity_form = new wxStaticBoxSizer(wxHORIZONTAL, this, "&Download Identity");
    identity_form->Add(new Text(this, _("I trust, and send email from, this machine: ")), DIALOGS_SIZER_OPTIONS);
    identity_form->AddStretchSpacer();
    download_button = new wxButton(this, BUTTON_DOWNLOAD_IDENTITY, _("Download"));
    identity_form->Add(download_button, DIALOGS_SIZER_OPTIONS);
    vert_sizer->Add(identity_form, DIALOGS_BOXSIZER_SIZER_OPTIONS);

	/* Decrypt form. */
	wxSizer *const decrypt_form = new wxStaticBoxSizer(wxVERTICAL, this, "&Decrypt Identity");
	wxSizer *const status_sizer = new wxBoxSizer(wxHORIZONTAL);
	
	//wxFlexGridSizer *const decrypt_grid_sizer = new wxFlexGridSizer(2, 5, 10);
	status_sizer->Add(new Text(this, _("Download status: ")), DIALOGS_SIZER_OPTIONS);
	identity_status = new Text(this, _("Not downloaded."));
	status_sizer->Add(this->identity_status, DIALOGS_SIZER_OPTIONS);
	decrypt_form->Add(status_sizer, DIALOGS_BOXSIZER_OPTIONS);

	/* Add decryption input. */
	this->AddForms(decrypt_form);

	//decrypt_form->Add(decrypt_grid_sizer, DIALOGS_BOXSIZER_OPTIONS);
	vert_sizer->Add(decrypt_form, DIALOGS_BOXSIZER_SIZER_OPTIONS);

    this->SetSizer(vert_sizer);
}

void IdentityPage::EnablePage()
{
    if (! this->identity_downloaded) {
        download_button->Enable();
        //download_button->SetLabelText("&Download");
    } else {
		this->wizard->EnableNext(true);
	}
	this->EnableForm();
	this->FocusForm();
}

void IdentityPage::DisablePage()
{
	if (! this->identity_downloaded) {
		download_button->Enable(false);
		//download_button->SetLabelText("&Downloaded");
	}
	this->DisableForm();
}

bool IdentityPage::GoNext(bool from_callback)
{
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
	this->DisablePage();

	SeruroServerAPI *api = new SeruroServerAPI(this);
	/* Try to install with the saved response, and the input key. */
	bool try_install = api->InstallP12(this->download_response, this->GetValue());
	if (! try_install) {
		this->EnablePage();
		this->SetIdentityStatus(_("Unable to install (incorrect key?)."));
		return false;
	}
	delete api;

	/* Make sure the certificates are available. */
	SeruroCrypto crypto_helper;
	try_install = crypto_helper.HaveIdentity(download_response["server_name"].AsString(),
		download_response["address"].AsString());
	if (! try_install) {
		this->SetIdentityStatus(_("Unable to install identity."));
		this->EnablePage();
		return false;
	}

	/* Proceed to the next page. */
	this->SetIdentityStatus(_("Success."));
	identity_installed = true;
	return true;
}