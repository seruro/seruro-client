
#include "SettingsWindows.h"
#include "../SeruroSettings.h"
#include "../UIDefs.h"

ExtensionsWindow::ExtensionsWindow(SeruroPanelSettings *window) : SettingsView(window)
{
    wxSizer *const sizer = new wxBoxSizer(wxHORIZONTAL);
    
    Text *warning = new Text(this, _("Extensions are disabled."));
    sizer->Add(warning, DIALOGS_SIZER_OPTIONS);
    
    this->SetSizer(sizer);
}
