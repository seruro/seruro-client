
#include "../crypto/SeruroCrypto.h"
#include "../frames/dialogs/AuthDialog.h"

#include "../wxJSON/wx/jsonreader.h"

#include <wx/log.h>
//#include "../wxJSON/wx/jsonwriter.h"

const char hexa[] = {"0123456789ABCDEFabcdef"};
const char nonencode[] = {"abcdefghijklmnopqrstuvwqyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.-_~"};

void URLEncode(char* dest, const char* source, int length)
{
	/* Used to encode POST data. */
	int destc = 0, i, a, clear = 0;
    
	for (i = 0; (destc + 1) < length && source[i]; i++) {
		clear=0;
        for (a = 0; nonencode[a]; a++) {
            if (nonencode[a] == source[i]) {
                clear=1;
                break;
            }
        }
		if (! clear) {
            if((destc + 3) >= length) break;
            
			dest[destc] = '%';
            dest[destc+1] = hexa[source[i]/16];
            dest[destc+2] = hexa[source[i]%16];
            destc += 3;
		} else {
			dest[destc] = source[i];
            destc++;
		}
	}
    
	dest[destc] = '\0';
}

wxJSONValue parseResponse(wxString raw_response)
{
    wxJSONValue response;
    wxJSONReader response_reader;
    int num_errors;
    
    /* Parse the response (raw content) string into JSON. */
    num_errors = response_reader.Parse(raw_response, &response);
    if (num_errors > 0) {
        wxLogStatus(wxT("SeruroRequest (parse)> could not parse response data as JSON."));
    }
	if (! response.HasMember("success")) {
		wxLogMessage(wxT("SeruroRequest (parse)> response does not contain a 'success' key."));
		/* Fill in JSON data with TLS/connection error. */
		response["success"] = false;
        if (! response.HasMember("error")) {
            response["error"] = wxT("Connection failed.");
        }
	}
    
    /* Error may be a string, dict, or list? */
    if (! response["success"].AsBool()) {
		wxLogMessage(wxT("SeruroRequest (parse)> failed: (%s)."), response["error"].AsString());
	}
    
    /* Todo: consider checking for error string that indicated invalid token. */
    
    return response;
}

wxJSONValue performRequest(wxJSONValue params)
{
    wxJSONValue response;
    wxString raw_response;
    
	/* Get a crypto helper object for TLS requests. */
	SeruroCrypto *cryptoHelper = new SeruroCrypto();
    
	/* Show debug statement, list the request. */
	wxLogMessage(wxT("SeruroRequest (request)> Server (%s), Verb (%s), Object (%s)."),
                 params["server"]["host"].AsString(), params["verb"].AsString(), params["object"].AsString());
    
	/* Perform the request, receive a raw content (string) response. */
	raw_response = cryptoHelper->TLSRequest(params);
    
	delete [] cryptoHelper;
    
    /* Try to parse response as JSON (on failure, bail, meaning fill in JSON error ourselves). */
    response = parseResponse(raw_response);
    
    return response;
}

wxString encodeData(wxJSONValue data)
{
	wxString data_string;
	wxArrayString data_names;
	
	wxString data_value;
	char *encoded_value;
    
	encoded_value = (char *) malloc(256 * sizeof(char)); /* Todo: increate size of buffer. */
	
	/* Construct a URL query string from JSON. */
	data_names = data.GetMemberNames();
	for (size_t i = 0; i < data_names.size(); i++) {
		if (i > 0) data_string = data_string + wxString(wxT("&"));
		/* Value must be URLEncoded. */
		data_value = data[data_names[i]].AsString();
		URLEncode(encoded_value, data_value.mbc_str(), 256);
        
		data_string = data_string + data_names[i] + wxString(wxT("=")) + wxString(encoded_value);
	}
	delete encoded_value;
    
	wxLogMessage(wxT("SeruroRequest (encode)> Encoded: (%s)."), data_string);
    
	return data_string;
}

wxJSONValue getAuthFromPrompt(wxString &server, const wxString &address = wxEmptyString, int selected = 0)
{
    wxJSONValue auth;
    
    /* Selected forces the user to use the provided address. Otherwise they may choose any. */
	AuthDialog *dialog = new AuthDialog(server, address, selected);
	if (dialog->ShowModal() == wxID_OK) {
		wxLogMessage(wxT("SeruroServerAPI::getAuthFromPrompt> OK"));
		auth = dialog->GetValues();
	}
	/* Todo: password_control potentially contains a user's password, ensure proper cleanup. */
	delete dialog;
    
	return auth;
}
