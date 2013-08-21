
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
    AppAccountList(wxWindow *parent, bool use_address = true);
    ~AppAccountList() {
        if (created_appshelper) {
            delete this->apps_helper;
        }
    }
    
    /* Access the internal list, only to add to a sizer. */
    void AddList(wxSizer *sizer);
    
    /* Pass in a helper pointer, or allow the AppAccountList to create one.*/
    void CreateHelper() {
        this->apps_helper = new SeruroApps();
        this->created_appshelper = true;
    }
    void SetHelper(SeruroApps *existing_helper) {
        this->apps_helper = existing_helper;
    }
    
    /* All the caller to restrict the apps/accounts displayed. */
    void SetAppWhitelist(wxArrayString whitelist) {
        this->app_whitelist = whitelist;
    }
    void SetAccountWhitelist(wxArrayString whitelist) {
        this->account_whitelist = whitelist;
    }
    
    void AddAccount(wxString app, wxString account);
    void GenerateAccountsList();
    
    //bool Assign(wxString app, wxString account);
    //bool Unassign(wxString app, wxString account);
    //void Refresh();
    
	void OnSelect(wxListEvent &event);
    void OnColumnDrag(wxListEvent &event);
    void OnDeselect(wxListEvent &event);
    
	void DoDeselect();
	void DeselectAccounts();
    
    /* The client changes something about an account. */
    void OnAccountStateChange(SeruroStateEvent &event);
    
protected:
    /* Information about the selection item. */
    wxString app_name;
    wxString account;
    /* How to display an account, by name or address. */
    bool use_address;
    
    /* Limit the results displayed. */
    wxArrayString app_whitelist;
    wxArrayString account_whitelist;
    
    /* Components. */
    wxListCtrl *accounts_list;
	wxImageList *list_images;
    
    /* Helpers. */
    SeruroApps *apps_helper;
    /* Determine if apps_helper should be deleted. */
    bool created_appshelper;
    wxWindow *parent;
    
    //DECLARE_EVENT_TABLE()
};