
#include "SettingsWindows.h"
#include "../SeruroPanelSettings.h"
#include "../UIDefs.h"

SettingsView::SettingsView(SeruroPanelSettings *window) : parent(window),
    wxWindow(window, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME) {
        SetBackgroundColour(_("white"));
    }

GeneralWindow::GeneralWindow(SeruroPanelSettings *window) : SettingsView(window)
{
    wxSizer *const sizer = new wxBoxSizer(wxHORIZONTAL);
    
	//wxButton *button = new wxButton(this, wxID_ANY, _("General"));
    //sizer->Add(button, DIALOGS_SIZER_OPTIONS);

    this->SetSizer(sizer);
}

