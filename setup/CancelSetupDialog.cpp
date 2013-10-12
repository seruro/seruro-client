
#include "CancelSetupDialog.h"

#define SETUP_CANCEL_TITLE "Do you want to exit Seruro?"

#define SETUP_CANCEL_TEXT "You are about to cancel the initial Seruro client setup.\
If you cancel without adding an account the application will quit.\
You may start Seruro again at anytime. Are you sure you want to cancel the setup and quit Seruro?"

CancelSetupDialog::CancelSetupDialog(wxWindow *parent)
  : wxMessageDialog(parent, wxEmptyString, _(SETUP_CANCEL_TITLE), wxSTAY_ON_TOP | wxNO_DEFAULT | wxYES_NO)
{
    this->SetTitle(_(SETUP_CANCEL_TITLE));
    this->SetMessage(_(SETUP_CANCEL_TEXT));
}
