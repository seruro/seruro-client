
#include "SeruroClient.h"
#include "SeruroConfig.h"

/* Used for option edits. */
#include "api/SeruroStateEvents.h"
#include "logging/SeruroLogger.h"

#include "wxJSON/wx/jsonreader.h"
#include "wxJSON/wx/jsonwriter.h"

#include <wx/stdpaths.h>
#include <wx/filename.h>
#include <wx/thread.h>

/* Token management methods.
 * Tokens are stored in (DataDir)/tokens, or the file name listed in Defs.h.
 * The format is as such:
 * { "tokens": { "server_uuid": { "address": "token", ...} ...} }
 * This means server names must be unique and a single server cannot have duplicat addresses.
 * This also means server names and addresses must be formatted correctly.
 */
bool GetTokenFile(wxTextFile** token_file);
bool WriteTokenData(wxJSONValue token_data);
wxJSONValue GetTokenData();

DECLARE_APP(SeruroClient);

wxString GetAppDir()
{
    wxStandardPaths paths = wxStandardPaths::Get();
    return paths.GetUserDataDir();
}

bool GetTokenFile(wxTextFile** token_file)
{
	/* We will read and write the data JIT. */
	bool results;

    wxFileName token_path(GetAppDir(), _(SERURO_TOKENS_FILE));
    
	VLDDisable();
    *token_file = new wxTextFile(token_path.GetFullPath());
	VLDEnable();

	/* If the token file does not exist, create and write a template. */
	if (! (*token_file)->Exists()) {
		results = (*token_file)->Create();
		if (! results) {
			/* Could not create the token file, bail. */
			ERROR_LOG(wxT("SeruroConfig> (GetTokenFile) Error while creating token data, this is serious."));
			delete *token_file;
			return false;
		}

		LOG(wxT("SeruroConfig> (GetTokenFile) Token file did not exist, successfully created."));
	}
	return true;
}

bool WriteTokenData(wxJSONValue token_data)
{
    wxTextFile *token_file;
    bool results;
    
	/* Get pointer to token file, or fail. */
	if (! GetTokenFile(&token_file)) {
        DEBUG_LOG(_("SeruroConfig> (WriteTokenFile) could not run GetTokenFile."));
		return false;
	}
    
    /* Possibly assure writes only occur within main thread. */
    if (! wxIsMainThread()) {
        /* Stop execution? */
        int i = 0;
    }
    
	/* Write the data into a string for saving. */
	wxJSONWriter token_writer(wxJSONWRITER_STYLED);
	wxString token_string;
	token_writer.Write(token_data, token_string);

	/* Add the line and write, either way delete the memory and return the results. */
	token_file->Open();
	token_file->Clear();
	token_file->InsertLine(token_string, 0);
	results = token_file->Write();
	token_file->Close();
    
    /* Clean up. */
    token_string.Clear();
	delete token_file;
    
	//wxLogMessage(wxT("Wrote token data: %s"), token_string);
	return results;
}

/* Read all of the token data. */
wxJSONValue GetTokenData()
{
	wxJSONValue token_data;
    wxTextFile *token_file;
    
	/* Get pointer to token file, or fail. */
	if (! GetTokenFile(&token_file)) {
        DEBUG_LOG(_("SeruroConfig> (GetTokenData) could not run GetTokenFile."));
		return token_data;
	}

	/* Read entire file into string. */
	wxString token_string;
	token_file->Open();
	for (token_string = token_file->GetFirstLine(); !token_file->Eof();
		token_string += token_file->GetNextLine() + wxT("\n"));
	token_file->Close();
	delete token_file;

	//wxLogMessage(wxT("Read token data: %s"), token_string);

	/* Parse that string as a JSON value. */
	wxJSONReader token_reader;
	VLDDisable();
    int num_errors = token_reader.Parse(token_string, &token_data);
	VLDEnable();

	if (num_errors > 0) {
		ERROR_LOG(_("SeruroConfig> (GetTokenData) error parsing token data."));
	}
 
	/* Either way they'll get a wxJSONValue, might be empty. */
	return token_data;
}

/* Set the UserAppData location of the expected config file. */
SeruroConfig::SeruroConfig()
{
    this->InitConfig();
    
    this->config_valid = false;
    this->LoadConfig();
}

bool SeruroConfig::InitConfig()
{
    wxFileName config_path(GetAppDir(), _(SERURO_CONFIG_NAME));
    
    /* Add the config file name to the path. */
    if (! wxFileName::DirExists(GetAppDir())) {
        LOG(_("SeruroConfig> user data directory does not exist, creating (%s)."),
            GetAppDir());
        if (! wxFileName::Mkdir(GetAppDir())) {
            ERROR_LOG(_("SeruroConfig> cannot create data directory."));
            return false;
        }
    }
    
	VLDDisable();
    this->config_file = new wxTextFile(config_path.GetFullPath());
	VLDEnable();
    
    if (! this->config_file->Exists()) {
		/* A config will be written using the SeruroSetup wizard. */
        DEBUG_LOG(_("SeruroConfig> Config does not exist, creating."));
        if (! this->config_file->Create()) {
            return false;
        }
	}
    
    return true;
}

bool SeruroConfig::HasConfig(wxArrayString layers)
{
    /* Make a decision to run the SeruroSetup wizard. */
    if (! config_file || ! config_file->Exists() || ! config_valid) return false;
    if (! config.HasMember("servers") || config["servers"].GetMemberNames().size() == 0) return false;
    
    if (layers.size() == 0) return true;
    
    /* Optionally check each layer for it's existance. */
    wxJSONValue *layer = &config;
    for (size_t i = 0; i < layers.size(); i++) {
        if (! layer->HasMember(layers[i])) return false;
        layer = &(layer->operator[](layers[i]));
    }
    
    return true;
}

bool SeruroConfig::WriteConfig()
{
	bool results;

    if (! wxIsMainThread()) {
        /* Stop execution? */
        int i = 0;
    }
    
    if (HasConfig() && ! this->config_file->Exists()) {
        ERROR_LOG(_("SeruroConfig> Configuration file was removed."));
        this->InitConfig();
    }
    
	wxString config_string;
	wxJSONWriter writer(wxJSONWRITER_STYLED);
	writer.Write(this->config, config_string);

	config_file->Open();
	config_file->Clear();
	config_file->InsertLine(config_string, 0);
	results = config_file->Write();
	config_file->Close();
    
    if (results) {
        this->config_valid = true;
    }

	return results;
}

void SeruroConfig::LoadConfig()
{
    if (! this->config_file->Exists())
        /* Cannot load a non-existing config file. */
        return;
    
    /* Read entire file into string. */
    wxString configString;
    config_file->Open();
	for (configString = config_file->GetFirstLine(); !config_file->Eof();
		configString += config_file->GetNextLine() + wxT("\n"));
	config_file->Close();

    wxLogStatus(configString);
    
    /* Parse the file into JSON. */
    wxJSONReader configReader;
	wxJSONValue tmpconfig;
	VLDDisable();
    int numErrors = configReader.Parse(configString, &tmpconfig);
	VLDEnable();
    if (numErrors > 0) {
        //wxLogMessage(reader.GetErrors());
        ERROR_LOG(_("SeruroConfig> (LoadConfig) could not parse config file."));
        return;
    }

	this->config = tmpconfig;
    
	/* Config must have an array of "servers". */
	if (! config.HasMember("servers") || ! config["servers"].IsObject()) {
		ERROR_LOG(_("SeruroConfig> (LoadConfig) could not find a 'servers' object."));
		return;
	}

    /* Indicate that the config is valid. */
    this->config_valid = true;
}

bool SeruroConfig::SetOption(wxString option, wxString value, bool save_config)
{
    if (! this->HasConfig()) {
        return false;
    }
    
    config["options"][option] = value;
    if (save_config) {
        this->WriteConfig();
    }
    
    /* Create an option change event. */
    SeruroStateEvent event(STATE_TYPE_OPTION);
    event.SetValue("option_name", option);
    event.SetValue("option_value", value);
    wxGetApp().AddEvent(event);
    
    return true;
}

bool SeruroConfig::SetServerOption(wxString server_uuid, wxString option, wxString value, bool save_config)
{
    if (! this->HasConfig()) {
        return false;
    }
    
    if (! config["servers"].HasMember(server_uuid)) {
        return false;
    }
    
    config["servers"][server_uuid]["options"][option] = value;
    if (save_config) {
        this->WriteConfig();
    }
    
    return true;
}

wxString SeruroConfig::GetOption(wxString option)
{
    if (! this->HasConfig() || ! config.HasMember("options") || ! config["options"].HasMember(option)) {
        return wxEmptyString;
    }
    
    return config["options"][option].AsString();
}

wxString SeruroConfig::GetServerOption(wxString server_uuid, wxString option)
{
    if (! this->HasConfig() || ! config["servers"].HasMember(server_uuid) ||
        ! config["servers"][server_uuid]["options"].HasMember(option)) {
        return wxEmptyString;
    }
    
    return config["servers"][server_uuid]["options"][option].AsString();
}

/*****************************************************************************************/
/************** TOKEN MANIPULATOR/ACCESSORS **********************************************/
/*****************************************************************************************/

wxString SeruroConfig::GetToken(const wxString &server_uuid, const wxString &address)
{
	wxString token = wxEmptyString;

    wxCriticalSectionLocker locker(wxGetApp().seruro_critsection_token);
    
    LOG(_("SeruroConfig> (GetToken) requested token (%s) (%s)."), server_uuid, address);
	/* Get current token data, check if the requested token exists and return, else an empty string. */
	wxJSONValue token_data = GetTokenData();
	if (token_data.HasMember(server_uuid) && token_data[server_uuid].HasMember(address)) {
		token = token_data[server_uuid][address].AsString();
	}

	return token;
}

bool SeruroConfig::RemoveTokens(const wxString &server_uuid)
{
	bool results;

    wxCriticalSectionLocker locker(wxGetApp().seruro_critsection_token);
    
    wxJSONValue token_data = GetTokenData();
	if (! token_data.HasMember(server_uuid)) {
		return true;
	}

	token_data.Remove(server_uuid);
	results = WriteTokenData(token_data);

	return results;
}

bool SeruroConfig::RemoveToken(const wxString &server_uuid, const wxString &address)
{
	bool results;

    wxCriticalSectionLocker locker(wxGetApp().seruro_critsection_token);
    
    wxJSONValue token_data = GetTokenData();
	if (! token_data.HasMember(server_uuid) || ! token_data[server_uuid].HasMember(address)) {
		/* Unexpected, maybe there was never a token for this account, 
		 * and it was the only for the given server?
		 */
		return true;
	}

	token_data[server_uuid].Remove(address);
	results = WriteTokenData(token_data);

	return results;
}

bool SeruroConfig::WriteToken(const wxString &server_uuid, const wxString &address, const wxString &token)
{
	bool results;

    wxCriticalSectionLocker locker(wxGetApp().seruro_critsection_token);
    
    /* Get current token data, then add this server,address entry. */
	wxJSONValue token_data = GetTokenData();
	if (! token_data.HasMember(server_uuid)) {
		LOG(_("SeruroConfig> (WriteToken) token file does not contain server (%s)."), server_uuid);
		token_data[server_uuid] = wxJSONValue(wxJSONTYPE_OBJECT);
	}

	token_data[server_uuid][address] = token;
	results = WriteTokenData(token_data);
    DEBUG_LOG(_("SeruroConfig> (WriteToken) wrote token data."));

	return results;
}

bool SeruroConfig::SetActiveToken(const wxString &server_uuid, const wxString &account)
{
    if (! config["servers"].HasMember(server_uuid)) return false;

    DEBUG_LOG(_("SeruroConfig> (SetActiveToken) setting (%s) active token (%s)."),
        server_uuid, account);
    /* Allow a token to be written before a server name exists. */
	config["servers"][server_uuid]["active_token"] = account;
	this->WriteConfig();
    
	return true;
}

wxString SeruroConfig::GetActiveToken(const wxString &server_uuid)
{
    LOG(_("SeruroConfig> (GetActiveToken) requested active token (%s)."), server_uuid);
    
	if (! config["servers"].HasMember(server_uuid)) {
        return wxEmptyString;
    }
	if (! config["servers"][server_uuid].HasMember("active_token")) {
		/* Return the first account. */
		wxArrayString account_list = GetAddressList(server_uuid);
		/* There may not be any accounts? */
		if (account_list.size() == 0) {
            return wxEmptyString;
        }
        return GetToken(server_uuid, account_list[0]);
	}
	
    /* Lookup token from tokens file using the active token (account). */
	return GetToken(server_uuid, config["servers"][server_uuid]["active_token"].AsString());
}

/*****************************************************************************************/
/************** SERVER/ACCOUNT MANIPULATORS **********************************************/
/*****************************************************************************************/

bool SeruroConfig::RemoveServer(wxString server_uuid)
{
	if (! config["servers"].HasMember(server_uuid)) {
		return true;
	}

	/* Remove tokens for all addresses belonging to this server. */
	RemoveTokens(server_uuid);

	config["servers"].Remove(server_uuid);
	return this->WriteConfig();
}

bool SeruroConfig::AddServer(wxJSONValue server_info)
{
	wxJSONValue new_server;
	//wxJSONValue servers_list;

	/* Only require a name and host to identity a server. */
	if (!server_info.HasMember("uuid") || ! server_info.HasMember("name") || ! server_info.HasMember("host")) {
		ERROR_LOG(_("SeruroConfig> (AddServer) Cannot add a server without knowing the uuid, name, and host."));
		return false;
	}

	if (ServerExists(server_info)) {
        LOG(_("SeruroConfig> (AddServer) the server with uuid (%s) already exists."), server_info["uuid"].AsString());
		return false;
	}

	/* Todo: potentially add config template. */
	/* Add a servers list if none exists. */
	if (! config.HasMember("servers")) {
		config["servers"] = wxJSONValue( wxJSONTYPE_OBJECT );
	}

    /* Todo, perform name conflict resolution? */
    new_server["name"] = server_info["name"];
	new_server["host"] = server_info["host"];
	if (server_info.HasMember("port")) {
        new_server["port"] = server_info["port"];
    }

	//this->config["servers"][server_info["name"].AsString()] = new_server;
    config["servers"][server_info["uuid"].AsString()] = new_server;

	LOG(_("SeruroConfig> (AddServer) Adding server uuid (%s)."), server_info["name"].AsString());
	return this->WriteConfig();
}

bool SeruroConfig::RemoveAddress(wxString server_uuid, wxString address)
{
	/* Todo: should this remove the token too? yes! */
	if (! AddressExists(server_uuid, address)) {
		return true;
	}

	/* Get list, and reset. */
	wxArrayString address_list = GetAddressList(server_uuid);
	config["servers"][server_uuid]["addresses"] =  wxJSONValue(wxJSONTYPE_OBJECT);

	for (size_t i = 0; i < address_list.size(); i++) {
		if (address_list[i].compare(address) != 0) {
			config["servers"][server_uuid]["addresses"].Append(address_list[i]);
		}
	}

	/* Remove token. */
	RemoveToken(server_uuid, address);
	return this->WriteConfig();
}

bool SeruroConfig::AddAddress(const wxString &server_uuid, const wxString &address)
{
	if (! this->config["servers"].HasMember(server_uuid)) {
		ERROR_LOG(_("SeruroConfig> (AddAddress) Cannot find server uuid (%s)."), server_uuid);
		return false;
	}

	if (AddressExists(server_uuid, address)) {
		LOG(_("SeruroConfig> (AddAddress) Found duplicate address (%s) for uuid (%s)."),
				address, server_uuid);
		return false;
	}

    if (! config["servers"][server_uuid].HasMember("addresses")) {
        config["servers"][server_uuid]["addresses"] = wxJSONTYPE_ARRAY;
    }
	config["servers"][server_uuid]["addresses"].Append(address);
    
	LOG(_("SeruroConfig> (AddAddress) Adding address (%s) for uuid (%s)."), server_uuid, address);
    DEBUG_LOG(_("SeruroConfig> (AddAddress) There are (%d) addresses for uuid (%s)."),
        config["servers"][server_uuid]["addresses"].Size(), server_uuid);
    
	return this->WriteConfig();
}

bool SeruroConfig::AddContact(wxString server_uuid, wxString address, wxString first_name, wxString last_name)
{
    wxArrayString layers;
    
    layers.Add("servers");
    layers.Add(server_uuid);
    
    if (! HasConfig(layers)) {
        return false;
    }
    
    /* Replace the contact. */
    config["servers"][server_uuid]["contacts"][address] = wxJSONValue(wxJSONTYPE_OBJECT);
    config["servers"][server_uuid]["contacts"][address]["name"] = wxJSONValue(wxJSONTYPE_OBJECT);
    config["servers"][server_uuid]["contacts"][address]["name"].Append(first_name);
    config["servers"][server_uuid]["contacts"][address]["name"].Append(last_name);
    
    return this->WriteConfig();
}

bool SeruroConfig::RemoveContact(wxString server_uuid, wxString address)
{
    wxArrayString layers;
    
    layers.Add("servers");
    layers.Add(server_uuid);
    layers.Add("contacts");
    layers.Add(address);
    
    if (! HasConfig(layers)) {
        return false;
    }
    
    config["servers"][server_uuid]["contacts"].Remove(address);
    return this->WriteConfig();
}

bool SeruroConfig::HasContact(wxString server_uuid, wxString address)
{
    wxArrayString layers;
    
    layers.Add("servers");
    layers.Add(server_uuid);
    layers.Add("contacts");
    layers.Add(address);
    
    if (! HasConfig(layers)) {
        return false;
    }
    
    return true;
}

wxJSONValue SeruroConfig::GetContact(wxString server_uuid, wxString address)
{
    wxArrayString layers;
    wxJSONValue contact;
    
    layers.Add("servers");
    layers.Add(server_uuid);
    layers.Add("contacts");
    layers.Add(address);
    
    if (! HasConfig(layers)) {
        return contact;
    }
    
    contact = config["servers"][server_uuid]["contacts"][address];
    return contact;
}

/*****************************************************************************************/
/************** SERVER/ACCOUNT ACCESSORS *************************************************/
/*****************************************************************************************/

long SeruroConfig::GetPort(wxString server_uuid)
{
	/* If this server does not exist, return an error state (0). */
	if (! HasConfig() || ! config.HasMember("servers") || 
		! config["servers"].HasMember(server_uuid)) {
		return 0;
	}

	return GetPortFromServer(config["servers"][server_uuid]);
}

long SeruroConfig::GetPortFromServer(wxJSONValue server_info)
{
	long port = 0;
	wxString port_string;

	/* The server entry may or may not have an explicit port. */
	if (server_info.HasMember("port")) {
		port_string = server_info["port"].AsString();
	} else {
		port_string = _(SERURO_DEFAULT_PORT);
	}

	port_string.ToLong(&port, 10);
	DEBUG_LOG(_("SeruroConfig> (GetPortFromServer) port for uuid (%s) (%lu)."),
		server_info["uuid"].AsString(), port);

	return port;
}

bool SeruroConfig::ServerExists(wxJSONValue server_info)
{
	if (! server_info.HasMember("name") || ! server_info.HasMember("host") ||
		! server_info.HasMember("port")) {
		DEBUG_LOG(_("SeruroConfig> (ServerExists) must know name/host/port."));
		return true;
	}

	/* If no servers list exists, there are no duplicates. */
	if (! HasConfig() || ! config.HasMember("servers")) {
		return false;
	}

	/* The canonical name is the index into the config server list. */
	if (config["servers"].HasMember(server_info["name"].AsString())) {
		DEBUG_LOG(_("SeruroConfig> (ServerExists) duplicate server name (%s) exists."),
			server_info["name"].AsString());
		return true;
	}

	/* Check the host/port pairs for each existing server. */
	wxArrayString server_list = GetServerList();
	for (size_t i = 0; i < server_list.size(); i++) {
		if (server_info["host"].AsString().compare(
				config["servers"][server_list[i]]["host"].AsString()
			) == 0 && GetPort(server_list[i]) == GetPortFromServer(server_info)) {
			/* Server host names and ports are identical, this is a duplicate. */
			DEBUG_LOG(_("SeruroConfig> (ServerExists) duplicate server host/port (%s) exists."),
				server_list[i]);
			return true;
		}
	}

	return false;
}

bool SeruroConfig::AddressExists(wxString server_uuid, wxString address)
{
	wxArrayString address_list = GetAddressList(server_uuid);
	/* Check for a duplicate address for this server. */
	for (size_t i = 0; i < address_list.size(); i++) {
		if (address_list[i].compare(address) == 0) {
			return true;
		}
	}
	return false;
}

bool SeruroConfig::AddressExists(wxString address)
{
    wxArrayString server_list;
    wxArrayString account_list;
    
    /* Is a server configured with the given address. */
    server_list = theSeruroConfig::Get().GetServerList();
    for (size_t i = 0; i < server_list.size(); i++) {
        account_list = theSeruroConfig::Get().GetAddressList(server_list[i]);
        for (size_t j = 0; j < account_list.size(); j++) {
            if (account_list[j] == address) return true;
        }
    }
    return false;
}

wxJSONValue SeruroConfig::GetServers()
{
	wxJSONValue servers;

	/* If they don't have a config, return an empty list. */
	if (! HasConfig()) return servers;
	servers = config["servers"];

	return servers;
}

wxJSONValue SeruroConfig::GetServer(const wxString &server_uuid)
{
	wxJSONValue server_info;

	if (! HasConfig() || ! config["servers"].HasMember(server_uuid)) {
		return server_info;
	}

	server_info = config["servers"][server_uuid];
	/* Helper reference for those confused by wxJSONValue. */
	server_info["uuid"] = server_uuid;

	return server_info;
}

wxString SeruroConfig::GetServerString(wxString server_uuid)
{
	wxString server_name;
	wxString port;

	/* Expext a "name" and "host", port can be the default value. */ 
	wxJSONValue server_info = this->GetServer(server_uuid);
	if (SERURO_DISPLAY_SERVER_INFO) {
        port = (server_info.HasMember("port")) ? server_info["port"].AsString() : SERURO_DEFAULT_PORT;
		server_name = server_info["name"].AsString() + wxT(" (") + server_info["host"].AsString() + wxT(":");
		server_name = server_name + port;
		server_name = server_name + wxT(")");
	} else {
		server_name = server_info["name"].AsString();
	}

	return server_name;
}

wxArrayString SeruroConfig::GetServerList()
{
	wxArrayString servers;

	if (! HasConfig()) return servers;
	servers = config["servers"].GetMemberNames();

	return servers;
}

wxArrayString SeruroConfig::GetServerNames()
{
    wxArrayString server_names;
    wxArrayString server_uuids;
    
    if (! HasConfig()) return server_names;
    /* ["name"] is an attribute for each server. */
    server_uuids = config["servers"].GetMemberNames();
    for (size_t i = 0; i < server_uuids.size(); i++) {
        server_names.Add(config["servers"][server_uuids[i]]["name"].AsString());
    }
    
    return server_names;
}

wxString SeruroConfig::GetServerUUID(const wxString &server_name)
{
    wxArrayString server_uuids;
    
    if (! HasConfig()) return wxEmptyString;
    server_uuids = config["servers"].GetMemberNames();
    for (size_t i = 0; i < server_uuids.size(); i++) {
        /* Each server entry in the config is indexed by UUID, but contains a "name" attribute. */
        if (server_name.compare(config["servers"][server_uuids[i]]["name"].AsString()) == 0) {
            return server_uuids[i];
        }
    }
    /* No UUID found for given name, this is bad... */
    return wxEmptyString;
}

wxString SeruroConfig::GetServerName(const wxString &server_uuid)
{
    if (! HasConfig()) return wxEmptyString;
    return config["servers"][server_uuid]["name"].AsString();
}

wxArrayString SeruroConfig::GetAddressList(const wxString &server_uuid)
{
	wxArrayString addresses;
    DEBUG_LOG(_("SeruroConfig> (GetAddressList) requested for uuid (%s)."), server_uuid);

	if (! HasConfig() || ! config["servers"].HasMember(server_uuid)) {
        DEBUG_LOG(_("SeruroConfig> (GetAddressList) invalid uuid."));
		return addresses;
	}
    
    if (! config["servers"][server_uuid].HasMember("addresses")) {
        DEBUG_LOG(_("SeruroConfig> (GetAddressList) no addresses for uuid."));
    }

	for (int i = 0; i < config["servers"][server_uuid]["addresses"].Size(); i++) {
		addresses.Add(config["servers"][server_uuid]["addresses"][i].AsString());
	}

	/* If the server name does not exist in the configured list, return an empty array. */
	return addresses;
}

wxArrayString SeruroConfig::GetMemberArray(const wxString &member)
{
	/* Semi pointless check. */
	wxArrayString values;
	if (HasConfig()) {
		for (int i = 0; i < config[member].Size(); i++) {
			values.Add(config[member][i].AsString());
		}
	}
	return values;
}

/*****************************************************************************************/
/************** CERTIFICATE / FINGERPRINTING *********************************************/
/*****************************************************************************************/

bool SeruroConfig::AddFingerprint(wxString location, wxString server_uuid,
	wxString fingerprint, wxString address, identity_type_t cert_type)
{
	wxJSONValue address_list; 

	if (! HasConfig() || ! config["servers"].HasMember(server_uuid)) return false;

	/* If there is no address, then the fingerprint is placed in the location. */
	if (address.compare(wxEmptyString) == 0) {
		config["servers"][server_uuid][location] = fingerprint;
		goto finished;
	}

	/* If there is an address, the location is a value. */
	if (! config["servers"][server_uuid].HasMember(location)) {
		config["servers"][server_uuid][location] = wxJSONValue(wxJSONTYPE_OBJECT);
	}

	/* All address-type fingerprints are many-to-one so they must also be a value. */
	if (! config["servers"][server_uuid][location].HasMember(address)) {
		config["servers"][server_uuid][location][address] = wxJSONValue(wxJSONTYPE_OBJECT);
	}
    
    /* If there is a cert_type, then create a key->value relationship. */
    if (cert_type == ID_AUTHENTICATION) {
        config["servers"][server_uuid][location][address]["authentication"] = fingerprint;
	} else if (cert_type == ID_ENCIPHERMENT) {
		config["servers"][server_uuid][location][address]["encipherment"] = fingerprint;
    } else {
        address_list = config["servers"][server_uuid][location][address];
        for (int i = 0; i < address_list.Size(); i++) {
            /* Prevent duplicates. */
            if (address_list[i].AsString().compare(fingerprint) == 0) return false;
        }

        config["servers"][server_uuid][location][address].Append(fingerprint);
    }
        
finished:
	return WriteConfig();
}

bool SeruroConfig::RemoveFingerprints(wxString location, wxString server_uuid, wxString address, identity_type_t cert_type)
{
	if (! HasConfig() || ! config["servers"].HasMember(server_uuid)) return false;

	if (address.compare(wxEmptyString) == 0) {
		config["servers"][server_uuid].Remove(location);
		goto finished;
	}

	if (! config["servers"][server_uuid].HasMember(location)) return false;
	if (! config["servers"][server_uuid][location].HasMember(address)) return false;
    
    /* For identities the certificate has a type. */
	if (cert_type == ID_AUTHENTICATION) {
        config["servers"][server_uuid][location][address].Remove(_("authentication"));
    } else if (cert_type == ID_ENCIPHERMENT) {
		config["servers"][server_uuid][location][address].Remove(_("encipherment"));
	} else {
        config["servers"][server_uuid][location].Remove(address);
    }
        
finished:
	return WriteConfig();
}

bool SeruroConfig::SetCAFingerprint(wxString server_uuid, wxString fingerprint)
{
	wxString location = _("ca");
	return AddFingerprint(location, server_uuid, fingerprint);
}

bool SeruroConfig::RemoveCACertificate(wxString server_uuid, bool write_config)
{
	wxString location = _("ca");
	return RemoveFingerprints(location, server_uuid);
}

bool SeruroConfig::AddIdentity(wxString server_uuid, wxString address, identity_type_t cert_type, wxString fingerprint)
{
	wxString location = _("identities");
	return AddFingerprint(location, server_uuid, fingerprint, address, cert_type);
}

bool SeruroConfig::RemoveIdentity(wxString server_uuid, wxString address, identity_type_t cert_type, bool write_config)
{
	wxString location = _("identities");
	return RemoveFingerprints(location, server_uuid, address, cert_type);
}

bool SeruroConfig::AddCertificate(wxString server_uuid, wxString address, identity_type_t cert_type, wxString fingerprint)
{
	wxString location = _("contacts");
	return AddFingerprint(location, server_uuid, fingerprint, address, cert_type);
}

bool SeruroConfig::RemoveCertificate(wxString server_uuid, wxString address, identity_type_t cert_type, bool write_config)
{
	wxString location = _("contacts");
	return RemoveFingerprints(location, server_uuid, address, cert_type);
}

bool SeruroConfig::HaveCertificates(wxString server_uuid, wxString address, wxString fingerprint)
{
    wxJSONValue contact;
    wxArrayString layers;
    
    layers.Add("servers");
    layers.Add(server_uuid);
    layers.Add("contacts");
    layers.Add(address);
    
	if (! HasConfig(layers)) {
        return false;
    }
    
    contact = config["servers"][server_uuid]["contacts"][address];
    
    if (fingerprint == wxEmptyString && contact.HasMember("authentication") && contact.HasMember("encipherment")) {
        return true;
    }
    
    if (fingerprint.compare(contact["authentication"].AsString()) == 0) return true;
    if (fingerprint.compare(contact["encipherment"].AsString()) == 0) return true;
    
	return false;
}

bool SeruroConfig::HaveIdentity(wxString server_uuid, wxString address, wxString fingerprint)
{
    wxJSONValue identity;
    wxArrayString layers;
    
    layers.Add("servers");
    layers.Add(server_uuid);
    layers.Add("identities");
    layers.Add(address);
    
    if (! HasConfig(layers)) {
        return false;
    }
    
    identity = config["servers"][server_uuid]["identities"][address];
    
    /* No fingerprint explicitly searched, are there are least 1 identities installed? */
    if (fingerprint.compare(wxEmptyString) == 0 &&
        identity.HasMember("authentication") && identity.HasMember("encipherment")) {
        return true;
    }
        
    if (fingerprint.compare(identity["authentication"].AsString()) == 0) return true;
    if (fingerprint.compare(identity["encipherment"].AsString()) == 0) return true;
    
	return false;
}

bool SeruroConfig::HaveCA(wxString server_uuid)
{
	if (HasConfig() && config.HasMember("servers") &&
		config["servers"].HasMember(server_uuid) &&
		config["servers"][server_uuid].HasMember("ca")) {
		return true;
	}
	return false;
}

wxArrayString SeruroConfig::GetIdentityList(wxString server_uuid)
{
    wxArrayString identities;
    
    if (! HasConfig() || ! config["servers"].HasMember(server_uuid) ||
        ! config["servers"][server_uuid].HasMember("identities")) {
        return identities;
    }
    
    return config["servers"][server_uuid]["identities"].GetMemberNames();
}

wxArrayString SeruroConfig::GetContactsList(wxString server_uuid)
{
    wxArrayString certificates;
    
    if (! HasConfig() || ! config["servers"].HasMember(server_uuid) ||
        ! config["servers"][server_uuid].HasMember("contacts")) {
        return certificates;
    }
    
    return config["servers"][server_uuid]["contacts"].GetMemberNames();
}

wxArrayString SeruroConfig::GetCertificates(wxString server_uuid, wxString address)
{
	wxArrayString certificates;
	if (! this->HaveCertificates(server_uuid, address)) {
		DEBUG_LOG(_("SeruroConfig> (GetCertificates) cannot find certificates (%s) (%s)."),
			server_uuid, address);
		return certificates;
	}

	//for (int i = config["servers"][server_uuid]["certificates"][address].Size()-1; i >= 0; i--) {
	//	certificates.Add(config["servers"][server_uuid]["certificates"][address][i].AsString());
	//}
    certificates.Add(config["servers"][server_uuid]["contacts"][address]["authentication"].AsString());
    certificates.Add(config["servers"][server_uuid]["contacts"][address]["encipherment"].AsString());
    
	return certificates;
}

wxArrayString SeruroConfig::GetIdentity(wxString server_uuid, wxString address)
{
	wxArrayString identity;
	if (! this->HaveIdentity(server_uuid, address)) {
		DEBUG_LOG(_("SeruroConfig> (GetIdentity) cannot find certificates (%s) (%s)."),
			server_uuid, address);
		return identity;
	}

	//for (int i = config["servers"][server_uuid]["identities"][address].Size()-1; i >= 0; i--) {
	//	identity.Add(config["servers"][server_uuid]["identities"][address][i].AsString());
	//}
    identity.Add(config["servers"][server_uuid]["identities"][address]["authentication"].AsString());
    identity.Add(config["servers"][server_uuid]["identities"][address]["encipherment"].AsString());

	return identity;
}

wxString SeruroConfig::GetIdentity(wxString server_uuid, wxString address, identity_type_t id_type)
{
    /* Todo: revisit. */
    wxString id_name;

	id_name = (id_type == ID_AUTHENTICATION) ? _("authentication") : _("encipherment");
    return config["servers"][server_uuid]["identities"][address][id_name].AsString();
}

wxString SeruroConfig::GetCA(wxString server_uuid)
{
	wxString ca;
	if (! this->HaveCA(server_uuid)) {
		return ca;
	}

	ca = config["servers"][server_uuid]["ca"].AsString();
	return ca;
}



