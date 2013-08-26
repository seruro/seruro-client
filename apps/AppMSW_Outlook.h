#pragma once

#if defined(__WXMSW__)

#ifndef H_AppMSW_Outlook
#define H_AppMSW_Outlook

#include "SeruroApps.h"

#include <Windows.h>
#include <MAPI.h>

/* Not available in the Windows 8 SDK, copied from 7A and linked explicitly. */
#include <MAPIX.h>
#include <MAPIUtil.h>

class AppMSW_Outlook : public AppHelper
{
public:
    /* Most likely, will do nothing. */
    AppMSW_Outlook() : AppHelper() {
		//this->info["accounts"] = wxJSONValue(wxJSONTYPE_OBJECT);
	}
    
    bool IsInstalled();
    wxString GetVersion();
    
    wxArrayString GetAccountList();
    account_status_t IdentityStatus(wxString address, wxString &server_uuid);
	bool AssignIdentity(wxString server_uuid, wxString address);
    
private:
    bool GetInfo();

void FindAccountProperties(LPSERVICEADMIN &service_admin, 
	SPropValue &uid, wxString &account_name);

	/* Get info from an account .oeaccount file. 
	 *  - Set the account info in this->info,
	 *  - return the address of the account.
	 */
	//wxString GetAccountInfo(const wxString &account_folder,
	//	const wxString &account_filename);
	//wxString GetAccountFile(wxString address);

	/* Populate account_file if true. */
	//bool GetAccountFile(const wxString &sub_folder, 
	//	wxString &account_filename);

};

#endif

/* OS detection. */
#endif