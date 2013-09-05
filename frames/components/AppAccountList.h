
#ifndef H_AppAccountList
#define H_AppAccountList

#include <wx/window.h>
#include <wx/listctrl.h>
#include <wx/imaglist.h>

#include "../../api/SeruroStateEvents.h"
#include "../../apps/SeruroApps.h"

enum app_account_ids_t {
    APPACCOUNT_LIST_ID
};

class AppAccountList
{
public:
    AppAccountList() {}
    void Create(wxWindow *parent, bool use_address = true, bool initial = false);
    
    ~AppAccountList() {}
    
    /* Access the internal list, only to add to a sizer. */
    void AddAccountList(wxSizer *sizer);
    
    /* All the caller to restrict the apps/accounts displayed. */
    void SetAppWhitelist(wxArrayString whitelist) {
        this->app_whitelist = whitelist;
    }
    void SetAccountWhitelist(wxArrayString whitelist) {
        this->account_whitelist = whitelist;
    }
    
    void SetAccountStatus(long index, const wxString &app_name,
        const wxString &account, bool pending_overrite = false);
    void AddAccount(wxString app, wxString account);
    void RemoveAccount(wxString account);
    void GenerateAccountsList();
    
    bool Assign();
    bool Unassign();

    bool HasAnyAssigned();
    
    bool SelectAccount(long index);
	void DeselectAccounts();
    
    /* BIG NOTE: The AppAccountList does not bind to events. */
    
    /* The client changes something about an account. */
    void OnAccountStateChange(SeruroStateEvent &event);
    /* The identity changes. */
    void OnIdentityStateChange(SeruroStateEvent &event);
    
    
protected:
    /* Information about the selection item. */
    wxString app_name;
    wxString account;
    /* How to display an account, by name or address. */
    bool use_address;
    bool is_initial;
    
    /* Limit the results displayed. */
    wxArrayString app_whitelist;
    wxArrayString account_whitelist;
    
    /* Components. */
    wxListCtrl *accounts_list;
	wxImageList *list_images;
    
    /* Helpers. */
    //SeruroApps *apps_helper;
    /* Determine if apps_helper should be deleted. */
    //bool created_appshelper;
    wxWindow *parent;
    
    /* Events use address, save a map. */
    wxJSONValue address_map;
	/* Keep a list of apps which we're assigned. */
	wxJSONValue pending_list;
    
    //DECLARE_EVENT_TABLE()
    
private:
    //void OnSelect(wxListEvent &event);
    void OnAccountColumnDrag(wxListEvent &event);
    //void OnDeselect(wxListEvent &event);
};

#endif
