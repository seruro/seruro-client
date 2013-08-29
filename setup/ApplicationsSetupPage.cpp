/* Remember to set enable_prev to false. */

#include "SeruroSetup.h"
#include "../SeruroClient.h"

#include "../frames/SeruroFrame.h"

enum application_setup_ids_t {
    BUTTON_ASSIGN,
    BUTTON_UNASSIGN,
    BUTTON_REFRESH
};

DECLARE_APP(SeruroClient);

BEGIN_EVENT_TABLE(ApplicationsPage, SetupPage)
    EVT_LIST_ITEM_SELECTED(APPACCOUNT_LIST_ID, ApplicationsPage::OnAccountSelected)
    EVT_LIST_ITEM_DESELECTED(APPACCOUNT_LIST_ID, ApplicationsPage::OnAccountDeselected)

    EVT_BUTTON(BUTTON_ASSIGN,   ApplicationsPage::OnAssign)
    EVT_BUTTON(BUTTON_UNASSIGN, ApplicationsPage::OnUnassign)
    EVT_BUTTON(BUTTON_REFRESH,  ApplicationsPage::OnRefresh)
END_EVENT_TABLE()

void ApplicationsPage::OnApplicationStateChange(SeruroStateEvent &event)
{
    //AppAccountList::OnApplicationStateChange(event);
    event.Skip();
}

void ApplicationsPage::OnIdentityStateChange(SeruroStateEvent &event)
{
    AppAccountList::OnIdentityStateChange(event);
    
    if (AppAccountList::HasAnyAssigned()) {
        /* This may have caused the automatic assigning (closing an application). */
        this->wizard->SetButtonText(wxEmptyString, "Next >");
    }
    
    event.Skip();
}

void ApplicationsPage::OnAccountSelected(wxListEvent &event)
{
    long index = event.GetIndex();
    
    if (! AppAccountList::SelectAccount(index)) {
        event.Veto();
        return;
    }
    
    /* Check if identity is unstalled (note: can assign does not matter for initial). */
    assign_button->Enable(
        //theSeruroApps::Get().CanAssign(this->app_name) &&
        accounts_list->GetItemData(index) != APP_ASSIGNED
    );
    
    unassign_button->Enable(
        //theSeruroApps::Get().CanUnassign(this->app_name) &&
        accounts_list->	GetItemData(index) == APP_ASSIGNED
    );
}

void ApplicationsPage::OnAccountDeselected(wxListEvent &event)
{
    /* No need to reset the account name, it is static. */
    this->app_name = wxEmptyString;
}

void ApplicationsPage::OnAssign(wxCommandEvent &event)
{
    if (! AppAccountList::Assign()) {
        /* If a restart was required, pause the app and wait for a choice? */
        return;
    }
    
    /* Todo: Should the assign controller create a failure alert? */
    
    /* Change the next text to indicate the assignment was successful. */
    this->wizard->SetButtonText(wxEmptyString, _("&Next >"));
}

void ApplicationsPage::OnUnassign(wxCommandEvent &event)
{
    AppAccountList::Unassign();
}

void ApplicationsPage::OnRefresh(wxCommandEvent &event)
{
    /* focus already re-generates the account list for us. */
    this->DoFocus();
}

void ApplicationsPage::DoFocus()
{
    wxArrayString whitelist;
    
    /* Set account from setup wizard. */
    this->account = wizard->GetAccount();
    whitelist.Add(this->account);
    
    AppAccountList::SetAccountWhitelist(whitelist);
    AppAccountList::GenerateAccountsList();

    if (AppAccountList::HasAnyAssigned()) {
        /* This may have caused the automatic assigning (closing an application). */
        this->wizard->SetButtonText(wxEmptyString, "Next >");
    }
    
    /* Make sure the list fits correctly. */
    this->Layout();
    this->AlignList();
    this->Layout();
}

void ApplicationsPage::AlignList()
{
    wxListCtrl * lists[] = {AppAccountList::accounts_list};
    MaximizeAndAlignLists(lists, 1, 1);
}

void ApplicationsPage::EnablePage()
{
    
}

void ApplicationsPage::DisablePage()
{
    
}

ApplicationsPage::ApplicationsPage(SeruroSetup *parent) : SetupPage(parent)
{
    wxSizer *const vert_sizer = new wxBoxSizer(wxVERTICAL);
    
    /* Show a skip, until they assign, unless there is already an application assigned. */
	this->next_button = _("&Skip");
    
    this->enable_next = true;
    this->enable_prev = false;
    
    /* Use false to display the name of the accounts, since this is bound to an addres. */
    /* Use true for initial so that assigned applications will be restarted. */
    AppAccountList::Create(this, false, true);
    //AppAccountList::CreateHelper();
    
    /* Generic explaination. */
    Text *msg = new Text(this, TEXT_ASSIGN_IDENTITY);
    vert_sizer->Add(msg, DIALOGS_SIZER_OPTIONS);
    
    /* Show each account for this address in a list. */
    AppAccountList::AddAccountList(vert_sizer);
    
    /* Show an assign/unassign button. */
    wxSizer *const actions_sizer = new wxBoxSizer(wxHORIZONTAL);
	assign_button = new wxButton(this, BUTTON_ASSIGN, _("Configure"));
	assign_button->Disable();
	unassign_button = new wxButton(this, BUTTON_UNASSIGN, _("Unassign"));
	unassign_button->Disable();
    refresh_button = new wxButton(this, BUTTON_REFRESH, _("Refresh"));
    
    /* Align these buttons to the right. */
    actions_sizer->AddStretchSpacer();
    actions_sizer->Add(assign_button, DIALOGS_SIZER_OPTIONS);
    actions_sizer->Add(unassign_button, DIALOGS_SIZER_OPTIONS);
    actions_sizer->Add(refresh_button, DIALOGS_SIZER_OPTIONS);
    vert_sizer->Add(actions_sizer, DIALOGS_SIZER_OPTIONS.FixedMinSize().Bottom());
    
    /* Relevant state events. */
    wxGetApp().Bind(SERURO_STATE_CHANGE, &ApplicationsPage::OnIdentityStateChange, this, STATE_TYPE_IDENTITY);
    wxGetApp().Bind(SERURO_STATE_CHANGE, &ApplicationsPage::OnApplicationStateChange, this, STATE_TYPE_APPLICATION);
    /* Why would the account/server change, unless someone is playing with the config. */
    
    this->SetSizer(vert_sizer);
}

