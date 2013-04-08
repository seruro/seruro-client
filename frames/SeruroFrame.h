
#ifndef H_SeruroFrame
#define H_SeruroFrame

/*
#include "SeruroFrameMain.h"
#include "SeruroPanelConfigure.h"
#include "SeruroPanelDecrypt.h"
#include "SeruroPanelEncrypt.h"
#include "SeruroPanelSearch.h"
#include "SeruroPanelUpdate.h"
*/

#include <wx/frame.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/bookctrl.h>

class SeruroFrame : public wxFrame
{
public:
	SeruroFrame(const wxString &title) : wxFrame(NULL, wxID_ANY, title)
	{
		/* All frames should use a default vertical sizer. */
		wxBoxSizer *vertSizer = new wxBoxSizer(wxVERTICAL);
		
		/* This may be taken out later. */
		mainSizer = new wxBoxSizer(wxHORIZONTAL);
		vertSizer->Add(mainSizer, 1, wxEXPAND, 5);

		this->SetSizer(mainSizer);
	}

protected:
	/* All the derived classes to use an already existing mainSizer. */
	wxBoxSizer *mainSizer;
};

class SeruroPanel : public wxPanel
{
public:
	SeruroPanel(wxBookCtrlBase *parent, const wxString &title) :
	  wxPanel(parent, wxID_ANY) {
		/* Todo, the last param is the imageID, it's currently static. */
		parent->AddPage(this, title, false, 0);

		/* All panels should use a default vertical sizer. */
		wxBoxSizer *vertSizer = new wxBoxSizer(wxVERTICAL);
		
		/* This may be taken out later. */
		mainSizer = new wxBoxSizer(wxHORIZONTAL);
		vertSizer->Add(mainSizer, 1, wxEXPAND, 5);

		this->SetSizer(mainSizer);
	}

protected:
	/* All the derived classes to use an already existing mainSizer. */
	wxBoxSizer *mainSizer;
};


#endif
