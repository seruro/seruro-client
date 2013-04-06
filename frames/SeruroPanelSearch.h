
#include <wx/wx.h>

#include "../SeruroClient.h"

#ifndef H_SeruroFrameSearch
#define H_SeruroFrameSearch

// Define a new frame type: this is going to be our main frame
class SeruroFrameSearch : public SeruroFrame
{
public:
    // ctor(s)
    SeruroFrameSearch(const wxString& title);

private:
    DECLARE_EVENT_TABLE()
};

#endif