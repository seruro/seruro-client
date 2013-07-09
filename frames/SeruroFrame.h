
#ifndef H_SeruroFrame
#define H_SeruroFrame

#include <wx/frame.h>
#include <wx/panel.h>
#include <wx/sizer.h>

#define LISTCTRL_ALIGN_RIGHT_PADDING 5

class wxBookCtrlBase;
class wxListCtrl;

/*** Utility functions. ***/

void MaximizeAndAlignLists(wxListCtrl **lists, size_t count, size_t first_column = 0);

/*** End: utility functions. ***/

class SeruroFrame : public wxFrame
{
public:
	SeruroFrame(const wxString &title);

protected:
	/* All the derived classes to use an already existing mainSizer. */
	wxBoxSizer *mainSizer;
};

class SeruroPanel : public wxPanel
{
public:
	SeruroPanel(wxBookCtrlBase *parent, const wxString &title);

protected:
	/* All the derived classes to use an already existing mainSizer. */
	wxBoxSizer *mainSizer;
	wxBoxSizer *_mainSizer;
};


#endif
