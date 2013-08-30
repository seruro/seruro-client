
//#include "../frames/dialogs/DecryptDialog.h"
#include "../crypto/SeruroCrypto.h"
#include "../api/SeruroStateEvents.h"

#include "SeruroSetup.h"
//#include "../frames/UIDefs.h"

#include <wx/event.h>

enum {
    BUTTON_DOWNLOAD_IDENTITY
};

#define SETUP_REQUIRE_DOWNLOAD 0

BEGIN_EVENT_TABLE(IdentityPage, SetupPage)
	//EVT_CHECKBOX(SERURO_INSTALL_IDENTITY_ID, IdentityPage::OnToggleInstall)
    EVT_BUTTON(BUTTON_DOWNLOAD_IDENTITY, IdentityPage::OnDownloadIdentity)
    EVT_SERURO_REQUEST(SERURO_API_CALLBACK_P12S, IdentityPage::OnP12sResponse)
END_EVENT_TABLE()

DECLARE_APP(SeruroClient);

void IdentityPage::DownloadIdentity()
{
    wxJSONValue params; /* no params */
    wxJSONValue server_info = wizard->GetServerInfo();
    wxString account = wizard->GetAccount();
    
	/* Disable interaction while thread is running. */
	this->DisablePage();
    this->SetIdentityStatus(_("Downloading certificates..."));

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
    wxString server_uuid, address, method_hint;

	/* Make sure the request worked, and enable the page. */
	if (! response["success"].AsBool()) {
        if (response.HasMember("error")) {
            this->SetIdentityStatus(response["error"].AsString());
        } else {
            this->SetIdentityStatus(_("Unable to download."));
        }
        
        if (! SETUP_REQUIRE_DOWNLOAD) {
            /* A download form is needed for retires. */
            this->AddDownloadForm();
        }
        
		this->EnablePage();
		/* The page will work, but the key entry will not. */
		this->DisableForm();
		return;
	}
    
    server_uuid = response["uuid"].AsString();
    address = response["address"].AsString();
    
	/* There is a potential unorthodox TOCTOU problem: the current method may not be the "used" method. */
	/* Todo: consider logging the method used within the certificate/user record. */
	if (response["method"].AsString() == "email") {
		method_hint = wxString::Format(_("The unlock code was sent to %s"), address);
	} else if (response["method"].AsString() == "sms") {
		method_hint = _("The unlock code was sent to your mobile number");
	}

    /* Compare the P12 response SKIDs to installed SKIDs (if any). These may (again) disable the form. */
    SeruroCrypto crypto;
    if (crypto.HaveIdentity(server_uuid, address, response["p12"]["encipherment"][0].AsString())) {
        this->SetEnciphermentHint(_(SERURO_ENCIPHERMENT L" already installed."));
        /* Make sure the identity skid is set in config. */
        wxGetApp().config->AddIdentity(server_uuid, address, ID_ENCIPHERMENT,
            response["p12"]["encipherment"][0].AsString());
		install_encipherment = false;
    } else {
        /* The install command should attempt to decrypt and install the encipherment P12. */
        this->SetEnciphermentHint(method_hint);
        install_encipherment = true;
    }
    
    if (crypto.HaveIdentity(server_uuid, address, response["p12"]["authentication"][0].AsString())) {
        this->SetAuthenticationHint(_(SERURO_AUTHENTICATION L" already installed."));
        /* Make sure the encryption skid is set in config. */
        wxGetApp().config->AddIdentity(server_uuid, address, ID_AUTHENTICATION,
            response["p12"]["authentication"][0].AsString());
		install_authentication = false;
    } else {
        /* Likewise for the authentication P12. */
        this->SetAuthenticationHint(method_hint);
        install_authentication = true;
    }

    /* Save the P12 information. */
	this->download_response = response;
	this->identity_downloaded = true;

	if (! install_authentication && ! install_encipherment) {
		this->SetIdentityStatus(_("Certificates already installed."));
		this->wizard->SetButtonText(wxEmptyString, _("Next >"));
		this->wizard->FocusNext();
		identity_installed = true;
        
        /* Create Identity event. */
        SeruroStateEvent identity_event(STATE_TYPE_IDENTITY, STATE_ACTION_UPDATE);
        identity_event.SetServerUUID(server_uuid);
        identity_event.SetAccount(address);
        this->ProcessWindowEvent(identity_event);
	} else {
		this->SetIdentityStatus(_("Certificates downloaded, please install."));
	}

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
    
    //if (! SETUP_REQUIRE_DOWNLOAD) {
    //    this->DisablePage();
    //    this->DownloadIdentity();
    //}

	/* Check to see if this page should automatically download. */
	if (this->force_download) {
        /* The client may not allow forced downloads. */
		this->DisablePage();
		this->DownloadIdentity();
		/* Only perform this action once. */
		this->force_download = false;
	}
}

/* Todo: remove download form. */



void IdentityPage::AddDownloadForm()
{
    if (SETUP_REQUIRE_DOWNLOAD || this->download_button != 0) {
        /* The download form already exists. */
        return;
    }
    
    wxSizer *const page_sizer = this->GetSizer();
    
    /* Generate a form, duplicate of the initializer. */
    wxSizer *const identity_form = new wxStaticBoxSizer(wxHORIZONTAL, this, "&Download Encryption and Digital Identity");
    identity_form->Add(new Text(this, _(TEXT_DOWNLOAD_INSTALL_WARNING)), DIALOGS_SIZER_OPTIONS);
    identity_form->AddStretchSpacer();
    
    download_button = new wxButton(this, BUTTON_DOWNLOAD_IDENTITY, _("Retry Download"));
    identity_form->Add(download_button, DIALOGS_SIZER_OPTIONS);
    
    page_sizer->Insert(1, identity_form, DIALOGS_BOXSIZER_SIZER_OPTIONS);
    
    this->Layout();
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
    wxString msg_text = _((SETUP_REQUIRE_DOWNLOAD) ? TEXT_DOWNLOAD_INSTALL_IDENTITY : TEXT_INSTALL_IDENTITY);
    Text *msg = new Text(this, msg_text);
    vert_sizer->Add(msg, DIALOGS_SIZER_OPTIONS);
    
    /* Download form if the P12 is not retreived automatically. */
    if (SETUP_REQUIRE_DOWNLOAD) {
        wxSizer *const identity_form = new wxStaticBoxSizer(wxHORIZONTAL, this, "&Download Identity and Encryption Containers");
        identity_form->Add(new Text(this, _("I trust this machine: ")), DIALOGS_SIZER_OPTIONS);
        identity_form->AddStretchSpacer();
        download_button = new wxButton(this, BUTTON_DOWNLOAD_IDENTITY, _("Download"));
        identity_form->Add(download_button, DIALOGS_SIZER_OPTIONS);
        vert_sizer->Add(identity_form, DIALOGS_BOXSIZER_SIZER_OPTIONS);
    } else {
        /* Tell the focus to download one time. */
        this->download_button = 0;
        this->force_download = true;
    }

	/* Decrypt form. */
	wxSizer *const decrypt_form = new wxStaticBoxSizer(wxVERTICAL, this, "&Unlock Digital Identity and Encryption Codes");

	/* Add decryption input. */
	this->AddForms(decrypt_form);
    this->DisableForm();

    /* Add status message. */
    wxSizer *const status_sizer = new wxBoxSizer(wxHORIZONTAL);
	status_sizer->Add(new Text(this, _("Install status: ")), DIALOGS_SIZER_OPTIONS);
    status_sizer->SetItemMinSize((size_t) 0, SERURO_SETTINGS_FLEX_LABEL_WIDTH, -1);
    
	identity_status = new Text(this, _("Not downloaded or unlocked."));
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
    
    if (download_button != 0) {
        download_button->Enable();
    }
    
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
    if (download_button != 0) {
        if (! this->identity_downloaded) {
            download_button->Enable(false);
        }
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
    bool authentication_result = true, encipherment_result = true;
    if (install_authentication) {
        authentication_result = api->InstallP12(server_uuid, address, 
			ID_AUTHENTICATION,
            this->download_response["p12"]["authentication"][1].AsString(),
            unlock_codes["authentication"].AsString(), true);
    }
    
    if (install_encipherment) {
        encipherment_result = api->InstallP12(server_uuid, address, 
			ID_ENCIPHERMENT,
            this->download_response["p12"]["encipherment"][1].AsString(),
            unlock_codes["encipherment"].AsString(), true);
    }
    
    delete api;
    
	wxString failure_text;
	if (authentication_result) {
		/* No longer need to install authentication p12. */
		install_authentication = false;
		/* A bit of a hack. */
		this->authentication_control->Clear();
		this->SetAuthenticationHint(_(SERURO_AUTHENTICATION L" installed."));
	} else {
		/* Advertize WHAT container (p12) failed. */
		failure_text = _(SERURO_AUTHENTICATION);
	}

	if (encipherment_result) {
		/* No longer need to install encipherment p12. */
		install_encipherment = false;
		/* A bit of a hack. */
		this->encipherment_control->Clear();
		this->SetEnciphermentHint(_(SERURO_ENCIPHERMENT L" installed."));
	} else {
		/* They may have failed both, or just encipherment? */
		failure_text = (failure_text.IsEmpty()) ? _(SERURO_ENCIPHERMENT) : _(SERURO_AUTHENTICATION L" and " SERURO_ENCIPHERMENT);
	}

	if (! authentication_result || ! encipherment_result) {
        /* Enable page is responsible for checking each p12/skid from download_response. */
		this->EnablePage();
		this->SetIdentityStatus(wxString::Format(_("Unable to unlock %s certificate."), failure_text));
		return false;
	}
    
    /* Create identity installed event. */
    SeruroStateEvent event(STATE_TYPE_IDENTITY, STATE_ACTION_UPDATE);
    event.SetServerUUID(download_response["server_uuid"].AsString());
    event.SetAccount(download_response["address"].AsString());
    this->ProcessWindowEvent(event);

	/* Proceed to the next page. */
	this->SetIdentityStatus(_("Success."));
	identity_installed = true;
    
	return true;
}