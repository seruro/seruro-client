
#ifndef H_CheckedListCtrl
#define H_CheckedListCtrl

#include <wx/listctrl.h>
#include <wx/imaglist.h>

/* The side of a square checkbox. */
#define CHECKBOX_SIZE 18

enum checked_list_ids_t
{
	SERURO_SEARCH_LIST_ID
};

/* The checked list control enhances the normal control to include a 
 * checkbox for each item within the report. 
 * http://wiki.wxwidgets.org/WxListCtrl#Implement_wxListCtrl_with_Checkboxes
 */
class CheckedListCtrl : public wxListCtrl
{
public:
	CheckedListCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pt,
		const wxSize& sz, long style);
    
	/* Replace the mouse event to capture clicks within the checkbox area. */
	void OnMouseEvent(wxMouseEvent &event);
	/* Do not allow the user to resize the first column. */
	void OnColumnDrag(wxListEvent &event);

	/* Provide helper methods which toggle the checkbox. */
	bool IsChecked(long item) const;
    void DoCheck(long item, bool checked);
    void SetCheck(long item, bool checked);
    void SetCheck(const wxString &address, bool checked);
    
	/* Remove results from servers which may not exist. */
	void FilterResultsByServers(wxArrayString servers);

private:
	/* Save an imagelist of rendered checkbox states. */
    wxWindow* parent;
    wxImageList m_imageList;
    
    DECLARE_EVENT_TABLE();
};

#endif 