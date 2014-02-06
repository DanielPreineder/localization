#pragma once
#ifndef WrapPointList_H
#define WrapPointList_H

#include <usp10.h>

// -------------------------------------------------------------
// Description:
//   Provides extended information from the script analysis.
// -------------------------------------------------------------
struct ItemRun
{
	SCRIPT_ANALYSIS		analysis; // http://msdn.microsoft.com/en-us/library/dd368797.aspx
	int					charPos;
	int					len;      // length of run in WCHARs
};

// -------------------------------------------------------------
// Description:
//   A Collator performs locale-sensitive string comparisons.
// -------------------------------------------------------------
BLUE_CLASS( WrapPointList) : public IRoot
{
public:
	EXPOSE_TO_BLUE();

	WrapPointList( IRoot* lockobj = 0 );
	~WrapPointList();

	const SCRIPT_LOGATTR& operator[]( const size_t index ) const
	{
		return m_wrapPointList[index];
	}

	size_t Size() const
	{
		return m_wrapPointListCount;
	}

	friend PyObject* Py__init__(PyObject *self, PyObject *args);

private:
	void MergeSimpleScripts();
	void BuildItemRunList();

	bool TextAnalyze( WCHAR* wstr, int wlen, SCRIPT_CONTROL* scriptControl, SCRIPT_STATE* scriptState );

	SCRIPT_LOGATTR* m_wrapPointList;
	int             m_wrapPointListCount;

	// The itemRun list that will be converted into the WrapPointList
	int      m_itemRunCount;
	ItemRun* m_itemRunList;

	// List of script items, will be converted into an ItemRun list
	SCRIPT_ITEM* m_tempItemList;
	int m_tempItemCount;
	int m_tempItemAllocLen;
};
TYPEDEF_BLUECLASS( WrapPointList );

#endif // WrapPointList_H
