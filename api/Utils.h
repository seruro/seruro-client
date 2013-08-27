
#ifndef H_SeruroAPIUtils
#define H_SeruroAPIUtils

class wxString;
class wxJSONValue;
class wxMemoryBuffer;

const char* AsChar(const wxString &input);
wxMemoryBuffer AsBinary(wxString hex_string);
wxString AsHex(wxMemoryBuffer binary_string);

void URLEncode(char* dest, const char* source, int length);

wxJSONValue parseResponse(wxString raw_response);

wxJSONValue performRequest(wxJSONValue params);

wxString encodeData(wxJSONValue data);

//wxJSONValue getAuthFromPrompt(wxString &server,
//    const wxString &address = wxEmptyString, int selected = 0);

#endif