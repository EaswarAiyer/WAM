#include "ldaputils.h"


using namespace std;

HRESULT ConnectToAD(IDirectorySearch **pContainerToSearch,CStringW ldapPath)
{
	HRESULT hr = S_OK;
	hr = ADsOpenObject(ldapPath.GetBuffer(),
				NULL,
				NULL,
				ADS_SECURE_AUTHENTICATION, //  Use Secure Authentication.
				IID_IDirectorySearch,
				(void**)pContainerToSearch);
	return hr;
}
HRESULT FetchObjects(IDirectorySearch *pContainerToSearch,CStringW searchFilter,std::vector<CStringW> &rows)
{
	if (!pContainerToSearch)
		return E_POINTER;
	DWORD dwLength = (MAX_PATH * 2);

	LPOLESTR pszSearchFilter = new OLECHAR[dwLength];

	pszSearchFilter = searchFilter.GetBuffer();
	
	ADS_SEARCHPREF_INFO SearchPrefs[2];
	SearchPrefs[0].dwSearchPref = ADS_SEARCHPREF_SEARCH_SCOPE;
	SearchPrefs[0].vValue.dwType = ADSTYPE_INTEGER;
	SearchPrefs[0].vValue.Integer = ADS_SCOPE_SUBTREE;

	SearchPrefs[1].dwSearchPref = ADS_SEARCHPREF_PAGESIZE;
    SearchPrefs[1].vValue.dwType = ADSTYPE_INTEGER;
    SearchPrefs[1].vValue.Integer = 1000;
	DWORD dwNumPrefs = 2;

	LPOLESTR pszColumn = NULL;
	ADS_SEARCH_COLUMN col;
	HRESULT hr = S_OK;

	IADs  *pObj = NULL;
	IADs  * pIADs = NULL;
	CStringW temp;

	ADS_SEARCH_HANDLE hSearch = NULL;
	hr = pContainerToSearch->SetSearchPreference(SearchPrefs, dwNumPrefs);
	if (FAILED(hr))
		return hr;

	LPOLESTR pszNonVerboseList[] = { L"name", L"distinguishedName" };

	LPOLESTR szName = new OLECHAR[MAX_PATH];
	LPOLESTR szDN = new OLECHAR[MAX_PATH];
		
	int iCount = 0;
	DWORD x = 0L;
	
	hr = pContainerToSearch->ExecuteSearch(pszSearchFilter,
		pszNonVerboseList,
		sizeof(pszNonVerboseList) / sizeof(LPOLESTR),
		&hSearch
		);
	printf("execute search\n");
	if (SUCCEEDED(hr))
	{		
		while (SUCCEEDED(hr = pContainerToSearch->GetNextRow(hSearch)))
			{
				//  Keep track of count.
				if(S_ADS_NOMORE_ROWS == hr)
				{
					DWORD 
						dwError = ERROR_SUCCESS;
					WCHAR 
						szError[512],
						szProvider[512];

					ADsGetLastError(&dwError, szError, 512, szProvider, 512);
					if(ERROR_MORE_DATA != dwError)
					{
						break;
					}
				}
				else if( hr == S_OK)
				{
					iCount++;
				}
				else
				{
					printf("break called\n");
					break;
				}
				printf("before column fetch\n");
				while (pContainerToSearch->GetNextColumnName(hSearch, &pszColumn) != S_ADS_NOMORE_COLUMNS)
				{	
					hr = pContainerToSearch->GetColumn(hSearch, pszColumn, &col);
					if (SUCCEEDED(hr))
					{
						if (0 == wcscmp(L"name", pszColumn))
						{
							szName = col.pADsValues->CaseIgnoreString;
						}
						if (0 == wcscmp(L"distinguishedName", pszColumn))
						{
							szDN = col.pADsValues->CaseIgnoreString;
						}
			
						pContainerToSearch->FreeColumn(&col);
					}
					FreeADsMem(pszColumn);
				}
				
				wprintf(L"%s\n  DN: %s\n\n", szName, szDN);
				temp = szName;
				rows.push_back(temp);
				
			}		
		pContainerToSearch->CloseSearchHandle(hSearch);
	}
	else
	{
		printf("execute search failed\n");
	}
	if (SUCCEEDED(hr) && 0 == iCount)
		hr = S_FALSE;
	return hr;
}

