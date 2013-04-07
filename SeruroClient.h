#pragma once

#ifndef H_SeruroClient
#define H_SeruroClient

#define SERURO_CONFIG_NAME  "SeruroClient.config"
#define SERURO_APP_NAME     "Seruro Client"

#include "wx/wxprec.h"
#include <wx/wx.h>

#include <wx/notebook.h>
#include <wx/log.h>

/* Remove when finished developing. */
#ifndef __WXDEBUG__
#define __WXDEBUG__ 1
#endif
#ifndef NDEBUG
#define NDEBUG 1
#endif

#if defined(__VLD__)
//#include <vld.h>
#endif

/* Sample icon to use for everything while testing. */
#include "resources/icon_good.xpm"

class SeruroConfig;

// Define a new application type, each program should derive a class from wxApp
class SeruroClient : public wxApp
{
public:
    // this one is called on application startup and is a good place for the app
    // initialization (doing it here and not in the ctor allows to have an error
    // return: if OnInit() returns false, the application terminates)
    
    /* Run networking thread from OnInit() */
    virtual bool OnInit();

private:
	SeruroConfig *config;
};

/* Config */
#include <boost/property_tree/ptree.hpp>
#include <wx/textfile.h>
#include "wxJSON/wx/jsonval.h"

class SeruroConfig
{
public:
	SeruroConfig();

    /* OS locations:
       MSW(XP): <UserDir>/AppData/Roaming/Seruro/SeruroClient.config
       MSW(6+): <UserDir>/Application Data/Seruro/SeruroClient.config
       OSX: <UserDir>/Library/Seruro/SeruroClient.config
       LNX: <UserDir>/.seruro/SeruroClient.config
     */
	void LoadConfig();
	void WriteConfig();
    bool HasConfig();

private:
    bool configValid;
    wxTextFile *configFile;
	wxJSONValue configData;
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