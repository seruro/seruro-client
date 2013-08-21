/* Remember to set enable_prev to false. */

#include "SeruroSetup.h"
#include "../SeruroClient.h"

DECLARE_APP(SeruroClient);

ApplicationsPage::ApplicationsPage(SeruroSetup *parent) : SetupPage(parent)
{
    wxSizer *const vert_sizer = new wxBoxSizer(wxVERTICAL);
    
    /* Show a skip, until they assign, unless there is already an application assigned. */
	this->next_button = _("&Skip");
    
    this->enable_next = true;
    this->enable_prev = false;
    
    /* Generic explaination. */
    Text *msg = new Text(this, TEXT_ASSIGN_IDENTITY);
    vert_sizer->Add(msg, DIALOGS_SIZER_OPTIONS);
    
    /* Show each account for this address in a list. */
    
    /* Show an assign/unassign button. */
    
    this->SetSizer(vert_sizer);
}
