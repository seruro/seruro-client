
#ifndef H_SeruroImageDefs
#define H_SeruroImageDefs

/* Include image data. */

/* Used for search, to match the size of a checkbox. */
#include "../resources/images/certificate_icon_18_flat.png.h"

#if defined(__WXMSW__)

#include "../resources/images/msw_icons/blank_icon.png.h"
#include "../resources/images/msw_icons/certificate_icon_flat.png.h"
#include "../resources/images/msw_icons/identity_icon_flat.png.h"

/* Status (for accounts/identities) */
#include "../resources/images/msw_icons/check_icon_flat.png.h" /* check */
#include "../resources/images/msw_icons/cross_icon_flat.png.h" /* x */
#include "../resources/images/msw_icons/check_empty_icon_flat.png.h" /* unchecked */

#elif defined(__WXOSX__) || defined(__WXMAC__)

#include "../resources/images/osx_icons/blank_icon.png.h"
#include "../resources/images/osx_icons/certificate_icon_flat.png.h"
#include "../resources/images/osx_icons/identity_icon_flat.png.h"

/* Status (for accounts/identities) */
#include "../resources/images/osx_icons/check_icon_flat.png.h" /* check */
#include "../resources/images/osx_icons/cross_icon_flat.png.h" /* x */
#include "../resources/images/osx_icons/check_empty_icon_flat.png.h" /* unchecked */

#endif

#endif