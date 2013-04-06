
#include "SeruroSetup.h"

SeruroSetup::SeruroSetup(SeruroFrameMain *parent)
{
	this->SetExtraStyle(wxWIZARD_EX_HELPBUTTON);

	this->Create(parent, wxID_ANY, wxT("Seruro Client Setup"),
		/* Todo: replace icon */
		wxIcon(icon_good), wxDefaultPosition,
		wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);

	this->page1 = new SeruroSetupAlert(this);

}
