
#include "../crypto/SeruroCrypto.h"
#include "../logging/SeruroLogger.h"
//#include "../frames/dialogs/AuthDialog.h"

#include "../wxJSON/wx/jsonreader.h"

#include <wx/string.h>
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
        wxLogStatus(wxT("Utils> (parseResponse) could not parse response data as JSON."));
    }
	if (! response.HasMember("success")) {
		wxLogMessage(wxT("Utils> (parseResponse) response does not contain a 'success' key."));
		/* Fill in JSON data with TLS/connection error. */
		response["success"] = false;
        if (! response.HasMember("error")) {
            response["error"] = wxT("Connection failed.");
        }
	}
    
    /* Error may be a string, dict, or list? */
    if (! response["success"].AsBool()) {
		wxLogMessage(wxT("Utils> (parseResponse) request failed: (%s)."), response["error"].AsString());
	}
    
    /* Todo: consider checking for error string that indicated invalid token. */
    
    return response;
}

wxJSONValue performRequest(wxJSONValue params)
{
    wxJSONValue response;
    wxString raw_response;
    
	/* Get a crypto helper object for TLS requests. */
	SeruroCrypto crypto;
    
	/* Show debug statement, list the request. */
	wxLogMessage(wxT("Utils> (performRequest) https://%s:%d%s (%s)."),
        params["server"]["host"].AsString(), params["server"]["port"].AsInt(),
        params["object"].AsString(), params["verb"].AsString());
    
	/* Perform the request, receive a raw content (string) response. */
	raw_response = crypto.TLSRequest(params);
    
    /* Try to parse response as JSON (on failure, bail, meaning fill in JSON error ourselves). */
    response = parseResponse(raw_response);
    
    return response;
}

wxString encodeData(wxJSONValue data)
{
    wxString      data_string;
    wxString      data_value;
	wxArrayString data_keys;
	char *encoded_value;
	char *decoded_value;
	
	/* Construct a URL query string from JSON. */
	data_keys = data.GetMemberNames();
	for (size_t i = 0; i < data_keys.size(); i++) {
		if (i > 0) {
            data_string = wxString::Format(wxT("%s&"), data_string);
        }
		/* Value must be URLEncoded (malloc * 3 as each char might be encode to %xx). */
		data_value = data[data_keys[i]].AsString();
        encoded_value = (char *) malloc((data_value.Length()+1) * sizeof(char) * 3);
		//decoded_value = data_value.mb_str(wxConvUTF8);

		URLEncode(encoded_value, data_value.ToAscii(), (data_value.Length()+1) * 3);
        
		data_string = wxString::Format(wxT("%s%s=%s"), data_string, data_keys[i], 
			wxString::FromAscii(encoded_value));
		delete encoded_value;
	}

	LOG(_("data_string: %s"), data_string);
    return data_string;
}

