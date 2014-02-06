#include "windows.h"

#define EVEFILEDESC "CCP EVE Localization\0"
#ifndef _DEBUG
#define EVEINTFILENAME "EveLocalization\0"
#define EVEFILENAME "EveLocalization.dll\0"
#else
#define EVEINTFILENAME "EveLocalization_d\0"
#define EVEFILENAME "EveLocalization_d.dll\0"
#endif
#define EVEFILETYPE VFT_DLL

#include "autoversion.h"
//standard file version thing
#include "../version/evebuildver.h"
