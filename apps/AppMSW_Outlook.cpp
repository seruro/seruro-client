
#if defined(__WXMSW__)

#include "AppMSW_Outlook.h"
#include "../SeruroClient.h"
#include "../SeruroConfig.h"

#include "../crypto/SeruroCrypto.h"
#include "../api/Utils.h"

//#include "../crypto/SeruroCryptoMSW.h"

#include <wx/msw/registry.h>
#include <wx/base64.h>

//#include "../include/MAPIX.h"
//#include "../include/MAPIUtil.h"

/* Add all keys which may contain Outlook. */
#define KEY_OFFICE_2010_PLUS L"\\UserData\\S-1-5-18\\Products\\00004109110000000000000000F01FEC"

#define HKCU_SUBSYSTEM_BASE L"Software\\Microsoft\\Windows NT\\CurrentVersion"
#define HKCU_SUBSYSTEM L"Windows Messaging Subsystem\\Profiles\\Outlook"

//const GUID CDECL GUID_Dilkie = {  0x53bc2ec0, 0xd953, 0x11cd, 
//   {0x97, 0x52, 0x00, 0xaa, 0x00, 0x4a, 0xe4, 0x0e}  };
#define DILKIE_GUID "\xc0\x2e\xbc\x53\x53\xd9\xcd\x11\x97\x52\x00\xaa\x00\x4a\xe4\x0e"
#define PR_SECURITY_PROFILES PROP_TAG(PT_MV_BINARY, 0x355)

enum security_properties {
	PR_CERT_PROP_VERSION     = 0x0001, /* Reserved = 1 */
	PR_CERT_MESSAGE_ENCODING = 0x0006, /* Type of encoding (S/MIME = 1) */
	PR_CERT_DEFAULTS         = 0x0020, /* Bitmask 0x1 (default), 0x2 (default), 0x3 (send) */
	PR_CERT_DISPLAY_NAME_A   = 0x000B, /* Display name. */
	PR_CERT_KEYEX_SHA1_HASH  = 0x0022, /* hash of encryption certificate. */
	PR_CERT_SIGN_SHA1_HASH   = 0x0009, /* hash of identity certificate. */
	PR_CERT_ASYMETRIC_CAPS   = 0x0002  /* ASN1-encoded S/MIME capabilities. */
};

#define ASYMETRIC_CAPS_BLOB "\
30819A300B060960864801650304012A300B0609\
608648016503040116300A06082A864886F70D03\
07300B0609608648016503040102300E06082A86\
4886F70D030202020080300706052B0E03020730\
0D06082A864886F70D0302020140300D06082A86\
4886F70D0302020128300706052B0E03021A300B\
0609608648016503040203300B06096086480165\
03040202300B0609608648016503040201"

wxDECLARE_APP(SeruroClient);

bool SafeANSIString(void *data, size_t length, wxString &safe_result)
{
	for (size_t i = 0; i < length; i++) {
		if ((unsigned char) ((byte *) data)[i] < 128) {
			safe_result.Append(((byte *) data)[i], 1);
		} else {
			safe_result.Clear();
			return false;
		}
	}
	return true;
}

bool AppMSW_Outlook::IsInstalled()
{
	if (! GetInfo()) return false;

	return this->is_installed;
}

wxString AppMSW_Outlook::GetVersion()
{
	if (! GetInfo()) return _("N/A");

	if (this->info.HasMember("version")) {
		return info["version"].AsString();
	}

	return _("0");
}

bool MAPIProfileExists(LPPROFADMIN &profile_admin)
{
	HRESULT result;
	LPMAPITABLE mapi_table;
	LPSRowSet profile_rows;
	SPropValue property;

	wxString profile_name;

	/* Access the profile table (information about all available profiles).*/
	result = profile_admin->GetProfileTable(0, &mapi_table);
	if (FAILED(result)) {
		DEBUG_LOG(_("AppMSW_Outlook> (MAPIProfileExists) cannot get profile table (%0x)."), result);
		return false;
	}

	/* Fetch all rows from the table. */
	profile_rows = NULL;
	result = ::HrQueryAllRows(mapi_table, NULL, NULL, NULL, 0, &profile_rows);
	if (FAILED(result) || profile_rows == NULL || profile_rows->cRows == 0) {
		DEBUG_LOG(_("AppMSW_Outlook> (MAPIProfileExists) cannot query profile table (%0x)."), result);
		mapi_table->Release();
		return false;
	}

	/* Search each profile. */
	for (unsigned long i = 0; i < profile_rows->cRows; ++i) {
		/* Search each property within the profile. */
		for (unsigned long j = 0; j < profile_rows->aRow[i].cValues; ++j) {

			property = profile_rows->aRow[i].lpProps[j];
			if (property.ulPropTag == PR_DISPLAY_NAME_A) {
				/* PR_DISPLAY_NAME, PR_DISPLAY_NAME_A, PR_DISPLAY_NAME_W are the same constant. */
				profile_name = _(property.Value.lpszA);
				if (profile_name == "Outlook") break;
			}
		}
	}

	/* Clean up. */
	::FreeProws(profile_rows);
	mapi_table->Release();

	if (profile_name != "Outlook") {
		/* Could not find an Outlook profile. */
		DEBUG_LOG(_("AppMSW_Outlook> (MAPIProfileExists) could not find profile table."));
		return false;
	}

	return true;
}

bool SetSecurityProperties(wxString &service_uid, wxJSONValue properties)
{
	HRESULT result;
	LPPROFADMIN profile_admin;

	/* Create a profile administration object. */
	result = ::MAPIInitialize(NULL);
	if (FAILED(result)) {
		DEBUG_LOG(_("AppMSW_Outlook> (SetSecurityProperties) cannot initialize MAPI (%0x)."), result);
		return false;
	}

	result = ::MAPIAdminProfiles(0, &profile_admin);
	if (FAILED(result)) {
		DEBUG_LOG(_("AppMSW_Outlook> (SetSecurityProperties) cannot open profile (%0x)."), result);
		return false;
	}

	if (! MAPIProfileExists(profile_admin)) {
		DEBUG_LOG(_("AppMSW_Outlook> (SetSecurityProperties) no MAPI profile exists (%0x)."), result);
		return false;
	}

	LPSERVICEADMIN service_admin;
	//LPMAPITABLE service_table;
	//LPSRowSet service_rows;

	/* The profile name is in ANSI? */
	result = profile_admin->AdminServices((LPTSTR) "Outlook", NULL, NULL, 0, &service_admin);
	if (FAILED(result)) {
		DEBUG_LOG(_("AppMSW_Outlook> (SetSecurityProperties) cannot open Outlook service (%0x)."), result);

		profile_admin->Release();
		return false;
	}
	//result = service_admin->GetMsgServiceTable(0, &service_table);

	//SRestriction service_restriction;
	SPropValue service_property;
	wxMemoryBuffer service_uid_buffer;

	//service_restriction.rt = RES_CONTENT;
	//service_restriction.res.resContent.ulFuzzyLevel = FL_FULLSTRING;
	//service_restriction.res.resContent.ulPropTag = PR_SERVICE_UID;
	//service_restriction.res.resContent.lpProp = &service_property;

	service_property.ulPropTag = PR_SERVICE_UID;
	service_uid_buffer = wxBase64Decode(service_uid);

	service_property.Value.bin.cb = service_uid_buffer.GetDataLen();
	service_property.Value.bin.lpb = (byte *) service_uid_buffer.GetData();
	//service_property.Value.bin.lpb = (byte *) malloc(service_uid_buffer.GetDataLen());
	//memcpy(service_property.Value.bin.lpb, service, 16);

	//service_property.Value.lpszA = "MSEMS";

	//result = ::HrQueryAllRows(service_table, NULL, &service_restriction, NULL, 1, &service_rows);
	//if (FAILED(result) || service_rows == NULL || service_rows->cRows == 0) {
	//	DEBUG_LOG(_("AppMSW_Outlook> (GetAccountList) cannot query service table (%0x)."), result);

	//	service_table->Release();
	//	service_admin->Release();
	//	profile_admin->Release();
	//	return false;
	//}

	LPPROVIDERADMIN provider_admin;
	LPPROFSECT profile_section;

	result = service_admin->AdminProviders((LPMAPIUID) service_property.Value.bin.lpb, 0, &provider_admin);
	if (FAILED(result)) {
		DEBUG_LOG(_("AppMSW_Outlook> (SetSecurityProperties) cannot open provider (%0x)."), result);

		service_admin->Release();
		profile_admin->Release();
		return false;
	}

	result = provider_admin->OpenProfileSection((LPMAPIUID) DILKIE_GUID, 
		NULL, MAPI_MODIFY, &profile_section);
	if (FAILED(result)) {
		DEBUG_LOG(_("AppMSW_Outlook> (SetSecurityProperties) cannot open section (%0x)."), result);

		provider_admin->Release();
		service_admin->Release();
		profile_admin->Release();
		return false;
	}

	/* Not sure how this is calculated. */
	wxMemoryBuffer asymetric_caps = AsBinary(ASYMETRIC_CAPS_BLOB);
	wxMemoryBuffer cert_buffer;

	unsigned short profile_offset;
	unsigned short entity_name_size = properties["entity_name"].AsString().size();
	unsigned long profile_size = (
		  (4+ 4) /* PR_CERT_PROP_VERSION */
		+ (4+ 4) /* PR_CERT_MESSAGE_ENCODING */
		+ (4+ 4) /* PR_CERT_DEFAULTS */
		+ (4+ entity_name_size+1) /* PR_CERT_DISPLAY_NAME_A */
		+ (4+ 20) /* PR_CERT_KEYEX_SHA1_HASH */
		+ (4+ 20) /* PR_CERT_SIGN_SHA1_HASH */
		+ (4+ asymetric_caps.GetDataLen()) /* PR_CERT_ASYMETRIC_CAPS */
	);

	unsigned short tag, length;
	unsigned long type_data;
	byte *profile_data;

	profile_data = (byte *) malloc(profile_size);
	profile_offset = 0;

	tag = PR_CERT_PROP_VERSION, length = 4+ 4, type_data = 0x1;
	memcpy(profile_data, &tag, 2);
	memcpy(profile_data+2, &length, 2);
	memcpy(profile_data+4, &type_data, 4);

	profile_offset = 8;
	tag = PR_CERT_MESSAGE_ENCODING, length = 4+ 4, type_data = 0x1;
	memcpy(profile_data+profile_offset, &tag, 2);
	memcpy(profile_data+profile_offset+2, &length, 2);
	memcpy(profile_data+profile_offset+4, &type_data, 4);

	profile_offset += 8;
	tag = PR_CERT_DEFAULTS, length = 4+ 4, type_data = 0x1 | 0x2 | 0x4;
	memcpy(profile_data+profile_offset, &tag, 2);
	memcpy(profile_data+profile_offset+2, &length, 2);
	memcpy(profile_data+profile_offset+4, &type_data, 4);

	profile_offset += 8;
	tag = PR_CERT_DISPLAY_NAME_A, length = 4+ entity_name_size+1;
	memcpy(profile_data+profile_offset, &tag, 2);
	memcpy(profile_data+profile_offset+2, &length, 2);
	memcpy(profile_data+profile_offset+4, AsChar(properties["entity_name"].AsString()), length-1);
	memset(profile_data+profile_offset+4+entity_name_size, 0, 1); 

	profile_offset += entity_name_size+1 + 4;
	tag = PR_CERT_KEYEX_SHA1_HASH, length = 4+ 20;
	memcpy(profile_data+profile_offset, &tag, 2);
	memcpy(profile_data+profile_offset+2, &length, 2);
	cert_buffer.Clear();
	cert_buffer = wxBase64Decode(properties["encipherment"].AsString());
	memcpy(profile_data+profile_offset+4, AsChar(AsHex(cert_buffer)), 20);

	profile_offset += 24;
	tag = PR_CERT_SIGN_SHA1_HASH, length = 4+ 20;
	memcpy(profile_data+profile_offset, &tag, 2);
	memcpy(profile_data+profile_offset+2, &length, 2);
	cert_buffer.Clear();
	cert_buffer = wxBase64Decode(properties["authentication"].AsString());
	memcpy(profile_data+profile_offset+4, AsChar(AsHex(cert_buffer)), 20);

	profile_offset += 24;
	tag = PR_CERT_ASYMETRIC_CAPS, length = 4+ asymetric_caps.GetDataLen();
	memcpy(profile_data+profile_offset, &tag, 2);
	memcpy(profile_data+profile_offset+2, &length, 2);
	memcpy(profile_data+profile_offset+4, asymetric_caps.GetData(), asymetric_caps.GetDataLen());

	SBinary security_profile;
	SPropValue property_values;

	/* Only support one MAPI security profile at a time. */
	property_values.ulPropTag = PR_SECURITY_PROFILES;
	property_values.Value.MVbin.cValues = 1;
	property_values.Value.MVbin.lpbin = &security_profile;
	property_values.Value.MVbin.lpbin[0].cb = profile_size;
	property_values.Value.MVbin.lpbin[0].lpb = profile_data;

	result = profile_section->SetProps(1, &property_values, NULL);

	delete profile_data;
	profile_section->Release();
	provider_admin->Release();
	service_admin->Release();
	profile_admin->Release();

	if (FAILED(result)) {
		DEBUG_LOG(_("AppMSW_Outlook> (SetSecurityProperties) cannot set properties (%0x)."), result);
		return false;
	}

	return true;
}

wxJSONValue GetSecurityProperties(SBinary *entry)
{
	wxJSONValue properties;
	wxString display_name;
	wxMemoryBuffer certificate_hash;
	wxString test;

	unsigned short entry_offset = 0;

	/* Parse TAG (2 bytes)/LENGTH (2 bytes)/DATA (LENGTH bytes) */
	unsigned short tag;
	unsigned short length;
	byte* data;

	/* Read through the entry, extracting each touple. */
	while (entry_offset < entry->cb) {
		tag = *(entry->lpb + entry_offset);
		length = *(entry->lpb + entry_offset + 2);
		//length = length / 2;

		if ((entry_offset + length) > entry->cb) {
			/* There is an overrun error in the ASN1 blob? */
			break;
		}

		if (tag == 0 || length == 0) {
			/* Problem reading data. */
			break;
		}

		data = entry->lpb + entry_offset + 4;
		entry_offset += length;

		if (tag == PR_CERT_MESSAGE_ENCODING && *((unsigned long *) data) != 1) {
			/* This is not an S/MIME profile, skip. */
			break;
		}

		if (tag == PR_CERT_DISPLAY_NAME_A) {
			if (! SafeANSIString(data, length, display_name)) {
				/* The ANSI data type contained non-ASCII characters? */
				break;
			}
			properties["entry_name"] = display_name;
		}

		if (tag == PR_CERT_DEFAULTS) {
			properties["default_smime"] = ((unsigned long) *(data) & 0x1);
			properties["default_certificate"] = ((unsigned long) *(data) & 0x2);
		}

		if (tag == PR_CERT_SIGN_SHA1_HASH) {
			certificate_hash.Clear();
			certificate_hash.AppendData(data, length-4);
			properties["authentication"] = wxBase64Encode(certificate_hash);
			test = properties["authentication"].AsString();
		}

		if (tag == PR_CERT_KEYEX_SHA1_HASH) {
			certificate_hash.Clear();
			certificate_hash.AppendData(data, length-4);
			properties["encipherment"] = wxBase64Encode(certificate_hash);
			test = properties["encipherment"].AsString();
		}
	}

	return properties;
}

void AppMSW_Outlook::FindAccountProperties(LPSERVICEADMIN &service_admin, SPropValue &uid, wxString &account_name)
{
	HRESULT result;
	LPPROVIDERADMIN provider_admin;
	LPPROFSECT profile_section;
	LPSPropValue property_values;
	unsigned long property_count;

	/* Set a reference to the account name using itself as an index. */
	this->info["accounts"][account_name]["name"] = account_name;

	result = service_admin->AdminProviders((LPMAPIUID) uid.Value.bin.lpb, 0, &provider_admin);
	if (FAILED(result)) {
		DEBUG_LOG(_("AppMSW_Outlook> (FindAccountProperties) cannot get provider (%s) (%0x)."), 
			account_name, result);
		return;
	}

	result = provider_admin->OpenProfileSection((LPMAPIUID) DILKIE_GUID, 
		NULL, MAPI_MODIFY, &profile_section);
	if (FAILED(result)) {
		DEBUG_LOG(_("AppMSW_Outlook> (FindAccountProperties) cannot open section (%s) (%0x)."),
			account_name, result);

		provider_admin->Release();
		return;
	}

	SizedSPropTagArray(1, property_columns) = {1, PR_SECURITY_PROFILES};

	result = profile_section->GetProps((LPSPropTagArray) &property_columns, 0, &property_count, &property_values);
	if (FAILED(result) || property_count == 0) {
		DEBUG_LOG(_("AppMSW_Outlook> (FindAccountProperties) cannot get DILKIE properties (%s) (%0x)."),
			account_name, result);

		profile_section->Release();
		provider_admin->Release();
		return;
	}

	/* If there are no sercurity profiles the count is still 1, but the property tag is not set correctly. */
	if (property_values[0].ulPropTag != PR_SECURITY_PROFILES) {

		::MAPIFreeBuffer(property_values);
		profile_section->Release();
		provider_admin->Release();
		return;
	}

	wxMemoryBuffer uid_buffer;
	uid_buffer.AppendData((void *) uid.Value.bin.lpb, uid.Value.bin.cb);

	/* The profile has a security "profile" property. Store it and parse later. */
	this->info["accounts"][account_name]["service_uid"] = wxBase64Encode(uid_buffer);

	/* The MVbin = # (4 bytes) of entires PT_BINARY: {size (2 bytes), data} */
	/* SProperty (data): http://support.microsoft.com/kb/312900 */

	/* The number of profile entries. */
	unsigned int profile_entries = property_values[0].Value.MVbin.cValues;
	/* The offset within the value to parse the "next" entry. */
	unsigned int entry_offset = 0;
	
	SBinary *profile_entry;
	wxJSONValue entry_properties;

	for (unsigned int i = 0; i < profile_entries; ++i) {
		//profile_entry = (property_values[0].Value.MVbin.lpbin)[i];
		profile_entry = (SBinary *) ((property_values[0].Value.MVbin.lpbin) + i);
		entry_offset += profile_entry->cb + 2;

		//entry_size = (unsigned short) *(property_values[0].Value.MVbin.lpbin + entry_offset);
		entry_properties = GetSecurityProperties(profile_entry);
		this->info["accounts"][account_name]["profiles"].Append(entry_properties);
	}

	::MAPIFreeBuffer(property_values);
	profile_section->Release();
	provider_admin->Release();
}

wxArrayString AppMSW_Outlook::GetAccountList()
{
	wxArrayString accounts;

	/* Reset previous account info. */
	this->info["accounts"] = wxJSONValue(wxJSONTYPE_OBJECT);

	/* Fail if no application info is detected. */
	if (! this->GetInfo()) {
		return accounts;
	}

	HRESULT result;
	LPPROFADMIN profile_admin;

	/* Create a profile administration object. */
	result = ::MAPIInitialize(NULL);
	if (FAILED(result)) {
		DEBUG_LOG(_("AppMSW_Outlook> (GetAccountList) cannot initialize MAPI (%0x)."), result);
		return accounts;
	}

	result = ::MAPIAdminProfiles(0, &profile_admin);
	if (FAILED(result)) {
		DEBUG_LOG(_("AppMSW_Outlook> (GetAccountList) cannot open profile (%0x)."), result);
		return accounts;
	}

	if (! MAPIProfileExists(profile_admin)) {
		DEBUG_LOG(_("AppMSW_Outlook> (GetAccountList) no MAPI profile exists (%0x)."), result);
		return accounts;
	}

	LPSERVICEADMIN service_admin;
	LPMAPITABLE service_table;
	LPSRowSet service_rows;

	/* The profile name is in ANSI? */
	result = profile_admin->AdminServices((LPTSTR) "Outlook", NULL, NULL, 0, &service_admin);
	if (FAILED(result)) {
		DEBUG_LOG(_("AppMSW_Outlook> (GetAccountList) cannot open Outlook service (%0x)."), result);

		profile_admin->Release();
		return accounts;
	}

	result = service_admin->GetMsgServiceTable(0, &service_table);
	if (FAILED(result)) {
		DEBUG_LOG(_("AppMSW_Outlook> (GetAccountList) cannot get service table (%0x)."), result);

		service_admin->Release();
		profile_admin->Release();
		return accounts;
	}

	SRestriction service_restriction;
	SPropValue service_property;

	service_restriction.rt = RES_CONTENT;
	service_restriction.res.resContent.ulFuzzyLevel = FL_FULLSTRING;
	service_restriction.res.resContent.ulPropTag = PR_SERVICE_NAME;
	service_restriction.res.resContent.lpProp = &service_property;

	service_property.ulPropTag = PR_SERVICE_NAME;
	service_property.Value.lpszW = L"INTERSTOR";
	//service_property.Value.lpszA = "MSEMS";

	/* Fetch the following columns from the services table. */
	enum {display_name_index, service_name_index, service_uid_index};
	SizedSPropTagArray(3, service_columns) = {3, 
		PR_DISPLAY_NAME,
		PR_SERVICE_NAME,
		PR_SERVICE_UID
	};

	result = ::HrQueryAllRows(service_table, (LPSPropTagArray) &service_columns,
		&service_restriction, NULL, 0, &service_rows);
	if (FAILED(result) || service_rows == NULL || service_rows->cRows == 0) {
		DEBUG_LOG(_("AppMSW_Outlook> (GetAccountList) cannot query service table (%0x)."), result);

		service_table->Release();
		service_admin->Release();
		profile_admin->Release();
		return accounts;
	}

	wxString account_name;
	for (unsigned long i = 0; i < service_rows->cRows; ++i) {
		/* Reset the account name, just incase a row does not contain a display name. */
		account_name = wxEmptyString;

		if (service_rows->aRow[i].lpProps[display_name_index].ulPropTag == PR_DISPLAY_NAME_W) {
			account_name = _(service_rows->aRow[i].lpProps[display_name_index].Value.lpszW);
		} else if (service_rows->aRow[i].lpProps[display_name_index].ulPropTag == PR_DISPLAY_NAME_A) {
			account_name = _(service_rows->aRow[i].lpProps[display_name_index].Value.lpszA);
		}

		if (account_name != wxEmptyString) {
			/* Found a row with a valid display name. */
			this->FindAccountProperties(service_admin, 
				service_rows->aRow[i].lpProps[service_uid_index], 
				account_name);
			accounts.Add(account_name);
		}
	}

	::FreeProws(service_rows);
	service_table->Release();
	service_admin->Release();

	/* Finally, release the admin profile. */
	profile_admin->Release();

	return accounts;
}

bool AppMSW_Outlook::AssignIdentity(wxString server_uuid, wxString address)
{
	wxString auth_skid, enc_skid;
	wxString service_uid;
	wxJSONValue properties;

	SeruroCrypto crypto;

	service_uid = this->info["accounts"][address]["service_uid"].AsString();
	if (service_uid == wxEmptyString) {
		/* The service uid was not probed during an account enumeration? */
		return false;
	}

	/* Get both certificate SKIDs from config. */
	auth_skid = wxGetApp().config->GetIdentity(server_uuid, address, ID_AUTHENTICATION);
	enc_skid  = wxGetApp().config->GetIdentity(server_uuid, address, ID_ENCIPHERMENT);

	/* For each, GetIdentityHashBySKID */
	properties["authentication"] = crypto.GetIdentityHashBySKID(auth_skid);
	properties["encipherment"]  = crypto.GetIdentityHashBySKID(enc_skid);

	properties["entity_name"] = wxString::Format(_("Seruro Identity (%s) for %s"), 
		theSeruroConfig::Get().GetServerName(server_uuid),	address);

	if (! SetSecurityProperties(service_uid, properties)) {
		/* Could not assign an entity for the service_uid, does the account exist? */
		return false;
	}

	return false;
}

account_status_t AppMSW_Outlook::IdentityStatus(wxString address, wxString &server_uuid)
{
	wxMemoryBuffer search_hash;
	wxJSONValue profile;
	/* Since profiles are not bound to accounts:
	 *   an alternate means at least one profile contains both certificates. */
	bool alternated_assigned = false;

	/* The account name is the address. */
	server_uuid.Clear();

	if (! this->info["accounts"].HasMember(address)) {
		/* There was no account found for the given address. */
		return APP_UNASSIGNED;
	}

	if (! this->info["accounts"][address].HasMember("profiles")) {
		/* There are no profiles, profiles are not paired to accounts, there are just 0. */
		return APP_UNASSIGNED;
	}

	for (int i = 0; i < this->info["accounts"][address]["profiles"].Size(); ++i) {
		profile = this->info["accounts"][address]["profiles"][i];
		if (! profile.HasMember("entry_name")) {
			/* The profile is missing an entry name. This is ODD. */
			continue;
		}

		if (! profile.HasMember("authentication") || ! profile.HasMember("encipherment")) {
			/* Profile is missing a certificate. */
			continue;
		}

		search_hash =  wxBase64Decode(profile["authentication"].AsString());
		if (! IsHashInstalledAndSet(address, search_hash)) {
			DEBUG_LOG(_("AppMSW_Outlook> (IsIdentityInstalled) Authentication hash does not exist for (%s)."), address);
			alternated_assigned = true;
			continue;
		}

		search_hash =  wxBase64Decode(profile["encipherment"].AsString());
		if (! IsHashInstalledAndSet(address, search_hash)) {
			DEBUG_LOG(_("AppMSW_Outlook> (IsIdentityInstalled) Encipherment hash does not exist for (%s)."), address);
			alternated_assigned = true;
			continue;
		}

		SeruroCrypto crypto;
		wxString fingerprint;

		fingerprint = crypto.GetIdentitySKIDByHash(profile["authentication"].AsString());

		/* If both certificates in this profile match, the identity is assigned. */
		server_uuid.Append(UUIDFromFingerprint(fingerprint));
		return APP_ASSIGNED;
	}

	if (alternated_assigned) {
		return APP_ALTERNATE_ASSIGNED;
	}

	return APP_UNASSIGNED;
}

bool AppMSW_Outlook::GetInfo()
{
	if (is_installed || is_detected) {
		return true;
	}

	/* Check each possible key. */
	wxRegKey *install_key;
	wxArrayString versions, products;
	wxString version;

	versions.Add(_(KEY_OFFICE_2010_PLUS));

	for (size_t i = 0; i < versions.size(); i++) {
		install_key = GetInstallKey(versions[i]);
		if (install_key->Exists() && install_key->HasSubKey(_(HKLM_KEY_PROPERTIES))) {
			/* The user may have multiple versions of Office installed. */
			products.Add(versions[i]);
		}
		delete install_key;
	}

	/* Just use the first version (most recent install by year) for now. */
	if (products.size() == 0) {
		is_detected = false;
		is_installed = false;
		return false;
	}

	install_key = GetInstallKey(products[0]);
	wxRegKey properties_key(*install_key, _(HKLM_KEY_PROPERTIES));
	delete install_key;

	if (! properties_key.QueryValue(_("DisplayVersion"), version, false)) {
		is_detected = false;
		return false;
	}

	/* Querying was successful. */
	is_detected = true;
	is_installed = true;

	this->info["version"] = version;

	return true;
}

#endif