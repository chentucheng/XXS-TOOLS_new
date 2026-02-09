#include <windows.h>
#include <commctrl.h>
#include <shellapi.h>
#include <algorithm>
#include <string>

using namespace std;

// 全局变量
HWND g_hMainWnd = NULL;
HWND g_hAdminWnd = NULL;
HINSTANCE g_hInst = NULL;

// ScreenWings程序路径 - 设置默认路径
char g_screenWingsPath[MAX_PATH] = "./data/screenwings.exe";

// 颜色定义 - 白色主题
COLORREF MAIN_BG_COLOR = RGB(255, 255, 255);
COLORREF MAIN_TEXT_COLOR = RGB(30, 30, 46);
COLORREF BUTTON_COLOR = RGB(88, 101, 242);
COLORREF BUTTON_HOVER_COLOR = RGB(105, 118, 255);
COLORREF BUTTON_PRESSED_COLOR = RGB(71, 82, 196);
COLORREF ADMIN_BUTTON_COLOR = RGB(138, 43, 226);
COLORREF CLOSE_BUTTON_COLOR = RGB(220, 60, 60);
COLORREF GREEN_BUTTON_COLOR = RGB(50, 168, 82);
COLORREF YELLOW_BUTTON_COLOR = RGB(220, 160, 0);
COLORREF ORANGE_BUTTON_COLOR = RGB(255, 140, 0);
COLORREF TEAL_BUTTON_COLOR = RGB(0, 180, 180);
COLORREF BORDER_COLOR = RGB(220, 220, 230);
COLORREF ADMIN_BG_COLOR = RGB(248, 248, 250);

// 按钮状态枚举
enum ButtonState {
	BS_NORMAL,
	BS_HOVER,
	BS_PRESSED
};

// 按钮信息结构
struct ButtonInfo {
	HWND hWnd;
	RECT rect;
	char text[100];
	ButtonState state;
	COLORREF color;
	int id;
};

ButtonInfo g_buttons[20];
int g_buttonCount = 0;

ButtonInfo g_adminButtons[20];
int g_adminButtonCount = 0;

// 函数声明
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK AdminWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void DrawModernButton(HDC hdc, const RECT& rect, const char* text, ButtonState state, COLORREF color);
void CreateModernButton(HWND hParent, const char* text, int x, int y, int width, 
	int height, int id, COLORREF color, ButtonInfo* buttonArray, int* count);
void UpdateButtonState(HWND hWnd, int x, int y, bool pressed, ButtonInfo* buttons, int count);
void ShowTaskKillDialog(HWND hParent);
void ShowAboutDialog(HWND hParent);
void ShowUpdateDialog(HWND hParent);
void ShowColorDialog(HWND hParent);
void ShowAdminConfirmDialog(HWND hParent);
void ExecuteScreenWings(HWND hParent);
bool BrowseForScreenWings(HWND hParent);
bool FileExists(const char* path);
void ConvertRelativePathToAbsolute(char* absolutePath, const char* relativePath, size_t bufferSize);

// 辅助函数
int GetXFromLPARAM(LPARAM lParam) {
	return LOWORD(lParam);
}

int GetYFromLPARAM(LPARAM lParam) {
	return HIWORD(lParam);
}

void SafeStringCopy(char* dest, const char* src, size_t destSize) {
	if (dest && src && destSize > 0) {
		strncpy(dest, src, destSize - 1);
		dest[destSize - 1] = '\0';
	}
}

// 将相对路径转换为绝对路径
void ConvertRelativePathToAbsolute(char* absolutePath, const char* relativePath, size_t bufferSize) {
	char currentDir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, currentDir);
	
	if (relativePath[0] == '.' && relativePath[1] == '/') {
		// 处理 ./ 开头的相对路径
		snprintf(absolutePath, bufferSize, "%s\\%s", currentDir, relativePath + 2);
	} else if (relativePath[0] == '.' && relativePath[1] == '.' && relativePath[2] == '/') {
		// 处理 ../ 开头的相对路径
		snprintf(absolutePath, bufferSize, "%s\\..\\%s", currentDir, relativePath + 3);
	} else if (relativePath[1] == ':' || relativePath[0] == '\\') {
		// 已经是绝对路径
		SafeStringCopy(absolutePath, relativePath, bufferSize);
	} else {
		// 其他相对路径
		snprintf(absolutePath, bufferSize, "%s\\%s", currentDir, relativePath);
	}
	
	// 将斜杠统一为反斜杠
	for (char* p = absolutePath; *p; p++) {
		if (*p == '/') *p = '\\';
	}
}

// 主入口点
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	g_hInst = hInstance;
	
	// 初始化通用控件
	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&icex);
	
	// 注册主窗口类
	WNDCLASSEX wc = {0};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MainWndProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = CreateSolidBrush(MAIN_BG_COLOR);
	wc.lpszClassName = "XXSToolsMain";
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	
	if (!RegisterClassEx(&wc)) {
		MessageBox(NULL, "Window Registration Failed!", "Error", MB_ICONERROR | MB_OK);
		return 0;
	}
	
	// 注册管理员窗口类
	WNDCLASSEX wcAdmin = {0};
	wcAdmin.cbSize = sizeof(WNDCLASSEX);
	wcAdmin.style = CS_HREDRAW | CS_VREDRAW;
	wcAdmin.lpfnWndProc = AdminWndProc;
	wcAdmin.hInstance = hInstance;
	wcAdmin.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcAdmin.hbrBackground = CreateSolidBrush(ADMIN_BG_COLOR);
	wcAdmin.lpszClassName = "XXSToolsAdmin";
	wcAdmin.hIcon = LoadIcon(NULL, IDI_SHIELD);
	
	RegisterClassEx(&wcAdmin);
	
	// 计算屏幕中心位置
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);
	int windowWidth = 720;   // 窗口宽度
	int windowHeight = 460;  // 窗口高度
	int windowX = (screenWidth - windowWidth) / 2;
	int windowY = (screenHeight - windowHeight) / 2;
	
	// 创建主窗口
	g_hMainWnd = CreateWindowEx(
		0,
		"XXSToolsMain",
		"XXS Tools",
		WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
		windowX, windowY,
		windowWidth, windowHeight,
		NULL, NULL, hInstance, NULL
		);
	
	if (!g_hMainWnd) {
		MessageBox(NULL, "Window Creation Failed!", "Error", MB_ICONERROR | MB_OK);
		return 0;
	}
	
	ShowWindow(g_hMainWnd, nCmdShow);
	UpdateWindow(g_hMainWnd);
	
	// 消息循环
	MSG msg = {0};
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	
	return (int)msg.wParam;
}

// 绘制现代按钮
void DrawModernButton(HDC hdc, const RECT& rect, const char* text, ButtonState state, COLORREF baseColor) {
	// 根据按钮状态调整颜色
	COLORREF fillColor, borderColor, textColor;
	int radius = 6;
	int borderWidth = 1;
	
	switch (state) {
	case BS_HOVER:
		fillColor = RGB(
			min(255, GetRValue(baseColor) + 20),
			min(255, GetGValue(baseColor) + 20),
			min(255, GetBValue(baseColor) + 20)
			);
		borderColor = RGB(
			min(255, GetRValue(baseColor) + 40),
			min(255, GetGValue(baseColor) + 40),
			min(255, GetBValue(baseColor) + 40)
			);
		textColor = RGB(255, 255, 255);
		borderWidth = 2;
		break;
		
	case BS_PRESSED:
		fillColor = RGB(
			max(0, GetRValue(baseColor) - 40),
			max(0, GetGValue(baseColor) - 40),
			max(0, GetBValue(baseColor) - 40)
			);
		borderColor = RGB(
			max(0, GetRValue(baseColor) - 60),
			max(0, GetGValue(baseColor) - 60),
			max(0, GetBValue(baseColor) - 60)
			);
		textColor = RGB(255, 255, 255);
		borderWidth = 2;
		break;
		
		default: // BS_NORMAL
		fillColor = baseColor;
		borderColor = RGB(
			GetRValue(baseColor) * 0.8,
			GetGValue(baseColor) * 0.8,
			GetBValue(baseColor) * 0.8
			);
		textColor = RGB(255, 255, 255);
		break;
	}
	
	// 创建圆角区域
	HRGN hRgn = CreateRoundRectRgn(rect.left, rect.top, rect.right, rect.bottom, radius, radius);
	
	// 填充按钮背景
	HBRUSH hBrush = CreateSolidBrush(fillColor);
	FillRgn(hdc, hRgn, hBrush);
	DeleteObject(hBrush);
	
	// 绘制边框
	HPEN hPen = CreatePen(PS_SOLID, borderWidth, borderColor);
	HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
	HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
	
	RoundRect(hdc, rect.left, rect.top, rect.right, rect.bottom, radius, radius);
	
	// 绘制文本 - 减小字体大小
	SetTextColor(hdc, textColor);
	SetBkMode(hdc, TRANSPARENT);
	
	HFONT hFont = CreateFont(
		10, 0, 0, 0, FW_SEMIBOLD,
		FALSE, FALSE, FALSE,
		DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE,
		"Arial"
		);
	
	HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
	
	RECT textRect = rect;
	textRect.left += 8;
	textRect.right -= 8;
	
	// 居中绘制文本
	DrawTextA(hdc, text, -1, &textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	
	// 绘制图标（根据按钮文本）- 减小图标大小
	int iconSize = 14;
	int iconX = rect.left + 10;
	int iconY = rect.top + (rect.bottom - rect.top - iconSize) / 2;
	
	// 保存原始画笔
	HPEN hIconPen = CreatePen(PS_SOLID, 1, textColor);
	HPEN hOriginalPen = (HPEN)SelectObject(hdc, hIconPen);
	
	if (strstr(text, "ScreenWings")) {
		// 绘制屏幕图标
		Rectangle(hdc, iconX, iconY, iconX + iconSize, iconY + iconSize - 3);
		Rectangle(hdc, iconX + 1, iconY + 1, iconX + iconSize - 1, iconY + iconSize - 4);
	}
	else if (strstr(text, "TaskKill")) {
		// 绘制X图标
		MoveToEx(hdc, iconX, iconY, NULL);
		LineTo(hdc, iconX + iconSize, iconY + iconSize);
		MoveToEx(hdc, iconX + iconSize, iconY, NULL);
		LineTo(hdc, iconX, iconY + iconSize);
	}
	else if (strstr(text, "About")) {
		// 绘制信息图标
		Ellipse(hdc, iconX + 3, iconY + 3, iconX + iconSize - 3, iconY + iconSize - 3);
		MoveToEx(hdc, iconX + iconSize/2, iconY + 5, NULL);
		LineTo(hdc, iconX + iconSize/2, iconY + 7);
		MoveToEx(hdc, iconX + iconSize/2, iconY + 9, NULL);
		LineTo(hdc, iconX + iconSize/2, iconY + 10);
	}
	else if (strstr(text, "Update")) {
		// 绘制刷新图标
		Arc(hdc, iconX + 2, iconY + 2, iconX + iconSize - 2, iconY + iconSize - 2, 
			iconX + iconSize/2, iconY + 2, iconX + iconSize/2, iconY + 2);
		MoveToEx(hdc, iconX + iconSize - 2, iconY + 5, NULL);
		LineTo(hdc, iconX + iconSize + 1, iconY + 3);
		LineTo(hdc, iconX + iconSize - 2, iconY);
	}
	else if (strstr(text, "Admin")) {
		// 绘制锁图标
		Rectangle(hdc, iconX + 3, iconY + 5, iconX + iconSize - 3, iconY + iconSize);
		Ellipse(hdc, iconX + 4, iconY + 2, iconX + iconSize - 4, iconY + 7);
	}
	else if (strstr(text, "Color") || strstr(text, "Colours")) {
		// 绘制调色板图标
		Ellipse(hdc, iconX + 2, iconY + 2, iconX + iconSize - 2, iconY + iconSize - 2);
		Ellipse(hdc, iconX + 5, iconY + 5, iconX + 9, iconY + 9);
		Ellipse(hdc, iconX + 10, iconY + 3, iconX + 14, iconY + 7);
	}
	
	// 恢复原始画笔
	SelectObject(hdc, hOriginalPen);
	DeleteObject(hIconPen);
	
	// 清理
	SelectObject(hdc, hOldFont);
	SelectObject(hdc, hOldPen);
	SelectObject(hdc, hOldBrush);
	DeleteObject(hFont);
	DeleteObject(hPen);
	DeleteObject(hRgn);
}

// 创建现代按钮
void CreateModernButton(HWND hParent, const char* text, int x, int y, int width, 
	int height, int id, COLORREF color, ButtonInfo* buttonArray, int* count) {
		int index = *count;
		
		// 创建窗口（自绘按钮）
		buttonArray[index].hWnd = CreateWindow(
			"BUTTON",
			text,
			WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
			x, y, width, height,
			hParent,
			(HMENU)(INT_PTR)id,
			g_hInst,
			NULL
			);
		
		// 存储按钮信息
		buttonArray[index].rect.left = x;
		buttonArray[index].rect.top = y;
		buttonArray[index].rect.right = x + width;
		buttonArray[index].rect.bottom = y + height;
		SafeStringCopy(buttonArray[index].text, text, sizeof(buttonArray[index].text));
		buttonArray[index].state = BS_NORMAL;
		buttonArray[index].color = color;
		buttonArray[index].id = id;
		
		(*count)++;
	}

// 更新按钮状态
void UpdateButtonState(HWND hWnd, int x, int y, bool pressed, ButtonInfo* buttons, int count) {
	POINT pt = {x, y};
	BOOL needRedraw = FALSE;
	
	for (int i = 0; i < count; i++) {
		ButtonState oldState = buttons[i].state;
		
		if (PtInRect(&buttons[i].rect, pt)) {
			if (pressed) {
				buttons[i].state = BS_PRESSED;
			} else {
				buttons[i].state = BS_HOVER;
			}
		} else {
			buttons[i].state = BS_NORMAL;
		}
		
		if (oldState != buttons[i].state) {
			InvalidateRect(buttons[i].hWnd, NULL, FALSE);
			needRedraw = TRUE;
		}
	}
	
	if (needRedraw) {
		UpdateWindow(hWnd);
	}
}

// 创建标签 - 增加创建列标题标签的函数
HWND CreateColumnTitle(HWND hParent, const char* text, int x, int y, int width, int height) {
	HWND hLabel = CreateWindow(
		"STATIC",
		text,
		WS_VISIBLE | WS_CHILD | SS_CENTER,
		x, y, width, height,
		hParent,
		NULL,
		g_hInst,
		NULL
		);
	
	// 设置特殊字体（加粗、稍大）
	HFONT hFont = CreateFont(
		14, 0, 0, 0, FW_BOLD,
		FALSE, FALSE, FALSE,
		DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE,
		"Arial"
		);
	SendMessage(hLabel, WM_SETFONT, (WPARAM)hFont, TRUE);
	
	return hLabel;
}

// 创建普通标签
HWND CreateLabel(HWND hParent, const char* text, int x, int y, int width, 
	int height, int fontSize = 20, BOOL bold = TRUE) {
		HWND hLabel = CreateWindow(
			"STATIC",
			text,
			WS_VISIBLE | WS_CHILD | SS_CENTER,
			x, y, width, height,
			hParent,
			NULL,
			g_hInst,
			NULL
			);
		
		// 设置字体
		HFONT hFont = CreateFont(
			fontSize, 0, 0, 0, 
			bold ? FW_BOLD : FW_NORMAL, 
			FALSE, FALSE, FALSE,
			DEFAULT_CHARSET, 
			OUT_DEFAULT_PRECIS, 
			CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY, 
			DEFAULT_PITCH | FF_DONTCARE, 
			"Arial"
			);
		SendMessage(hLabel, WM_SETFONT, (WPARAM)hFont, TRUE);
		
		return hLabel;
	}

// 检查文件是否存在
bool FileExists(const char* path) {
	DWORD dwAttrib = GetFileAttributes(path);
	return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

// 浏览选择ScreenWings程序
bool BrowseForScreenWings(HWND hParent) {
	OPENFILENAMEA ofn;
	char szFile[MAX_PATH] = "";
	
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hParent;
	ofn.lpstrFile = szFile;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "可执行文件\0*.exe\0所有文件\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	
	if (GetOpenFileName(&ofn) == TRUE) {
		SafeStringCopy(g_screenWingsPath, szFile, sizeof(g_screenWingsPath));
		return true;
	}
	return false;
}

// 执行ScreenWings程序
void ExecuteScreenWings(HWND hParent) {
	// 检查默认路径是否存在
	char absolutePath[MAX_PATH];
	ConvertRelativePathToAbsolute(absolutePath, g_screenWingsPath, sizeof(absolutePath));
	
	if (!FileExists(absolutePath)) {
		// 默认路径不存在，让用户选择程序
		int result = MessageBox(hParent,
			"默认的ScreenWings程序未找到。\n\n路径: ./data/screenwings.exe\n\n是否手动选择ScreenWings.exe文件？",
			"ScreenWings设置",
			MB_YESNO | MB_ICONQUESTION);
		
		if (result == IDYES) {
			if (!BrowseForScreenWings(hParent)) {
				MessageBox(hParent,
					"未选择文件。ScreenWings功能将无法使用。",
					"提示",
					MB_OK | MB_ICONINFORMATION);
				return;
			}
		} else {
			return;
		}
	} else {
		// 使用绝对路径
		SafeStringCopy(absolutePath, g_screenWingsPath, sizeof(absolutePath));
		ConvertRelativePathToAbsolute(absolutePath, g_screenWingsPath, sizeof(absolutePath));
	}
	
	// 最终检查文件是否存在
	if (!FileExists(absolutePath)) {
		MessageBox(hParent,
			"ScreenWings程序文件不存在或已被移动。\n请重新选择程序文件。",
			"错误",
			MB_OK | MB_ICONERROR);
		// 重置为默认路径
		SafeStringCopy(g_screenWingsPath, "./data/screenwings.exe", sizeof(g_screenWingsPath));
		ExecuteScreenWings(hParent); // 重新尝试
		return;
	}
	
	// 执行程序
	SHELLEXECUTEINFO sei = {0};
	sei.cbSize = sizeof(SHELLEXECUTEINFO);
	sei.fMask = SEE_MASK_NOCLOSEPROCESS;
	sei.hwnd = hParent;
	sei.lpVerb = "open";
	sei.lpFile = absolutePath;
	sei.lpParameters = "";
	sei.nShow = SW_SHOWNORMAL;
	
	if (ShellExecuteEx(&sei)) {
		char successMsg[512];
		// 只显示文件名
		const char* filename = strrchr(absolutePath, '\\');
		if (filename) filename++;
		else filename = absolutePath;
		
		sprintf(successMsg, 
			"ScreenWings程序已启动成功！\n\n程序文件: %s\n\n状态: 运行中",
			filename);
		
		MessageBox(hParent,
			successMsg,
			"ScreenWings",
			MB_OK | MB_ICONINFORMATION);
	} else {
		DWORD error = GetLastError();
		char errorMsg[256];
		sprintf(errorMsg, 
			"无法启动ScreenWings程序。\n\n错误代码: %lu\n\n请检查程序文件是否完整或权限是否足够。\n\n路径: %s",
			error, absolutePath);
		MessageBox(hParent,
			errorMsg,
			"错误",
			MB_OK | MB_ICONERROR);
	}
}

// 主窗口过程 - 修复布局，适应460高度
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	static int windowWidth = 720;
	static int windowHeight = 460;  // 窗口高度改为460
	
	switch (message) {
		case WM_CREATE: {
			// 获取窗口尺寸
			RECT rc;
			GetClientRect(hWnd, &rc);
			windowWidth = rc.right - rc.left;
			windowHeight = rc.bottom - rc.top;
			
			// 标题区域 - 紧凑布局
			CreateLabel(hWnd, "XXS TOOLS", 50, 15, windowWidth - 100, 40, 28, TRUE);
			CreateLabel(hWnd, "Professional System Tools Suite", 50, 60, windowWidth - 100, 25, 12, FALSE);
			
			// 创建列标题 - 放在按钮上方
			int buttonStartY = 110;  // 上移按钮开始位置
			int buttonHeight = 36;   // 减小按钮高度
			int buttonWidth = 250;   // 减小按钮宽度
			int horizontalSpacing = 40;
			int verticalSpacing = 45;  // 减小垂直间距
			
			// 计算列位置
			int totalWidth = buttonWidth * 2 + horizontalSpacing;
			int startX = (windowWidth - totalWidth) / 2;
			int leftColumnX = startX;
			int rightColumnX = leftColumnX + buttonWidth + horizontalSpacing;
			
			// 创建列标题
			CreateColumnTitle(hWnd, "System Tools", leftColumnX, 90, buttonWidth, 20);
			CreateColumnTitle(hWnd, "Utility Tools", rightColumnX, 90, buttonWidth, 20);
			
			// 第一行：ScreenWings 和 TaskKill
			CreateModernButton(hWnd, "ScreenWings", leftColumnX, buttonStartY, 
				buttonWidth, buttonHeight, 1001,
				BUTTON_COLOR, g_buttons, &g_buttonCount);
			
			CreateModernButton(hWnd, "TaskKill", rightColumnX, buttonStartY,
				buttonWidth, buttonHeight, 1002,
				ORANGE_BUTTON_COLOR, g_buttons, &g_buttonCount);
			
			// 第二行：About 和 Update Information
			CreateModernButton(hWnd, "About", leftColumnX, buttonStartY + verticalSpacing,
				buttonWidth, buttonHeight, 1003,
				TEAL_BUTTON_COLOR, g_buttons, &g_buttonCount);
			
			CreateModernButton(hWnd, "Update Info", rightColumnX, buttonStartY + verticalSpacing,
				buttonWidth, buttonHeight, 1004,
				GREEN_BUTTON_COLOR, g_buttons, &g_buttonCount);
			
			// 第三行：Admin Mode 和 Close
			CreateModernButton(hWnd, "Admin Mode", leftColumnX, buttonStartY + verticalSpacing * 2,
				buttonWidth, buttonHeight, 1005,
				ADMIN_BUTTON_COLOR, g_buttons, &g_buttonCount);
			
			CreateModernButton(hWnd, "Close", rightColumnX, buttonStartY + verticalSpacing * 2,
				buttonWidth, buttonHeight, 27,
				CLOSE_BUTTON_COLOR, g_buttons, &g_buttonCount);
			
			// 创建功能说明标签
			int infoY = buttonStartY + verticalSpacing * 3 + 5;
			CreateLabel(hWnd, "Quick Access Tools", 50, infoY, windowWidth - 100, 25, 14, TRUE);
			
			// 创建功能说明（分成两列）- 减小字体和间距
			int descY = infoY + 25;
			int descHeight = 18;
			
			// 左列说明
			CreateLabel(hWnd, "ScreenWings: Screen capture", 
				leftColumnX, descY, buttonWidth, descHeight, 9, FALSE);
			CreateLabel(hWnd, "About: Program info", 
				leftColumnX, descY + 18, buttonWidth, descHeight, 9, FALSE);
			CreateLabel(hWnd, "Admin Mode: Advanced controls", 
				leftColumnX, descY + 36, buttonWidth, descHeight, 9, FALSE);
			
			// 右列说明
			CreateLabel(hWnd, "TaskKill: Process terminator", 
				rightColumnX, descY, buttonWidth, descHeight, 9, FALSE);
			CreateLabel(hWnd, "Update: Check version", 
				rightColumnX, descY + 18, buttonWidth, descHeight, 9, FALSE);
			CreateLabel(hWnd, "Close: Exit application", 
				rightColumnX, descY + 36, buttonWidth, descHeight, 9, FALSE);
			
			// 检查默认路径是否存在
			char absolutePath[MAX_PATH];
			ConvertRelativePathToAbsolute(absolutePath, g_screenWingsPath, sizeof(absolutePath));
			bool fileExists = FileExists(absolutePath);
			
			// 创建底部状态栏 - 显示ScreenWings状态
			char statusText[256];
			if (fileExists) {
				// 只显示文件名
				const char* filename = strrchr(g_screenWingsPath, '/');
				if (filename) filename++;
				else filename = g_screenWingsPath;
				sprintf(statusText, "(c) 2026 XXS Tools | ScreenWings: %s (已就绪)", filename);
			} else {
				sprintf(statusText, "(c) 2026 XXS Tools | ScreenWings: 未找到 (./data/screenwings.exe)");
			}
			CreateLabel(hWnd, statusText, 
				50, windowHeight - 25, windowWidth - 100, 20, 9, FALSE);
			
			break;
		}
		
		case WM_DRAWITEM: {
			LPDRAWITEMSTRUCT lpDraw = (LPDRAWITEMSTRUCT)lParam;
			int id = lpDraw->CtlID;
			
			// 查找对应的按钮
			for (int i = 0; i < g_buttonCount; i++) {
				if (g_buttons[i].id == id) {
					DrawModernButton(lpDraw->hDC, lpDraw->rcItem, 
						g_buttons[i].text, g_buttons[i].state,
						g_buttons[i].color);
					return TRUE;
				}
			}
			
			// 查找管理员窗口的按钮
			for (int i = 0; i < g_adminButtonCount; i++) {
				if (g_adminButtons[i].id == id) {
					DrawModernButton(lpDraw->hDC, lpDraw->rcItem, 
						g_adminButtons[i].text, g_adminButtons[i].state,
						g_adminButtons[i].color);
					return TRUE;
				}
			}
			
			break;
		}
		
		case WM_MOUSEMOVE: {
			int x = GetXFromLPARAM(lParam);
			int y = GetYFromLPARAM(lParam);
			UpdateButtonState(hWnd, x, y, false, g_buttons, g_buttonCount);
			break;
		}
		
		case WM_LBUTTONDOWN: {
			int x = GetXFromLPARAM(lParam);
			int y = GetYFromLPARAM(lParam);
			UpdateButtonState(hWnd, x, y, true, g_buttons, g_buttonCount);
			break;
		}
		
		case WM_LBUTTONUP: {
			int x = GetXFromLPARAM(lParam);
			int y = GetYFromLPARAM(lParam);
			UpdateButtonState(hWnd, x, y, false, g_buttons, g_buttonCount);
			
			// 检查点击了哪个按钮
			POINT pt = {x, y};
			for (int i = 0; i < g_buttonCount; i++) {
				if (PtInRect(&g_buttons[i].rect, pt)) {
					PostMessage(hWnd, WM_COMMAND, g_buttons[i].id, 0);
					break;
				}
			}
			break;
		}
		
		case WM_COMMAND: {
			int id = LOWORD(wParam);
			switch (id) {
				case 1001: // ScreenWings
				ExecuteScreenWings(hWnd);
				// 更新状态栏
				InvalidateRect(hWnd, NULL, TRUE);
				break;
				
				case 1002: // TaskKill
				ShowTaskKillDialog(hWnd);
				break;
				
				case 1003: // About
				ShowAboutDialog(hWnd);
				break;
				
				case 1004: // Update Information
				ShowUpdateDialog(hWnd);
				break;
				
				case 1005: // Admin Mode
				ShowAdminConfirmDialog(hWnd);
				break;
				
				case 27: // Close
				DestroyWindow(hWnd);
				break;
			}
			break;
		}
		
		case WM_CTLCOLORSTATIC: {
			HDC hdcStatic = (HDC)wParam;
			// 检查是否是列标题
			HWND hWndStatic = (HWND)lParam;
			char buffer[100];
			GetWindowText(hWndStatic, buffer, sizeof(buffer));
			
			if (strcmp(buffer, "System Tools") == 0 || strcmp(buffer, "Utility Tools") == 0) {
				// 列标题使用特殊颜色
				SetTextColor(hdcStatic, RGB(88, 101, 242));
				SetBkMode(hdcStatic, TRANSPARENT);
				return (LRESULT)GetStockObject(NULL_BRUSH);
			} else if (strcmp(buffer, "Admin Functions") == 0 || strcmp(buffer, "Admin Settings") == 0) {
				// 管理员列标题
				SetTextColor(hdcStatic, RGB(138, 43, 226));
				SetBkMode(hdcStatic, TRANSPARENT);
				return (LRESULT)GetStockObject(NULL_BRUSH);
			} else {
				// 普通标签 - 深色文本在白色背景上
				SetTextColor(hdcStatic, MAIN_TEXT_COLOR);
				SetBkMode(hdcStatic, TRANSPARENT);
				return (LRESULT)GetStockObject(NULL_BRUSH);
			}
		}
		
		case WM_PAINT: {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			
			// 获取客户区
			RECT rc;
			GetClientRect(hWnd, &rc);
			
			// 绘制白色背景
			FillRect(hdc, &rc, CreateSolidBrush(MAIN_BG_COLOR));
			
			// 绘制主容器边框 - 浅灰色
			RECT container = rc;
			container.left += 20;
			container.top += 15;
			container.right -= 20;
			container.bottom -= 15;
			
			HPEN hPen = CreatePen(PS_SOLID, 2, BORDER_COLOR);
			HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
			HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
			
			Rectangle(hdc, container.left, container.top, container.right, container.bottom);
			
			// 绘制标题装饰线 - 蓝色，调整位置
			HPEN hLinePen = CreatePen(PS_SOLID, 1, BUTTON_COLOR);
			SelectObject(hdc, hLinePen);
			MoveToEx(hdc, 50, 85, NULL);
			LineTo(hdc, rc.right - 50, 85);
			
			// 绘制列分隔线（虚线） - 调整位置和长度
			int centerX = rc.right / 2;
			HPEN hDividerPen = CreatePen(PS_DOT, 1, BUTTON_COLOR);
			SelectObject(hdc, hDividerPen);
			MoveToEx(hdc, centerX, 110, NULL);
			LineTo(hdc, centerX, 245);
			
			// 清理
			SelectObject(hdc, hOldPen);
			DeleteObject(hPen);
			DeleteObject(hLinePen);
			DeleteObject(hDividerPen);
			
			EndPaint(hWnd, &ps);
			break;
		}
		
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
		
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
		
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// 管理员窗口过程 - 同样适应460高度
LRESULT CALLBACK AdminWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	static int windowWidth = 720;
	static int windowHeight = 460;  // 窗口高度改为460
	
	switch (message) {
		case WM_CREATE: {
			// 获取窗口尺寸
			RECT rc;
			GetClientRect(hWnd, &rc);
			windowWidth = rc.right - rc.left;
			windowHeight = rc.bottom - rc.top;
			
			// 标题区域 - 紧凑布局
			CreateLabel(hWnd, "SUPER ADMIN PANEL", 50, 15, windowWidth - 100, 40, 28, TRUE);
			CreateLabel(hWnd, "Elevated Privileges | Advanced Controls", 
				50, 60, windowWidth - 100, 25, 12, FALSE);
			
			// 创建列标题 - 放在按钮上方
			int buttonStartY = 110;
			int buttonHeight = 36;
			int buttonWidth = 250;
			int horizontalSpacing = 40;
			int verticalSpacing = 45;
			
			// 计算列位置
			int totalWidth = buttonWidth * 2 + horizontalSpacing;
			int startX = (windowWidth - totalWidth) / 2;
			int leftColumnX = startX;
			int rightColumnX = leftColumnX + buttonWidth + horizontalSpacing;
			
			// 创建列标题
			CreateColumnTitle(hWnd, "Admin Functions", leftColumnX, 90, buttonWidth, 20);
			CreateColumnTitle(hWnd, "Admin Settings", rightColumnX, 90, buttonWidth, 20);
			
			// 第一行：ScreenWings 和 TaskKill
			CreateModernButton(hWnd, "ScreenWings", leftColumnX, buttonStartY, 
				buttonWidth, buttonHeight, 2001,
				ADMIN_BUTTON_COLOR, g_adminButtons, &g_adminButtonCount);
			
			CreateModernButton(hWnd, "TaskKill", rightColumnX, buttonStartY,
				buttonWidth, buttonHeight, 2002,
				ORANGE_BUTTON_COLOR, g_adminButtons, &g_adminButtonCount);
			
			// 第二行：About 和 Color Settings
			CreateModernButton(hWnd, "About", leftColumnX, buttonStartY + verticalSpacing,
				buttonWidth, buttonHeight, 2003,
				TEAL_BUTTON_COLOR, g_adminButtons, &g_adminButtonCount);
			
			CreateModernButton(hWnd, "Color Settings", rightColumnX, buttonStartY + verticalSpacing,
				buttonWidth, buttonHeight, 2004,
				GREEN_BUTTON_COLOR, g_adminButtons, &g_adminButtonCount);
			
			// 第三行：Update 和 Back to Main
			CreateModernButton(hWnd, "Update Info", leftColumnX, buttonStartY + verticalSpacing * 2,
				buttonWidth, buttonHeight, 2005,
				BUTTON_COLOR, g_adminButtons, &g_adminButtonCount);
			
			CreateModernButton(hWnd, "Back to Main", rightColumnX, buttonStartY + verticalSpacing * 2,
				buttonWidth, buttonHeight, 27,
				YELLOW_BUTTON_COLOR, g_adminButtons, &g_adminButtonCount);
			
			// 创建警告说明
			int infoY = buttonStartY + verticalSpacing * 3 + 5;
			CreateLabel(hWnd, "WARNING: Admin Mode Active", 50, infoY, windowWidth - 100, 25, 14, TRUE);
			CreateLabel(hWnd, "You have elevated privileges. Use with caution.", 
				50, infoY + 20, windowWidth - 100, 20, 10, FALSE);
			
			break;
		}
		
		case WM_DRAWITEM: {
			LPDRAWITEMSTRUCT lpDraw = (LPDRAWITEMSTRUCT)lParam;
			int id = lpDraw->CtlID;
			
			for (int i = 0; i < g_adminButtonCount; i++) {
				if (g_adminButtons[i].id == id) {
					DrawModernButton(lpDraw->hDC, lpDraw->rcItem, 
						g_adminButtons[i].text, g_adminButtons[i].state,
						g_adminButtons[i].color);
					return TRUE;
				}
			}
			break;
		}
		
		case WM_MOUSEMOVE: {
			int x = GetXFromLPARAM(lParam);
			int y = GetYFromLPARAM(lParam);
			UpdateButtonState(hWnd, x, y, false, g_adminButtons, g_adminButtonCount);
			break;
		}
		
		case WM_LBUTTONDOWN: {
			int x = GetXFromLPARAM(lParam);
			int y = GetYFromLPARAM(lParam);
			UpdateButtonState(hWnd, x, y, true, g_adminButtons, g_adminButtonCount);
			break;
		}
		
		case WM_LBUTTONUP: {
			int x = GetXFromLPARAM(lParam);
			int y = GetYFromLPARAM(lParam);
			UpdateButtonState(hWnd, x, y, false, g_adminButtons, g_adminButtonCount);
			
			POINT pt = {x, y};
			for (int i = 0; i < g_adminButtonCount; i++) {
				if (PtInRect(&g_adminButtons[i].rect, pt)) {
					PostMessage(hWnd, WM_COMMAND, g_adminButtons[i].id, 0);
					break;
				}
			}
			break;
		}
		
		case WM_COMMAND: {
			int id = LOWORD(wParam);
			switch (id) {
				case 2001: // ScreenWings
				ExecuteScreenWings(hWnd);
				break;
				
				case 2002: // TaskKill
				ShowTaskKillDialog(hWnd);
				break;
				
				case 2003: // About
				ShowAboutDialog(hWnd);
				break;
				
				case 2004: // Colours
				ShowColorDialog(hWnd);
				break;
				
				case 2005: // Update Information
				ShowUpdateDialog(hWnd);
				break;
				
				case 27: // Back to Main
				ShowWindow(hWnd, SW_HIDE);
				ShowWindow(g_hMainWnd, SW_SHOW);
				break;
			}
			break;
		}
		
		case WM_CTLCOLORSTATIC: {
			HDC hdcStatic = (HDC)wParam;
			// 检查是否是列标题
			HWND hWndStatic = (HWND)lParam;
			char buffer[100];
			GetWindowText(hWndStatic, buffer, sizeof(buffer));
			
			if (strcmp(buffer, "Admin Functions") == 0 || strcmp(buffer, "Admin Settings") == 0) {
				// 管理员列标题使用特殊颜色
				SetTextColor(hdcStatic, ADMIN_BUTTON_COLOR);
				SetBkMode(hdcStatic, TRANSPARENT);
				return (LRESULT)GetStockObject(NULL_BRUSH);
			} else {
				// 普通标签 - 深色文本在浅灰色背景上
				SetTextColor(hdcStatic, MAIN_TEXT_COLOR);
				SetBkMode(hdcStatic, TRANSPARENT);
				return (LRESULT)GetStockObject(NULL_BRUSH);
			}
		}
		
		case WM_PAINT: {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			
			RECT rc;
			GetClientRect(hWnd, &rc);
			
			// 绘制浅灰色背景
			FillRect(hdc, &rc, CreateSolidBrush(ADMIN_BG_COLOR));
			
			// 绘制紫色边框容器
			RECT container = rc;
			container.left += 20;
			container.top += 15;
			container.right -= 20;
			container.bottom -= 15;
			
			HPEN hPen = CreatePen(PS_SOLID, 3, ADMIN_BUTTON_COLOR);
			HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
			HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
			
			Rectangle(hdc, container.left, container.top, container.right, container.bottom);
			
			// 绘制标题装饰线
			HPEN hLinePen = CreatePen(PS_SOLID, 1, ADMIN_BUTTON_COLOR);
			SelectObject(hdc, hLinePen);
			MoveToEx(hdc, 50, 85, NULL);
			LineTo(hdc, rc.right - 50, 85);
			
			// 绘制列分隔线（虚线）
			int centerX = rc.right / 2;
			HPEN hDividerPen = CreatePen(PS_DOT, 1, ADMIN_BUTTON_COLOR);
			SelectObject(hdc, hDividerPen);
			MoveToEx(hdc, centerX, 110, NULL);
			LineTo(hdc, centerX, 245);
			
			// 绘制警告图标
			int iconX = rc.right / 2 - 7;
			int iconY = 320;
			HPEN hIconPen = CreatePen(PS_SOLID, 2, ORANGE_BUTTON_COLOR);
			SelectObject(hdc, hIconPen);
			
			// 绘制三角形
			POINT triangle[3] = {
				{iconX, iconY - 10},
				{iconX + 10, iconY + 6},
				{iconX - 10, iconY + 6}
			};
			Polygon(hdc, triangle, 3);
			
			// 绘制感叹号
			MoveToEx(hdc, iconX, iconY - 3, NULL);
			LineTo(hdc, iconX, iconY + 3);
			MoveToEx(hdc, iconX, iconY + 5, NULL);
			LineTo(hdc, iconX, iconY + 6);
			
			// 清理
			SelectObject(hdc, hOldPen);
			DeleteObject(hPen);
			DeleteObject(hLinePen);
			DeleteObject(hDividerPen);
			DeleteObject(hIconPen);
			
			EndPaint(hWnd, &ps);
			break;
		}
		
	case WM_CLOSE:
		ShowWindow(hWnd, SW_HIDE);
		ShowWindow(g_hMainWnd, SW_SHOW);
		break;
		
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// 对话框函数
void ShowTaskKillDialog(HWND hParent) {
	int result = MessageBox(hParent, 
		"This will close the StudentMain.exe\n\nDo you want to continue?",
		"TaskKill Confirmation",
		MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2);
	
	if (result == IDYES) {
		system("taskkill /f /im studentmain.exe >nul 2>&1");
		
		if (GetLastError() == 0) {
			MessageBox(hParent, 
				"StudentMain.exe has been terminated successfully.",
				"Success", 
				MB_OK | MB_ICONINFORMATION);
		} else {
			MessageBox(hParent, 
				"Failed to terminate StudentMain.exe.\nIt might not be running.",
				"Error", 
				MB_OK | MB_ICONWARNING);
		}
	}
}

void ShowAboutDialog(HWND hParent) {
	MessageBox(hParent,
		"XXS Tools v2.0\n\n"
		"Made by: XXS Development Team\n"
		"Version: 2.0 (Professional Edition)\n"
		"Year: 2026\n\n"
		"For educational purposes only.",
		"About XXS Tools",
		MB_OK | MB_ICONINFORMATION);
}

void ShowUpdateDialog(HWND hParent) {
	MessageBox(hParent,
		"Update Information\n\n"
		"Current Version: 2.0.0\n"
		"Status: Up to date\n\n"
		"No updates available at this time.\n"
		"Please check back later.",
		"Update Information",
		MB_OK | MB_ICONINFORMATION);
}

void ShowColorDialog(HWND hParent) {
	MessageBox(hParent,
		"Color Settings\n\n"
		"Select color scheme:\n"
		"1. Light Theme (Current)\n"
		"2. Dark Blue\n"
		"3. Charcoal Gray\n"
		"4. Forest Green\n\n"
		"Note: Color changes require restart.",
		"Color Selection",
		MB_ICONINFORMATION | MB_OK);
}

void ShowAdminConfirmDialog(HWND hParent) {
	int result = MessageBox(hParent,
		"Admin Mode Access\n\n"
		"You are about to enter Super Admin mode.\n"
		"This grants elevated privileges.\n\n"
		"Do you want to proceed?",
		"Admin Mode Confirmation",
		MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2);
	
	if (result == IDYES) {
		ShowWindow(g_hMainWnd, SW_HIDE);
		
		if (!g_hAdminWnd) {
			int screenWidth = GetSystemMetrics(SM_CXSCREEN);
			int screenHeight = GetSystemMetrics(SM_CYSCREEN);
			int windowWidth = 720;
			int windowHeight = 460;  // 使用相同高度
			int windowX = (screenWidth - windowWidth) / 2;
			int windowY = (screenHeight - windowHeight) / 2;
			
			g_hAdminWnd = CreateWindowEx(
				0,
				"XXSToolsAdmin",
				"XXS Tools (Super Admin)",
				WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
				windowX, windowY,
				windowWidth, windowHeight,
				NULL, NULL, g_hInst, NULL
				);
		}
		
		if (g_hAdminWnd) {
			ShowWindow(g_hAdminWnd, SW_SHOW);
			UpdateWindow(g_hAdminWnd);
		}
	}
}
