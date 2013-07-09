
#include "SeruroFrame.h"

#include <wx/bookctrl.h>
#include <wx/listctrl.h>

void MaximizeAndAlignLists(wxListCtrl **lists, size_t count, size_t first_column)
{
    // 
    /* Try to show all the values in the list (columns) with a preference for the first. */
    size_t first_min_size, first_column_size;
    
    for (size_t i = 0; i < count; i++) {
        first_column_size = ((wxListCtrl *) lists[i])->GetColumnWidth(first_column);
        first_min_size = (first_min_size > first_column_size) ? first_min_size : first_column_size;
    }
    
    /* Then try to give the first row the extra room in the list (pushing the rest to the right). */
    
}

SeruroFrame::SeruroFrame(const wxString &title) : wxFrame(NULL, wxID_ANY, title)
{
    /* All frames should use a default vertical sizer. */
    wxBoxSizer *vertSizer = new wxBoxSizer(wxVERTICAL);
    
    /* This may be taken out later. */
    mainSizer = new wxBoxSizer(wxHORIZONTAL);
    vertSizer->Add(mainSizer, 1, wxEXPAND, 5);
    
    this->SetSizer(vertSizer);
}

SeruroPanel::SeruroPanel(wxBookCtrlBase *parent, const wxString &title) : wxPanel(parent, wxID_ANY)
{
    /* Todo, the last param is the imageID, it's currently static. */
    parent->AddPage(this, title, false, 0);
    
    /* All panels should use a default vertical sizer. */
    mainSizer = new wxBoxSizer(wxVERTICAL);
    
    /* This may be taken out later. */
    //mainSizer = new wxBoxSizer(wxHORIZONTAL);
    //_mainSizer->Add(mainSizer, 1, wxLEFT | wxRIGHT | wxTOP | wxEXPAND, 5);
    
    this->SetSizer(mainSizer);
}
