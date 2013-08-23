
#ifndef H_AlertDialog
#define H_AlertDialog

#include <wx/frame.h>
#include <wx/dialog.h>

#include <wx/custombgwin.h>


class AlertDialog : public wxDialog
{
public:
    AlertDialog();
    
private:
    void CreateRoundedCorners();
};

#endif
