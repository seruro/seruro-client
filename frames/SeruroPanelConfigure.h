
#include <wx/wx.h>
#include <wx/icon.h>

#include "../SeruroClient.h"

#ifndef H_SeruroFrameConfigure
#define H_SeruroFrameConfigure

// Define a new frame type: this is going to be our main frame
class SeruroFrameConfigure : public SeruroFrame
{
public:
    SeruroFrameConfigure(const wxString& title);
};

#endif