
#include "ServerMonitor.h"
#include "SeruroStateEvents.h"
#include "SeruroServerAPI.h"
#include "../crypto/SeruroCrypto.h"

#include "../SeruroClient.h"
#include "../SeruroConfig.h"
#include "../apps/SeruroApps.h"
#include "../logging/SeruroLogger.h"

DECLARE_APP(SeruroClient);

ServerMonitor::ServerMonitor(bool is_transient)
{
    this->requests = wxJSONValue(wxJSONTYPE_OBJECT);
    
    if (is_transient) {
        /* Bind statically. */
        wxGetApp().Bind(SERURO_REQUEST_RESPONSE, &ServerMonitor::StaticUpdateResponse, SERURO_API_CALLBACK_UPDATE);
    } else {
        wxGetApp().Bind(SERURO_REQUEST_RESPONSE, &ServerMonitor::OnUpdateResponse, this, SERURO_API_CALLBACK_UPDATE);
    }
}

void ServerMonitor::StaticUpdateResponse(SeruroRequestEvent &event)
{
    ServerMonitor *transient_monitor = new ServerMonitor(true);
    transient_monitor->OnUpdateResponse(event);
    delete transient_monitor;
}

void ServerMonitor::OnUpdateResponse(SeruroRequestEvent &event)
{
    size_t request_id = 0;
    wxJSONValue response = event.GetResponse();
    wxString server_uuid;
    bool auto_install;
    
    server_uuid = response["server_uuid"].AsString();
    if (response.HasMember("request_id")) {
        //this->requests[response["server_uuid"]]
        request_id = (size_t) response["request_id"].AsUInt();
    }

	/* Remove from the queue */
    if (this->requests.HasMember(server_uuid)) {
        this->requests.Remove(server_uuid);
    }

	if (response.HasMember("success") && response["success"].AsBool() == false) {
		DEBUG_LOG(_("ServerMonitor> (OnUpdateResponse) update responded with (%s)."), response["error"].AsString());

		/* Reset the last_update to backtrack. */
		//theSeruroConfig::Get().SetServerOption(server_uuid, "last_update", "0");
		return;
	}
    
    /* Make sure the client is still auto downloading from servers. */
    auto_install = (theSeruroConfig::Get().GetOption("auto_download") == "true");
    if (auto_install) {
    
        /* Store the last_update as the last time the server responded. */
        theSeruroConfig::Get().SetServerOption(server_uuid, "last_update", response["now"].AsString());
    
        if (response["results"].HasMember("users")) {
            ProcessContacts(server_uuid, response["results"]["users"]);
        }
    
        if (response["results"].HasMember("certificates")) {
            ProcessCertificates(server_uuid, response["results"]["certificates"]);
        }
    }

}

bool ServerMonitor::ProcessContacts(wxString server_uuid, wxJSONValue contacts)
{
    int num_contacts;
    wxString address;
    bool update_contact;
    wxJSONValue existing_contact;
    
    num_contacts = contacts.Size();
    for (int i = 0; i < num_contacts; ++i) {
        address = contacts[i]["address"].AsString();
        
        update_contact = theSeruroConfig::Get().HasContact(server_uuid, address);
        if (update_contact) {
            existing_contact = theSeruroConfig::Get().GetContact(server_uuid, address);
            if (existing_contact["name"][0].AsString() == contacts[i]["first_name"].AsString() &&
                existing_contact["name"][1].AsString() == contacts[i]["last_name"].AsString()) {
                /* There was no change to the contact, do not create an event. */
                continue;
            }
        }
        
        theSeruroConfig::Get().AddContact(server_uuid, address,
            contacts[i]["first_name"].AsString(), contacts[i]["last_name"].AsString());
        
        SeruroStateEvent event(STATE_TYPE_CONTACT, (update_contact) ? STATE_ACTION_UPDATE : STATE_ACTION_ADD);
        event.SetAccount(address);
        event.SetServerUUID(server_uuid);
        wxGetApp().AddEvent(event);
    }
    
    return true;
}

bool ServerMonitor::ProcessCertificates(wxString server_uuid, wxJSONValue certificates)
{
    SeruroCrypto crypto;
    wxString cert_fingerprint;
    wxMemoryBuffer cert_decoded;
    wxString cert_encoded;
    bool result;
    
	wxArrayString address_list;
	wxString cert_type;

	address_list = certificates.GetMemberNames();
	for (size_t i = 0; i < address_list.size(); ++i) {
		for (int j = 0; j < certificates[address_list[i]].Size(); j++) {
			/* For each address provided, parse certificates. */
			cert_encoded = certificates[address_list[i]][j]["cert"].AsString();
			cert_type = certificates[address_list[i]][j]["type"].AsString();
			if (! DecodeBase64(cert_encoded, &cert_decoded)) continue;

			cert_fingerprint = wxEmptyString; /* must reset the fingerprint. */
			result = crypto.InstallCertificate(cert_decoded, cert_fingerprint);
			if (! result) continue;

			theSeruroConfig::Get().AddCertificate(server_uuid, address_list[i], 
				(cert_type == _("authentication")) ? ID_AUTHENTICATION : ID_ENCIPHERMENT, cert_fingerprint);
		}

		/* Add the contact, this may add the contact "again", if so it will update certificates. */
		theSeruroApps::Get().AddContact(server_uuid, address_list[i]);

		/* Create the update event. */
        SeruroStateEvent event(STATE_TYPE_CONTACT, STATE_ACTION_UPDATE);
        event.SetAccount(address_list[i]);
        event.SetServerUUID(server_uuid);
        wxGetApp().AddEvent(event);
	}
    
    return true;
}

bool ServerMonitor::Monitor()
{
    SeruroServerAPI *api;
    SeruroRequest *request;
    wxArrayString servers;
    wxJSONValue params;
    
    /* Only run the server monitor if this client is auto downloading. */
    if (! theSeruroConfig::Get().GetOption("auto_download") == _("true")) {
        return true;
    }
    
    DEBUG_LOG("ServerMonitor> (Monitor) running...");
    servers = theSeruroConfig::Get().GetServerList();
    
    api = new SeruroServerAPI();
    for (size_t i = 0; i < servers.size(); ++i) {
        if (this->requests.HasMember(servers[i])) {
            /* We are already waiting for a response from this server.*/
            continue;
        }
        
        /* Use each server's specific "last_update" timestamp. */
        params["server"] = theSeruroConfig::Get().GetServer(servers[i]);
        params["update"] = theSeruroConfig::Get().HasServerOption(servers[i], "last_update") ?
			theSeruroConfig::Get().GetServerOption(servers[i], "last_update") : "1";
        
        /* Create thread, watch the thread ID, and run. */
        request = api->CreateRequest(SERURO_API_UPDATE, params, SERURO_API_CALLBACK_UPDATE);
        this->requests[servers[i]] = (unsigned int) request->request_id;
        request->Run();
    }
    delete api;
    
    
    return true;
}