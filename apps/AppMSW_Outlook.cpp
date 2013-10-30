
#if defined(__WXMSW__)

#include "AppMSW_Outlook.h"
#include "../SeruroClient.h"
#include "../SeruroConfig.h"
#include "../logging/SeruroLogger.h"

#include "../crypto/SeruroCrypto.h"
#include "../api/Utils.h"

#include <wx/msw/registry.h>
#include <wx/base64.h>

/* Add all keys which may contain Outlook. */
#define KEY_OFFICE_2010_PLUS L"\\UserData\\S-1-5-18\\Products\\00004109110000000000000000F01FEC"
#define KEY_OFFICE_2007      L"\\UserData\\S-1-5-18\\Products\\00002109A10000000000000000F01FEC"

#define HKCU_SUBSYSTEM_BASE L"Software\\Microsoft\\Windows NT\\CurrentVersion"
#define HKCU_SUBSYSTEM L"Windows Messaging Subsystem\\Profiles\\Outlook"

/* const GUID CDECL GUID_Dilkie = {  0x53bc2ec0, 0xd953, 0x11cd, 
 *   {0x97, 0x52, 0x00, 0xaa, 0x00, 0x4a, 0xe4, 0x0e}  };
 */
#define DILKIE_GUID "\xc0\x2e\xbc\x53\x53\xd9\xcd\x11\x97\x52\x00\xaa\x00\x4a\xe4\x0e"
#define PR_SECURITY_PROFILES PROP_TAG(PT_MV_BINARY, 0x355)

#define PR_USER_X509_CERTIFICATE PROP_TAG( PT_MV_BINARY, 0x3A70)

#define CERT_DEFAULTS_SMIME 0x01
#define CERT_DEFAULTS_CERT  0x02
#define CERT_DEFAULTS_SEND  0x04

/* Reference: http://support.microsoft.com/kb/312900 */
enum security_properties {
	PR_CERT_PROP_VERSION     = 0x0001, /* Reserved = 1 */
	PR_CERT_MESSAGE_ENCODING = 0x0006, /* Type of encoding (S/MIME = 1) */
	PR_CERT_DEFAULTS         = 0x0020, /* Bitmask 0x1 (default), 0x2 (default), 0x3 (send) */
	PR_CERT_DISPLAY_NAME_A   = 0x000B, /* Display name. */
	PR_CERT_KEYEX_SHA1_HASH  = 0x0022, /* hash of encryption certificate. */
	PR_CERT_SIGN_SHA1_HASH   = 0x0009, /* hash of identity certificate. */
	PR_CERT_ASYMETRIC_CAPS   = 0x0002, /* ASN.1 DER encoded S/MIME capabilities. */

	PR_CERT_DISPLAY_NAME_W	 = 0x0051, /* Wide display name. */ 
	PR_CERT_KEYEX_CERT       = 0x0003, /* ASN.1 DER encoded x509 certificate. */
};

/* ASN.1 DER encoded encryption capabilities. */
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

bool MAPIProfileExists(LPPROFADMIN &profile_admin, wxString match_name)
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
				if (profile_name == match_name) break;
			}
		}
	}

	/* Clean up. */
	::FreeProws(profile_rows);
	mapi_table->Release();

	if (profile_name != match_name) {
		/* Could not find an Outlook profile. */
		DEBUG_LOG(_("AppMSW_Outlook> (MAPIProfileExists) could not find profile table."));
		return false;
	}

	return true;
}

/* Contact-related methods. */
wxJSONValue GetContactProperties(LPMAPISESSION &logon_session, LPMAPITABLE &address_table, wxString address);
wxJSONValue GetContactProperties(LPMAPISESSION &logon_session, SRow &contact);
wxJSONValue GetMessageX509Properties(SBinary &x509_property);
wxJSONValue GetSecurityProperties(SBinary &entry);
bool SetContactProperties(LPMAPISESSION &logon_session, wxString &entryid, wxJSONValue properties);
wxMemoryBuffer CreateSecurityProperties(wxJSONValue properties, bool add_caps = false);
wxString CreateOutlookContact(LPADRBOOK &address_book, IABContainer* &container, wxString address);

bool GetOutlookProfile(LPPROFADMIN *profile_admin)
{
	HRESULT result;

	result = ::MAPIInitialize(NULL);
	if (FAILED(result)) {
		DEBUG_LOG(_("AppMSW_Outlook> (GetContactProperties) cannot initialize MAPI (%0x)."), result);
		return false;
	}

	/* Create admin. */
	result = ::MAPIAdminProfiles(0, profile_admin);
	if (FAILED(result)) {
		DEBUG_LOG(_("AppMSW_Outlook> (GetContactProperties) cannot open profile (%0x)."), result);
		return false;
	}

	/* Make sure the Outlook profile exists. */
	if (! MAPIProfileExists((*profile_admin), _("Outlook"))) {
		DEBUG_LOG(_("AppMSW_Outlook> (GetContactProperties) no MAPI profile exists (%0x)."), result);
		(*profile_admin)->Release();
		return false;
	}

	return true;
}

bool OpenOutlookAddressBook(LPMAPISESSION *logon_session, LPADRBOOK *address_book, 
	IABContainer **container, LPMAPITABLE *address_table)
{
	HRESULT result;
	ULONG logon_flags;
	LPPROFADMIN profile_admin;

	ULONG open_entry_flags;
	ULONG opened_entry_type;

	ULONG entry_size;
	ENTRYID *entry;

	if (! GetOutlookProfile(&profile_admin)) {
		return false;
	}
	/* Profile only needed to check existance. */
	profile_admin->Release();

	/* Login to MAPI to retreive the user's address book. */
	logon_flags = 0 | MAPI_LOGON_UI | MAPI_NO_MAIL | MAPI_EXTENDED | MAPI_NEW_SESSION;
	result = MAPILogonEx(0, (LPTSTR) "Outlook", NULL, logon_flags, logon_session);
	if (FAILED(result)) {
		DEBUG_LOG(_("AppMSW_Outlook> (CreateOutlookContact) cannot logon to MAPI (%0x)."), result);
		return false;
	}

	/* Open MAPI address book. */
	result = (*logon_session)->OpenAddressBook(0, NULL, 0, address_book);
	if (FAILED(result)) {
		DEBUG_LOG(_("AppMSW_Outlook> (CreateOutlookContact) cannot open address book (%0x)."), result);
		(*logon_session)->Release();
		return false;
	}

	/* Get the personal address book. */
	result = (*address_book)->GetPAB(&entry_size, &entry);
	if (FAILED(result)) {
		DEBUG_LOG(_("AppMSW_Outlook> (GetContactProperties) cannot get personal address book (%0x)."), result);
		(*address_book)->Release();
		(*logon_session)->Release();
		return false;
	}

	/* Chooses best READ/WRITE access possible (based on client/server). */
	open_entry_flags = MAPI_BEST_ACCESS;
	result = (*address_book)->OpenEntry(entry_size, 
		entry, NULL, open_entry_flags, &opened_entry_type, (LPUNKNOWN FAR *) container);
	if (FAILED(result)) {
		DEBUG_LOG(_("AppMSW_Outlook> (GetContactProperties) cannot open PAB entry (%0x)."), result);
		(*address_book)->Release();
		(*logon_session)->Release();
		return false;
	}

	/* PAB entry no longer needed. */
	::MAPIFreeBuffer(entry);

	/* Only support the "Address Book Container" type. */
	if (opened_entry_type != MAPI_ABCONT /* 4 */) {
		DEBUG_LOG(_("AppMSW_Outlook> (GetContactProperties) PAB is not an AB container (%0x)."), result);
		(*address_book)->Release();
		(*logon_session)->Release();
		return false;
	}

	/* The returned container keeps a table for accesses (a table of contacts). */
	result = (*container)->GetContentsTable(0, address_table);
	if (FAILED(result)) {
		DEBUG_LOG(_("AppMSW_Outlook> (GetContactProperties) cannot get PAB contents (%0x)."), result);
		(*container)->Release();
		(*address_book)->Release();
		(*logon_session)->Release();
		return false;
	}

	return true;
}

bool ReleaseOutlookAddressBook(LPMAPISESSION &logon_session, LPADRBOOK &address_book, 
	IABContainer* &container, LPMAPITABLE &address_table)
{
	address_table->Release();
	container->Release();
	address_book->Release();
	logon_session->Logoff(0, 0, 0);
	logon_session->Release();

	return true;
}

wxString CreateOutlookContact(LPADRBOOK &address_book, IABContainer* &container, wxString address)
{
	/* On success, sets the entryid to the created encoded value. */
	wxString new_entryid;
	HRESULT result;

	ULONG oneoff_entryid_size;
	LPENTRYID oneoff_entryid;
	result = address_book->CreateOneOff(AsLongString(address), L"SMTP", AsLongString(address), 
		MAPI_UNICODE, &oneoff_entryid_size, &oneoff_entryid);
	if (FAILED(result)) {
		DEBUG_LOG(_("AppMSW_Outlook> (CreateOutlookContact) cannot create one-off contact (%0x)."), result);
		return wxEmptyString;
	}

	LPMAPIPROP contact_property;
	result = container->CreateEntry(oneoff_entryid_size, oneoff_entryid, 0, &contact_property);
	if (FAILED(result)) {
		DEBUG_LOG(_("AppMSW_Outlook> (CreateOutlookContact) cannot add one-off to PAB (%0x)."), result);
		::MAPIFreeBuffer(oneoff_entryid);
		return wxEmptyString;
	}

	/* Write the entryid into the output parameter. */
	new_entryid = wxBase64Encode(oneoff_entryid, oneoff_entryid_size);

	/* Clean up jumps. */
	contact_property->Release();

	return new_entryid;
}

wxJSONValue GetContactProperties(LPMAPISESSION &logon_session, LPMAPITABLE &address_table, wxString address)
{
	wxJSONValue contacts;
	wxJSONValue contact;

	HRESULT result;

	/* The number of rows is the number of contacts. */
	ULONG num_rows;
	result = address_table->GetRowCount(0, &num_rows);
	if (FAILED(result)) {
		DEBUG_LOG(_("AppMSW_Outlook> (GetContactProperties) cannot get PAB row count (%0x)."), result);
		return contacts;
	}

	/* Allocate each of the contacts (potentially large, revisit). */
	LPSRowSet address_rows;
	result = address_table->QueryRows(num_rows, 0, &address_rows);
	if (FAILED(result)) {
		DEBUG_LOG(_("AppMSW_Outlook> (GetContactProperties) cannot get PAB rows (%0x)."), result);
		return contacts;
	}

	/* Iterate over each contact. */
	for (ULONG i = 0; i < num_rows; ++i) {
		contact = GetContactProperties(logon_session, address_rows->aRow[i]);
		if (contact.HasMember("email_address") && contact["email_address"].AsString() == address) {
			contacts.Append(contact);
		}
	}

	/* Clean up jumps. */
	::FreeProws(address_rows);

	return contacts;
}

wxJSONValue GetContactProperties(LPMAPISESSION &logon_session, SRow &contact)
{
	LPSPropValue contact_entryid;
	wxJSONValue properties;

	/* Iterate over each contact property. */
	properties["success"] = false;
	for (ULONG j = 0; j < contact.cValues; ++j) {
		if (contact.lpProps[j].ulPropTag == PR_ENTRYID) {
			contact_entryid = &contact.lpProps[j];
		} else if (contact.lpProps[j].ulPropTag == PR_EMAIL_ADDRESS_A) {
			properties["email_address"] = _(contact.lpProps[j].Value.lpszA);
		}
	}

	ULONG opened_entry_type;
	HRESULT result;

	/* Used to query all contact properties, not just the limited IAdrBook properties. */
	IMessage *message;

	/* Access properties only available in IMessage. */
	SizedSPropTagArray(3, property_columns) = {3, PR_USER_X509_CERTIFICATE, PR_SURNAME_A, PR_GIVEN_NAME_A};

	LPSPropValue property_values;
	ULONG property_count;

	/* Use the stored entryid to format the iMessage/Contact entryid. */
	/* http://msdn.microsoft.com/en-us/library/ee203147(v=exchg.80).aspx 
	 * Flags: 4 bytes 
	 * ProviderUID: 16 bytes
	 * Version (4), Type (4), Index (4), EntryIDCount (4), EntryID Bytes (variable)
	 */

	ULONG entryid_size;
	//LPENTRYID entryid;
	if (contact_entryid->Value.bin.cb < 36) {
		/* Incorrect size, no entryid count given. */
		DEBUG_LOG(_("AppMSW_Outlook> (GetContactProperties) ENTRYID has incorrect size (%d)."), 
			contact_entryid->Value.bin.cb);
		return properties;
	}

	/* Todo: copy unsigned int bytes, no application of endianess. */
	memcpy(&entryid_size, (contact_entryid->Value.bin.lpb) + 32, 4);
	if (contact_entryid->Value.bin.cb < (36 + entryid_size)) {
		/* The entryid_size is larger than the contact_entryid buffer. */
		DEBUG_LOG(_("AppMSW_Outlook> (GetContactProperties) ENTRYID has incorrect internal size (%d)."), entryid_size);
		return properties;
	}

	/* Open IMessage (IMAPSession) using the entryid found in the PAB. */
	result = logon_session->OpenEntry(24, (LPENTRYID) (contact_entryid->Value.bin.lpb + 36), 
		&IID_IMessage, MAPI_BEST_ACCESS, &opened_entry_type, (LPUNKNOWN FAR *) &message);
	if (FAILED(result)) {
		DEBUG_LOG(_("AppMSW_Outlook> (GetContactProperties) cannot open IMessage ENTRYID (%0x)."), result);
		return properties;
	}

	/* Requested IMessage type */
	if (opened_entry_type != MAPI_MESSAGE /* 5 */) {
		DEBUG_LOG(_("AppMSW_Outlook> (GetContactProperties) entry is not an IMessage (%0x)."), result);
		//message->Release();
		return properties;
	}

	result = message->GetProps((LPSPropTagArray) &property_columns, 0, &property_count, &property_values);
	if (FAILED(result) || property_count != 3) {
		/* The property count SHOULD = 3. */
		DEBUG_LOG(_("AppMSW_Outlook> (GetContactProperties) cannot get IMessage properties (%0x)."), result);
		message->Release();
		return properties;
	}

	/* Store the relevant properties. */
	for (ULONG i = 0; i < property_count; ++i) {
		if (property_values[i].ulPropTag == PR_USER_X509_CERTIFICATE) {
			/* Parse each certificate. */
			for (ULONG j = 0; j < property_values[i].Value.MVbin.cValues; ++j) {
				properties["user_x509_certificates"].Append(
					GetMessageX509Properties(property_values[i].Value.MVbin.lpbin[j])
				);
			}
		} else if (property_values[i].ulPropTag == PR_SURNAME_A) {
			properties["last_name"] = _(property_values[i].Value.lpszA);
		} else if (property_values[i].ulPropTag == PR_GIVEN_NAME_A) {
			properties["first_name"] = _(property_values[i].Value.lpszA);
		}
	}

	/* Add the entryID to properties, to allow the caller to set. */
	wxMemoryBuffer entry_buffer;
	entry_buffer.AppendData((contact_entryid->Value.bin.lpb + 36), 24);
	properties["entryid"] = wxBase64Encode(entry_buffer);

	/* Cleanups. */
	::MAPIFreeBuffer(property_values);
	message->Release();

	return properties;
}

wxJSONValue GetMessageX509Properties(SBinary &x509_property)
{
	wxJSONValue properties;

	properties = GetSecurityProperties(x509_property);
	if (! properties.HasMember("exchange")) {
		/* There is no certificate attached. */
		return properties;
	}

	/* The exchange property must be ASN.1 DER decoded, as a certificate. */
	wxMemoryBuffer exchange_buffer;
	PCCERT_CONTEXT exchange_cert;

	/* Try to parse the exchange field as a certificate. */
	exchange_buffer = wxBase64Decode(properties["exchange"].AsString());
	exchange_cert = CertCreateCertificateContext(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 
		(BYTE *) exchange_buffer.GetData(), exchange_buffer.GetDataLen());

	if (exchange_cert == NULL) {
		DEBUG_LOG(_("AppMSW_Outlook> (GetMessageX509Properties) cannot parse ASN.1."));
		return properties;
	}

	wxString fingerprint;
	//bool have_fingerprint;

	fingerprint = GetFingerprintFromCertificate(exchange_cert, BY_SKID);
	/* This might be optional, or better, checked in a more appropriate location. */
	//have_fingerprint = HaveCertificateByFingerprint(fingerprint, CERTSTORE_CONTACTS, BY_SKID);
	CertFreeCertificateContext(exchange_cert);

	properties["skid"] = fingerprint;
	//properties["skid_exists"] = have_fingerprint;

	return properties;
}

bool SetContactProperties(LPMAPISESSION &logon_session, wxString &entryid, wxJSONValue properties)
{
	HRESULT result;
	bool status;
	ULONG opened_entry_type;
	
	/* Used to store the entryid. */
	wxMemoryBuffer entry_buffer;

	/* Used to open advanced properties for contact. */
	IMessage *message;

	/* Create profiles for each provided (in the properties parameter). */
	SBinary *x509_profiles;
	size_t num_certificates;

	/* The new propery value. */
	SPropValue property_values;
	wxMemoryBuffer property_data;

	status = false;
	entry_buffer = wxBase64Decode(entryid);

	/* Open IMessage (IMAPSession) using the entryid found in the PAB. */
	result = logon_session->OpenEntry(24, (LPENTRYID) entry_buffer.GetData(), 
		&IID_IMessage, MAPI_BEST_ACCESS, &opened_entry_type, (LPUNKNOWN FAR *) &message);
	if (FAILED(result)) {
		DEBUG_LOG(_("AppMSW_Outlook> (SetContactProperties) cannot open IMessage ENTRYID (%0x)."), result);
		return false;
	}

	/* Requested IMessage type */
	if (opened_entry_type != MAPI_MESSAGE /* 5 */) {
		DEBUG_LOG(_("AppMSW_Outlook> (SetContactProperties) entry is not an IMessage (%0x)."), result);
		return false;
	}

	/* There will "most likely" be two user_x509_certificate binaries. */	
	num_certificates = (size_t) properties["user_x509_certificates"].Size();
	//num_certificates = 1;
	x509_profiles = (SBinary *) malloc(sizeof(SBinary) * num_certificates);

	/* This is only setting the certificate data, not the first/last name. */
	property_values.ulPropTag = PR_USER_X509_CERTIFICATE;
	property_values.Value.MVbin.cValues = num_certificates;
	property_values.Value.MVbin.lpbin = x509_profiles;
	for (ULONG i = 0; i < num_certificates; ++i) {
		/* Create each OXCDATA/ASN.1 blob and add to the list of profiles. */
		property_data = CreateSecurityProperties(properties["user_x509_certificates"][i], false);
		property_values.Value.MVbin.lpbin[i].lpb = (BYTE *) malloc(property_data.GetDataLen());
		memcpy(property_values.Value.MVbin.lpbin[i].lpb, property_data.GetData(), property_data.GetDataLen());
		//property_values.Value.MVbin.lpbin[i].lpb = (BYTE *) property_data.GetData();
		property_values.Value.MVbin.lpbin[i].cb = property_data.GetDataLen();
	}
	
	result = message->SetProps(1, &property_values, NULL);
	if (FAILED(result)) {
		/* Problem saving the properties. */
		DEBUG_LOG(_("AppMSW_Outlook> (SetContactProperties) could write properties (%0x)."), result);
		goto release_profiles;
	}

	result = message->SaveChanges(0);
	if (FAILED(result)) {
		DEBUG_LOG(_("AppMSW_Outlook> (SetContactProperties) could not save properties (%0x)."), result);
		goto release_profiles;
	}

	/* Success, return the correct status. */
	status = true;

	/* Cleanup jumps. */
release_profiles:
	delete x509_profiles;
	message->Release();

	return status;
}

bool SetSecurityProperties(wxString &service_uid, wxJSONValue properties)
{
	HRESULT result;
	LPPROFADMIN profile_admin;

	if (! GetOutlookProfile(&profile_admin)) {
		return false;
	}

	LPSERVICEADMIN service_admin;

	/* The profile name is in ANSI? */
	result = profile_admin->AdminServices((LPTSTR) "Outlook", NULL, NULL, 0, &service_admin);
	if (FAILED(result)) {
		DEBUG_LOG(_("AppMSW_Outlook> (SetSecurityProperties) cannot open Outlook service (%0x)."), result);

		profile_admin->Release();
		return false;
	}

	SPropValue service_property;
	wxMemoryBuffer service_uid_buffer;

	service_property.ulPropTag = PR_SERVICE_UID;
	service_uid_buffer = wxBase64Decode(service_uid);

	service_property.Value.bin.cb = service_uid_buffer.GetDataLen();
	service_property.Value.bin.lpb = (byte *) service_uid_buffer.GetData();

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

	SBinary security_profile;
	SPropValue property_values;
	wxMemoryBuffer entry_buffer;

	SPropTagArray property_tags;
	//ULONG tag_array[1];

	/* Only support one MAPI security profile at a time. */
	if (! properties.HasMember("clear")) {
		property_values.ulPropTag = PR_SECURITY_PROFILES;
		property_values.Value.MVbin.lpbin = &security_profile;
		
		entry_buffer = CreateSecurityProperties(properties, true);
		property_values.Value.MVbin.cValues = 1;
		property_values.Value.MVbin.lpbin[0].cb = entry_buffer.GetDataLen();
		property_values.Value.MVbin.lpbin[0].lpb = (BYTE *) malloc(entry_buffer.GetDataLen());
		memcpy(property_values.Value.MVbin.lpbin[0].lpb, entry_buffer.GetData(), entry_buffer.GetDataLen());
		
		result = profile_section->SetProps(1, &property_values, NULL);
	} else {
		//property_tags.aulPropTag = PR_SECURITY_PROFILES;
		//tag_array[0] = PR_SECURITY_PROFILES;
		property_tags.aulPropTag[0] = PR_SECURITY_PROFILES;
		property_tags.cValues = 1;

		result = profile_section->DeleteProps(&property_tags, NULL);
	}

	/* Clean up. */
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

wxMemoryBuffer CreateSecurityProperties(wxJSONValue properties, bool add_caps)
{
	wxMemoryBuffer entry_buffer;
	wxMemoryBuffer cert_buffer;

	USHORT tag, length;
	ULONG long_data;

	/* Set mandatory property version. */
	tag = PR_CERT_PROP_VERSION, length = 4+4, long_data = 0x1;
	entry_buffer.AppendData(&tag, 2);
	entry_buffer.AppendData(&length, 2);
	entry_buffer.AppendData(&long_data, 4);

	/* Set the OXCDATA encoding, also static. */
	tag = PR_CERT_MESSAGE_ENCODING, length = 4+4, long_data = 0x1;
	entry_buffer.AppendData(&tag, 2);
	entry_buffer.AppendData(&length, 2);
	entry_buffer.AppendData(&long_data, 4);

	/* Set all of the defaults. */
	tag = PR_CERT_DEFAULTS, length = 4+4;
	long_data = CERT_DEFAULTS_SMIME | CERT_DEFAULTS_CERT | CERT_DEFAULTS_SEND;
	entry_buffer.AppendData(&tag, 2);
	entry_buffer.AppendData(&length, 2);
	entry_buffer.AppendData(&long_data, 4);

	//ULONG defaults = 0;
	//defaults += (properties["default_smime"].AsBool() == true) ? CERT_DEFAULTS_SMIME : 0;
	//defaults += (properties["default_cert"].AsBool()  == true) ? CERT_DEFAULTS_CERT  : 0;
	//defaults += (properties["default_send"].AsBool()  == true) ? CERT_DEFAULTS_SEND  : 0;

	if (properties.HasMember("entry_name")) {
		tag = PR_CERT_DISPLAY_NAME_A, length = 4 + properties["entry_name"].AsString().size() + 1;
		entry_buffer.AppendData(&tag, 2);
		entry_buffer.AppendData(&length, 2);
		entry_buffer.AppendData(AsChar(properties["entry_name"].AsString()), length - 4 - 1);
		entry_buffer.AppendByte(0);
	}
	
	if (properties.HasMember("authentication")) {
		cert_buffer.Clear();
		cert_buffer = wxBase64Decode(properties["authentication"].AsString());
		if (cert_buffer.GetDataLen() == 20) {
			tag = PR_CERT_SIGN_SHA1_HASH, length = 4 + 20;
			entry_buffer.AppendData(&tag, 2);
			entry_buffer.AppendData(&length, 2);
			entry_buffer.AppendData(cert_buffer.GetData(), 20);
		}
	} 
	
	if (properties.HasMember("encipherment")) {
		cert_buffer.Clear();
		cert_buffer = wxBase64Decode(properties["encipherment"].AsString());
		if (cert_buffer.GetDataLen() == 20) {
			tag = PR_CERT_KEYEX_SHA1_HASH, length = 4 + 20;
			entry_buffer.AppendData(&tag, 2);
			entry_buffer.AppendData(&length, 2);
			entry_buffer.AppendData(cert_buffer.GetData(), 20);
		}
	}
	
	if (properties.HasMember("exchange")) {
		cert_buffer.Clear();
		cert_buffer = wxBase64Decode(properties["exchange"].AsString());
		tag = PR_CERT_KEYEX_CERT, length = 4 + cert_buffer.GetDataLen();
		entry_buffer.AppendData(&tag, 2);
		entry_buffer.AppendData(&length, 2);
		entry_buffer.AppendData(cert_buffer.GetData(), cert_buffer.GetDataLen());
	}

	tag = PR_CERT_ASYMETRIC_CAPS;
	entry_buffer.AppendData(&tag, 2);
	if (add_caps) {
		wxMemoryBuffer asymetric_caps = AsBinary(ASYMETRIC_CAPS_BLOB);
		length = 4 + asymetric_caps.GetDataLen();
		entry_buffer.AppendData(&length, 2);
		entry_buffer.AppendData(asymetric_caps.GetData(), asymetric_caps.GetDataLen());
	} else {
		/* Capabilities entry added, but blank. */
		length = 4, long_data = 0;
		entry_buffer.AppendData(&length, 2);
		//entry_buffer.AppendData(&long_data, 4);
	}

	return entry_buffer;
}

wxJSONValue GetSecurityProperties(SBinary &entry)
{
	wxJSONValue properties;
	wxString display_name;
	wxMemoryBuffer certificate_hash;
	wxString test;

	USHORT entry_offset = 0;

	/* Parse TAG (2 bytes)/LENGTH (2 bytes)/DATA (LENGTH bytes) */
	USHORT tag, length;
	byte* data;

	/* Read through the entry, extracting each touple. */
	while (entry_offset < entry.cb) {
		tag = *(entry.lpb + entry_offset);
		length = (UCHAR) *(entry.lpb + entry_offset + 2);
		length += ((UCHAR) *(entry.lpb + entry_offset + 3) * 256);

		if ((entry_offset + length) > (unsigned short) entry.cb || tag == 0 || length == 0) {
			/* There is an overrun/underrun error in the OXCDATA blob? */
			break;
		}

		data = entry.lpb + entry_offset + 2 + 2 /* tag and length are both uint16 */;
		entry_offset += length;

		if (tag == PR_CERT_MESSAGE_ENCODING && *((unsigned long *) data) != 1) {
			/* This is not an S/MIME profile, skip. */
			break;
		}

		if (tag == PR_CERT_DISPLAY_NAME_A) {
			if (! SafeANSIString(data, length, display_name)) {
				/* The OXCDATA data type contained non-ASCII characters? */
				break;
			}
			properties["entry_name"] = display_name;
		}

		if (tag == PR_CERT_DEFAULTS) {
			properties["default_smime"] = ((ULONG) *(data)) && CERT_DEFAULTS_SMIME;
			properties["default_cert"]  = ((ULONG) *(data)) && CERT_DEFAULTS_CERT;
			properties["default_send"]  = ((ULONG) *(data)) && CERT_DEFAULTS_SEND;
		} else if (tag == PR_CERT_SIGN_SHA1_HASH) {
			certificate_hash.Clear();
			certificate_hash.AppendData(data, length-4);
			properties["authentication"] = wxBase64Encode(certificate_hash);
			test = properties["authentication"].AsString();
		} else if (tag == PR_CERT_KEYEX_SHA1_HASH) {
			certificate_hash.Clear();
			certificate_hash.AppendData(data, length-4);
			properties["encipherment"] = wxBase64Encode(certificate_hash);
			test = properties["encipherment"].AsString();
		} else if (tag == PR_CERT_KEYEX_CERT) {
			certificate_hash.Clear();
			certificate_hash.AppendData(data, length-4);
			properties["exchange"] = wxBase64Encode(certificate_hash);
		}
	}

	return properties;
}

wxJSONValue GetAccountProperties(LPSERVICEADMIN &service_admin, SPropValue &uid, wxString &account_name)
{
	wxJSONValue properties;

	HRESULT result;
	LPPROVIDERADMIN provider_admin;
	LPPROFSECT profile_section;
	LPSPropValue property_values;
	ULONG property_count;

	/* Set a reference to the account name using itself as an index. */
	properties["name"] = account_name;

	wxMemoryBuffer uid_buffer;
	wxJSONValue entry_properties;

	SizedSPropTagArray(1, property_columns) = {1, PR_SECURITY_PROFILES};

	/* The profile has a security "profile" property. Store it and parse later. */
	uid_buffer.AppendData((void *) uid.Value.bin.lpb, uid.Value.bin.cb);
	properties["service_uid"] = wxBase64Encode(uid_buffer);

	result = service_admin->AdminProviders((LPMAPIUID) uid.Value.bin.lpb, 0, &provider_admin);
	if (FAILED(result)) {
		DEBUG_LOG(_("AppMSW_Outlook> (FindAccountProperties) cannot get provider (%s) (%0x)."), 
			account_name, result);
		return properties;
	}

	result = provider_admin->OpenProfileSection((LPMAPIUID) DILKIE_GUID, NULL, MAPI_MODIFY, &profile_section);
	if (FAILED(result)) {
		DEBUG_LOG(_("AppMSW_Outlook> (FindAccountProperties) cannot open section (%s) (%0x)."),
			account_name, result);
		goto release_provider_admin;
	}

	result = profile_section->GetProps((LPSPropTagArray) &property_columns, 0, &property_count, &property_values);
	if (FAILED(result) || property_count == 0) {
		DEBUG_LOG(_("AppMSW_Outlook> (FindAccountProperties) cannot get DILKIE properties (%s) (%0x)."),
			account_name, result);
		goto release_profile_section;
	}

	/* If there are no sercurity profiles the count is still 1, but the property tag is not set correctly. */
	if (property_values[0].ulPropTag != PR_SECURITY_PROFILES) {
		goto release_property_values;
	}

	/* The MVbin = # (4 bytes) of entires PT_BINARY: {size (2 bytes), data} */
	/* SProperty (data): http://support.microsoft.com/kb/312900 */

	for (ULONG i = 0; i < property_values[0].Value.MVbin.cValues; ++i) {
		entry_properties = GetSecurityProperties(property_values[0].Value.MVbin.lpbin[i]);
		properties["profiles"].Append(entry_properties);
	}

release_property_values:
	::MAPIFreeBuffer(property_values);

release_profile_section:
	profile_section->Release();
release_provider_admin:
	provider_admin->Release();

	return properties;
}

bool HasAlternateSecurityProperties(wxJSONValue properties)
{
	/* Check to make sure the associated certificates have Seruro usage. */
	return false;
}

bool ValidContactSecurityProperties(wxString &server_uuid, wxString &address, wxJSONValue properties)
{
	/* Check the security properties and if they include ONLY the auth/enc for the given contact. */
	return false;
}

wxJSONValue GetOutlookMAPIAccounts()
{
	wxJSONValue accounts;
	wxString account_name;

	HRESULT result;
	LPPROFADMIN profile_admin;

	if (! GetOutlookProfile(&profile_admin)) {
		return accounts;
	}

	LPSERVICEADMIN service_admin;
	LPMAPITABLE service_table;
	LPSRowSet service_rows;

	/* Fetch the following columns from the services table. */
	enum {display_name_index, service_name_index, service_uid_index};
	SizedSPropTagArray(3, service_columns) = {3, 
		PR_DISPLAY_NAME,
		PR_SERVICE_NAME,
		PR_SERVICE_UID
	};

	/* The profile name is in ANSI? */
	result = profile_admin->AdminServices((LPTSTR) "Outlook", NULL, NULL, 0, &service_admin);
	if (FAILED(result)) {
		DEBUG_LOG(_("AppMSW_Outlook> (GetAccountList) cannot open Outlook service (%0x)."), result);
		goto release_profile_admin;
	}

	result = service_admin->GetMsgServiceTable(0, &service_table);
	if (FAILED(result)) {
		DEBUG_LOG(_("AppMSW_Outlook> (GetAccountList) cannot get service table (%0x)."), result);
		goto release_service_admin;
	}

	SRestriction service_restriction;
	SPropValue service_property;

	/* Create a "restriction", a filter, for service name "INTERSTOR". */
	service_restriction.rt = RES_CONTENT;
	service_restriction.res.resContent.ulFuzzyLevel = FL_FULLSTRING;
	service_restriction.res.resContent.ulPropTag = PR_SERVICE_NAME;
	service_restriction.res.resContent.lpProp = &service_property;

	service_property.ulPropTag = PR_SERVICE_NAME;
	service_property.Value.lpszW = L"INTERSTOR";

	result = ::HrQueryAllRows(service_table, (LPSPropTagArray) &service_columns,
		&service_restriction, NULL, 0, &service_rows);
	if (FAILED(result) || service_rows == NULL || service_rows->cRows == 0) {
		DEBUG_LOG(_("AppMSW_Outlook> (GetAccountList) cannot query service table (%0x)."), result);
		goto release_service_table;
	}

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
			accounts[account_name] = GetAccountProperties(service_admin, 
				service_rows->aRow[i].lpProps[service_uid_index], account_name);
		}
	}

	::FreeProws(service_rows);

release_service_table:
	service_table->Release();
release_service_admin:
	service_admin->Release();

release_profile_admin:
	profile_admin->Release();

	return accounts;
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

wxArrayString AppMSW_Outlook::GetAccountList()
{
	wxArrayString accounts;

	/* Reset previous account info. */
	//this->info["accounts"] = wxJSONValue(wxJSONTYPE_OBJECT);

	/* Fail if no application info is detected. */
	if (! this->GetInfo()) {
		return accounts;
	}

	/* Set/ReSet the account information. */
	this->info["accounts"] = GetOutlookMAPIAccounts();
	accounts = this->info["accounts"].GetMemberNames();

	return accounts;
}

bool AppMSW_Outlook::AddContact(wxString server_uuid, wxString address)
{
	/* A list of matching contacts. */
	wxJSONValue contacts;
	/* The security parameters to get/set for contact. */
	wxJSONValue properties;
	/* The working contact, if more than one, or it's created. */
	wxString contact_entryid;
	/* A contact contains non-seruro x509 properties. */
	bool alternate_properties, status;

	/* State managers for address book operations. */
	LPMAPISESSION logon_session;
	LPADRBOOK address_book;
	IABContainer *container;
	LPMAPITABLE address_table;

	if (! OpenOutlookAddressBook(&logon_session, &address_book, &container, &address_table)) {
		return false;
	}

	alternate_properties = false;
	contacts = GetContactProperties(logon_session, address_table, address);
	if (contacts.Size() > 0) {
		/* Look for contact with appropraite x509, make sure they match. */
		for (int i = 0; i < contacts.Size(); ++i) {
			if (HasAlternateSecurityProperties(contacts[i])) {
				alternate_properties = true;
			}
			if (ValidContactSecurityProperties(server_uuid, address, contacts[i])) {
				/* This contact already exists and has the correct certificates. */
				ReleaseOutlookAddressBook(logon_session, address_book, container, address_table);
				return true;
			}
		}
		/* Operate on the first contact. */
		contact_entryid = contacts[0]["entryid"].AsString();
	} else {
		/* Create contact, then get it's properties. */
		CreateOutlookContact(address_book, container, address);

		/* Reset the address book event table (this could be done better). */
		ReleaseOutlookAddressBook(logon_session, address_book, container, address_table);
		if (! OpenOutlookAddressBook(&logon_session, &address_book, &container, &address_table)) {
			return false;
		}
		contacts = GetContactProperties(logon_session, address_table, address);
		contact_entryid = contacts[0]["entryid"].AsString();
	}

	if (contact_entryid == wxEmptyString) {
		/* There was a logic problem with finding/creating a contact. */
		ReleaseOutlookAddressBook(logon_session, address_book, container, address_table);
		return false;
	}

	if (alternate_properties) {
		/* Todo: warn the user about potentially removing custom certifcates. */
	}

	/* Create the property containers (with certificate data). */
    wxArrayString certs = theSeruroConfig::Get().GetCertificates(server_uuid, address);
	wxString encoded_cert;
    for (size_t i = 0; i < certs.size(); ++i) {
		encoded_cert = GetEncodedByFingerprint(certs[i], CERTSTORE_CONTACTS, BY_SKID);
        properties["user_x509_certificates"][i]["exchange"] = encoded_cert;
	}
    status = SetContactProperties(logon_session, contact_entryid, properties);

	ReleaseOutlookAddressBook(logon_session, address_book, container, address_table);
	return status;
}

bool AppMSW_Outlook::UnassignIdentity(wxString address)
{
	wxString service_uid;
	wxJSONValue properties;

	service_uid = this->info["accounts"][address]["service_uid"].AsString();
	if (service_uid == wxEmptyString) {
		/* The service uid was not probed during an account enumeration? */
		return false;
	}

	/* Set the profile to clear. */
	properties["clear"] = true;
	if (! SetSecurityProperties(service_uid, properties)) {
		return false;
	}

	this->info["accounts"][address].Remove("profiles");
	return true;
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
	auth_skid = theSeruroConfig::Get().GetIdentity(server_uuid, address, ID_AUTHENTICATION);
	enc_skid  = theSeruroConfig::Get().GetIdentity(server_uuid, address, ID_ENCIPHERMENT);

	/* For each, GetIdentityHashBySKID */
	properties["authentication"] = crypto.GetIdentityHashBySKID(auth_skid);
	properties["encipherment"]  = crypto.GetIdentityHashBySKID(enc_skid);

	properties["entry_name"] = wxString::Format(_("Seruro Identity (%s) for %s"), 
		theSeruroConfig::Get().GetServerName(server_uuid),	address);

	if (! SetSecurityProperties(service_uid, properties)) {
		/* Could not assign an entity for the service_uid, does the account exist? */
		return false;
	}

	this->info["accounts"][address]["profiles"].Append(properties);
	return true;
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

		/* Todo: this is a temporary hack while the security blob data is investigated. */
		//server_uuid.Append((theSeruroConfig::Get().GetServerList().size() > 0) ?
		//	theSeruroConfig::Get().GetServerList()[0] : _("(None)"));
		//return APP_ASSIGNED;
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
	versions.Add(_(KEY_OFFICE_2007));

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