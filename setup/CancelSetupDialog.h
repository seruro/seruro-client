
#ifndef H_SeruroCancelSetupDialog
#define H_SeruroCancelSetupDialog

#include <wx/msgdlg.h>
#include <wx/checkbox.h>

/* Need to access default port. */
//#include "../Defs.h"

//#include "../wxJSON/wx/jsonval.h"

class CancelSetupDialog : public wxMessageDialog
{
public:
    CancelSetupDialog(wxWindow *parent);
};

#endif