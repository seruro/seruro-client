
#include "SeruroClient.h"
#include "SeruroConfig.h"

#include "wxJSON/wx/jsonreader.h"
#include "wxJSON/wx/jsonwriter.h"

#include <wx/stdpaths.h>

/* Set the UserAppData location of the expected config file. */
SeruroConfig::SeruroConfig()
{
    wxStandardPaths *paths = new wxStandardPaths();
	wxString configPath = paths->GetUserDataDir() + wxString(wxT("/")) + wxString(wxT(SERURO_CONFIG_NAME));

	VLDDisable();
    this->configFile = new wxTextFile(configPath);
	VLDEnable();
    delete paths;

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

bool SeruroConfig::AddServer(wxJSONValue server_info)
{
	wxJSONValue new_server;
	wxJSONValue servers_list;

	/* Only require a name and host to identity a server. */
	if (! server_info.HasMember("name") || ! server_info.HasMember("host")) {
		wxLogMessage(wxT("SeruroConfig> Cannot add a server without knowing the name and host."));
		return false;
	}

	/* Add a servers list if none exists. */
	if (! configData.HasMember("servers")) {
		configData["servers"] = servers_list;
	}

	/* Check for duplicate servers (based on name). */
	if (configData["servers"].HasMember(server_info["name"].AsString())) {
		wxLogMessage(_("SeruroConfig> Cannot add server (duplicate name exists)."));
		return false;
	}

	new_server["host"] = server_info["host"];
	if (server_info.HasMember("port")) new_server["port"] = server_info["port"];

	this->configData["servers"][server_info["name"].AsString()] = new_server;

	wxLogMessage(_("SeruroConfig> Adding server (%s)."), server_info["name"].AsString());
	return this->WriteConfig();
}

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
	long port;
	wxString port_string;
	/* The server entry may or may not have an explicit port. */
	if (server_info.HasMember("port")) {
		port_string = server_info.AsString();
	} else {
		port_string = _(SERURO_DEFAULT_PORT);
	}

	port_string.ToLong(&port, 10);
	return port;
}

bool SeruroConfig::AddAddress(const wxString &server_name, const wxString &address)
{
	wxJSONValue new_list;
	wxArrayString address_list;

	if (! this->configData["servers"].HasMember(server_name)) {
		wxLogMessage(_("SeruroConfig> (AddAddress) Cannot find server (%s)."), server_name);
		return false;
	}

	address_list = GetAddressList(server_name);

	/* Check for a duplicate address for this server. */
	for (size_t i = 0; i < address_list.size(); i++) {
		if (address_list[i].compare(address) == 0) {
			wxLogMessage(_("SeruroConfig> (AddAddress) Found duplicate address (%s) for (%s)."),
				address, server_name);
			return false;
		}
	}

	configData["servers"][server_name]["addresses"].Append(address);

	wxLogMessage(_("SeruroConfig> Adding address (%s) (%s)."), server_name, address);
	return this->WriteConfig();
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

bool SeruroConfig::HasConfig()
{
	/* Make a decision to run the SeruroSetup wizard. */
    return (this->configFile->Exists() && this->configValid);
}

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

    wxStandardPaths *paths = new wxStandardPaths();
	wxString token_path = paths->GetUserDataDir() + wxString(wxT("/")) + wxString(wxT(SERURO_TOKENS_FILE));

	VLDDisable();
    *token_file = new wxTextFile(token_path);
	VLDEnable();
    delete paths;

	/* If the token file does not exist, create and write a template. */
	if (! (*token_file)->Exists()) {
		results = (*token_file)->Create();
		if (! results) {
			/* Could not create the token file, bail. */
			wxLogMessage(wxT("Error while handling token data, this is serious."));
			delete *token_file;
			return false;
		}
		/* Write out the root. */
		//(*token_file)->Open();
		//(*token_file)->InsertLine(wxT("{\"tokens\": {}}"), 0);
		//results = (*token_file)->Write();
		//(*token_file)->Close();

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

wxString SeruroConfig::GetToken(const wxString &server, const wxString &address)
{
	wxString token;

	/* Get current token data, check if the requested token exists and return, else an empty string. */
	wxJSONValue token_data = GetTokenData();
	if (token_data.HasMember(server) && token_data[server].HasMember(address)) {
		token = token_data[server][address].AsString();	
	} else {
		token = wxT("");
	}

	return token;
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


