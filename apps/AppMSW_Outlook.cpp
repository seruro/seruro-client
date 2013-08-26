
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

//const GUID CDECL GUID_Dilkie = {  0x53bc2ec0, 0xd953, 0x11cd, {0x97, 0x52, 0x00, 0xaa, 0x00, 0x4a, 0xe4, 0x0e}  };
#define DILKIE_GUID "\xc0\x2e\xbc\x53\x53\xd9\xcd\x11\x97\x52\x00\xaa\x00\x4a\xe4\x0e"
#define PR_SECURITY_PROFILES PROP_TAG(PT_MV_BINARY, 0x355)


wxDECLARE_APP(SeruroClient);

bool AppMSW_Outlook::IsInstalled()
{
	if (! GetInfo()) return false;

	//return this->is_installed;
	return false;
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

wxJSONValue GetSecurityProperties(SBinary *entry)
{
	wxJSONValue properties;

	/* Parse TAG (2 bytes)/LENGTH (2 bytes)/DATA (LENGTH bytes) */

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
		DEBUG_LOG(_("AppMSW_Outlook> (FindAccountProperties) cannot get provider for (%s) (%0x)."), 
			account_name, result);
		return;
	}

	result = provider_admin->OpenProfileSection((LPMAPIUID) DILKIE_GUID, 
		NULL, MAPI_MODIFY, &profile_section);
	if (FAILED(result)) {
		DEBUG_LOG(_("AppMSW_Outlook> (FindAccountProperties) cannot open profile section for (%s) (%0x)."),
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

	/* Fail if no application info is detected. */
	if (! this->GetInfo()) {
		return accounts;
	}

	HRESULT result;
	LPPROFADMIN profile_admin;

	/* Create a profile administration object. */
	result = ::MAPIInitialize(NULL);
	result = ::MAPIAdminProfiles(0, &profile_admin);

	if (! MAPIProfileExists(profile_admin)) {
		return accounts;
	}

	LPSERVICEADMIN service_admin;
	LPMAPITABLE service_table;
	LPSRowSet service_rows;

	/* The profile name is in ANSI? */
	result = profile_admin->AdminServices((LPTSTR) "Outlook", NULL, NULL, 0, &service_admin);
	result = service_admin->GetMsgServiceTable(0, &service_table);

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
	return false;
}

account_status_t AppMSW_Outlook::IdentityStatus(wxString address, wxString &server_uuid)
{
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