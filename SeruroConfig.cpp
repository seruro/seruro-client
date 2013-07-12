
#include "SeruroClient.h"
#include "SeruroConfig.h"

#include "wxJSON/wx/jsonreader.h"
#include "wxJSON/wx/jsonwriter.h"

#include <wx/stdpaths.h>

/* Token management methods.
 * Tokens are stored in (DataDir)/tokens, or the file name listed in Defs.h.
 * The format is as such:
 * { "tokens": { "server_name": { "address": "token", ...} ...} }
 * This means server names must be unique and a single server cannot have duplicat addresses.
 * This also means server names and addresses must be formatted correctly.
 */
bool GetTokenFile(wxTextFile** token_file);
bool WriteTokenData(wxJSONValue token_data);
wxJSONValue GetTokenData();

bool GetTokenFile(wxTextFile** token_file)
{
	/* We will read and write the data JIT. */
	bool results;

    wxStandardPaths paths = wxStandardPaths::Get();
	wxString token_path = paths.GetUserDataDir() + wxString(wxT("/")) + wxString(wxT(SERURO_TOKENS_FILE));

	VLDDisable();
    *token_file = new wxTextFile(token_path);
	VLDEnable();

	/* If the token file does not exist, create and write a template. */
	if (! (*token_file)->Exists()) {
		results = (*token_file)->Create();
		if (! results) {
			/* Could not create the token file, bail. */
			wxLogMessage(wxT("Error while handling token data, this is serious."));
			delete *token_file;
			return false;
		}

		wxLogMessage(wxT("Token file did not exist, successfully created."));
	}
	return true;
}

bool WriteTokenData(wxJSONValue token_data)
{
	bool results;

	/* Get pointer to token file, or fail. */
	wxTextFile *token_file;
	results = GetTokenFile(&token_file);
	if (! results) {
		return false;
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
	delete token_file;

	wxLogMessage(wxT("Wrote token data: %s"), token_string);

	return results;
}

/* Read all of the token data. */
wxJSONValue GetTokenData()
{
	wxJSONValue token_data;
	bool results;

	/* Get pointer to token file, or fail. */
	wxTextFile *token_file;
	results = GetTokenFile(&token_file);
	if (! results) {
		return token_data;
	}

	/* Read entire file into string. */
	wxString token_string;
	token_file->Open();
	for (token_string = token_file->GetFirstLine(); !token_file->Eof();
		token_string += token_file->GetNextLine() + wxT("\n"));
	token_file->Close();
	delete token_file;

	wxLogMessage(wxT("Read token data: %s"), token_string);

	/* Parse that string as a JSON value. */
	wxJSONReader token_reader;
	VLDDisable();
    int num_errors = token_reader.Parse(token_string, &token_data);
	VLDEnable();

	if (num_errors > 0) {
		wxLogMessage(wxT("Error parsing token data (could be a blank token file)."));
	}
 
	/* Either way they'll get a wxJSONValue, might be empty. */
	return token_data;
}

/* Set the UserAppData location of the expected config file. */
SeruroConfig::SeruroConfig()
{
    wxStandardPaths paths = wxStandardPaths::Get();
	wxString configPath = paths.GetUserDataDir() + wxString(wxT("/")) + wxString(wxT(SERURO_CONFIG_NAME));

	VLDDisable();
    this->configFile = new wxTextFile(configPath);
	VLDEnable();
    //delete paths;

    wxLogStatus(wxT("Config file: " + this->configFile->GetName()));
    if (! this->configFile->Exists()) {
		/* A config will be written using the SeruroSetup wizard. */
        wxLogMessage(wxT("Config does not exist"));
	}
    
    this->configValid = false;
    this->LoadConfig();

    /* May be a good idea to remove loadConfig, and put the code here. */

}

bool SeruroConfig::WriteConfig()
{
	bool results;

	wxString config_string;
	wxJSONWriter writer(wxJSONWRITER_STYLED);

	writer.Write(this->configData, config_string);

	configFile->Open();
	configFile->Clear();
	configFile->InsertLine(config_string, 0);
	results = configFile->Write();
	configFile->Close();

	return results;
}

void SeruroConfig::LoadConfig()
{
    if (! this->configFile->Exists())
        /* Cannot load a non-existing config file. */
        return;
    
    /* Read entire file into string. */
    wxString configString;
    configFile->Open();
	for (configString = configFile->GetFirstLine(); !configFile->Eof();
		configString += configFile->GetNextLine() + wxT("\n"));
	configFile->Close();

    wxLogStatus(configString);
    
    /* Parse the file into JSON. */
    wxJSONReader configReader;
	wxJSONValue tmpConfigData;
	VLDDisable();
    int numErrors = configReader.Parse(configString, &tmpConfigData);
	VLDEnable();
    if (numErrors > 0) {
        //wxLogMessage(reader.GetErrors());
        wxLogStatus(wxT("Error: could not parse config file."));
        return;
    }

	this->configData = tmpConfigData;
    
	/* Config must have an array of "servers". */
	if (! configData.HasMember("servers") || ! configData["servers"].IsObject()) {
		wxLogStatus(wxT("Config: could not find a 'servers' object."));
		return;
	}

    /* Indicate that the config is valid. */
    this->configValid = true;
}

/*****************************************************************************************/
/************** TOKEN MANIPULATOR/ACCESSORS **********************************************/
/*****************************************************************************************/

wxString SeruroConfig::GetToken(const wxString &server, const wxString &address)
{
	wxString token = wxEmptyString;

	/* Get current token data, check if the requested token exists and return, else an empty string. */
	wxJSONValue token_data = GetTokenData();
	if (token_data.HasMember(server) && token_data[server].HasMember(address)) {
		token = token_data[server][address].AsString();	
	}

	return token;
}

bool SeruroConfig::RemoveTokens(const wxString &server_name)
{
	bool results;

	wxJSONValue token_data = GetTokenData();
	if (! token_data.HasMember(server_name)) {
		return true;
	}

	token_data.Remove(server_name);
	results = WriteTokenData(token_data);

	return results;
}

bool SeruroConfig::RemoveToken(const wxString &server_name, const wxString &address)
{
	bool results;

	wxJSONValue token_data = GetTokenData();
	if (! token_data.HasMember(server_name) || ! token_data[server_name].HasMember(address)) {
		/* Unexpected, maybe there was never a token for this account, 
		 * and it was the only for the given server?
		 */
		return true;
	}

	token_data[server_name].Remove(address);
	results = WriteTokenData(token_data);

	return results;
}

bool SeruroConfig::WriteToken(const wxString &server, const wxString &address, const wxString &token)
{
	bool results;

	/* Get current token data, then add this server,address entry. */
	wxJSONValue token_data = GetTokenData();
	if (! token_data.HasMember(server)) {
		wxLogMessage(wxT("Token file does not contain server: %s"), server);
		wxJSONValue server_value;
		token_data[server] = server_value;
	}

	token_data[server][address] = token;
	results = WriteTokenData(token_data);

	return results;
}

bool SeruroConfig::SetActiveToken(const wxString &server_name, const wxString &account)
{
	if (! configData["servers"].HasMember(server_name)) return false;

	configData["servers"][server_name]["active_token"] = account;
	this->WriteConfig();
	return true;
}

wxString SeruroConfig::GetActiveToken(const wxString &server_name)
{
	wxString account; 

	if (! configData["servers"].HasMember(server_name)) return wxEmptyString;
	if (! configData["servers"][server_name].HasMember("active_token")) {
		/* Return the first account. */
		wxArrayString account_list = GetAddressList(server_name);
		/* There may not be any accounts? */
		if (account_list.size() == 0) return wxEmptyString;
		return account_list[0];
	}
	
	return configData["servers"][server_name]["active_token"].AsString();
}

/*****************************************************************************************/
/************** SERVER/ACCOUNT MANIPULATORS **********************************************/
/*****************************************************************************************/

bool SeruroConfig::RemoveServer(wxString server_name)
{
	if (! configData["servers"].HasMember(server_name)) {
		return true;
	}

	/* Remove tokens for all addresses belonging to this server. */
	//wxArrayString address_list = GetAddressList(server_name);
	//for (size_t i = 0; i < address_list.size(); i
	RemoveTokens(server_name);

	configData["servers"].Remove(server_name);
	return this->WriteConfig();
}

bool SeruroConfig::AddServer(wxJSONValue server_info)
{
	wxJSONValue new_server;
	//wxJSONValue servers_list;

	/* Only require a name and host to identity a server. */
	if (! server_info.HasMember("name") || ! server_info.HasMember("host")) {
		wxLogMessage(wxT("SeruroConfig> Cannot add a server without knowing the name and host."));
		return false;
	}

	if (ServerExists(server_info)) {
		return false;
	}

	/* Todo: potentially add config template. */
	/* Add a servers list if none exists. */
	if (! configData.HasMember("servers")) {
		configData["servers"] = wxJSONValue( wxJSONTYPE_OBJECT );
	}

	new_server["host"] = server_info["host"];
	if (server_info.HasMember("port")) new_server["port"] = server_info["port"];

	this->configData["servers"][server_info["name"].AsString()] = new_server;

	wxLogMessage(_("SeruroConfig> Adding server (%s)."), server_info["name"].AsString());
	return this->WriteConfig();
}

bool SeruroConfig::RemoveAddress(wxString server_name, wxString address)
{
	/* Todo: should this remove the token too? yes! */
	if (! AddressExists(server_name, address)) {
		return true;
	}

	/* Get list, and reset. */
	wxArrayString address_list = GetAddressList(server_name);
	configData["servers"][server_name]["addresses"] =  wxJSONValue(wxJSONTYPE_OBJECT);

	for (size_t i = 0; i < address_list.size(); i++) {
		if (address_list[i].compare(address) != 0) {
			configData["servers"][server_name]["addresses"].Append(address_list[i]);
		}
	}

	/* Remove token. */
	RemoveToken(server_name, address);
	return this->WriteConfig();
}

bool SeruroConfig::AddAddress(const wxString &server_name, const wxString &address)
{
	if (! this->configData["servers"].HasMember(server_name)) {
		wxLogMessage(_("SeruroConfig> (AddAddress) Cannot find server (%s)."), server_name);
		return false;
	}

	if (AddressExists(server_name, address)) {
		wxLogMessage(_("SeruroConfig> (AddAddress) Found duplicate address (%s) for (%s)."),
				address, server_name);
		return false;
	}

	configData["servers"][server_name]["addresses"].Append(address);

	wxLogMessage(_("SeruroConfig> Adding address (%s) (%s)."), server_name, address);
	return this->WriteConfig();
}

/*****************************************************************************************/
/************** SERVER/ACCOUNT ACCESSORS *************************************************/
/*****************************************************************************************/

long SeruroConfig::GetPort(wxString server_name)
{
	/* If this server does not exist, return an error state (0). */
	if (! HasConfig() || ! configData.HasMember("servers") || 
		! configData["servers"].HasMember(server_name)) {
		return 0;
	}

	return GetPortFromServer(configData["servers"][server_name]);
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
	wxLogMessage(_("SeruroConfig> (GetPortFromServer) port for (%s) is (%s)= (%d)."), 
		server_info["name"].AsString(), port_string, port);

	return port;
}

bool SeruroConfig::ServerExists(wxJSONValue server_info)
{
	if (! server_info.HasMember("name") || ! server_info.HasMember("host") ||
		! server_info.HasMember("port")) {
		wxLogMessage(_("SeruroConfig> (ServerExists) must know name/host/port."));
		return true;
	}

	/* If no servers list exists, there are no duplicates. */
	if (! HasConfig() || ! configData.HasMember("servers")) {
		return false;
	}

	/* The canonical name is the index into the config server list. */
	if (configData["servers"].HasMember(server_info["name"].AsString())) {
		wxLogMessage(_("SeruroConfig> (ServerExists) duplicate server name (%s) exists."),
			server_info["name"].AsString());
		return true;
	}

	/* Check the host/port pairs for each existing server. */
	wxArrayString server_list = GetServerList();
	for (size_t i = 0; i < server_list.size(); i++) {
		if (server_info["host"].AsString().compare(
				configData["servers"][server_list[i]]["host"].AsString()
			) == 0 && GetPort(server_list[i]) == GetPortFromServer(server_info)) {
			/* Server host names and ports are identical, this is a duplicate. */
			wxLogMessage(_("SeruroConfig> (ServerExists) duplicate server host/port (%s) exists."),
				server_list[i]);
			return true;
		}
	}

	return false;
}

bool SeruroConfig::AddressExists(wxString server_name, wxString address)
{
	wxArrayString address_list = GetAddressList(server_name);
	/* Check for a duplicate address for this server. */
	for (size_t i = 0; i < address_list.size(); i++) {
		if (address_list[i].compare(address) == 0) {
			return true;
		}
	}
	return false;
}

wxJSONValue SeruroConfig::GetServers()
{
	wxJSONValue servers;

	/* If they don't have a config, return an empty list. */
	if (! HasConfig()) {
		return servers;
	}

	servers = configData["servers"];

	return servers;
}

wxJSONValue SeruroConfig::GetServer(const wxString &server)
{
	wxJSONValue server_info;

	if (! HasConfig() || ! configData["servers"].HasMember(server)) {
		return server_info;
	}

	server_info = configData["servers"][server];
	/* Helper reference for those confused by wxJSONValue. */
	server_info["name"] = server;

	return server_info;
}

wxString SeruroConfig::GetServerString(wxString server)
{
	wxString server_name;
	wxString port;

	/* Expext a "name" and "host", port can be the default value. */ 
	wxJSONValue server_info = this->GetServer(server);
	if (SERURO_DISPLAY_SERVER_INFO) {
        port = (server_info.HasMember("port")) ? server_info["port"].AsString() : SERURO_DEFAULT_PORT;
		server_name = server + wxT(" (") + server_info["host"].AsString() + wxT(":");
		server_name = server_name + port;
		server_name = server_name + wxT(")");
	} else {
		server_name = server;
	}

	return server_name;
}

wxArrayString SeruroConfig::GetServerList()
{
	wxArrayString servers;

	if (! HasConfig())
		return servers;

	servers = configData["servers"].GetMemberNames();

	return servers;
}

wxArrayString SeruroConfig::GetAddressList(const wxString &server)
{
	wxArrayString addresses;

	if (! HasConfig() || ! configData["servers"].HasMember(server) ||
		! configData["servers"][server].HasMember("addresses")) {
		return addresses;
	}

	for (int i = 0; i < configData["servers"][server]["addresses"].Size(); i++) {
		addresses.Add(configData["servers"][server]["addresses"][i].AsString());
	}

	/* If the server name does not exist in the configured list, return an empty array. */
	return addresses;
}

wxArrayString SeruroConfig::GetMemberArray(const wxString &member)
{
	/* Semi pointless check. */
	wxArrayString values;
	if (HasConfig()) {
		for (int i = 0; i < configData[member].Size(); i++) {
			values.Add(configData[member][i].AsString());
		}
	}
	return values;
}

/*****************************************************************************************/
/************** CERTIFICATE / FINGERPRINTING *********************************************/
/*****************************************************************************************/

/* Certificate tracking methods.
 */
bool SeruroConfig::AddFingerprint(wxString location, wxString server_name, 
	wxString fingerprint, wxString address)
{
	wxJSONValue address_list; 

	if (! HasConfig() || ! configData["servers"].HasMember(server_name)) return false;

	/* If there is no address, then the fingerprint is placed in the location. */
	if (address.compare(wxEmptyString) == 0) {
		configData["servers"][server_name][location] = fingerprint;
		goto finished;
	}

	/* If there is an address, the location is a value. */
	if (! configData["servers"][server_name].HasMember(location)) {
		configData["servers"][server_name][location] = wxJSONValue(wxJSONTYPE_OBJECT);
	}

	/* All address-type fingerprints are many-to-one so they must also be a value. */
	if (! configData["servers"][server_name][location].HasMember(address)) {
		configData["servers"][server_name][location][address] = wxJSONValue(wxJSONTYPE_OBJECT);
	}

	address_list = configData["servers"][server_name][location][address];
	for (int i = 0; i < address_list.Size(); i++) {
		/* Prevent duplicates. */
		if (address_list[i].AsString().compare(fingerprint) == 0) return false;
	}

	configData["servers"][server_name][location][address].Append(fingerprint);

finished:
	return WriteConfig();
}

bool SeruroConfig::RemoveFingerprints(wxString location, wxString server_name, wxString address)
{
	if (! HasConfig() || ! configData["servers"].HasMember(server_name)) return false;

	if (address.compare(wxEmptyString) == 0) {
		configData["servers"][server_name].Remove(location);
		goto finished;
	}

	if (! configData["servers"][server_name].HasMember(location)) return false;
	if (! configData["servers"][server_name][location].HasMember(address)) return false;

	configData["servers"][server_name][location].Remove(address);

finished:
	return WriteConfig();
}

bool SeruroConfig::SetCAFingerprint(wxString server_name, wxString fingerprint)
{
	wxString location = _("ca");
	return AddFingerprint(location, server_name, fingerprint);
}

bool SeruroConfig::RemoveCACertificate(wxString server_name, bool write_config)
{
	wxString location = _("ca");
	return RemoveFingerprints(location, server_name);
}

bool SeruroConfig::AddIdentity(wxString server_name, wxString address, wxString fingerprint)
{
	wxString location = _("identities");
	return AddFingerprint(location, server_name, fingerprint, address);
}

bool SeruroConfig::RemoveIdentity(wxString server_name, wxString address, bool write_config)
{
	wxString location = _("identities");
	return RemoveFingerprints(location, server_name, address);
}

bool SeruroConfig::AddCertificate(wxString server_name, wxString address, wxString fingerprint)
{
	wxString location = _("certificates");
	return AddFingerprint(location, server_name, fingerprint, address);
}

bool SeruroConfig::RemoveCertificates(wxString server_name, wxString address, bool write_config)
{
	wxString location = _("certificates");
	return RemoveFingerprints(location, server_name, address);
}

bool SeruroConfig::HaveCertificates(wxString server_name, wxString address)
{
	if (HasConfig() && configData.HasMember("servers") &&
		configData["servers"].HasMember(server_name) &&
		configData["servers"][server_name].HasMember("certificates") &&
		configData["servers"][server_name]["certificates"].HasMember(address)) {
		return (configData["servers"][server_name]["certificates"][address].Size() > 0);
	}
	return false;
}

bool SeruroConfig::HaveIdentity(wxString server_name, wxString address)
{
	if (HasConfig() && configData.HasMember("servers") &&
		configData["servers"].HasMember(server_name) &&
		configData["servers"][server_name].HasMember("identities") &&
		configData["servers"][server_name]["identities"].HasMember(address)) {
		return (configData["servers"][server_name]["identities"][address].Size() > 0);
	}
	return false;
}

bool SeruroConfig::HaveCA(wxString server_name)
{
	if (HasConfig() && configData.HasMember("servers") &&
		configData["servers"].HasMember(server_name) &&
		configData["servers"][server_name].HasMember("ca")) {
		return true;
	}
	return false;
}

wxArrayString SeruroConfig::GetCertificates(wxString server_name, wxString address)
{
	wxArrayString certificates;
	if (! this->HaveCertificates(server_name, address)) {
		wxLogMessage(_("SeruroConfig> (GetCertificates) cannot find certificates (%s) (%s)."),
			server_name, address);
		return certificates;
	}

	for (int i = configData["servers"][server_name]["certificates"][address].Size()-1; i >= 0; i--) {
		certificates.Add(configData["servers"][server_name]["certificates"][address][i].AsString());
	}
	//certificates = configData["servers"][server_name]["certificates"][address].AddComment
	return certificates;
}

wxArrayString SeruroConfig::GetIdentity(wxString server_name, wxString address)
{
	wxArrayString identity;
	if (! this->HaveIdentity(server_name, address)) {
		wxLogMessage(_("SeruroConfig> (GetIdentity) cannot find certificates (%s) (%s)."),
			server_name, address);
		return identity;
	}

	for (int i = configData["servers"][server_name]["identities"][address].Size()-1; i >= 0; i--) {
		identity.Add(configData["servers"][server_name]["identities"][address][i].AsString());
	}
	return identity;
}

wxString SeruroConfig::GetCA(wxString server_name)
{
	wxString ca;
	if (! this->HaveCA(server_name)) {
		return ca;
	}

	ca = configData["servers"][server_name]["ca"].AsString();
	return ca;
}



