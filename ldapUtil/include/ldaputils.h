#include <objbase.h>
#include <wchar.h>
#include <activeds.h>
#include <sddl.h>
#include <comutil.h>
#include <string.h>
#include <stdio.h>
#include "atlstr.h"
#include<string>
#include<vector>

HRESULT FetchObjects(IDirectorySearch *pContainerToSearch,CStringW searchFilter,std::vector<CStringW> &rows);
HRESULT ConnectToAD(IDirectorySearch **pContainerToSearch,CStringW ldapPath);