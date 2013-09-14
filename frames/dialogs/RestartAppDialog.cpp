
#include "RestartAppDialog.h"
#include "../UIDefs.h"

#include <wx/sizer.h>
//#include "../../lang/en_US.h"

#define TEXT_RESTART_WARNING_TITLE "These applications must be closed before the software can be installed:"
#define TEXT_RESTART_WARNING_NOTE  "If you don't want to close these applications now, choose Install Later."

#define RESTART_WARNING_CANCEL "Install Later"
#define RESTART_WARNING_ACCEPT "Close Applications and Install"

RestartAppDialog::RestartAppDialog(wxWindow *parent, const wxString &app_name)
  //: wxDialog(parent, wxID_ANY, _("Restart Pending"), wxDefaultPosition, wxSize(300, -1),
  //      wxSTAY_ON_TOP | wxDIALOG_EX_METAL | wxDEFAULT_DIALOG_STYLE), /* | wxYES_DEFAULT | wxOK | wxCANCEL */
    : wxMessageDialog(parent, wxEmptyString, wxEmptyString, wxSTAY_ON_TOP | wxOK_DEFAULT | wxOK | wxCANCEL),
    app_name(app_name)
{
    //wxSizer *vert_sizer = new wxBoxSizer(wxVERTICAL);
    
    wxString ok_text = wxString::Format(_("Restart %s"), this->app_name);
    wxString cancel_text = "Cancel";
    
    /* Set message. */
    this->SetMessage(wxString::Format(_(TEXT_RESTART_WARNING), this->app_name));
    this->SetOKCancelLabels(ok_text, cancel_text);
    
    //vert_sizer->Add(new Text(this, wxString::Format(_(TEXT_RESTART_WARNING), this->app_name)), DIALOGS_BOXSIZER_OPTIONS);
    //vert_sizer->Add(CreateButtonSizer(wxOK | wxCANCEL));
    
    //this->SetSizer(vert_sizer);
    //this->Center();
}

wxString RestartAppDialog::GetAppName()
{
    return this->app_name;
}