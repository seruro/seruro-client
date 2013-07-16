
#ifndef H_SeruroAPIUtils
#define H_SeruroAPIUtils

void URLEncode(char* dest, const char* source, int length);

wxJSONValue parseResponse(wxString raw_response);

wxJSONValue performRequest(wxJSONValue params);

wxString encodeData(wxJSONValue data);

//wxJSONValue getAuthFromPrompt(wxString &server,
//    const wxString &address = wxEmptyString, int selected = 0);

#endif