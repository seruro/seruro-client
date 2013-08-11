
#include "ErrorDialog.h"

ErrorDialog::ErrorDialog(wxWindow *parent, const wxString &message, bool allow_report)
	: wxMessageDialog(parent, message, wxMessageBoxCaptionStr, wxYES_NO | wxNO_DEFAULT | wxICON_EXCLAMATION)
{
	ButtonLabel send_report(_("Create Error Report"));
	ButtonLabel exit(_("Exit"));
	
	if (! allow_report) {
		this->Create(parent, wxID_ANY, _("Seruro Error"), wxDefaultPosition, wxDefaultSize,
			wxNO | wxICON_EXCLAMATION);
		this->SetMessage(message);
	}

	this->SetYesNoLabels(send_report, exit);
}

void ErrorDialog::SendReport()
{
	/* Use the message and data provided during initialization,
	 * along with the data available in the log to create and send
	 * and error report.
	 *
	 * Alternatively, launch a second modal/dialog for the Debug report.
	 */
}