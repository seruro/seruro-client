
/* First thing, detect OS */
#if defined(__WXMSW__)

#ifndef H_SeruroCryptoMSW
#define H_SeruroCryptoMSW

class SeruroCryptoMSW
{
public:
	SeruroCryptoMSW() {}
	void OnInit();

	/* For easy overloading access. */
	void SeruroCryptoMSW::TLSRequest(wxString &p_serverAddress, 
		int p_options, wxString &p_verb, wxString &p_object);
	void TLSRequest(wxString &p_serverAddress, 
		int p_options, wxString &p_verb, wxString &p_object, 
		wxString &p_data);

};

#endif 

#endif /* __WXMSW__ */