
#ifndef H_SeruroPanelSettings
#define H_SeruroPanelSettings

#include "SeruroFrame.h"

#define SERURO_SETTINGS_MIN_WIDTH 150

// Define a new frame type: this is going to be our main frame
class SeruroPanelSettings : public SeruroPanel
{
public:
    SeruroPanelSettings(wxBookCtrlBase *book);
};


class SettingsPanel : public wxPanel
{
public:
	SettingsPanel(wxWindow *parent);
};

class SettingsPanelTree : public wxPanel
{
public:
	SettingsPanelTree(wxWindow *parent);
};


#endif