#include "stdafx.h"
#include "WrapPointList.h"

#ifdef _WIN32

// DOM-IGNORE-BEGIN
WrapPointList::WrapPointList( IRoot* lockobj /* = 0 */ )
:   m_itemRunCount( 0 ),
    m_itemRunList( nullptr ),
    m_tempItemList( nullptr ),
    m_tempItemAllocLen( 0 ),
    m_tempItemCount( 0 ),
    m_wrapPointList( nullptr ),
    m_wrapPointListCount( 0 )
{
}

WrapPointList::~WrapPointList()
{
	if ( m_wrapPointList )
	{
		CCP_DELETE[] m_wrapPointList;
	}

	if ( m_itemRunList )
	{
		CCP_DELETE[] m_itemRunList;
	}

	if ( m_tempItemList )
	{
		CCP_FREE( ( void* ) m_tempItemList );
	}
}
// DOM-IGNORE-END

// -------------------------------------------------------------
// Description:
//   Traverses the script analysis result and merges non-complex,
//   composited symbols into a single one.
// See also:
//   Microsoft's Uniscribe API, specifically: 
//     http://msdn.microsoft.com/en-us/library/dd368556.aspx
//     http://msdn.microsoft.com/en-us/library/dd319118.aspx
// -------------------------------------------------------------
void WrapPointList::MergeSimpleScripts()
{
	// get pointer to the global script table, used for merging non-complex runs together
	const SCRIPT_PROPERTIES **propList;
	int propCount;

	ScriptGetProperties( &propList, &propCount );

	//	coalesce item-runs that are based on simple-scripts
	for ( int i = 0; i < m_tempItemCount - 1; i++ )
	{
		// use each item-run's SCRIPT_ANALYSIS::eScript member to lookup the
		// appropriate script in the global-table
		if ( propList[m_tempItemList[i+0].a.eScript]->fComplex == 0 && 
			 propList[m_tempItemList[i+1].a.eScript]->fComplex == 0 )
		{
			// be careful which SCRIPT_ITEM we overwrite as we need to
			// maintain correct iCharPos members
			memmove( &m_tempItemList[i+1], &m_tempItemList[i+2], ( m_tempItemCount-i-1 ) * sizeof( SCRIPT_ITEM ) );
			m_tempItemCount--;

			m_tempItemList[i].a.eScript = SCRIPT_UNDEFINED;
		}
	}
}

// -------------------------------------------------------------
// Description:
//   Converts a script analysis list into an ItemRun list
// See also:
//   Microsoft's Uniscribe API, specifically: 
//     http://msdn.microsoft.com/en-us/library/dd368556.aspx
//     http://msdn.microsoft.com/en-us/library/dd319118.aspx
// -------------------------------------------------------------
void WrapPointList::BuildItemRunList()
{
	int mergePos = 0;
	int i = 0;

	m_itemRunList = CCP_NEW( "EveLocalization::WrapPointList::BuildItemRunList" ) ItemRun[m_tempItemCount];
	while ( i < m_tempItemCount )
	{
		memset( &m_itemRunList[m_itemRunCount], 0, sizeof( ItemRun ) );
		m_itemRunList[m_itemRunCount].analysis = m_tempItemList[i].a;

		int itemPos = m_tempItemList[i].iCharPos;
		int itemLen = m_tempItemList[i+1].iCharPos - itemPos;

		m_itemRunList[m_itemRunCount].charPos = mergePos;
		m_itemRunList[m_itemRunCount].len = ( itemPos + itemLen ) - mergePos;
		i++;

		mergePos += m_itemRunList[m_itemRunCount].len;
		m_itemRunCount++;
	}
}

// -------------------------------------------------------------
// Description:
//   Runs the actual analysis on the text
// Arguments:
//   wstr - the text that shall be analyzed
//   wlen - length of said text
//   scriptControl - SCRIPT_CONTROL settings, see MSDN
//   scriptState - SCRIPT_STATE settings, see MSDN
// Return value:
//   True if analysis was successful, False otherwise
// See also:
//   Microsoft's Uniscribe API, specifically: 
//     http://msdn.microsoft.com/en-us/library/dd368556.aspx
//     http://msdn.microsoft.com/en-us/library/dd319118.aspx
// -------------------------------------------------------------
bool WrapPointList::TextAnalyze ( WCHAR* wstr, int wlen, SCRIPT_CONTROL* scriptControl, SCRIPT_STATE* scriptState )
{
	CCP_STATS_ZONE( __FUNCTION__ );

	if ( wstr == 0 || wlen == 0 )
	{
		return true;
	}

	int allocLen = std::max(m_tempItemAllocLen, 16);

	HRESULT hr;

	//	Create the Uniscribe SCRIPT_ITEM list which just describes
	//  the spans of plain unicode text (grouped by script)
	do
	{
		// allocate memory for SCRIPT_ITEM list
		if ( m_tempItemAllocLen < allocLen )
		{
			m_tempItemList = ( SCRIPT_ITEM* ) CCP_REALLOC( "EveLocalization::TextAnalyze::temptItemList",
													       m_tempItemList, allocLen * sizeof( SCRIPT_ITEM ) );
			
			if( m_tempItemList == 0 )
			{
				return false;
			}

			// store this temporary-item list so we don't have to free/alloc all the time
			m_tempItemAllocLen = allocLen;
		}

		hr = ScriptItemize( wstr, wlen, allocLen, scriptControl, scriptState, m_tempItemList, &m_tempItemCount );

		if ( hr != S_OK && hr != E_OUTOFMEMORY )
		{
			return false;
		}

		allocLen *= 2;
	}
	while ( hr != S_OK );

	// avoid indicating softbreaks in the middle of symbols
	MergeSimpleScripts();

	//  Convert to item-runs which contain length information
	BuildItemRunList();

	return true;
}

#endif
