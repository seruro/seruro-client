
#ifndef H_AppOSX_OutlookHelperBridge
#define H_AppOSX_OutlookHelperBridge

#include <wx/string.h>
#include <wx/memory.h>

namespace CocoaBridge {
    bool addContact(wxString address, wxString first_name, wxString last_name);
    //bool addContactCertificates(wxString address, wxMemoryBuffer auth_cert, wxMemoryBuffer enc_cert);
    //bool haveContact(wxString address);
}

#endif