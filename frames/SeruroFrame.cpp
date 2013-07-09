
#include "SeruroFrame.h"

#include <wx/bookctrl.h>
#include <wx/listctrl.h>

size_t GetMaxColumnWidth(wxListCtrl *list, int col)
{
	size_t width;

	/* Get the width for the header. */
	list->SetColumnWidth(col, wxLIST_AUTOSIZE_USEHEADER);
	width = list->GetColumnWidth(col);

	/* Compare to the width for the data, take the larger. */
	list->SetColumnWidth(col, wxLIST_AUTOSIZE);
    width = (width > list->GetColumnWidth(col)) ? width : list->GetColumnWidth(col);
	return width;
}

void MaximizeAndAlignLists(wxListCtrl **lists, size_t count, size_t first_column)
{
    /* Try to show all the values in the list (columns) with a preference for the first. */
	size_t first_column_width = 0, extra_column_width = 0, min_width = 0;
    size_t min_total_width = 0, total_width = 0, buffer_width = 0;
	wxListCtrl *the_list;

    for (size_t i = 0; i < count; i++) {
		the_list = ((wxListCtrl *) lists[i]);
		if (the_list->GetSizer() != 0) the_list->GetSizer()->Layout();

		/* Get the width of the list (starting from first column. */
		total_width = the_list->GetSize().GetWidth();
		for (size_t j = 0; j < first_column; j++) {
			/* Expect the columns to the left of the first to be sized. */
			//the_list->SetColumnWidth(j, wxLIST_AUTOSIZE_USEHEADER);

			/* Total width does not include columns before the first. */
			total_width -= the_list->GetColumnWidth(j);
		}
		min_total_width = (total_width > min_total_width) ? total_width : min_total_width;

		/* Get the width of the first column. */
		first_column_width = GetMaxColumnWidth(the_list, first_column);
		/* Set the first column to take up the entire space, to force extra column size. */
		the_list->SetColumnWidth(first_column, total_width);

		/* Resize all columns to the right of the first column. */
		buffer_width = total_width;
		for (int j = first_column+1; j < the_list->GetColumnCount(); j++) {
			extra_column_width = GetMaxColumnWidth(the_list, j);
			the_list->SetColumnWidth(j, extra_column_width);
			/* Decrease the available buffer. */
			buffer_width = (buffer_width - extra_column_width > 0) ? buffer_width - extra_column_width : 0;
		}

		/* Calculate the smallest acceptable size amoung all lists. */
		buffer_width -= LISTCTRL_ALIGN_RIGHT_PADDING;
		first_column_width = (buffer_width > first_column_width) ? buffer_width : first_column_width;
		if (i == 0) min_width = first_column_width;
		min_width = (min_width > first_column_width) ? first_column_width : min_width;
    }
    
	/* Finally set the width of all first columns to the minimum width. */
	for (size_t i = 0; i < count; i++) {
		the_list = ((wxListCtrl *) lists[i]);

		the_list->SetColumnWidth(first_column, min_width);
	}    
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
