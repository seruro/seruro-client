
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
    void Create(wxWindow *parent, bool use_address = true);
    
    ~AppAccountList() {
        //if (created_appshelper) {
        //    delete this->apps_helper;
        //}
    }
    
    /* Access the internal list, only to add to a sizer. */
    void AddAccountList(wxSizer *sizer);
    
    /* Pass in a helper pointer, or allow the AppAccountList to create one.*/
    //void CreateHelper() {
    //    this->apps_helper = new SeruroApps();
    //    this->created_appshelper = true;
    //}
    //void SetHelper(SeruroApps *existing_helper) {
    //    this->apps_helper = existing_helper;
    //}
    
    /* All the caller to restrict the apps/accounts displayed. */
    void SetAppWhitelist(wxArrayString whitelist) {
        this->app_whitelist = whitelist;
    }
    void SetAccountWhitelist(wxArrayString whitelist) {
        this->account_whitelist = whitelist;
    }
    
    void AddAccount(wxString app, wxString account);
    void GenerateAccountsList();
    
    bool Assign();
    bool Unassign();
    //void Refresh();
    
    bool SelectAccount(long index);
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
    //SeruroApps *apps_helper;
    /* Determine if apps_helper should be deleted. */
    //bool created_appshelper;
    wxWindow *parent;
    
    //DECLARE_EVENT_TABLE()
    
private:
    //void OnSelect(wxListEvent &event);
    void OnAccountColumnDrag(wxListEvent &event);
    //void OnDeselect(wxListEvent &event);
};

#endif
