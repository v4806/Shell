#include <pch.h>
#include "Include/ContextMenu.h"
#include "Library/detours.h"
#include "RegistryConfig.h"
#include <UIAutomation.h>
#include <propkey.h>      // PKEY_AppUserModel_ID（固定快捷方式时要用）
#include <shlobj.h>       // CSIDL_APPDATA / IShellLink
#include <shobjidl.h>     // IStartMenuPinnedList（官方「从任务栏取消固定」API）
using namespace Nilesoft::Shell;
using namespace Diagnostics;

//MS also offers EV Code Certificates, which enable immediate acceptance by SmartScreeen of Windows Defender
//https://blogs.msdn.microsoft.com/ie/2012/08/14/microsoft-smartscreen-extended-validation-ev-code-signing-certificates/

#ifdef _DEBUG
#include <crtdbg.h>
#endif
/*
Windows 10
Version Build
-------------
1507	10240
1511	10586
1607	14393
1703	15063
1709	16299
1803	17134
1809	17763
1903	18362
1909	18363
2004	19041
20H2	19042
21H1	19043
21H2	19044
22H2	19045
*/

//#pragma comment(lib, "mincore.lib")
//#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "UxTheme.lib")
#pragma comment(lib, "Comctl32.lib")
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "WindowsCodecs.lib")
#pragma comment(lib, "d2d1")
#pragma comment(lib, "dwrite")
#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "oleaut32.lib")
#pragma comment(lib, "Shlwapi.lib")

#if defined(_M_ARM64)
#pragma comment(lib, "plutosvg-arm64.lib")
#pragma comment(lib, "detours-arm64.lib")
#elif defined(_M_ARM)
#pragma comment(lib, "plutosvg-arm.lib")
#pragma comment(lib, "detours-arm.lib")
#elif defined(_M_X64)
#pragma comment(lib, "plutosvg-x64.lib")
#pragma comment(lib, "detours-x64.lib")
#elif defined(_M_IX86)
#pragma comment(lib, "plutosvg-x86.lib")
#pragma comment(lib, "detours-x86.lib")
#endif

//#pragma optimize("ts", on) 

/*
#if defined(_M_X64)
#pragma comment(linker, "/export:DllCanUnloadNow=_DllCanUnloadNow")
#else
#pragma comment(linker, "/export:DllCanUnloadNow=__DllCanUnloadNow@0")
#endif
*/

#ifdef _DEBUG

std::unordered_map< uint32_t, const wchar_t *> msg_map0 = {
	{ WM_DESTROY, L"WM_DESTROY" },
	{ WM_NCDESTROY, L"WM_NCDESTROY" },
	{ WM_CREATE, L"WM_CREATE" },
	{ WM_NCCREATE, L"WM_NCCREATE" },
	{ WM_PAINT, L"WM_PAINT" },
	{ WM_NCPAINT, L"WM_NCPAINT" },
	{ WM_PRINTCLIENT, L"WM_PRINTCLIENT" },
	{ WM_PRINT, L"WM_PRINT" },
	{ WM_NCCALCSIZE, L"WM_NCCALCSIZE" },
	{ WM_ERASEBKGND, L"WM_ERASEBKGND" },
	{ MN_SELECTITEM, L"MN_SELECTITEM" },
	{ WM_KEYDOWN, L"WM_KEYDOWN" },
	{ WM_WINDOWPOSCHANGED, L"WM_WINDOWPOSCHANGED" },
	{ WM_WINDOWPOSCHANGING, L"WM_WINDOWPOSCHANGING" },
	{ WM_NCHITTEST, L"WM_NCHITTEST" },
	{ WM_TIMER, L"WM_TIMER" },
	{ WM_SIZE, L"WM_SIZE" },
	{ WM_MOVE, L"WM_MOVE" },
	{ MN_SIZEWINDOW, L"MN_SIZEWINDOW" },
	{ MN_DBLCLK, L"MN_DBLCLK" },
	{ MN_CLOSEHIERARCHY, L"MN_CLOSEHIERARCHY" },
	{ MN_SETTIMERTOOPENHIERARCHY, L"MN_SETTIMERTOOPENHIERARCHY" },
	{ MN_FINDMENUWINDOWFROMPOINT, L"MN_FINDMENUWINDOWFROMPOINT" },
	{ MN_ENDMENU, L"MN_ENDMENU" },
	{ MN_CANCELMENUS, L"MN_CANCELMENUS" },
	{ MN_BUTTONDOWN, L"MN_BUTTONDOWN" },
	{ MN_BUTTONUP, L"MN_BUTTONUP" },
	{ WM_UAHDESTROYWINDOW, L"WM_UAHDESTROYWINDOW" },
	{ WM_UAHMEASUREMENUITEM, L"WM_UAHMEASUREMENUITEM" },
	{ WM_UAHINITMENUPOPUP, L"WM_UAHINITMENUPOPUP" },
	{ WM_UAHNCPAINTMENUPOPUP, L"WM_UAHNCPAINTMENUPOPUP" },
	{ WM_UAHDRAWMENU, L"WM_UAHDRAWMENU" },
	{ WM_NCUAHDRAWFRAME, L"WM_NCUAHDRAWFRAME" },
	{ WM_UAHDRAWMENUITEM, L"WM_UAHDRAWMENUITEM" },
	{ WM_ENTERMENULOOP, L"WM_ENTERMENULOOP" },
	{ WM_EXITMENULOOP, L"WM_EXITMENULOOP" },
	{ WM_INITMENU, L"WM_INITMENU" },
	{ WM_INITMENUPOPUP, L"WM_INITMENUPOPUP" },
	{ WM_UNINITMENUPOPUP, L"WM_UNINITMENUPOPUP" },
	{ WM_MENUSELECT, L"WM_MENUSELECT" },
	{ WM_NEXTMENU, L"WM_NEXTMENU" },
	{ WM_MENUCHAR, L"WM_MENUCHAR" },
	{ WM_MEASUREITEM, L"WM_MEASUREITEM" },
	{ WM_DRAWITEM, L"WM_DRAWITEM" },
	{ WM_MENUCOMMAND, L"WM_MENUCOMMAND" },
	{ WM_COMMAND, L"WM_COMMAND" },
	{ WM_SYSCOMMAND, L"WM_SYSCOMMAND" },
	{ WM_MENURBUTTONUP, L"WM_MENURBUTTONUP" },
	{ WM_CAPTURECHANGED, L"WM_CAPTURECHANGED" },
	{ WM_ENTERIDLE, L"WM_ENTERIDLE" },
	{ WM_SETCURSOR, L"WM_SETCURSOR" },
	{ WM_SHOWWINDOW , L"WM_SHOWWINDOW" },
	{ WM_PARENTNOTIFY, L"WM_PARENTNOTIFY"},
	{ WM_MOUSEACTIVATE, L"WM_MOUSEACTIVATE"},
	{ WM_CONTEXTMENU, L"WM_CONTEXTMENU"},
	{ WM_NOTIFY, L"WM_NOTIFY"},
	{ WM_SETFOCUS, L"WM_SETFOCUS"},
	{ WM_KILLFOCUS, L"WM_KILLFOCUS"},
	{ WM_GETOBJECT, L"WM_GETOBJECT"},
	{ WM_ACTIVATE, L"WM_ACTIVATE"},
	{ WM_NCACTIVATE, L"WM_NCACTIVATE"},
	{ WM_ACTIVATEAPP, L"WM_ACTIVATEAPP"},
	{ WM_CHANGEUISTATE, L"WM_CHANGEUISTATE"},
	{ WM_IME_SETCONTEXT, L"WM_IME_SETCONTEXT"},
	{ WM_IME_NOTIFY, L"WM_IME_NOTIFY"},
	{ WM_MOUSEMOVE, L"WM_MOUSEMOVE"},
	{WM_DEVICECHANGE, L"WM_DEVICECHANGE"},
	{WM_IME_REQUEST, L"WM_IME_REQUEST"},
	{MN_GETHMENU, L"MN_GETHMENU"},
	{WM_LBUTTONDOWN, L"WM_LBUTTONDOWN"},
	{WM_RBUTTONDOWN, L"WM_RBUTTONDOWN"},
	{WM_LBUTTONUP, L"WM_LBUTTONUP"},
	{WM_RBUTTONUP, L"WM_RBUTTONUP"},
	{WM_MOUSELEAVE,L"WM_MOUSELEAVE"},
	{WM_USER, L"WM_USER"},
	{WM_APP, L"WM_APP"},
	{ 0, nullptr }
};

#endif

#pragma region Entry Point

#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))  // windowsx.h
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))  // windowsx.h

HINSTANCE _hInstance{};
Initializer _initializer;
extern Logger &_log = Logger::Instance();
const Windows::Version *ver = &Windows::Version::Instance();

HANDLE watch_event = nullptr;

WindowsHook _taskbar_mouse;

//std::unordered_map<HMODULE, IATHook> _detours_ci;
IATHook iathook_NtUserTrackPopupMenuEx;
std::vector<IATHook> iathook_TrackPopupMenu; //OverrideFunction
//Detours<decltype(::TrackPopupMenu)> _TrackPopupMenu;
//Detours<decltype(::TrackPopupMenuEx)> _TrackPopupMenuEx;
Detours<decltype(::DllGetClassObject)> _DllGetClassObject;
Detours<decltype(::CoCreateInstance)> _CoCreateInstance;

uint32_t MN_CONTEXTMENU = 0;
std::unordered_map<HWND, Window> _window_taskbar;

LRESULT __stdcall TaskbarSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
LRESULT __stdcall TaskbarProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
// [PROBE] Win10 任务栏按钮区（MSTaskListWClass）探针
LRESULT __stdcall TaskListProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static void tasklist_hook_all(uint32_t delay);

__inline auto is_registered() -> bool
{
	//auto_regkey r;
	//return ERROR_SUCCESS == auto_regkey::exists_key(HKCR, HKCR_CLSID_ContextMenu);
	return RegistryConfig::IsRegistered();
}

struct
{
	string path;
	bool explorer{};
	bool contextmenuhandler{};
	HMODULE handle{};

	bool init()
	{
		handle = ::GetModuleHandleW(nullptr);
		if(!(explorer) && path.empty())
		{
			path = Path::Module(nullptr);
			explorer = path.ends_with(def_EXPLORER, true);
		}
		return explorer;
	}

	explicit operator bool() const { return explorer; }

} _loader;

struct taskbar_t
{
	inline static IComPtr<IUIAutomation> _IUIAutomation;
	inline static auto wShell_TrayWnd = ::RegisterWindowMessageW(Windows::WC_Shell_TrayWnd);
	inline static auto wShell_SecondaryTrayWnd = ::RegisterWindowMessageW(Windows::WC_Shell_SecondaryTrayWnd);

	static bool hook(HWND hTaskbar, UINT_PTR id)
	{
		Window window = Window::Find(Windows::WC_Composition_DesktopWindowContentBridge, hTaskbar);
		if(window && !window.is_prop(UxSubclass))
		{
			if(!_window_taskbar.contains(window))
			{
				if(window.subclass(TaskbarSubclassProc, id, hTaskbar))
				{
					window.prop(UxSubclass, CONTEXTMENUSUBCLASS);
					_window_taskbar[window] = window;
					return true;
				}
			}
		}
		return false;
	}

	static void try_hook(HWND hTaskbar, UINT_PTR id, int time = 500)
	{
		if(hTaskbar)
		{
			::SetTimer(hTaskbar, id, time, [](HWND hWnd, UINT, UINT_PTR id, DWORD)
			{
				static int atttempt = 0;
				if(hook(hWnd, id))
					atttempt = 50;
				if(atttempt++ >= 50)
					::KillTimer(hWnd, id);
			});
		}
	}

	static void hook_all()
	{
		::EnumWindows([](HWND hWnd, LPARAM)->BOOL {
			auto atom = Window::Atom(hWnd);
			if(atom == wShell_TrayWnd || atom == wShell_SecondaryTrayWnd)
			{
				Window window = hWnd;
				if(!window.is_prop(UxSubclass))
				{
					if(!_window_taskbar.contains(window))
					{
						auto proc = window.get_long(GWLP_WNDPROC);
						window.prop(UxSubclass, proc);
						window.set_long(GWLP_WNDPROC, TaskbarProc);
						_window_taskbar[window] = window;
						return true;
					}
				}
			}
			return TRUE;
		}, 0);
	}

	static void hook_all(uint32_t delay)
	{
		::SetTimer(::GetShellWindow(), 0, delay, [](HWND hWnd, UINT, UINT_PTR id, DWORD)
		{						
			static int atttempt = 0;
			if(atttempt++ >= 10)
				::KillTimer(hWnd, id);
			
			taskbar_t::hook_all();
		});
	}
	
	static void unhook(HWND hWnd)
	{
		for(auto &window : _window_taskbar)
		{
			if(window.first == hWnd)
			{
				window.second.set_long(GWLP_WNDPROC, window.second.prop(UxSubclass));
				window.second.remove_prop(UxSubclass);
				_window_taskbar.erase(hWnd);
				break;
			}
		}
	}
	static void unhook_all()
	{
		for(auto &window : _window_taskbar)
		{
			window.second.set_long(GWLP_WNDPROC, window.second.prop(UxSubclass));
			window.second.remove_prop(UxSubclass);
			_window_taskbar.erase(window.first);
			break;
		}
		_window_taskbar.clear();
	}

	static bool is_allowed_area(HWND hTaskbar, const Point &pt)
	{
		bool ret = false;

		if(!_IUIAutomation)
			_IUIAutomation.CreateInstance(__uuidof(CUIAutomation), CLSCTX_INPROC_SERVER);

		if(_IUIAutomation)
		{
			IComPtr<IUIAutomationElement> pIUIAutomationElement;
			if(SUCCEEDED(_IUIAutomation->ElementFromPoint(pt, pIUIAutomationElement)) && pIUIAutomationElement)
			{
				struct elem_t {
					BSTR type = nullptr;
					BSTR name = nullptr;
					BSTR id = nullptr;
					~elem_t() {
						freeString(type);
						freeString(name);
						freeString(id);
					}
					void freeString(BSTR s) {
						if(s) DLL::Invoke(L"OleAut32.dll", "SysFreeString", s);
					}
				} elem;
				
				pIUIAutomationElement->get_CurrentAutomationId(&elem.id);
				pIUIAutomationElement->get_CurrentClassName(&elem.type);
				pIUIAutomationElement->get_CurrentName(&elem.name);
				/*
				id=[TaskbarFrame], type=[Taskbar.TaskbarFrameAutomationPeer], name=[]
				id=[StartButton], type=[ToggleButton], name=[Start]
				id=[SearchButton], type=[ToggleButton], name=[Search]
				id=[SystemTrayIcon], type=[SystemTray.NormalButton], name=[Show Hidden Icons]
				id=[SystemTrayIcon], type=[SystemTray.AccentButton], name=[Network wifi-1 Internet access]
				id=[SystemTrayIcon], type=[SystemTray.OmniButton], name=[Clock 7:04 PM 3/20/2023]
				id=[SystemTrayIcon], type=[SystemTray.ShowDesktopButton], name=[Show Desktop]
				*/
				if(elem.name && elem.type && elem.id)
				{
					if(!::lstrcmpiW(elem.id, L"TaskbarFrame"))
						ret = !::lstrcmpiW(elem.name, L"") && !::lstrcmpiW(elem.type, L"Taskbar.TaskbarFrameAutomationPeer");
					else if(is_secondary(hTaskbar))
					{
						if(!::lstrcmpiW(elem.id, L"SystemTrayIcon"))
							ret = !::lstrcmpiW(elem.type, L"SystemTray.OmniButton");
						//_log.info(L"id=[%s],\t[%s], [%s]", elem.id, elem.type, elem.name);
					}
				}
			}
		}
		return ret;
	}

	static bool is_primary(HWND hTaskbar)
	{
		if(hTaskbar)
		{
			Window taskbar = hTaskbar;
			if(taskbar.hash() == WindowClass::WC_Shell_TrayWnd)
			{
				// Only the primary taskbar has a RebarWindow32
				return taskbar.find(Windows::WC_ReBarWindow32);
			}
		}
		return false;
	}

	static bool is_secondary(HWND hTaskbar)
	{
		return hTaskbar && (Window::class_hash(hTaskbar) == WindowClass::WC_Shell_SecondaryTrayWnd);
	}
};

// ===== Win10 任务栏按钮（MSTaskListWClass）右键接管 =====
// 前置：这两个符号原本定义/声明在本文件更靠后的位置（约 515 行）
extern bool is_in_taskbar;
extern bool in_taskbar_button_menu;
// 定义在文件后部（约 1630 行），此处前置声明给按钮菜单与 command.kill_process_tree 用
extern std::vector<HWND> taskbar_button_windows;
BOOL WINAPI TrackPopupMenuExProc(HMENU hMenu, uint32_t uFlags, int x, int y, HWND hWnd, LPTPMPARAMS lptpm);
// 注意：subclass 标记不能沿用 UxSubclass —— ContextMenu::Initialize() 里会用
// `if(0 == ::GetPropW(hwnd.owner, UxSubclass))` 判断该窗口是否已由 Shell 处理过，
// 命中就【跳过 build_system_menuitems】，于是我们传进去的菜单项会被完全忽略。
constexpr auto TaskListSubclassProp = L"ShellTaskListSubclass";

// 原理：Win10 的按钮菜单是 ShellExperienceHost 的 XAML 跳转列表（不是 Win32 菜单），Hook 不到。
// 这里直接 subclass 任务栏按钮区窗口，拦下 WM_CONTEXTMENU 并吞掉，然后自己造一个 HMENU 交给 Shell：
// Shell 会抓快照并按自己的主题重绘 → 菜单变成 Shell 样式，而内容由我们决定。
struct taskbar_button_info
{
	bool is_button{};   // 只有点在真正的应用按钮上才为 true
	// 注意：UIA 对任务栏按钮返回的 pid 是 shell 进程，不是应用进程，不能用来定位窗口
	int pid{};
	HWND window{};              // 主窗口（第一个匹配到的，优先非最小化）
	std::vector<HWND> windows;  // 所有匹配到的窗口：合并按钮要能「关闭所有窗口」
	string title;   // 形如 "任务管理器 - 1 个运行窗口"
};

static taskbar_button_info query_taskbar_button(const Point &pt)
{
	taskbar_button_info out;
	if(!taskbar_t::_IUIAutomation)
		taskbar_t::_IUIAutomation.CreateInstance(__uuidof(CUIAutomation), CLSCTX_INPROC_SERVER);
	if(!taskbar_t::_IUIAutomation)
		return out;

	IComPtr<IUIAutomationElement> elem;
	if(FAILED(taskbar_t::_IUIAutomation->ElementFromPoint(pt, elem)) || !elem)
		return out;

	// MSTaskListWClass 覆盖整个按钮区（含按钮之间的空白）。只有 ControlType 是按钮类
	// 才当成「应用按钮」；点在空白处必须放行，让 explorer 弹它自己的任务栏菜单（Shell 会接管）。
	// ⚠️ 应用有多个窗口（合并按钮）时，UIA 把任务栏按钮暴露成 MenuItem 而不是 Button
	// （实测 Edge 3 个窗口 → ControlType=MenuItem），只认 Button 会让合并按钮弹回原生跳转列表。
	CONTROLTYPEID ctype = 0;
	elem->get_CurrentControlType(&ctype);
	if(ctype != UIA_ButtonControlTypeId && ctype != UIA_MenuItemControlTypeId && ctype != UIA_SplitButtonControlTypeId)
		return out;

	BSTR name = nullptr;
	elem->get_CurrentName(&name);
	if(name)
	{
		out.title = name;
		DLL::Invoke(L"OleAut32.dll", "SysFreeString", name);
	}
	int epid = 0;
	elem->get_CurrentProcessId(&epid);
	out.pid = epid;
	out.is_button = !out.title.empty();
	return out;
}

struct taskbar_window_finder
{
	string keyword;   // 从按钮名解析出的应用名关键词
	bool file_explorer{};     // 该按钮是「文件资源管理器」→ 允许用窗口类认窗口（它没有任何 AppUserModel 属性）
	HWND hwnd{};              // 主窗口（优先非最小化）
	std::vector<HWND> all;    // 所有匹配到的窗口
};

// Chromium 系应用（Edge/Chrome）会在窗口标题里插入零宽字符：
// 实际标题是 "Microsoft<U+200B> Edge"，用 wcsstr 跟 "Microsoft Edge" 匹配必然失败。
// 比较前先去掉零宽字符、把非断空格换成普通空格。
static void normalize_for_match(wchar_t *s)
{
	if(!s)
		return;
	wchar_t *d = s;
	for(const wchar_t *p = s; *p; ++p)
	{
		if(*p == 0x200B || *p == 0x200C || *p == 0x200D || *p == 0xFEFF || *p == 0x2060)
			continue;
		*d++ = (*p == 0x00A0) ? L' ' : *p;
	}
	*d = 0;
}

// 窗口自己带着「这个应用在任务栏里是谁」的完整信息（都是公开属性，不用猜名字）：
//   AppUserModel.ID                          —— 分组主键
//   AppUserModel.RelaunchCommand             —— 任务栏算 AppID 也用它。缺了它，固定项会跟运行窗口
//                                               分成两个图标（实测 Blender：lnk 只有 ID、没这条，
//                                               target 还是 blender.exe 而窗口 relaunch 是 blender-launcher.exe）
//   AppUserModel.RelaunchDisplayNameResource —— 任务栏按钮名（实测 Blender 窗口='Blender 5.2'）
//   AppUserModel.RelaunchIconResource        —— 图标
struct window_appmodel
{
	wchar_t aumid[256]{};
	wchar_t relaunch_command[1024]{};
	wchar_t relaunch_display_name[512]{};
	wchar_t relaunch_icon[512]{};
};

static void read_prop_str(IPropertyStore *ps, REFPROPERTYKEY key, wchar_t *out, size_t outLen)
{
	out[0] = 0;
	PROPVARIANT pv{};
	if(SUCCEEDED(ps->GetValue(key, &pv)))
	{
		if(pv.vt == VT_LPWSTR && pv.pwszVal)
			::wcsncpy_s(out, outLen, pv.pwszVal, outLen - 1);
		::PropVariantClear(&pv);
	}
}

static bool get_window_appmodel(HWND hWnd, window_appmodel &out)
{
	if(!hWnd)
		return false;
	IPropertyStore *ps = nullptr;
	if(FAILED(::SHGetPropertyStoreForWindow(hWnd, IID_IPropertyStore, reinterpret_cast<void **>(&ps))) || !ps)
		return false;
	read_prop_str(ps, PKEY_AppUserModel_ID, out.aumid, 256);
	read_prop_str(ps, PKEY_AppUserModel_RelaunchCommand, out.relaunch_command, 1024);
	read_prop_str(ps, PKEY_AppUserModel_RelaunchDisplayNameResource, out.relaunch_display_name, 512);
	read_prop_str(ps, PKEY_AppUserModel_RelaunchIconResource, out.relaunch_icon, 512);
	ps->Release();
	// 注意：AUMID 为空也算成功（explorer 的窗口就是这样，一个属性都不带），
	// 调用方要自己回退到 exe 路径。
	return true;
}

// 查一个应用在 shell:AppsFolder 里的显示名 —— 任务栏按钮名用的就是它。
// 入参可以写 AppUserModelID（如 Microsoft.Windows.Explorer → 「文件资源管理器」），
// 也可以写 exe 路径（Blender 没注册 AUMID，按 ...\blender-launcher.exe → 「Blender 5.2」）。
static bool appsfolder_display_name(const wchar_t *aumid_or_path, wchar_t *out, size_t outLen)
{
	out[0] = 0;
	if(!aumid_or_path || !aumid_or_path[0])
		return false;

	wchar_t spec[1088]{};
	::swprintf_s(spec, 1088, L"shell:AppsFolder\\%s", aumid_or_path);
	IShellItem *item = nullptr;
	if(FAILED(::SHCreateItemFromParsingName(spec, nullptr, IID_IShellItem, reinterpret_cast<void **>(&item))) || !item)
		return false;

	bool ok = false;
	LPWSTR name = nullptr;
	if(SUCCEEDED(item->GetDisplayName(SIGDN_NORMALDISPLAY, &name)) && name)
	{
		::wcsncpy_s(out, outLen, name, outLen - 1);
		ok = out[0] != 0;
		::CoTaskMemFree(name);
	}
	item->Release();
	return ok;
}

// exe 全路径查不到时的兜底：AppsFolder 里的项**不一定**拿 exe 全路径当键 ——
// 例如 cmd.exe 是 shell:AppsFolder\{1AC14E77-02E7-4E5D-B744-2EB1AE5198B7}\cmd.exe
// （System32 的 KNOWNFOLDERID），拿 C:\Windows\system32\cmd.exe 去查必然失败。
// 所以这里按 exe 文件名在 AppsFolder 里找同名项，取它的显示名（cmd.exe → 「命令提示符」）。
// 整张表只建一次并缓存。
static const std::map<std::wstring, std::wstring> &appsfolder_name_map()
{
	static std::once_flag once;
	static std::map<std::wstring, std::wstring> cache;
	std::call_once(once, []()
	{
		IShellItem *folder = nullptr;
		if(FAILED(::SHCreateItemFromParsingName(L"shell:AppsFolder", nullptr, IID_IShellItem,
												reinterpret_cast<void **>(&folder))) || !folder)
			return;

		IEnumShellItems *en = nullptr;
		if(SUCCEEDED(folder->BindToHandler(nullptr, BHID_EnumItems, IID_IEnumShellItems,
										   reinterpret_cast<void **>(&en))) && en)
		{
			IShellItem *child = nullptr;
			while(en->Next(1, &child, nullptr) == S_OK)
			{
				LPWSTR path = nullptr;
				if(SUCCEEDED(child->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &path)) && path)
				{
					const wchar_t *file = ::wcsrchr(path, L'\\');
					file = file ? file + 1 : path;
					if(file[0] && cache.find(file) == cache.end())
					{
						LPWSTR name = nullptr;
						if(SUCCEEDED(child->GetDisplayName(SIGDN_NORMALDISPLAY, &name)) && name)
						{
							cache[file] = name;
							::CoTaskMemFree(name);
						}
					}
					::CoTaskMemFree(path);
				}
				child->Release();
			}
			en->Release();
		}
		folder->Release();
	});
	return cache;
}

static bool appsfolder_display_name_by_exe_filename(const wchar_t *exe_path, wchar_t *out, size_t outLen)
{
	out[0] = 0;
	if(!exe_path || !exe_path[0])
		return false;

	const wchar_t *file = ::wcsrchr(exe_path, L'\\');
	file = file ? file + 1 : exe_path;
	if(!file[0])
		return false;

	auto &map = appsfolder_name_map();
	auto it = map.find(file);
	if(it == map.end())
		return false;

	::wcsncpy_s(out, outLen, it->second.c_str(), outLen - 1);
	return out[0] != 0;
}

// 这个窗口在任务栏上叫的名字：先按 AUMID 查 AppsFolder，查不到再按 exe 路径查
static bool window_button_name(HWND hWnd, wchar_t *out, size_t outLen)
{
	out[0] = 0;
	window_appmodel am;
	get_window_appmodel(hWnd, am);   // 读不到就全是空串，下面按 exe 路径兜底
	if(am.aumid[0] && appsfolder_display_name(am.aumid, out, outLen))
		return true;

	DWORD pid = 0;
	::GetWindowThreadProcessId(hWnd, &pid);
	if(!pid)
		return false;
	HANDLE hp = ::OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
	if(!hp)
		return false;
	wchar_t exe[MAX_PATH]{};
	DWORD size = MAX_PATH;
	bool ok = ::QueryFullProcessImageNameW(hp, 0, exe, &size) != FALSE;
	::CloseHandle(hp);
	if(!ok)
		return false;

	if(appsfolder_display_name(exe, out, outLen))
		return true;

	// 控制台（cmd.exe）这类：AppsFolder 里不是用 exe 全路径当键，按文件名再找一次。
	return appsfolder_display_name_by_exe_filename(exe, out, outLen);
}

// 「文件资源管理器」是唯一必须特判的应用：它的窗口**一个 AppUserModel 属性都不带**
// （实测 AUMID vt=0），`shell:AppsFolder\C:\Windows\explorer.exe` 也查不到（未注册），
// 而任务栏照样把它显示成「文件资源管理器」/「Windows 资源管理器」—— 那是 Windows 内部规则。
// 所以这里按名字认按钮，窗口匹配再配合窗口类 CabinetWClass。
static bool is_file_explorer_button(const wchar_t *button_name)
{
	if(!button_name || !button_name[0])
		return false;

	wchar_t key[256]{};
	const wchar_t *sep = ::wcsstr(button_name, L" - ");
	size_t n = sep ? static_cast<size_t>(sep - button_name) : ::wcslen(button_name);
	if(n == 0 || n >= 256)
		return false;
	::wcsncpy_s(key, 256, button_name, n);

	// 运行中按钮名是「文件资源管理器」，固定后被固定项 lnk 的显示名覆盖成「Windows 资源管理器」
	return ::_wcsicmp(key, L"\u6587\u4EF6\u8D44\u6E90\u7BA1\u7406\u5668") == 0 ||
		::_wcsicmp(key, L"Windows \u8D44\u6E90\u7BA1\u7406\u5668") == 0 ||
		::_wcsicmp(key, L"File Explorer") == 0;
}

// 任务栏按钮的名字形如 "任务管理器 - 1 个运行窗口"，取 " - " 之前的部分作为关键词，
// 三级回退匹配：标题精确 → 标题包含 → 进程名相同。
static BOOL CALLBACK find_app_window_proc(HWND hWnd, LPARAM lParam)
{
	auto f = reinterpret_cast<taskbar_window_finder *>(lParam);
	if(!::IsWindowVisible(hWnd) || ::GetWindow(hWnd, GW_OWNER))
		return TRUE;
	if(::GetWindowLongPtrW(hWnd, GWL_EXSTYLE) & WS_EX_TOOLWINDOW)
		return TRUE;

	wchar_t title[512]{};
	int tlen = ::GetWindowTextW(hWnd, title, 512);
	if(tlen > 0)
		normalize_for_match(title);

	const wchar_t *btn = f->keyword.c_str();
	const wchar_t *sep = ::wcsstr(btn, L" - ");
	size_t n = sep ? static_cast<size_t>(sep - btn) : ::wcslen(btn);
	if(n == 0 || n >= 256)
		return TRUE;
	wchar_t key[256]{};
	::wcsncpy_s(key, 256, btn, n);
	normalize_for_match(key);

	bool matched = false;
	// 1) 标题精确  2) 标题包含
	if(tlen > 0 && ::_wcsicmp(title, key) == 0)
		matched = true;
	else if(tlen > 0 && ::wcsstr(title, key))
		matched = true;
	else
	{
		// 3) 进程名相同（QQ.exe / WeChat.exe …）
		DWORD pid = 0;
		::GetWindowThreadProcessId(hWnd, &pid);
		if(pid)
		{
			wchar_t path[MAX_PATH]{};
			if(HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid))
			{
				DWORD size = MAX_PATH;
				if(::QueryFullProcessImageNameW(hProcess, 0, path, &size))
				{
					const wchar_t *file = ::wcsrchr(path, L'\\');
					file = file ? file + 1 : path;
					wchar_t base[MAX_PATH]{};
					::wcsncpy_s(base, MAX_PATH, file, MAX_PATH);
					if(wchar_t *dot = ::wcsrchr(base, L'.'))
						*dot = 0;
					matched = ::_wcsicmp(base, key) == 0;
				}
				::CloseHandle(hProcess);
			}
		}
	}

	// 4) 用「该窗口在任务栏上叫的名字」匹配：任务栏按钮名就是 shell:AppsFolder 里的显示名
	//    （窗口 AUMID = Microsoft.Windows.Explorer → 「文件资源管理器」；
	//     没注册 AUMID 的应用按 exe 路径查，如 ...\blender-launcher.exe → 「Blender 5.2」）。
	//    这一级完全不看窗口标题，所以标题与按钮名差很远的程序也能认出来。
	if(!matched)
	{
		wchar_t btn_name[512]{};
		if(window_button_name(hWnd, btn_name, 512))
		{
			normalize_for_match(btn_name);
			matched = ::_wcsicmp(btn_name, key) == 0;
		}
	}

	// 5) 兜底：「文件资源管理器」的窗口不带任何 AppUserModel 属性、AppsFolder 也查不到，
	//    只能按窗口类认（2 个文件夹窗口就是 2 个 CabinetWClass）
	if(!matched && f->file_explorer)
	{
		wchar_t cls[64]{};
		if(::GetClassNameW(hWnd, cls, 64) > 0 &&
			(::_wcsicmp(cls, L"CabinetWClass") == 0 || ::_wcsicmp(cls, L"ExploreWClass") == 0))
			matched = true;
	}

	if(!matched)
		return TRUE;

	// 收集**全部**匹配窗口：应用有多个窗口（合并按钮）时要能一次全关。
	// 主窗口优先取非最小化的：UWP 常同时有「最小化的 ApplicationFrameWindow」和
	// 「真正显示中的 CoreWindow」，只关前者会没反应（实测）。
	f->all.push_back(hWnd);
	if(!f->hwnd)
		f->hwnd = hWnd;
	// ⚠️ 这里不能像以前那样在找到非最小化窗口时 return FALSE 提前结束枚举，否则窗口收不全
	return TRUE;
}

static void find_app_windows(string const &button_name, std::vector<HWND> &out)
{
	out.clear();
	if(button_name.empty())
		return;

	// 按钮名形如 "任务管理器 - 1 个运行窗口" / "Microsoft Edge (2) - 1 个运行窗口"。
	// 先取 " - " 之前的部分；注意还可能带 " (2)" 这类窗口数/配置文件后缀
	// （Edge 多配置文件时会出现），它会破坏标题匹配，需要剥掉再试一次。
	wchar_t key[256]{};
	const wchar_t *btn = button_name.c_str();
	const wchar_t *sep = ::wcsstr(btn, L" - ");
	size_t n = sep ? static_cast<size_t>(sep - btn) : ::wcslen(btn);
	if(n == 0)
		return;
	if(n >= 256)
		n = 255;
	::wcsncpy_s(key, 256, btn, n);

	taskbar_window_finder f;
	f.keyword = key;
	f.file_explorer = is_file_explorer_button(button_name.c_str());
	::EnumWindows(find_app_window_proc, reinterpret_cast<LPARAM>(&f));
	if(!f.all.empty())
	{
		out = f.all;
		return;
	}

	// 剥离尾部 " (数字)" 再匹配
	wchar_t stripped[256]{};
	::wcsncpy_s(stripped, 256, key, 256);
	if(wchar_t *p = ::wcsrchr(stripped, L'('))
	{
		if(p > stripped && *(p - 1) == L' ')
		{
			wchar_t *q = p + 1;
			bool digits = *q != 0;
			for(; *q && *q != L')'; ++q)
			{
				if(*q < L'0' || *q > L'9')
				{
					digits = false;
					break;
				}
			}
			if(digits && *q == L')' && *(q + 1) == 0)
			{
				*(p - 1) = 0;
				taskbar_window_finder g;
				g.keyword = stripped;
				g.file_explorer = f.file_explorer;
				::EnumWindows(find_app_window_proc, reinterpret_cast<LPARAM>(&g));
				out = g.all;
			}
		}
	}
}

static HWND find_app_window(string const &button_name)
{
	std::vector<HWND> w;
	find_app_windows(button_name, w);
	return w.empty() ? nullptr : w.front();
}

// ===== 任务栏固定 / 取消固定 =====
// 任务栏靠 AppUserModelID 合并「固定项」与「运行中的窗口」：
// 若 .lnk 的 AUMID 与运行窗口的不一致，固定后就会出现两个图标（QQ 就是这种）。
// 图标同理，.lnk 的 IconLocation 为空时某些应用（Edge）会显示成空白图标。

static bool taskbar_pinned_dir(wchar_t *out, size_t outLen)
{
	wchar_t appdata[MAX_PATH]{};
	if(!::SHGetSpecialFolderPathW(nullptr, appdata, CSIDL_APPDATA, FALSE))
		return false;
	::swprintf_s(out, outLen, L"%s\\Microsoft\\Internet Explorer\\Quick Launch\\User Pinned\\TaskBar", appdata);
	return true;
}

// 该 .lnk 的目标是否就是这个 exe
static bool lnk_targets(const wchar_t *lnk_path, const wchar_t *exe_path)
{
	IShellLinkW *link = nullptr;
	if(FAILED(::CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLinkW, reinterpret_cast<void **>(&link))) || !link)
		return false;
	bool ok = false;
	IPersistFile *pf = nullptr;
	if(SUCCEEDED(link->QueryInterface(IID_IPersistFile, reinterpret_cast<void **>(&pf))) && pf)
	{
		if(SUCCEEDED(pf->Load(lnk_path, STGM_READ)))
		{
			wchar_t target[MAX_PATH]{};
			if(SUCCEEDED(link->GetPath(target, MAX_PATH, nullptr, SLGP_UNCPRIORITY)))
				ok = ::_wcsicmp(target, exe_path) == 0;
		}
		pf->Release();
	}
	link->Release();
	return ok;
}

// 任务栏真正显示的固定项记录在 HKCU\...\Taskband\Favorites（REG_BINARY）里；
// User Pinned\TaskBar 目录里的 .lnk 只是素材 —— 被取消固定后文件会残留（幽灵项），
// 只扫目录会把幽灵误判成「已固定」（实测：目录里有 Microsoft Edge.lnk，Favorites 里没有，
// 任务栏上也没有 Edge 按钮 → Edge 一运行就被错当成「已固定」）。
// 这里在 blob 里按 UTF-16LE 搜 lnk 文件名；字符串在 blob 里的偏移奇偶不定，两种对齐都要试。
static bool is_pinned_lnk_active(const wchar_t *lnk_path)
{
	const wchar_t *name = ::wcsrchr(lnk_path, L'\\');
	name = name ? name + 1 : lnk_path;
	size_t nlen = ::wcslen(name);
	if(nlen == 0 || nlen > 128)
		return false;

	HKEY key = nullptr;
	if(::RegOpenKeyExW(HKEY_CURRENT_USER,
		L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Taskband", 0, KEY_READ, &key) != ERROR_SUCCESS)
		return false;

	DWORD type = 0, size = 0;
	if(::RegQueryValueExW(key, L"Favorites", nullptr, &type, nullptr, &size) != ERROR_SUCCESS || size < nlen * 2)
	{
		::RegCloseKey(key);
		return false;
	}
	std::vector<BYTE> blob(size);
	bool read = ::RegQueryValueExW(key, L"Favorites", nullptr, &type, blob.data(), &size) == ERROR_SUCCESS;
	::RegCloseKey(key);
	if(!read)
		return false;

	auto fold = [](BYTE c) -> BYTE { return (c >= 'A' && c <= 'Z') ? static_cast<BYTE>(c + 32) : c; };

	for(int align = 0; align < 2; ++align)
	{
		for(size_t i = static_cast<size_t>(align); i + nlen * 2 <= size; i += 2)
		{
			bool hit = true;
			for(size_t j = 0; j < nlen; ++j)
			{
				const wchar_t wc = name[j];
				if(fold(blob[i + j * 2]) != fold(static_cast<BYTE>(wc & 0xFF)) ||
					fold(blob[i + j * 2 + 1]) != fold(static_cast<BYTE>((wc >> 8) & 0xFF)))
				{
					hit = false;
					break;
				}
			}
			if(hit)
				return true;
		}
	}
	return false;
}

// 在 User Pinned\TaskBar 里找该 exe 的固定项快捷方式（只认真正在任务栏上的，排除残留幽灵项）
static bool find_pinned_lnk(const wchar_t *exe_path, wchar_t *out, size_t outLen)
{
	wchar_t dir[MAX_PATH]{};
	if(!taskbar_pinned_dir(dir, MAX_PATH))
		return false;

	wchar_t pattern[MAX_PATH]{};
	::swprintf_s(pattern, MAX_PATH, L"%s\\*.lnk", dir);
	WIN32_FIND_DATAW fd{};
	HANDLE h = ::FindFirstFileW(pattern, &fd);
	if(h == INVALID_HANDLE_VALUE)
		return false;
	bool found = false;
	do
	{
		wchar_t full[MAX_PATH]{};
		::swprintf_s(full, MAX_PATH, L"%s\\%s", dir, fd.cFileName);
		if(lnk_targets(full, exe_path) && is_pinned_lnk_active(full))
		{
			::wcsncpy_s(out, outLen, full, outLen - 1);
			found = true;
			break;
		}
	} while(::FindNextFileW(h, &fd));
	::FindClose(h);
	return found;
}

// 读 .lnk 的 target 路径。注意有些固定项没有 target（例如「文件资源管理器」的 lnk
// 只写了 IconLocation=%windir%\explorer.exe），这时返回 false。
static bool lnk_target(const wchar_t *lnk_path, wchar_t *out, size_t outLen)
{
	out[0] = 0;
	IShellLinkW *link = nullptr;
	if(FAILED(::CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLinkW, reinterpret_cast<void **>(&link))) || !link)
		return false;
	bool ok = false;
	IPersistFile *pf = nullptr;
	if(SUCCEEDED(link->QueryInterface(IID_IPersistFile, reinterpret_cast<void **>(&pf))) && pf)
	{
		if(SUCCEEDED(pf->Load(lnk_path, STGM_READ)))
		{
			wchar_t target[MAX_PATH]{};
			if(SUCCEEDED(link->GetPath(target, MAX_PATH, nullptr, SLGP_UNCPRIORITY)) && target[0])
			{
				::wcsncpy_s(out, outLen, target, outLen - 1);
				ok = true;
			}
		}
		pf->Release();
	}
	link->Release();
	return ok;
}

// 读 .lnk 在资源管理器里显示的**本地化名字**（不带扩展名）。
// 关键用例："File Explorer (2).lnk" 在中文系统上显示为「文件资源管理器」，
// 而任务栏按钮名用的就是这个显示名，磁盘文件名对不上。
static bool lnk_display_name(const wchar_t *lnk_path, wchar_t *out, size_t outLen)
{
	out[0] = 0;
	IShellItem *item = nullptr;
	if(FAILED(::SHCreateItemFromParsingName(lnk_path, nullptr, IID_IShellItem, reinterpret_cast<void **>(&item))) || !item)
		return false;

	bool ok = false;
	LPWSTR name = nullptr;
	if(SUCCEEDED(item->GetDisplayName(SIGDN_NORMALDISPLAY, &name)) && name)
	{
		::wcsncpy_s(out, outLen, name, outLen - 1);
		ok = out[0] != 0;
		::CoTaskMemFree(name);
	}
	item->Release();
	return ok;
}

// 未运行的固定项按钮上拿不到窗口，UIA 也**只给名字**（AutomationId 为空、没有 AUMID），
// 所以只能拿按钮名去 User Pinned\TaskBar 里比对固定项的 .lnk。
// 三级匹配：lnk 文件名 == 按钮名 → 去掉尾部 " (数字)" 再比 → lnk 的显示名 / target 主名 == 按钮名。
// 命中后还要过 is_pinned_lnk_active（目录里残留的幽灵项不算固定项）。
static bool find_pinned_lnk_by_name(const wchar_t *button_name, wchar_t *lnkOut, size_t lnkLen, wchar_t *exeOut, size_t exeLen)
{
	if(!button_name || !button_name[0])
		return false;

	// 按钮名形如 "任务管理器 - 1 个运行窗口"；取 " - " 之前的部分
	wchar_t key[256]{};
	const wchar_t *sep = ::wcsstr(button_name, L" - ");
	size_t n = sep ? static_cast<size_t>(sep - button_name) : ::wcslen(button_name);
	if(n == 0 || n >= 256)
		return false;
	::wcsncpy_s(key, 256, button_name, n);
	normalize_for_match(key);

	// 再去掉尾部的 " (数字)"（同一应用有多个固定项时任务栏会加序号，如 "Microsoft Edge (2)"）
	wchar_t stripped[256]{};
	::wcsncpy_s(stripped, 256, key, 256);
	if(wchar_t *p = ::wcsrchr(stripped, L'('))
	{
		if(p > stripped && *(p - 1) == L' ')
		{
			wchar_t *q = p + 1;
			bool digits = *q != 0;
			for(; *q && *q != L')'; ++q)
			{
				if(*q < L'0' || *q > L'9')
				{
					digits = false;
					break;
				}
			}
			if(digits && *q == L')' && *(q + 1) == 0)
				*(p - 1) = 0;
		}
	}

	wchar_t dir[MAX_PATH]{};
	if(!taskbar_pinned_dir(dir, MAX_PATH))
		return false;

	wchar_t pattern[MAX_PATH]{};
	::swprintf_s(pattern, MAX_PATH, L"%s\\*.lnk", dir);
	WIN32_FIND_DATAW fd{};
	HANDLE h = ::FindFirstFileW(pattern, &fd);
	if(h == INVALID_HANDLE_VALUE)
		return false;

	bool found = false;
	do
	{
		if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			continue;

		wchar_t full[MAX_PATH]{};
		::swprintf_s(full, MAX_PATH, L"%s\\%s", dir, fd.cFileName);

		// ① lnk 文件名（去扩展名）
		wchar_t stem[MAX_PATH]{};
		::wcsncpy_s(stem, MAX_PATH, fd.cFileName, MAX_PATH);
		if(wchar_t *dot = ::wcsrchr(stem, L'.'))
			*dot = 0;
		normalize_for_match(stem);

		bool matched = ::_wcsicmp(stem, key) == 0;
		if(!matched && stripped[0])
			matched = ::_wcsicmp(stem, stripped) == 0;

		// ② lnk 的本地化显示名（「文件资源管理器」↔ File Explorer (2).lnk）
		if(!matched)
		{
			wchar_t disp[MAX_PATH]{};
			if(lnk_display_name(full, disp, MAX_PATH))
			{
				normalize_for_match(disp);
				matched = ::_wcsicmp(disp, key) == 0 || (stripped[0] && ::_wcsicmp(disp, stripped) == 0);
			}
		}

		// ③ lnk 的 target 主名（没扩展名）
		if(!matched)
		{
			wchar_t target[MAX_PATH]{};
			if(lnk_target(full, target, MAX_PATH))
			{
				const wchar_t *file = ::wcsrchr(target, L'\\');
				file = file ? file + 1 : target;
				wchar_t base[MAX_PATH]{};
				::wcsncpy_s(base, MAX_PATH, file, MAX_PATH);
				if(wchar_t *dot = ::wcsrchr(base, L'.'))
					*dot = 0;
				matched = ::_wcsicmp(base, key) == 0 || (stripped[0] && ::_wcsicmp(base, stripped) == 0);
			}
		}

		if(matched)
		{
			// 已被取消固定、只剩文件的幽灵项不算固定项
			if(!is_pinned_lnk_active(full))
				continue;

			::wcsncpy_s(lnkOut, lnkLen, full, lnkLen - 1);
			if(exeOut && exeLen && !lnk_target(full, exeOut, exeLen))
				exeOut[0] = 0;
			found = true;
			break;
		}
	} while(::FindNextFileW(h, &fd));
	::FindClose(h);
	return found;
}

static bool is_pinned_to_taskbar(const wchar_t *exe_path)
{
	wchar_t lnk[MAX_PATH]{};
	return find_pinned_lnk(exe_path, lnk, MAX_PATH);
}

static void set_lnk_prop(IPropertyStore *ps, REFPROPERTYKEY key, const wchar_t *value)
{
	if(!value || !value[0])
		return;
	PROPVARIANT pv{};
	pv.vt = VT_LPWSTR;
	pv.pwszVal = const_cast<LPWSTR>(value);
	ps->SetValue(key, pv);
}

// pin 完成后修正刚生成的快捷方式：
// ① 补上**整组** AppUserModel 属性（ID + RelaunchCommand + RelaunchDisplayNameResource + IconResource）——
//    只补 AUMID 不够：任务栏算 AppID 还看 RelaunchCommand，缺了它固定项会跟运行窗口分成两个图标
//    （实测 Blender：lnk 只有 ID、target 是 blender.exe，而窗口 relaunch 是 blender-launcher.exe）。
// ② 补图标位置，避免空白图标。
static void fix_pinned_shortcut(const wchar_t *lnk_path, const wchar_t *exe_path, const window_appmodel &am)
{
	IShellLinkW *link = nullptr;
	if(FAILED(::CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLinkW, reinterpret_cast<void **>(&link))) || !link)
		return;
	IPersistFile *pf = nullptr;
	if(SUCCEEDED(link->QueryInterface(IID_IPersistFile, reinterpret_cast<void **>(&pf))) && pf)
	{
		if(SUCCEEDED(pf->Load(lnk_path, STGM_READWRITE)))
		{
			link->SetIconLocation(exe_path, 0);
			IPropertyStore *ps = nullptr;
			if(SUCCEEDED(link->QueryInterface(IID_IPropertyStore, reinterpret_cast<void **>(&ps))) && ps)
			{
				set_lnk_prop(ps, PKEY_AppUserModel_ID, am.aumid);
				set_lnk_prop(ps, PKEY_AppUserModel_RelaunchCommand, am.relaunch_command);
				set_lnk_prop(ps, PKEY_AppUserModel_RelaunchDisplayNameResource, am.relaunch_display_name);
				set_lnk_prop(ps, PKEY_AppUserModel_RelaunchIconResource, am.relaunch_icon);
				ps->Commit();
				ps->Release();
			}
			pf->Save(lnk_path, TRUE);
		}
		pf->Release();
	}
	link->Release();
}

// 调用 shell 上下文菜单里的某个动词（按语言无关的规范名，如 "TaskbarPin"）。
//
// 为什么不能直接用 ShellExecuteEx(verb)：
//   Win10 上 "TaskbarPin" 这个动词虽然存在，但程序化调用会被**静默忽略**
//   （老的 "pin to taskbar" 更是直接 ERROR_NO_ASSOCIATION）——微软的「用户意图」保护。
//   文件资源管理器里点「固定到任务栏」能生效，是因为 explorer 走的是 IContextMenu::InvokeCommand。
//   本函数复刻这条路径，所以能真正固定。
static bool invoke_shell_verb(const wchar_t *path, const wchar_t *verb_name)
{
	if(!path || !verb_name)
		return false;

	PIDLIST_ABSOLUTE pidl = nullptr;
	if(FAILED(::SHParseDisplayName(path, nullptr, &pidl, 0, nullptr)) || !pidl)
		return false;

	bool ok = false;
	IShellFolder *parent = nullptr;
	PCUITEMID_CHILD child = nullptr;
	if(SUCCEEDED(::SHBindToParent(pidl, IID_IShellFolder, reinterpret_cast<void **>(&parent), &child)) && parent)
	{
		IContextMenu *cm = nullptr;
		if(SUCCEEDED(parent->GetUIObjectOf(nullptr, 1, &child, IID_IContextMenu, nullptr, reinterpret_cast<void **>(&cm))) && cm)
		{
			HMENU menu = ::CreatePopupMenu();
			if(menu && SUCCEEDED(cm->QueryContextMenu(menu, 0, 1, 0x7FFF, CMF_NORMAL)))
			{
				// 找不到目标动词时，把枚举到的动词名记进日志：不同对象上挂的动词集合不一样
				// （「从任务栏取消固定」只在固定项 .lnk 上有），下次失败可直接看清单定位。
				wchar_t seen[512]{};
				int count = ::GetMenuItemCount(menu);
				for(int i = 0; i < count; ++i)
				{
					UINT id = ::GetMenuItemID(menu, i);
					if(id == 0 || id == static_cast<UINT>(-1))
						continue;

					wchar_t verb[128]{};
					if(SUCCEEDED(cm->GetCommandString(id - 1, GCS_VERBW, nullptr, reinterpret_cast<char *>(verb), 128)))
					{
						if(verb[0])
						{
							size_t used = ::wcslen(seen);
							if(used > 0 && used < 500)
								seen[used++] = L',';
							for(const wchar_t *p = verb; *p && used < 505; ++p)
								seen[used++] = *p;
							seen[used] = 0;
						}
						if(::_wcsicmp(verb, verb_name) == 0)
						{
							CMINVOKECOMMANDINFOEX ci{};
							ci.cbSize = sizeof(ci);
							ci.fMask = CMIC_MASK_UNICODE | CMIC_MASK_FLAG_NO_UI;
							ci.lpVerb = MAKEINTRESOURCEA(id - 1);
							ci.lpVerbW = MAKEINTRESOURCEW(id - 1);
							ci.nShow = SW_SHOWNORMAL;
							ok = SUCCEEDED(cm->InvokeCommand(reinterpret_cast<LPCMINVOKECOMMANDINFO>(&ci)));
							break;
						}
					}
				}
				if(!ok && !seen[0])
					_log.info(L"[BTN] verb '%s' not found on '%s' (no verbs)", verb_name, path);
				else if(!ok)
					_log.info(L"[BTN] verb '%s' not found on '%s' (verbs: %s)", verb_name, path, seen);
			}
			if(menu)
				::DestroyMenu(menu);
			cm->Release();
		}
		parent->Release();
	}
	::CoTaskMemFree(pidl);
	return ok;
}

// 微软官方的「从任务栏 / 开始菜单取消固定」API（CLSID_StartMenuPin，实现于 shell32.dll）。
// 它**只有 RemoveFromList**：官方没有提供「固定到任务栏」的 API（防止程序擅自改任务栏），
// 所以「固定」只能走 shell 动词，「取消固定」在动词不可用时用这条兜底。
// 实测用例：File Explorer (2).lnk 上压根没有 taskbarunpin 动词
// （枚举结果只有 open/opencontaining/OpenNewWindow/cut/copy/link/delete/properties），
// 但它在 Taskband\Favorites 里确实固定着，只能靠这条 API 取消。
static bool start_menu_remove_from_list(const wchar_t *path)
{
	if(!path || !path[0])
		return false;

	IStartMenuPinnedList *pinned = nullptr;
	if(FAILED(::CoCreateInstance(CLSID_StartMenuPin, nullptr, CLSCTX_INPROC_SERVER, IID_IStartMenuPinnedList, reinterpret_cast<void **>(&pinned))) || !pinned)
	{
		_log.info(L"[BTN] StartMenuPin: CoCreateInstance failed");
		return false;
	}

	IShellItem *item = nullptr;
	HRESULT hr = ::SHCreateItemFromParsingName(path, nullptr, IID_IShellItem, reinterpret_cast<void **>(&item));
	if(FAILED(hr) || !item)
	{
		_log.info(L"[BTN] StartMenuPin: SHCreateItemFromParsingName('%s') hr=0x%08X", path, (unsigned)hr);
		pinned->Release();
		return false;
	}

	hr = pinned->RemoveFromList(item);
	_log.info(L"[BTN] StartMenuPin::RemoveFromList('%s') hr=0x%08X", path, (unsigned)hr);
	item->Release();
	pinned->Release();
	return SUCCEEDED(hr);
}

// 后台线程里跑：QueryContextMenu 会加载一堆 shell 扩展，别拖住 explorer 的 UI 线程
// lnk 为「该应用的固定项快捷方式」路径（可能为空）。取消固定的动词只挂在这种 .lnk 上。
static void set_taskbar_pin_async(string path, string lnk, HWND hWnd, bool pin)
{
	std::thread([path, lnk, hWnd, pin]()
	{
		::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

		// 固定前先把运行窗口的整组 AppUserModel 属性记下来，固定后写进快捷方式 ——
		// 只写 AUMID 不够，任务栏算 AppID 还看 RelaunchCommand，缺了就会分成两个图标
		window_appmodel am;
		if(pin)
		{
			get_window_appmodel(hWnd, am);

			// 「文件资源管理器」例外：explorer 的窗口一个 AppUserModel 属性都不带，
			// 但固定项必须写上这组值才能与运行窗口合并（实测写入后重启 explorer，
			// 固定项与文件夹窗口合并成一个按钮）。这些值是 Windows 自己用的 ——
			// AppsFolder 里 Microsoft.Windows.Explorer 的显示名就是「文件资源管理器」。
			const wchar_t *file = ::wcsrchr(path.c_str(), L'\\');
			file = file ? file + 1 : path.c_str();
			if(!am.aumid[0] && ::_wcsicmp(file, L"explorer.exe") == 0)
			{
				::wcsncpy_s(am.aumid, 256, L"Microsoft.Windows.Explorer", 255);
				if(!am.relaunch_command[0])
					::wcsncpy_s(am.relaunch_command, 1024, L"\"C:\\Windows\\explorer.exe\"", 1023);
				if(!am.relaunch_display_name[0])
					::wcsncpy_s(am.relaunch_display_name, 512, L"\u6587\u4EF6\u8D44\u6E90\u7BA1\u7406\u5668", 511);
			}
		}

		// 「从任务栏取消固定」的动词实测只挂在固定项的 .lnk 上
		// （msedge.exe / DeepSeek Harness.exe / notepad.exe / 普通 lnk 上都没有），
		// 对 exe 调用自然枚举不到 → 这是取消固定时灵时不灵的根因。
		// 所以先从 .lnk 试，失败再回退到原来的 exe 路径。
		bool ok = false;
		if(!pin && !lnk.empty())
			ok = invoke_shell_verb(lnk.c_str(), L"taskbarunpin");
		if(!ok)
			ok = invoke_shell_verb(path.c_str(), pin ? L"TaskbarPin" : L"TaskbarUnpin");
		// 有些固定项的 .lnk 上压根没有 taskbarunpin 动词（实测「文件资源管理器」），
		// 这时用官方 API 兜底（它只有取消固定，没有固定）。
		if(!ok && !pin)
			ok = start_menu_remove_from_list(lnk.empty() ? path.c_str() : lnk.c_str());
		_log.info(L"[BTN] %s exe='%s' lnk='%s' -> %s",
			pin ? L"TaskbarPin" : L"TaskbarUnpin", path.c_str(), lnk.c_str(), ok ? L"ok" : L"failed");

		// 固定成功后补写快捷方式：AUMID + 图标位置
		if(pin && ok)
		{
			for(int i = 0; i < 25; ++i)
			{
				::Sleep(200);
				wchar_t lnk[MAX_PATH]{};
				if(find_pinned_lnk(path.c_str(), lnk, MAX_PATH))
				{
					fix_pinned_shortcut(lnk, path.c_str(), am);
					_log.info(L"[BTN] patched lnk aumid='%s' relaunch='%s' name='%s'",
						am.aumid, am.relaunch_command, am.relaunch_display_name);
					break;
				}
			}
		}
		::CoUninitialize();
	}).detach();
}

// 「关闭窗口」= 请求窗口关闭，**不是结束进程**（结束进程会丢数据、应用没机会保存）。
//   1) UIA 的 WindowPattern.Close()：标准「关闭窗口」语义（等同用户点标题栏 ×，应用可弹保存/取消）。
//      UWP / 现代应用只认这条 —— 实测「设置」窗口对 WM_CLOSE、WM_SYSCOMMAND/SC_CLOSE、
//      SendMessage(WM_CLOSE) 三种传统发法全部无响应。
//   2) 普通 Win32 窗口兜底：WM_CLOSE。
// UIA 调用可能较慢，放后台线程，避免卡住 explorer 的 UI 线程。
static bool close_window_via_uia(HWND hWnd)
{
	if(!hWnd || !taskbar_t::_IUIAutomation)
		return false;

	IComPtr<IUIAutomationElement> elem;
	if(FAILED(taskbar_t::_IUIAutomation->ElementFromHandle(hWnd, elem)) || !elem)
		return false;

	IUnknown *unk = nullptr;
	if(FAILED(elem->GetCurrentPattern(UIA_WindowPatternId, &unk)) || !unk)
		return false;

	bool ok = false;
	IUIAutomationWindowPattern *wp = nullptr;
	if(SUCCEEDED(unk->QueryInterface(__uuidof(IUIAutomationWindowPattern), reinterpret_cast<void **>(&wp))) && wp)
	{
		ok = SUCCEEDED(wp->Close());
		wp->Release();
	}
	unk->Release();
	return ok;
}

static void close_app_window_async(HWND hWnd)
{
	if(!hWnd)
		return;
	std::thread([hWnd]()
	{
		if(close_window_via_uia(hWnd))
			return;
		::PostMessageW(hWnd, WM_CLOSE, 0, 0);
	}).detach();
}

// 把匹配到的窗口收敛成「真正要关闭的顶层窗口」：按 GA_ROOT 去重 ——
// UWP 会同时出现 CoreWindow 和它的宿主 ApplicationFrameWindow，其实是同一个窗口。
static void distinct_top_windows(const std::vector<HWND> &in, std::vector<HWND> &out)
{
	out.clear();
	for(HWND w : in)
	{
		HWND root = ::GetAncestor(w, GA_ROOT);
		if(!root)
			root = w;
		bool dup = false;
		for(HWND t : out)
		{
			if(t == root)
			{
				dup = true;
				break;
			}
		}
		if(!dup)
			out.push_back(root);
	}
}

static void show_taskbar_button_menu(HWND hWnd, const Point &pt, taskbar_button_info &info)
{
	find_app_windows(info.title, info.windows);
	info.window = info.windows.empty() ? nullptr : info.windows.front();

	HMENU menu = ::CreatePopupMenu();
	if(!menu)
		return;

	const UINT id_pin = 0x1001;
	const UINT id_close = 0x1002;
	const UINT id_unpin = 0x1003;

	// 源码文件无 BOM，MSVC 按 GBK(936) 解析源文件，中文字面量必须用 \u 转义，
	// 否则会乱码："固定到任务栏" → "鏁哄畾鍒颁换鍔℃爮"
	const wchar_t *text_pin = L"\u56FA\u5B9A\u5230\u4EFB\u52A1\u680F";                       // 固定到任务栏
	const wchar_t *text_unpin = L"\u4ECE\u4EFB\u52A1\u680F\u53D6\u6D88\u56FA\u5B9A";         // 从任务栏取消固定
	const wchar_t *text_close = L"\u5173\u95ED\u7A97\u53E3";                                 // 关闭窗口
	const wchar_t *text_close_all = L"\u5173\u95ED\u6240\u6709\u7A97\u53E3";                 // 关闭所有窗口

	auto title = info.title.empty() ? string(L"Application") : info.title;
	::AppendMenuW(menu, MF_STRING, 0, title.c_str());
	::AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);

	// 固定 / 取消固定：按当前是否已固定决定显示哪个，并记下 exe / 固定项 .lnk 给动作分支用
	wchar_t exe_path[MAX_PATH]{};
	bool has_exe = false;
	if(info.window)
	{
		DWORD pid = 0;
		::GetWindowThreadProcessId(info.window, &pid);
		if(pid)
		{
			if(HANDLE hp = ::OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid))
			{
				DWORD size = MAX_PATH;
				if(::QueryFullProcessImageNameW(hp, 0, exe_path, &size))
					has_exe = true;
				::CloseHandle(hp);
			}
		}
	}

	// 没运行（纯固定项按钮）时上面拿不到 exe：改用按钮名去固定项目录里找对应的 .lnk，
	// 顺带取它的 target 当 exe（「文件资源管理器」这类 lnk 没有 target，只拿到 .lnk）
	wchar_t pinned_lnk[MAX_PATH]{};
	bool has_pinned_lnk = false;
	if(!has_exe)
	{
		has_pinned_lnk = find_pinned_lnk_by_name(info.title.c_str(), pinned_lnk, MAX_PATH, exe_path, MAX_PATH);
		if(has_pinned_lnk && exe_path[0])
			has_exe = true;
	}
	// 运行中的固定项：由 exe 反查它的固定项 .lnk（取消固定的动词挂在 .lnk 上）
	if(!has_pinned_lnk && has_exe)
		has_pinned_lnk = find_pinned_lnk(exe_path, pinned_lnk, MAX_PATH);

	if(has_pinned_lnk)
		::AppendMenuW(menu, MF_STRING, id_unpin, text_unpin);
	else if(has_exe)
		::AppendMenuW(menu, MF_STRING, id_pin, text_pin);

	// 「关闭窗口」只在真的有运行窗口时才给：没有窗口时点它没有任何作用。
	// 应用有多个窗口（合并按钮）时改成「关闭所有窗口」，一次全关。
	std::vector<HWND> close_targets;
	distinct_top_windows(info.windows, close_targets);
	if(!close_targets.empty())
		::AppendMenuW(menu, MF_STRING, id_close, close_targets.size() > 1 ? text_close_all : text_close);

	_log.info(L"[BTN] pid=%d window=0x%x windows=%d class='%s' title='%s' exe='%s' lnk='%s'",
		info.pid, (unsigned)(ULONG_PTR)info.window, static_cast<int>(close_targets.size()),
		info.window ? Window::class_name(info.window).c_str() : L"<null>",
		info.title.c_str(), exe_path, pinned_lnk);

	// 菜单必须先拿到前台/焦点：否则键盘导航无效，鼠标第一次点击只当"激活"，
	// 且任何进程抢焦点都会把菜单关掉（返回 0）。
	if(auto root = ::GetAncestor(hWnd, GA_ROOT))
		::SetForegroundWindow(root);
	::SetForegroundWindow(hWnd);

	// 供 command.kill_process_tree 用（见下）：按钮菜单当前对应的顶层窗口，已按 GA_ROOT 去重。
	taskbar_button_windows = close_targets;

	is_in_taskbar = true;
	in_taskbar_button_menu = true;
	auto cmd = TrackPopupMenuExProc(menu, TPM_RETURNCMD | TPM_RIGHTBUTTON, pt.x, pt.y, hWnd, nullptr);
	in_taskbar_button_menu = false;
	is_in_taskbar = false;

	_log.info(L"[BTN] command=%d", static_cast<int>(cmd));

	if(cmd == id_close && !close_targets.empty())
	{
		// UWP 应用（设置 / 商店等）里层的 Windows.UI.Core.CoreWindow 不响应 SC_CLOSE，
		// 必须把关闭消息发给顶层宿主（ApplicationFrameWindow）—— 上面已按 GA_ROOT 收敛过。
		// 多窗口（合并按钮）时逐个发关闭请求，应用自己仍可弹保存/取消框。
		_log.info(L"[BTN] close %d window(s)", static_cast<int>(close_targets.size()));
		for(HWND t : close_targets)
			close_app_window_async(t);
	}
	else if(cmd == id_pin && has_exe)
	{
		_log.info(L"[BTN] pin '%s'", exe_path);
		set_taskbar_pin_async(exe_path, string(), info.window, true);
	}
	else if(cmd == id_unpin && has_pinned_lnk)
	{
		_log.info(L"[BTN] unpin exe='%s' lnk='%s'", exe_path, pinned_lnk);
		set_taskbar_pin_async(has_exe ? string(exe_path) : string(), string(pinned_lnk), info.window, false);
	}

	::DestroyMenu(menu);
}

// command.kill_process_tree 的实现（FuncExpression.cpp 里 extern 调用）：
// 杀掉按钮菜单当前对应窗口的进程树。顺序很关键 —— 先按 Toolhelp 快照把后代全部展开，
// 再从叶子往根终止：反过来先杀父进程的话，子进程会被系统改挂到别的父上，杀树就漏了。
bool kill_taskbar_button_process_tree()
{
	std::vector<uint32_t> roots;
	for(auto hwnd : taskbar_button_windows)
	{
		if(!::IsWindow(hwnd))
			continue;
		DWORD pid = 0;
		::GetWindowThreadProcessId(hwnd, &pid);
		if(pid == 0)
			continue;
		bool seen = false;
		for(auto r : roots)
		{
			if(r == pid)
			{
				seen = true;
				break;
			}
		}
		if(!seen)
			roots.push_back(pid);
	}

	if(roots.empty())
	{
		_log.info(L"[KILL] no window/pid for taskbar button (windows=%d)", static_cast<int>(taskbar_button_windows.size()));
		return false;
	}

	_log.info(L"[KILL] root pid=%u", roots.front());

	auto snapshot = Diagnostics::Process::EnumInfo();

	std::vector<uint32_t> order;
	std::function<void(uint32_t)> collect = [&](uint32_t pid)
	{
		for(auto done : order)
			if(done == pid)
				return;
		for(auto &p : snapshot)
		{
			if(p.th32ProcessID != 0 && p.th32ParentProcessID == pid && p.th32ProcessID != pid)
				collect(p.th32ProcessID);
		}
		order.push_back(pid);
	};

	for(auto r : roots)
		collect(r);

	int killed = 0;
	for(auto pid : order)
	{
		if(auto hProcess = ::OpenProcess(PROCESS_TERMINATE, FALSE, pid))
		{
			if(::TerminateProcess(hProcess, 0))
				killed++;
			::CloseHandle(hProcess);
		}
	}

	_log.info(L"[KILL] roots=%d processes=%d killed=%d",
			  static_cast<int>(roots.size()), static_cast<int>(order.size()), killed);
	return killed > 0;
}

LRESULT __stdcall TaskListProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	auto proc = reinterpret_cast<WNDPROC>(::GetPropW(hWnd, TaskListSubclassProp));
	if(!proc)
		return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);

	switch(uMsg)
	{
		case WM_CONTEXTMENU:
		{
			auto pt = Point::CursorPos();
			auto info = query_taskbar_button(pt);
			if(!info.is_button)
				break;   // 按钮之间的空白：放行，走 explorer/Shell 原有的任务栏菜单
			_log.info(L"[BTN] WM_CONTEXTMENU cursor=(%d,%d)", pt.x, pt.y);
			show_taskbar_button_menu(hWnd, pt, info);
			return 0;   // 只对真正的应用按钮吞掉，阻止 XAML 跳转列表
		}
	}
	return ::CallWindowProcW(proc, hWnd, uMsg, wParam, lParam);
}

static bool tasklist_hook()
{
	auto tray = ::FindWindowW(Windows::WC_Shell_TrayWnd, nullptr);
	_log.info(L"[PROBE] step1 tray=0x%x", (unsigned)(ULONG_PTR)tray);
	if(!tray)
		return false;
	auto rebar = ::FindWindowExW(tray, nullptr, Windows::WC_ReBarWindow32, nullptr);
	_log.info(L"[PROBE] step2 rebar=0x%x", (unsigned)(ULONG_PTR)rebar);
	if(!rebar)
		return false;
	auto sw = ::FindWindowExW(rebar, nullptr, Windows::WC_MSTaskSwWClass, nullptr);
	_log.info(L"[PROBE] step3 sw=0x%x", (unsigned)(ULONG_PTR)sw);
	if(!sw)
		return false;
	auto list = ::FindWindowExW(sw, nullptr, Windows::WC_MSTaskListWClass, nullptr);
	_log.info(L"[PROBE] step4 list=0x%x", (unsigned)(ULONG_PTR)list);
	if(!list)
		return false;
	if(::GetPropW(list, TaskListSubclassProp))
		return true;

	auto proc = reinterpret_cast<WNDPROC>(::GetWindowLongPtrW(list, GWLP_WNDPROC));
	::SetPropW(list, TaskListSubclassProp, reinterpret_cast<HANDLE>(proc));
	::SetWindowLongPtrW(list, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(TaskListProc));
	_log.info(L"[PROBE] MSTaskListWClass hooked hwnd=0x%x", (unsigned)(ULONG_PTR)list);
	return true;
}

static void tasklist_hook_all(uint32_t delay)
{
	std::thread([delay]()
	{
		for(int i = 0; i < 15; ++i)
		{
			::Sleep(delay);
			if(tasklist_hook())
				break;
		}
		_log.info(L"[PROBE] tasklist_hook_all thread finished");
	}).detach();
}

HRESULT __stdcall CoCreateInstanceHook(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID *ppv)
{
	auto is_local_server = (dwClsContext & CLSCTX_LOCAL_SERVER) != 0;
	auto is_inproc_server = (dwClsContext & CLSCTX_INPROC_SERVER) != 0;

	if(_initializer.cache && (is_inproc_server || is_local_server))
	{
		auto hr = E_NOINTERFACE; //CLASS_E_CLASSNOTAVAILABLE;
		auto process = true;

		if(rclsid == IID_FileExplorerContextMenu /*&& riid == {706461D1-AC5F-4730-BFE3-CAC6CAD5EF5E}*/)
		{
			if(_initializer.cache->settings.priority)
			{
				Context context;
				Object obj = context.Eval(_initializer.cache->settings.priority).move();
				if(obj.is_number())
					process = obj;
			}

			if(process)
				return hr;
		}
		else
		{
			/*auto xx = Guid(L"{f3d06e7c-1e45-4a26-847e-f9fcdee59be0}");

			if(xx.equals({riid,rclsid}))
			{
				MB(L"0000000000000");
			}*/

			//if(riid== IID_IExplorerCommandProvider)
			//	_log.info(L"IID_IExplorerCommandProvider %s", Guid(rclsid).to_string(2).c_str());

			//if(riid == IID_IExecuteCommand)
			//	_log.info(L"IID_IExecuteCommand %s", Guid(rclsid).to_string(2).c_str());
			/*
			if(riid == IID_IExplorerCommandState)
			{
				hr = _CoCreateInstance.invoke(rclsid, pUnkOuter, dwClsContext, riid, ppv);

				auto cc = (IExplorerCommand *)*ppv;

				IExplorerCommand *f = 0;
				cc->QueryInterface(IID_IExplorerCommand, (void**)&f);
				//printto;rotate90;rotate270;setwallpaper;slideshow

				LPWSTR p;
				f->GetTitle(0, &p);
				_log.info(L"IExplorerCommandState %s", p);
				return hr;
			}
		*/
			auto is_UWP = false;
			process = false;
			/*
			if(Guid(rclsid).equals(L"{E6950302-61F0-4FEB-97DB-855E30D4A991}"))
			{
				*ppv = new ExplorerCommandBase;
				return S_OK;
			}
			*/
			if(is_inproc_server)
				process = riid == IID_IContextMenu || riid == IID_IContextMenu2 || riid == IID_IContextMenu3 || riid == IID_IExplorerCommand;
			else
			{
				process = riid == IID_IExplorerCommand;
				is_UWP = is_local_server && process;
			}

			if(process)
			{
				Context context;
				this_item _this; context._this = &_this;
				_this.is_uwp = is_UWP;
				_this.clsid = Guid(rclsid).to_string(2);

				bool test = Keyboard::IsKeyDown(VK_MENU);
				if(test)
				{
					Timer t;
					t.start();
					hr = _CoCreateInstance.invoke(rclsid, pUnkOuter, dwClsContext, riid, ppv);
					t.stop();
					auto elapsed = (int)t.elapsed_milliseconds();
					_log.write(L"%d%s\t%s\t%s\r\n", elapsed, elapsed > 0 ? L"ms" : L"" , _this.clsid.c_str(), is_UWP ? L"UWP":L"");
					return hr;
				}

				for(auto si : _initializer.cache->statics)
				{
					if(si->clsid.empty() || (si->where && !context.eval_bool(si->where)))
						continue;

					if(si->has_clsid)
					{
						for(auto &id : si->clsid)
						{
							if(id.equals(rclsid))
							{
								if(test && *ppv)
								{
									((IUnknown *)*ppv)->Release();
									*ppv = nullptr;
								}
								return E_NOINTERFACE;
							}
						}
					}
				}

				if(test)
					return hr;
			}
		}
	}

	return _CoCreateInstance.invoke(rclsid, pUnkOuter, dwClsContext, riid, ppv);
}

STDAPI WINAPI DllGetClassObjectHook(REFCLSID rclsid, _In_ REFIID riid, LPVOID *ppv)
{
	auto hr = CLASS_E_CLASSNOTAVAILABLE;
	if(IID_FileExplorerContextMenu == rclsid/* || rclsid == IID_FileExplorerCommandBar*/)
		return hr;
	hr = _DllGetClassObject.invoke(rclsid, riid, ppv);
	return hr;
}

#define TPM_SYSMENU	0x0200L
bool is_in_taskbar;
// 任务栏「应用按钮」自建菜单期间为 true（见 show_taskbar_button_menu）。
// ContextMenu 用它把这次菜单的上下文类型标成 FSO_TASKBAR_BUTTON（而不是 FSO_TASKBAR），
// 于是配置项可以用 where=window.is_taskbar_button 精确命中按钮菜单，
// 而「转到」、imports/taskbar.nss 等既有 taskbar 项不会自动涌进来。
bool in_taskbar_button_menu;
// 按钮菜单当前对应的顶层窗口（弹菜单前填好）：command.kill_process_tree 用它杀进程树。
std::vector<HWND> taskbar_button_windows;

BOOL WINAPI TrackPopupMenuProc(HMENU hMenu, uint32_t uFlags, int x, int y, int nReserved, HWND hWnd, const RECT *prcRect);
BOOL WINAPI TrackPopupMenuExProc(HMENU hMenu, uint32_t uFlags, int x, int y, HWND hWnd, LPTPMPARAMS lptpm);

BOOL WINAPI NtUserTrackPopupMenu(HMENU hMenu, uint32_t uFlags, int x, int y, HWND hWnd, LPTPMPARAMS lptpm, int tfunc)
{
#ifdef _DIJA
	return (BOOL)XXX::OnContextMenu(hWnd, x, y, hMenu);;
#endif

	auto result = FALSE;
	auto invoked = false;


	auto invoke = [&](HMENU hmenu, uint32_t flag, Point pt)
	{
		if(!invoked)
		{
			if(tfunc == 0)
				result = ::TrackPopupMenu(hmenu, flag, pt.x, pt.y, 0, hWnd, nullptr);
			else if(iathook_NtUserTrackPopupMenuEx.installed())
				result = iathook_NtUserTrackPopupMenuEx.invoke<decltype(TrackPopupMenuExProc)>(hmenu, flag, pt.x, pt.y, hWnd, lptpm);
			else
				result = ::TrackPopupMenuEx(hmenu, flag, pt.x, pt.y, hWnd, lptpm);
			invoked = true;
		}
	};

	__try
	{
		__try
		{

			if(!is_registered())
			{
				__trace(L"TrackPopupMenu unregistered");
				__leave;
			}

			//cs.lock();
			auto has_inited = 0; Initializer::OnState(is_in_taskbar);

			if(!_initializer.Status.Disabled)
			{
				// is injected from explorer
				if(!_initializer.Status.Loaded)
				{
					if(_initializer.has_error() || has_inited)
						__leave; //goto skip;
					_initializer.init();
				}

				auto ctx = ContextMenu::CreateAndInitialize(hWnd, hMenu, { x, y }, _loader.explorer, _loader.contextmenuhandler);
				if(ctx != nullptr)
				{

					Flag<uint32_t> flag(uFlags);
					//Selections::point = { x, y };
					//	flag.remove(TPM_RETURNCMD);
					flag.remove(TPM_NONOTIFY);
					//	flag.remove(TPM_NOANIMATION);

				//	flag.add(TPM_HORPOSANIMATION);

				//	BOOL v = 1;
				//	SystemParametersInfoW(SPI_SETMENUANIMATION, 0, &v, SPIF_SENDCHANGE);
				//	v = 0;
				//	SystemParametersInfoW(SPI_GETMENUFADE, 0, &v, SPIF_SENDCHANGE);
					
					if(ctx->is_layoutRTL == 1)
						flag.add(TPM_LAYOUTRTL);
					else if(ctx->is_layoutRTL == 0)
						flag.remove(TPM_LAYOUTRTL);

					auto z = ctx->_theme.border.size + ctx->_theme.border.size;

					if(ctx->Selected.Window.parent == WINDOW_TASKBAR)
					{
						HWND hTaskbar = hWnd;
						if(ctx->Selected.Window.id == WINDOW_START)
							hTaskbar = ::WindowFromPoint({ x, y });
						else
							hTaskbar = ::GetAncestor(hWnd, GA_ROOTOWNER);

						if(!hTaskbar)
							hTaskbar = hWnd;

						Monitor monitor(hTaskbar);
						monitor.info();

						Rect rcW = monitor.rcWork;
						Rect rcM = monitor.rcMonitor;
						Rect rc = hTaskbar;

						if(ctx->Selected.Window.id == WINDOW_START)
						{
							if(rc.top > 0)
								y = (rc.top + 5) - (z + ctx->_theme.border.padding.height());
							else if(rc.left == 0)
								y = rc.bottom;
							else
								x = rc.left;
						}
						else
						{
							if(rcM.top == rc.top && rcM.bottom == rc.bottom) // Vertical
							{
								if(rcM.left == rc.left)
									x = rc.right;
								else if(rcM.right == rc.right)
									x = rc.left - 20;
							}
							else if(rcM.left == rc.left && rcM.right == rc.right) // Horizontal
							{
								if(rcM.top == rc.top)
									y = rc.bottom;
								else if(rcM.bottom == rc.bottom)
									y = (rc.top + 5) - (z + ctx->_theme.border.padding.height());
							}
						}
					}
					else
					{
						y -= z;
					}
					
					invoke(ctx->MenuHandle(), flag, { x, y });
				
					// v = 0;
					// SystemParametersInfoW(SPI_SETMENUANIMATION, 0, &v, SPIF_SENDCHANGE);
					result = ctx->InvokeCommand(result);
					__leave;
				}
			}
		}
		except
		{
#ifdef _DEBUG
			_log.exception(__func__);
#endif
		}
	}
	__finally
	{
		invoke(hMenu, uFlags, { x, y });
		_loader.contextmenuhandler = false;
		is_in_taskbar = false;
		return result;
	}
}

BOOL WINAPI TrackPopupMenuProc(HMENU hMenu, uint32_t uFlags, int x, int y, [[maybe_unused]] int nReserved, HWND hWnd, [[maybe_unused]] const RECT *prcRect)
{
	//_log.write(L"%s\n", Window::class_name(hWnd).c_str());
	return NtUserTrackPopupMenu(hMenu, uFlags, x, y, hWnd, nullptr, 0);
}

//win32u.dll win10 1607	Redstone 1 14393
BOOL WINAPI TrackPopupMenuExProc(HMENU hMenu, uint32_t uFlags, int x, int y, HWND hWnd, LPTPMPARAMS lptpm)
{
	//_log.write(L"%s\n", Window::class_name(hWnd).c_str());
	return NtUserTrackPopupMenu(hMenu, uFlags, x, y, hWnd, lptpm, 1);
}

bool ShowTaskbarContextMenu(HWND hTaskbar, const Point &pt, uint32_t uMsg)
{
	auto ret = false;
	//if(taskbar_t::is_allowed_area(hTaskbar, pt))
	{
		struct menu {
			HMENU handle{};
			menu() { handle = ::CreatePopupMenu(); }
			~menu() { if(handle) ::DestroyMenu(handle); }
		}_menu;

		if(_menu.handle)
		{
			is_in_taskbar = true;
			//::SetForegroundWindow(hTaskbar);
			auto uFlags = TPM_RETURNCMD | TPM_RIGHTBUTTON | (::GetSystemMetrics(SM_MIDEASTENABLED) ? TPM_LAYOUTRTL : 0);
			if(uMsg == WM_CONTEXTMENU)
			{
				//_log.info(L"************");
				TrackPopupMenuExProc(_menu.handle, uFlags, pt.x, pt.y, hTaskbar, nullptr);
			}
			else 
			{
				iathook_NtUserTrackPopupMenuEx.invoke<decltype(TrackPopupMenuExProc)>(_menu.handle, uFlags, 0, 0, hTaskbar, nullptr);
			}
			::PostMessageW(hTaskbar, WM_NULL, 0, 0); // send benign message to window to make sure the menu goes away.
			ret = true;
		}
	}
	return ret;
}

LRESULT __stdcall TaskbarProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	auto wndproc = (WNDPROC)::GetPropW(hWnd, UxSubclass);
	
	if(wndproc == nullptr)
		return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);

	if(uMsg == WM_CONTEXTMENU)
	{
		auto pt = Point::CursorPos();
		if(taskbar_t::is_allowed_area(hWnd, pt))
		{
			if(ShowTaskbarContextMenu(hWnd, pt, wParam == 0 && lParam == 0 ? WM_RBUTTONDOWN : WM_CONTEXTMENU))
				return 0;
		}
	}
	else if(uMsg == WM_MOUSEACTIVATE)
	{
		auto msg = HIWORD(lParam);
		if(msg != WM_RBUTTONDOWN)
			return wndproc(hWnd, uMsg, wParam, lParam);

		auto pt = Point::CursorPos();

		if(taskbar_t::is_allowed_area(hWnd, pt))
		{
			int disabled_taskbar = 0;
			if(RegistryConfig::get(L"\\disable", L"taskbar", disabled_taskbar) && disabled_taskbar == 1)
				return wndproc(hWnd, uMsg, wParam, lParam);

			if(msg == WM_RBUTTONDOWN)
			{
				Keyboard kb;
				if(auto count = kb.get_keys_excloude_contextmenu(); count > 0)
				{
					if(kb.key_ctrl())
					{
						if(count == 2 && kb.key_win())
							return wndproc(hWnd, uMsg, wParam, lParam);

						if(count == 1 && !_initializer.has_error())
						{
							if(!_initializer.Status.Loaded)
								_initializer.OnState();
						}
					}
				}
			}

			if(_initializer.has_error())
			{
				_initializer.OnState();
				if(_initializer.has_error())
					return wndproc(hWnd, uMsg, wParam, lParam);
			}
			else
			{
				if(_initializer.Status.Disabled)
					return wndproc(hWnd, uMsg, wParam, lParam);
			}

			if(!is_registered())
			{
				return wndproc(hWnd, uMsg, wParam, lParam);
			}

			::SetFocus(hWnd);
			::SendMessageW(hWnd, WM_CONTEXTMENU, 0, 0);

			return MA_ACTIVATEANDEAT;
		}
	}
	else if(uMsg == WM_SETTINGCHANGE)
	{
		if(SPI_SETWORKAREA == wParam && ::GetSystemMetrics(SM_CMONITORS) > 1)
		{
			taskbar_t::hook_all(1000);
		}
	}
	else if(uMsg == WM_DESTROY)
	{
		taskbar_t::unhook(hWnd);
		taskbar_t::hook_all(1000);
	}

	return wndproc(hWnd, uMsg, wParam, lParam);
}

LRESULT __stdcall TaskbarSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, [[maybe_unused]] UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	if(uMsg == WM_CONTEXTMENU)
	{
		auto hTaskbar = reinterpret_cast<HWND>(dwRefData);
		auto pt = Point::CursorPos();
		if(taskbar_t::is_allowed_area(hTaskbar, pt))
		{
			if(ShowTaskbarContextMenu(hTaskbar, pt, wParam == 0 && lParam == 0? WM_RBUTTONDOWN : WM_CONTEXTMENU))
				return 0;			
		}
	}
	else if(uMsg == WM_MOUSEACTIVATE)
	{
		auto lret = ::DefSubclassProc(hWnd, uMsg, wParam, lParam);
		auto msg = HIWORD(lParam);

		if(/*msg != WM_LBUTTONDOWN && */msg != WM_RBUTTONDOWN)
			return lret;

		auto hTaskbar = reinterpret_cast<HWND>(dwRefData);
		auto pt = Point::CursorPos();

		if(taskbar_t::is_allowed_area(hTaskbar, pt))
		{
			int disabled_taskbar = 0;
			if(RegistryConfig::get(L"\\disable", L"taskbar", disabled_taskbar) && disabled_taskbar == 1)
				return lret;

			if(msg == WM_RBUTTONDOWN)
			{
				Keyboard kb;
				if(auto count = kb.get_keys_excloude_contextmenu(); count > 0)
				{
					if(kb.key_ctrl())
					{
						if(count == 2 && kb.key_win())
							return lret;

						if(count == 1 && !_initializer.has_error())
						{
							if(!_initializer.Status.Loaded)
								_initializer.OnState();
						}
					}
				}
			}

			if(_initializer.has_error())
			{
				if(msg != WM_RBUTTONDOWN)
					return lret;

				_initializer.OnState();

				if(_initializer.has_error())
					return lret;
			}
			else
			{
				if(_initializer.Status.Disabled)
					return lret;
			}

			if(!is_registered())
			{
				_taskbar_mouse.unhook();
				taskbar_t::unhook(hWnd);
			//	for(auto &window : _window_taskbar)
			//		window.second.remove_prop(UxSubclass);
				return lret;
			}

			if(msg == WM_RBUTTONDOWN)
			{
				_taskbar_mouse.hook(WH_MOUSE, [](int nCode, WPARAM wParam, LPARAM lParam)->LRESULT
				{
					//auto lret = _taskbar_mouse.callnext(nCode, wParam, lParam);
					if(nCode == HC_ACTION && wParam == WM_RBUTTONUP)
					{
						auto mh = reinterpret_cast<MOUSEHOOKSTRUCT *>(lParam);
						Window window = ::GetParent(mh->hwnd);
						if(window.is_prop(UxSubclass))
						{
							_taskbar_mouse.unhook();
							::SetFocus(window);
							ShowTaskbarContextMenu(window, mh->pt, WM_CONTEXTMENU);
							return 1;
						}
					}
					return _taskbar_mouse.callnext(nCode, wParam, lParam);
				}, hWnd);
				
				return MA_ACTIVATEANDEAT;
			}

			//if(msg != WM_RBUTTONDOWN)
			//	::SetFocus(hWnd);

			/*
			if(msg == WM_RBUTTONDOWN)
			{
				::SetFocus(hWnd);
				//ShowTaskbarContextMenu(hWnd, pt, WM_RBUTTONDOWN);
				SendMessageW(hWnd, WM_CONTEXTMENU, 0, 0);
			}
			*/
		}
		return lret;
	}
	else if(uMsg == WM_SETTINGCHANGE)
	{
		if(SPI_SETWORKAREA == wParam && ::GetSystemMetrics(SM_CMONITORS) > 1)
		{
			taskbar_t::hook_all(1000);
		}
	}
	else if(uMsg == WM_DESTROY)
	{
		taskbar_t::unhook(hWnd);
		taskbar_t::hook_all(1000);
	}

	return ::DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

//integrate 
BOOL APIENTRY DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID)
{
	try
	{
		if(dwReason == DLL_PROCESS_ATTACH)
		{
			::DisableThreadLibraryCalls(hInstance);
			//InitCommonControls();
			/*INITCOMMONCONTROLSEX icex{};
			// Load the ToolTip class from the DLL.
			icex.dwSize = sizeof(icex);
			icex.dwICC = ICC_BAR_CLASSES;
			InitCommonControlsEx(&icex);
			*/

			_hInstance = hInstance;
			_initializer.HInstance = _hInstance;

#if _DEBUG
			// detect memory leaks
			_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG
						   | _CRTDBG_CHECK_ALWAYS_DF
						   | _CRTDBG_LEAK_CHECK_DF
						   | _CRTDBG_ALLOC_MEM_DF);

			//::_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
#endif
			_loader.init();

			if(!_loader.explorer && !_loader.path.ends_with(L"\\shell.exe"))
			{
				
				//	if(_loader.path.ends_with(L"\\firefox.exe") || _loader.path.ends_with(L"\\devenv.exe"))
				//		return 0;
				
				int disabled_3rdparty = 0;
				if(RegistryConfig::get(L"\\disable", L"3rdparty", disabled_3rdparty) && disabled_3rdparty == 1)
					return 0;

				/*if(RegistryConfig::IsFolderExtensions())
				{
					if(!hasImportTrackPopupMenu())
						return 0;
				}*/
			}

			//if(!loader)
			//	_log.info(L"Load '%s'", Path::Module(nullptr).c_str());

			if(_initializer.init(_hInstance))
			{
				// DisableComExceptionHandling
				IComPtr<IGlobalOptions> globalOptions;
				if(globalOptions.CreateInstance(CLSID_GlobalOptions, CLSCTX_SERVER))
					globalOptions->Set(COMGLB_EXCEPTION_HANDLING, COMGLB_EXCEPTION_DONOT_HANDLE_ANY);

				_initializer.process.hModule = _loader.handle;
				_initializer.process.path = Path::Module(_loader.handle).move();
				_initializer.process.name = Path::Title(_initializer.process.path).move();
				_initializer.process.id = ::GetCurrentProcessId();
				_initializer.process.handle = ::GetCurrentProcess();

				//_log.info(L"%s", _initializer.process.name.c_str());

				ContextMenu::RegisterLayer();

				iathook_NtUserTrackPopupMenuEx
					.init(L"user32.dll", "win32u.dll", "NtUserTrackPopupMenuEx", TrackPopupMenuExProc)
					.install();

				auto hook = []()
				{
					__trace(L"hook all the modules in '%s' process", _initializer.process.name.c_str());

					auto user32 = "user32.dll";

					// hooh all the modules in this process.
					for(auto hModule : Process::Modules(_initializer.process.handle))
					{
						if(_hInstance == hModule)
							continue;

						bool module_exists = false;
						for(const auto &m : iathook_TrackPopupMenu)
						{
							if(m._hModule == hModule)
							{
								module_exists = true;
								break;
							}
						}

						if(!module_exists)
						{
							iathook_TrackPopupMenu.emplace_back(hModule, user32, ::TrackPopupMenu, TrackPopupMenuProc).install();
							iathook_TrackPopupMenu.emplace_back(hModule, user32, ::TrackPopupMenuEx, TrackPopupMenuExProc).install();
						}
					}
				};

				//_TrackPopupMenu.init(::TrackPopupMenu, TrackPopupMenuProc).hook();
				//_TrackPopupMenuEx.init(::TrackPopupMenuEx, TrackPopupMenuExProc).hook();
				//_CoCreateInstance.init(::CoCreateInstance, CoCreateInstanceProc).hook();
				
				if(_loader.explorer)
				{
					//_log.info(_loader.path);

					_CoCreateInstance.Begin();
					//_DllGetClassObject.init(hModule, "DllGetClassObject", DllGetClassObjectHook).hook();
					_CoCreateInstance.init(::CoCreateInstance, CoCreateInstanceHook).hook();
					//auto m = &_detours_ci[hModule];
					//if(!m->installed())
					//m->init(hModule, "api-ms-win-core-com-l1-1-0.dll", ::CoCreateInstance, CoCreateInstanceProc).install();
					_CoCreateInstance.Commit();

					// [注] 曾试过「无条件安装 IAT hook」（方案 B），实测：
					//   1) 对任务栏按钮菜单无效（它不走 user32!TrackPopupMenu）；
					//   2) 会破坏 explorer 的系统菜单填充（桌面菜单只剩配置项，系统项全丢）。
					// 故回退为官方原有条件。
					if(!iathook_NtUserTrackPopupMenuEx.installed())
					{
						std::thread([=]() { ::Sleep(2000); hook(); }).detach();
					}

					if(ver->IsWindows11OrGreater())
					{

						/*
						auto keyCLSID = Registry::ClassesRoot.OpenSubKey(L"CLSID\\" CLS_FileExplorerContextMenu "\\TreatAs", false, false);
						if(!(keyCLSID && keyCLSID.GetString(nullptr).equals(CLS_ContextMenu)))
						{
							auto hModule = ::GetModuleHandleW(Windows_UI_FileExplorer);

							if(!hModule)
								hModule = ::LoadLibraryW(Windows_UI_FileExplorer);

							if(hModule)
							{
								auto r = _CoCreateInstance.Begin();
								_log.info(L"%x", r);
								//_DllGetClassObject.init(hModule, "DllGetClassObject", DllGetClassObjectHook).hook();
								_CoCreateInstance.init(::CoCreateInstance, CoCreateInstanceHook).hook();
								//auto m = &_detours_ci[hModule];
								//if(!m->installed())
								//m->init(hModule, "api-ms-win-core-com-l1-1-0.dll", ::CoCreateInstance, CoCreateInstanceProc).install();
								_CoCreateInstance.Commit();
							}
						}
						keyCLSID.Close();
						*/

						taskbar_t::hook_all(1000);
					}
					else
					{
						// [PROBE] Win10：任务栏按钮区消息探针（只记录）
						tasklist_hook_all(1000);
					}
				}
				else if(!iathook_NtUserTrackPopupMenuEx.install())
				{
					hook();
				}
			}
			//IsRegistered
			/*
			IID iid{};
			CLSIDFromString(L"{23170F69-40C1-278A-1000-000100020000}", &iid);
			Guid guid = iid;
			MB(guid.to_string(2).c_str());
			*/
			//UuidFromStringW

			return TRUE;
		}
		else if(dwReason == DLL_PROCESS_DETACH)
		{
			if(!is_registered())
			{
				if(_loader.explorer)
				{
					// Perform any necessary cleanup.
					_taskbar_mouse.unhook();
					taskbar_t::unhook_all();
					//detour_NtUserTrackPopupMenuEx.uninstall(true);

					if(ver->IsWindows11OrGreater())
					{
						_CoCreateInstance.Begin();
						//_DllGetClassObject.unhook();
						_CoCreateInstance.unhook();
						_CoCreateInstance.Commit();
					}
				}

				for(auto &d : iathook_TrackPopupMenu)
					d.uninstall(true);

				//_TrackPopupMenu.unhook();
				//_TrackPopupMenuEx.unhook();
				//_CoCreateInstance.unhook();

				ContextMenu::UnRegisterLayer();
			}

			taskbar_t::_IUIAutomation.release();

			_log.close();

			//::SendNotifyMessageW(HWND_BROADCAST, WM_NULL, 0, 0);	

			return TRUE;
		}
	}
	catch(...) // handle all exceptions
	{
#ifdef _DEBUG
		_log.exception(__func__);
		_log.close();
#endif
	}
	return FALSE;
}

//IID_FolderExtensions
//#pragma comment(linker, "/export:DllGetClassObject=DllGetClassObject")
_Check_return_
STDAPI DllGetClassObject(_In_ REFCLSID rclsid, [[maybe_unused]] _In_ REFIID riid, [[maybe_unused]] _Outptr_ LPVOID FAR *ppv)
{
	auto hr = E_NOTIMPL;// CLASS_E_CLASSNOTAVAILABLE,E_NOTIMPL, E_OUTOFMEMORY, and E_UNEXPECTED,E_INVALIDARG
	Guid iid = rclsid;
	//Guid iid2 = riid;
	if(ppv) *ppv = nullptr;

	__try
	{
		if(iid.equals(IID_FolderExtensions))
			return E_NOTIMPL;

		if(!iid.equals({ IID_ContextMenu, IID_IconOverlay }))
			return hr;

		if(Initializer::Status.Disabled)
			return hr;

		if(!is_registered())
			return hr;
		
		if(_initializer.has_error())
			return hr;
		
		if(rclsid == IID_ContextMenu)
		{
			_loader.contextmenuhandler = true;
			Selections::point.GetCursorPos();
		}

		if(!_initializer.Status.Loaded)
			_initializer.init();
	}
	except
	{
#ifdef _DEBUG
		_log.exception(__func__);
#endif
	}
	_log.close();

	return hr;
}

//DllCanUnloadNow. COM calls this function to determine whether the object is serving any clients. 
//If not, the system can unload the DLL and free the associated memory.
// Check if we can unload the component from the memory.
// NOTE: The component can be unloaded from the memory when its reference 
// count is zero (i.e. nobody is still using the component).
//#pragma comment(linker, "/export:DllCanUnloadNow=DllCanUnloadNow")
__control_entrypoint(DllExport)
STDAPI DllCanUnloadNow(void)
{
	if(!_loader.explorer || !is_registered())
		return S_OK;
	return S_FALSE;
	/*
	if(ContextMenu::Processes.size() == 0)
	{
		int p = 0;
		EnumThreadWindows(GetCurrentThreadId(), [](HWND, LPARAM lp) {
		(*((int *)lp))++;
			return 0;
		}, (LPARAM)&p);

		//if(p)MBF(L"%d", p);
		return p > 0 ? S_FALSE : S_OK;
	}

	//if(_loader.explorer)
	//	return release() ? S_OK : S_FALSE;
	return ContextMenu::Processes.size() == 0 ? S_OK : S_FALSE;
	//return S_OK;
	*/
}

// Register the COM server and the context menu handler.
HRESULT Register(bool reg)
{
	try
	{
		if(reg)
		{
			Path::Delete(_log.path());
			//_log.reset();
		}

		if(!ver->IsWindows7OrGreater())
		{
			//windows compatibility
			_log.error(L"%s", string::Extract(_hInstance, IDS_WINDOWS_COMPATIBILITY).c_str());
			return ERROR_EXE_MACHINE_TYPE_MISMATCH;//ERROR_DS_VERSION_CHECK_FAILURE
		}

		auto is_elevated = Security::Elevation::IsElevated();

		if(!is_elevated)
		{
			// Missing administrative privileges!
			string msg = string::Extract(_hInstance, IDS_ADMIN_PRIVILEGES).move();
			//You will need to provide administrator permission to run this Shell
			_log.error(L"%s", msg.c_str());
			return ERROR_ACCESS_DENIED;
		}

		string path = Path::Module(_hInstance).move();

		if(reg)
		{
			string dir = Path::Parent(path).move();
			if(!dir.empty())
				Security::Permission::SetFile(dir.c_str());
			//_log.reset();
			//IO::Path::Delete(_log.path());
		}

		string msg;

		if(reg)
		{
			//_log.create();
			REGOP regop;
			regop.REGISTER = regop.CONTEXTMENU = regop.ICONOVERLAY = true;

			if(!RegistryConfig::Register(path, regop))
			{
				msg = string::Extract(_hInstance, IDS_REGISTER_NOT_SUCCESS).move();
				_log.error(L"%s", msg.c_str());
				return S_FALSE;
			}
			msg = string::Extract(_hInstance, IDS_REGISTER_SUCCESS).move();
		}
		else
		{
			if(!RegistryConfig::IsRegistered())
				return S_OK;

			if(!RegistryConfig::Unregister())
			{
				msg = string::Extract(_hInstance, IDS_UNREGISTER_NOT_SUCCESS).move();
				_log.error(L"%s", msg.c_str());
				return S_FALSE;;
			}
			msg = string::Extract(_hInstance, IDS_UNREGISTER_SUCCESS).move();
		}

		_log.info(L"%s", msg.c_str());

		_log.close();

		return S_OK;
	}
	catch(...)
	{
#ifdef _DEBUG
		_log.exception(__func__);
#endif
	}
	_log.close();
	return S_FALSE;
}
/*
//  Register the COM server and the context menu handler.
__control_entrypoint(DllExport)
STDAPI DllRegisterServer()
{
	return Register(true);
}

__control_entrypoint(DllExport)
STDAPI DllUnregisterServer()
{
	return Register(false);
}
*/
/*
STDAPI DllInstall(BOOL bInstall, _In_opt_ PCWSTR pszCmdLine)
{
	MBF(L"%d %s", bInstall, pszCmdLine);
	return E_NOTIMPL;
}
*/

/*
#include <Shldisp.h>

CoInitialize(NULL);
// Create an instance of the shell class
IShellDispatch4 *pShellDisp = NULL;
HRESULT sc = CoCreateInstance(CLSID_Shell, NULL, CLSCTX_SERVER, IID_IDispatch, (LPVOID *) &pShellDisp );
// Show the desktop
sc = pShellDisp->ToggleDesktop();
// Restore the desktop
sc = pShellDisp->ToggleDesktop();
pShellDisp->Release();
 */

#pragma endregion

uint32_t ImmersiveColor::GetColorByColorType(uint32_t colorType)
{
	auto colorSet = GetImmersiveUserColorSetPreference(false, false);
	return GetImmersiveColorFromColorSetEx(colorSet, colorType, false, 0);
}

uint32_t ImmersiveColor::GetColorByName(const wchar_t *name)
{
	auto colorSet = GetImmersiveUserColorSetPreference(false, false);
	auto colorType = GetImmersiveColorTypeFromName(name);
	return GetImmersiveColorFromColorSetEx(colorSet, colorType, false, 0);
}

uint32_t ImmersiveColor::GetImmersiveUserColorSetPreference(bool bForceCheckRegistry, bool bSkipCheckOnFail)
{
	return DLL::Invoke<uint32_t>(hUxTheme, 98, bForceCheckRegistry, bSkipCheckOnFail);
}

uint32_t ImmersiveColor::GetImmersiveColorFromColorSetEx(uint32_t dwImmersiveColorSet, uint32_t dwImmersiveColorType, bool bIgnoreHighContrast, uint32_t dwHighContrastCacheMode)
{
	return DLL::Invoke<uint32_t>(hUxTheme, 95, dwImmersiveColorSet, dwImmersiveColorType, bIgnoreHighContrast, dwHighContrastCacheMode);
}

uint32_t ImmersiveColor::GetImmersiveColorTypeFromName(const wchar_t *name)
{
	return DLL::Invoke<uint32_t>(hUxTheme, 96, name);
}

uint32_t ImmersiveColor::GetImmersiveColorSetCount()
{
	return DLL::Invoke<uint32_t>(hUxTheme, 94);
}

bool ImmersiveColor::IsSupported()
{
	return  ::IsCompositionActive() && DLL::IsFunc(hUxTheme, 95);
}
