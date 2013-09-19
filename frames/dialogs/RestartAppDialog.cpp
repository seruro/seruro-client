
#include "RestartAppDialog.h"
#include "../UIDefs.h"

#include "../../apps/SeruroApps.h"
#include "../../api/SeruroStateEvents.h"
#include "../../SeruroClient.h"

#include "../../setup/SeruroSetup.h"
#include "../SeruroMain.h"

#include <wx/sizer.h>
#include <wx/evtloop.h>

//#include "../../lang/en_US.h"

#define TEXT_RESTART_WARNING_TITLE "These applications must be closed before the software can be installed:"
#define TEXT_RESTART_WARNING_NOTE  "If you don't want to close these applications now, choose Install Later."

#define RESTART_WARNING_CANCEL "Install Later"
#define RESTART_WARNING_ACCEPT "Close Applications and Install"

DECLARE_APP(SeruroClient);

RestartAppDialog::RestartAppDialog(wxWindow *parent, const wxString &app_name)
  //: wxDialog(parent, wxID_ANY, _("Restart Pending"), wxDefaultPosition, wxSize(300, -1),
  //      wxSTAY_ON_TOP | wxDIALOG_EX_METAL | wxDEFAULT_DIALOG_STYLE), /* | wxYES_DEFAULT | wxOK | wxCANCEL */
    : wxMessageDialog(parent, wxEmptyString, wxEmptyString, wxSTAY_ON_TOP | wxOK_DEFAULT | wxOK | wxCANCEL),
    app_name(app_name), identity(wxEmptyString)
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

void RestartAppDialog::CloseModal()
{
    //wxEventLoopBase *loop;
    //int i = 0;
    
    //wxWindowModalDialogEvent close_event(wxEVT_WINDOW_MODAL_DIALOG_CLOSED);
    
    this->SendIdentityEvent();
    //this->ProcessEvent(close_event);
    //this->SendCloseButtonClickEvent();
    //EndDialog(0);
    //EmulateButtonClickIfPresent(wxID_OK);
    //EmulateButtonClickIfPresent(wxID_CANCEL);
    //this->Close();
    //this->EndModal(0);
    //this->Destroy();
    
    //wxCommandEvent cancelEvent(wxEVT_COMMAND_BUTTON_CLICKED, wxID_OK);
    //wxWindowModalDialogEvent cancelEvent(wxEVT_WINDOW_MODAL_DIALOG_CLOSED, wxID_OK);
    //cancelEvent.SetEventObject( this );
    //GetEventHandler()->ProcessEvent(cancelEvent);
    //this->SendCloseButtonClickEvent();
    
    //loop = wxEventLoopBase::GetActive();
    
    //wxEventLoopBase::GetActive()->Exit();
    //wxEventLoop::Exit();
    //this->Hide();
    //this->Destroy();
    //((SeruroSetup *) (((SeruroFrameMain *) wxGetApp().GetFrame())->GetTop()))->MainLoop();
}

void RestartAppDialog::SendIdentityEvent()
{
    /* The app was restarted, generate the appropriate event. */
    SeruroStateEvent identity_event(STATE_TYPE_IDENTITY, STATE_ACTION_UPDATE);
    identity_event.SetValue("app", this->app_name);
    identity_event.SetAccount(this->identity);
    
    /* A call to IdentityStatus with an initial boolean will send PENDING_RESTART.
     * This will allow a 'checker' to override.
     */
    identity_event.SetValue("assign_override", "true");
    wxGetApp().AddEvent(identity_event);
}

void RestartAppDialog::DialogClosed(wxWindowModalDialogEvent& event)
{
    RestartAppDialog* dialog = (RestartAppDialog *) event.GetDialog();
    if (dialog == NULL) {
        return;
    }
    
    theSeruroApps::Get().RestartDialogFinished();
    if (dialog->GetReturnCode() != wxID_OK) {
        /* Nothing to do. */
        return;
    }
    
    if (! theSeruroApps::Get().RestartApp(dialog->app_name)) {
        /* The restart did not succeed. */
        return;
    }
    
    this->SendIdentityEvent();
}

void RestartAppDialog::SetIdentity(wxString address)
{
    this->identity = address;
}

wxString RestartAppDialog::GetAppName()
{
    return this->app_name;
}