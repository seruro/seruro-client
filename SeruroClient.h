#pragma once

#ifndef H_SeruroClient
#define H_SeruroClient

#include "wx/wxprec.h"
#include <wx/wx.h>

#include <wx/notebook.h>

#if defined(__VLD__)
#include <vld.h>
#endif

/* Sample icon to use for everything while testing. */
#include "resources/icon_good.xpm"

// Define a new application type, each program should derive a class from wxApp
class SeruroClient : public wxApp
{
public:
    // this one is called on application startup and is a good place for the app
    // initialization (doing it here and not in the ctor allows to have an error
    // return: if OnInit() returns false, the application terminates)
    virtual bool OnInit();

};

class SeruroFrame : public wxFrame
{
public:
	SeruroFrame(const wxString &title) : wxFrame(NULL, wxID_ANY, title) {}
};

class SeruroPanel : public wxPanel
{
public:
	SeruroPanel(wxBookCtrlBase *parent, const wxString &title);
};

#endif