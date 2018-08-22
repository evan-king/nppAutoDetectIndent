#include "stdafx.h"
#include "nppAutoDetectIndent.h"

#include <algorithm>
#include <cassert>
#include <type_traits>

#include <Shellapi.h>
#include <Strsafe.h>


#define PLUGIN_VERSION "1.0.1"
#define PLUGIN_HOMEPAGE "https://github.com/evan-king/nppAutoDetectIndent"

static const int MAX_LINES = 5000;

static const int MIN_INDENT = 2; // minimum width of a single indentation
static const int MAX_INDENT = 8; // maximum width of a single indentation

static const int MIN_DEPTH = MIN_INDENT; // ignore lines below this indentation level
static const int MAX_DEPTH = 3*MAX_INDENT; // ignore lines beyond this indentation level

static const char TAB = '\t';
static const char SPACE = ' ';

extern MyPlugin plugin;


namespace MenuAction
{
	void doNothing();
	void gotoWebsite();
}


nppAutoDetectIndent::IndentInfo nppAutoDetectIndent::getIndentInfo()
{
	const nppAutoDetectIndent::IndentType type = nppAutoDetectIndent::detectIndentType();
	switch (type)
	{
		case nppAutoDetectIndent::IndentType::Spaces:
			return IndentInfo {type, nppAutoDetectIndent::detectIndentSpaces()};
		case nppAutoDetectIndent::IndentType::Tabs:
			return IndentInfo {type, 0};
		default:
			return IndentInfo {nppAutoDetectIndent::IndentType::Invalid, 0};
	};
}

void nppAutoDetectIndent::applyIndentInfo(const nppAutoDetectIndent::IndentInfo &info)
{
	const HWND sciHwnd = plugin.getCurrentScintillaHwnd();
	switch (info.type)
	{
		case nppAutoDetectIndent::IndentType::Spaces:
		{
			::PostMessage(sciHwnd, SCI_SETTABINDENTS, true, 0);

			::PostMessage(sciHwnd, SCI_SETINDENT, info.num, 0);
			::PostMessage(sciHwnd, SCI_SETUSETABS, false, 0);
			break;
		}

		case nppAutoDetectIndent::IndentType::Tabs:
		{
			::PostMessage(sciHwnd, SCI_SETTABINDENTS, false, 0);

			// no need of SCI_SETINDENT
			::PostMessage(sciHwnd, SCI_SETUSETABS, true, 0);
			break;
		}

		case nppAutoDetectIndent::IndentType::Invalid:
		{
			break;  // do nothing
		}

		default:
			assert(false);
	};
}

int nppAutoDetectIndent::detectIndentSpaces()
{
	// return: number of spaces for indention

	const auto sciCall = plugin.getScintillaDirectCall();

	const int line_count = (std::min)(int(sciCall(SCI_GETLINECOUNT)), MAX_LINES);
	const float grace = float(line_count) / 50.0;

	int frequency[MAX_DEPTH+1] = {0}; // indentation => count(lines of that exact indentation)
	int margin[MAX_INDENT+1] = {0}; // indentation => count(lines with incompatible indentation)

	// track frequency of lines fitting each exact indentation level between DEPTH limits
	for(int i = 0; i < line_count; ++i) {
		const int indentWidth = int(sciCall(SCI_GETLINEINDENTATION, i));
		if(indentWidth < MIN_DEPTH || indentWidth > MAX_DEPTH) continue;

		const int linePos = int(sciCall(SCI_POSITIONFROMLINE, i));
		const char lineHeadChar = char(sciCall(SCI_GETCHARAT, linePos));
		if(lineHeadChar == TAB) continue;
		
		frequency[indentWidth]++;
	}

	// every counted line incompatible with this indentation level increases margin by one
	for(int i = MIN_DEPTH; i <= MAX_DEPTH; i++) {
		for(int k = MIN_INDENT; k <= MAX_INDENT; k++) {
			if(i % k == 0) continue;
			margin[k] += frequency[i];
		}
	}

	// choose the last indent with the smallest margin (ties go to larger indent)
	// Considers margins within grace of zero as =zero,
	// so occasional typos don't force smaller indentation
	int which = MIN_INDENT;
	for(int i = MIN_INDENT; i <= MAX_INDENT; ++i) {
		if(frequency[i] == 0) continue;
		if(margin[i] <= margin[which] || margin[i] < grace) which = i;
	}

	return which;
}

nppAutoDetectIndent::IndentType nppAutoDetectIndent::detectIndentType()
{
	// return -1: cannot decide
	// return  0: indent type is spaces
	// return  1: indent type is tabs

	const auto sciCall = plugin.getScintillaDirectCall();

	size_t tabs = 0;
	size_t spaces = 0;

	const int Lines = (std::max)(int(sciCall(SCI_GETLINECOUNT)), MAX_LINES);
	for (int i = 0; i < Lines; ++i)
	{
		const int indentWidth = int(sciCall(SCI_GETLINEINDENTATION, i));  // tabs width depends on SCI_SETTABWIDTH
		if (indentWidth < 2)
			continue;

		const int linePos = int(sciCall(SCI_POSITIONFROMLINE, i));
		const char lineHeadChar = char(sciCall(SCI_GETCHARAT, linePos));
		if (lineHeadChar == '\t')
			++tabs;
		else if (lineHeadChar == ' ')
		{
			if (indentWidth >(80 / 3))  // 1/3 of 80-width screen, this line must be alignment, skip
				;
			else
				++spaces;
		}
	}

	if ((tabs == 0) && (spaces == 0))
		return nppAutoDetectIndent::IndentType::Invalid;

	if (spaces > (tabs * 4))
		return nppAutoDetectIndent::IndentType::Spaces;
	else if (tabs > (spaces * 4))
		return nppAutoDetectIndent::IndentType::Tabs;
	else
	{
		bool useTab = (bool) sciCall(SCI_GETUSETABS);
		return useTab ? nppAutoDetectIndent::IndentType::Tabs : nppAutoDetectIndent::IndentType::Spaces;
	}
}


MyPlugin::MyPlugin()
{
	const FuncItem items[] = {
		funcItemCreate(TEXT("Version: " PLUGIN_VERSION), MenuAction::doNothing, NULL, false),
		funcItemCreate(TEXT("Goto website..."), MenuAction::gotoWebsite, NULL, false)
	};

	for (const auto &i : items)
		m_funcItems.emplace_back(i);
}

void MyPlugin::setupNppData(const NppData &data)
{
	m_nppData = data;
}

HWND MyPlugin::getNppHwnd() const
{
	return m_nppData._nppHandle;
}

HWND MyPlugin::getCurrentScintillaHwnd() const
{
	int view = -1;
	::SendMessage(m_nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM) &view);
	return (view == 0) ? m_nppData._scintillaMainHandle : m_nppData._scintillaSecondHandle;
}

MyPlugin::DirectCallFunctor MyPlugin::getScintillaDirectCall(HWND scintillaHwnd) const
{
	if (scintillaHwnd == NULL)
		scintillaHwnd = getCurrentScintillaHwnd();

	const SciFnDirect func = (SciFnDirect) ::SendMessage(scintillaHwnd, SCI_GETDIRECTFUNCTION, 0, 0);
	const sptr_t hnd = (sptr_t) ::SendMessage(scintillaHwnd, SCI_GETDIRECTPOINTER, 0, 0);

	return DirectCallFunctor(func, hnd);
}

FuncItem * MyPlugin::getFunctionsArray() const
{
	return (FuncItem *) m_funcItems.data();
}

size_t MyPlugin::functionsCount() const
{
	return m_funcItems.size();
}

FuncItem MyPlugin::funcItemCreate(const TCHAR *cmdName, const PFUNCPLUGINCMD pFunc, const bool check0nInit, ShortcutKey *sk)
{
	FuncItem item = {0};

	StringCchCopy(item._itemName, ARRAY_LENGTH(item._itemName), cmdName);
	item._pFunc = pFunc;
	item._init2Check = check0nInit;
	item._pShKey = sk;

	return item;
}


void MenuAction::doNothing()
{
	/*
	typedef std::chrono::high_resolution_clock Clock;
	auto t1 = Clock::now();
	// do something
	auto t2 = Clock::now();
	auto ms = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
	*/
}

void MenuAction::gotoWebsite()
{
	ShellExecute(NULL, TEXT("open"), TEXT(PLUGIN_HOMEPAGE), NULL, NULL, SW_SHOWDEFAULT);
}
