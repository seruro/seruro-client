
#include "RestartAppDialog.h"
//#include "../../lang/en_US.h"

RestartAppDialog::RestartAppDialog(wxWindow *parent, const wxString &app_name)
  : wxMessageDialog(parent, wxEmptyString, wxMessageBoxCaptionStr,
        wxSTAY_ON_TOP | wxYES_DEFAULT | wxOK | wxCANCEL)
{
    wxString ok_text = wxString::Format(_("Restart %s"), app_name);
    wxString cancel_text = "Cancel";
    
    /* Set message. */
    this->SetMessage(wxString::Format(_(TEXT_RESTART_WARNING), app_name));
    this->SetOKCancelLabels(ok_text, cancel_text);
}