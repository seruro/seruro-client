
#ifndef H_SeruroServerAPI
#define H_SeruroServerAPI

#include <wx/socket.h>

/* The SeruroServerAPI is a socket-based RESTful client for the SeruroServer.
 * When the SeruroClient must make an API call a thread is spawned (or run)
 * which takes the call command and optional parameters and data.
 */



class SeruroRequest : public wxThread
{
public:
	SeruroRequest() : wxThread() {}
	virtual ~SeruroRequest();

	virtual void *Entry(); /* thread execution starts */
	/* Todo: consider an OnExit() is the thread can be terminated by user action. */

private:
	unsigned m_count; /* not sure? */
	/* Todo: consider naming the frame that spawns the request.
	 * to provide a progress update? */
};

class SeruroServerAPI;


#endif
