#pragma once

#if defined(__WXMSW__)

#ifndef H_AppMSW_LiveMail
#define H_AppMSW_LiveMail

#include "SeruroApps.h"

class AppMSW_LiveMail : public AppHelper
{
public:
    /* Most likely, will do nothing. */
    AppMSW_LiveMail() : AppHelper() {
		this->info["accounts"] = wxJSONValue(wxJSONTYPE_OBJECT);
	}
    
    bool IsInstalled();
    wxString GetVersion();
    
    wxArrayString GetAccountList();
    bool IsIdentityInstalled(wxString address);
    
private:
    bool GetInfo();

	/* Get info from an account .oeaccount file. 
	 *  - Set the account info in this->info,
	 *  - return the address of the account.
	 */
	wxString GetAccountInfo(const wxString &account_folder,
		const wxString &account_filename);

	/* Populate account_file if true. */
	bool GetAccountFile(const wxString &sub_folder, 
		wxString &account_filename);

};

#endif

/* OS detection. */
#endif