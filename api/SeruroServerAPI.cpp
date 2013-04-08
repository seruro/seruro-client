
#include "SeruroServerAPI.h"
#include "../SeruroClient.h"

DECLARE_APP(SeruroClient);

SeruroRequest::~SeruroRequest()
{
	/* Start client (all) threads accessor, to delete this, with a critial section. */
	wxCriticalSectionLocker locker(wxGetApp().seruro_critSection);

	wxArrayThread& threads = wxGetApp().seruro_threads;
	threads.Remove(this);

	/* Todo: (check for an empty threads array) might want to signal to main thread if
	 * this was the last item, meaning it is OK to shutdown if it were waiting. */
	
}

wxThread::ExitCode SeruroRequest::Entry()
{
	wxLogMessage("Seruro Thread started...");

	/* Do some stuff */

	wxLogMessage("Seruro Thread finished...");
}
