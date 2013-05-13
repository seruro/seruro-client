
#ifndef H_SeruroPanelSettings
#define H_SeruroPanelSettings

#include "SeruroFrame.h"

// Define a new frame type: this is going to be our main frame
class SeruroPanelSettings : public SeruroPanel
{
public:
    SeruroPanelSettings(wxBookCtrlBase *book);
};


class SettingsPanel : public wxPanel
{
public:
	SettingsPanel(SeruroPanelSettings *parent);
};

class SettingsPanelTree : public SettingsPanel
{
public:
	SettingsPanelTree(SeruroPanelSettings *parent);
};


#endif