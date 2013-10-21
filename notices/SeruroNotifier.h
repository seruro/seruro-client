#pragma once

#ifndef H_SeruroNotifier
#define H_SeruroNotifier

#include "Defs.h"

//#include <wx/textfile.h>
#include "wxJSON/wx/jsonval.h"

/**
 * The Seruro notice framework provides OS-specific alerting as well as an action manager
 * facilitated by the toolbar widget. Notices are more than state events in that they may
 * require attention, but will at least require user action. 
 *
 * Notices include:
 *   Personal certificate revocation.
 *   Personal account banning (deletion will not be detected).
 *   Server CA (root) removal.
 *   Certificate approval & account approval.
 *   Server subscription expiration.
 *   Server not found (unabled to be reached).
 */

/**
 * Notifier display/actions:
 *   The notifier will place notices in the OS-agnostic tray icon list.
 *   There will be an initial heading with the current notices, which can be cleared.
 *   The tray bar icon will flash an (x) for a critical problem, or (!) for attention.
 * Notices will also implement OS-specific information alerting (libnotify, growl).
 */

/* Notice events:
 *   Notices are generated like state events, in response to some server monitor feedback
 *   or as a response to an action taken by the user. 
 * Server monitor: server status, subscription, request approval, account approval, CA removal.
 *  account banned, certificate revocations (personal and contacts).
 *  - Note: this may be able to function in a verbose mode where the notices are generated for
 *  new contacts as well.
 * Crypto monitor: certificate removal (from key store), additional seruro certificates
 * Application monitor: applications becoming "unassigned" of their security profiles.
 */

