//-----------------------------------------------------------------
// Game Engine Object
// C++ Source - GameEngine.cpp - version v7_01
//-----------------------------------------------------------------

//-----------------------------------------------------------------
// Include Files
//-----------------------------------------------------------------
#include "GameEngine.h"

#define _USE_MATH_DEFINES	// necessary for including (among other values) PI  - see math.h
#include <math.h>			// used in various draw methods
#include <stdio.h>
#include <tchar.h>			// used for unicode strings

#include <memory.h>
#include <vector>			// using std::vector for tab control logic

//-----------------------------------------------------------------
// Static Variable Initialization
//-----------------------------------------------------------------
GameEngine* GameEngine::m_GameEnginePtr{nullptr};

//-----------------------------------------------------------------
// Windows Functions
//-----------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWindow, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Route all Windows messages to the game engine
	return GameEngine::GetSingleton()->HandleEvent(hWindow, msg, wParam, lParam);
}

DWORD WINAPI KeybThreadProc(GameEngine* gamePtr)
{
	return gamePtr->KeybThreadProc();
}

//-----------------------------------------------------------------
// GameEngine Constructor(s)/Destructor
//-----------------------------------------------------------------
GameEngine::GameEngine() :	m_hInstance(NULL), 
							m_Window(NULL),
							m_TitlePtr(0),
							m_FrameDelay(20),		// 50 FPS default
							m_RunGameLoop(false),
							m_KeybRunning(true),	// create the keyboard monitoring thread
							m_KeyListPtr(nullptr),
							m_KeybMonitor(0x0),		// binary ; 0 = key not pressed, 1 = key pressed
							m_IsPainting(false),
							m_IsDoublebuffering(false),
							m_ColDraw(RGB(0,0,0)),
							m_FontDraw(0),
							m_GamePtr(0),
							m_PaintDoublebuffered(true),
							m_Fullscreen(false),
							m_WindowRegionPtr(0)
{
	m_hKeybThread = CreateThread(nullptr, NULL, (LPTHREAD_START_ROUTINE) ::KeybThreadProc, this, NULL, &m_dKeybThreadID);
}

GameEngine::~GameEngine()
{
	// Clean up the keyboard monitoring thread
	m_KeybRunning = false;
	WaitForSingleObject( m_hKeybThread, INFINITE );
	CloseHandle( m_hKeybThread );	
	
	// clean up keyboard monitor buffer after the thread that uses it is closed
	if (m_KeyListPtr != 0) 
	{
		delete m_KeyListPtr;
		m_KeyListPtr = 0;
	}

	// clean up the font
	if (m_FontDraw != 0) 
	{
		DeleteObject(m_FontDraw);
		m_FontDraw = 0;
	}

	// delete the game object
	delete m_GamePtr;
}

//-----------------------------------------------------------------
// Game Engine Static Methods
//-----------------------------------------------------------------
GameEngine* GameEngine::GetSingleton()
{
	if ( m_GameEnginePtr == nullptr) m_GameEnginePtr = new GameEngine();
	return m_GameEnginePtr;
}

void GameEngine::SetGame(AbstractGame* gamePtr)
{
	m_GamePtr = gamePtr;
}

DWORD GameEngine::KeybThreadProc()
{
	while (m_KeybRunning)
	{
		GetFocus();																					// for some reason the next GetFocus() doesn't work without this GetFocus() call
		if (m_KeyListPtr != nullptr && GetFocus() == m_Window)
		{
			int count{};
			int key{ m_KeyListPtr[0] };

			while (key != '\0' && count < (8 * sizeof(unsigned int)))
			{	
				if ( !(GetAsyncKeyState(key)<0) ) // key is not pressed
				{	    
					if (m_KeybMonitor & (0x1 << count)) {
						m_GamePtr->KeyPressed(key); // if the bit was 1, this fires a keypress
					}
					m_KeybMonitor &= ~(0x1 << count);   // the bit is set to 0: key is not pressed
				}
				else m_KeybMonitor |= (0x1 << count);	// the bit is set to 1: key is pressed

				key = m_KeyListPtr[++count]; // increase count and get next key
			}
		}	

		Sleep(1000 / KEYBCHECKRATE);
	}
	return 0;
}

//-----------------------------------------------------------------
// Game Engine General Methods
//-----------------------------------------------------------------
void GameEngine::SetTitle(const tstring& titleRef)
{
	delete m_TitlePtr; // delete the title string if it already exists
	m_TitlePtr = new tstring(titleRef);
}

bool GameEngine::Run(HINSTANCE hInstance, int cmdShow)
{
	MSG msg;
	ULONGLONG iTickTrigger{};
	ULONGLONG iTickCount{};

	// create the game engine object, exit if failure
	if (GameEngine::GetSingleton() == nullptr) return false;

	// set the instance member variable of the game engine
	GameEngine::GetSingleton()->SetInstance(hInstance);

	// Game Initialization
	m_GamePtr->Initialize(hInstance);

	// Initialize the game engine
	if (!GameEngine::GetSingleton()->ClassRegister(cmdShow)) return false;

	// Attach the keyboard thread to the main thread. This gives the keyboard events access to the window state
	// In plain English: this allows a KeyPressed() event to hide the cursor of the window. 
	AttachThreadInput(m_dKeybThreadID, GetCurrentThreadId(), true);

	// Enter the main message loop
	while (true)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			// Process the message
			if (msg.message == WM_QUIT) break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			// Make sure the game engine isn't sleeping
			if (m_RunGameLoop)
			{
				// Check the tick count to see if a cycle has elapsed
				iTickCount = GetTickCount64();
				if (iTickCount > iTickTrigger)
				{
					iTickTrigger = iTickCount + m_FrameDelay;

					// Do user defined keyboard check
					//if (GetFocus() == m_Window) m_GamePtr->CheckKeyboard();
					m_GamePtr->CheckKeyboard();

					// Get rectangle and HDC
					RECT rect;
					HDC hDC = GetDC(m_Window);
					GetClientRect(m_Window, &rect);

					// Double buffering code
					HDC hBufferDC = CreateCompatibleDC(hDC);
					// Create the buffer
					HBITMAP hBufferBmp = CreateCompatibleBitmap(hDC, m_Width, m_Height);
					HBITMAP hOldBmp = (HBITMAP) SelectObject(hBufferDC, hBufferBmp);

					// Do user defined drawing functions on the buffer, parameters added
					// for ease of drawing
					m_HdcDraw = hBufferDC;
					m_RectDraw = rect;
					m_IsDoublebuffering = true;
					m_GamePtr->Tick();
					m_GamePtr->Paint(rect);
					m_IsDoublebuffering = false;

					// As a last step copy the memdc to the hdc
					BitBlt(hDC, 0, 0, m_Width, m_Height, hBufferDC, 0, 0, SRCCOPY);

					// Reset the old bmp of the buffer, mainly for show since we kill it anyway
					SelectObject(hBufferDC, hOldBmp);
					// Kill the buffer
					DeleteObject(hBufferBmp);
					DeleteDC(hBufferDC);

					// Release HDC
					ReleaseDC(m_Window, hDC);
				}                             
				else Sleep(1);//Sleep for one ms te bring cpu load from 100% to 1%. if removed this loops like roadrunner
			}
			else WaitMessage(); // if the engine is sleeping or the loop isn't supposed to run, wait for the next windows message.
		}
	}
	return msg.wParam?true:false;
}

bool GameEngine::SetGameValues(const tstring& titleRef, WORD icon, WORD smallIcon, int width = 640, int height = 480)
{
	SetTitle(titleRef);
	SetIcon(icon);
	SetSmallIcon(smallIcon);
	SetWidth(width);
	SetHeight(height);

	return true;
}

void GameEngine::ShowMousePointer(bool value) const
{
	// set the value
	ShowCursor(value);	
	
	// redraw the screen
	InvalidateRect(m_Window, 0, true);
}

bool GameEngine::SetWindowRegion(const HitRegion* regionPtr)
{
	if (m_Fullscreen) return false;

	if (regionPtr == 0) 
	{	
		// turn off window region
		SetWindowRgn(m_Window, 0, true);

		// delete the buffered window region (if it exists)
		delete m_WindowRegionPtr;
		m_WindowRegionPtr = nullptr;
	}
	else 
	{
		// if there is already a window region set, release the buffered region object
		if (m_WindowRegionPtr != 0)
		{
			// turn off window region for safety
			SetWindowRgn(m_Window, 0, true);
				
			// delete the buffered window region 
			delete m_WindowRegionPtr;
			m_WindowRegionPtr = nullptr;
		}

		// create a copy of the submitted region (windows will lock the region handle that it receives)
		m_WindowRegionPtr = regionPtr->Clone();

		// translate region coordinates in the client field to window coordinates, taking title bar and frame into account
		m_WindowRegionPtr->Move(GetSystemMetrics(SM_CXFIXEDFRAME), GetSystemMetrics(SM_CYFIXEDFRAME) + GetSystemMetrics(SM_CYCAPTION));

		// set the window region
		SetWindowRgn(m_Window, m_WindowRegionPtr->GetHandle(), true);
	}

	return true;
}

bool GameEngine::HasWindowRegion() const
{
	return (m_WindowRegionPtr?true:false);
}

bool GameEngine::GoFullscreen()
{
	// exit if already in fullscreen mode
	if (m_Fullscreen) return false;

	// turn off window region without redraw
	SetWindowRgn(m_Window, 0, false);

	DEVMODE newSettings;	

	// request current screen settings
	EnumDisplaySettings(0, 0, &newSettings);

	//  set desired screen size/res	
 	newSettings.dmPelsWidth  = GetWidth();		
	newSettings.dmPelsHeight = GetHeight();		
	newSettings.dmBitsPerPel = 32;		

	//specify which aspects of the screen settings we wish to change 
 	newSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

	// attempt to apply the new settings 
	long result = ChangeDisplaySettings(&newSettings, CDS_FULLSCREEN);

	// exit if failure, else set datamember to fullscreen and return true
	if ( result != DISP_CHANGE_SUCCESSFUL )	return false;
	else 
	{
		// store the location of the window
		m_OldLoc = GetLocation();

		// switch off the title bar
	    DWORD dwStyle = (DWORD) GetWindowLongPtr(m_Window, GWL_STYLE);
	    dwStyle &= ~WS_CAPTION;
	    SetWindowLongPtr(m_Window, GWL_STYLE, dwStyle);

		// move the window to (0,0)
		SetWindowPos(m_Window, 0, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		InvalidateRect(m_Window, 0, true);		

		m_Fullscreen = true;

		return true;
	}
}

bool GameEngine::GoWindowedMode()
{
	// exit if already in windowed mode
	if (!m_Fullscreen) return false;

	// this resets the screen to the registry-stored values
  	ChangeDisplaySettings(0, 0);

	// replace the title bar
	DWORD dwStyle = (DWORD) GetWindowLongPtr(m_Window, GWL_STYLE);
    dwStyle = dwStyle | WS_CAPTION;
    SetWindowLongPtr(m_Window, GWL_STYLE, dwStyle);

	// move the window back to its old position
	SetWindowPos(m_Window, 0, m_OldLoc.x, m_OldLoc.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	InvalidateRect(m_Window, 0, true);

	m_Fullscreen = false;

	return true;
}

bool GameEngine::IsFullscreen() const
{
	return m_Fullscreen;
}

bool GameEngine::ClassRegister(int cmdShow)
{
  WNDCLASSEX    wndclass;

  // Create the window class for the main window
  wndclass.cbSize         = sizeof(wndclass);
  wndclass.style          = CS_HREDRAW | CS_VREDRAW;
  wndclass.lpfnWndProc    = WndProc;
  wndclass.cbClsExtra     = 0;
  wndclass.cbWndExtra     = 0;
  wndclass.hInstance      = m_hInstance;
  wndclass.hIcon          = LoadIcon(m_hInstance, MAKEINTRESOURCE(GetIcon()));
  wndclass.hIconSm        = LoadIcon(m_hInstance, MAKEINTRESOURCE(GetSmallIcon()));
  wndclass.hCursor        = LoadCursor(NULL, IDC_ARROW);
  wndclass.hbrBackground  = m_PaintDoublebuffered?NULL:(HBRUSH)(COLOR_WINDOW + 1);
  wndclass.lpszMenuName   = NULL;
  wndclass.lpszClassName  = m_TitlePtr->c_str();

  // Register the window class
  if (!RegisterClassEx(&wndclass))
    return false;

  // Calculate the window size and position based upon the size
  int iWindowWidth = m_Width + GetSystemMetrics(SM_CXFIXEDFRAME) * 2,
      iWindowHeight = m_Height + GetSystemMetrics(SM_CYFIXEDFRAME) * 2 +
        GetSystemMetrics(SM_CYCAPTION);

  if (wndclass.lpszMenuName != NULL)
    iWindowHeight += GetSystemMetrics(SM_CYMENU);

  int iXWindowPos = (GetSystemMetrics(SM_CXSCREEN) - iWindowWidth) / 2,
      iYWindowPos = (GetSystemMetrics(SM_CYSCREEN) - iWindowHeight) / 2;

  // Create the window
  m_Window = CreateWindow(	m_TitlePtr->c_str(), 
							m_TitlePtr->c_str(), 
							WS_POPUPWINDOW | WS_CAPTION | WS_MINIMIZEBOX | WS_CLIPCHILDREN, 
							iXWindowPos, 
							iYWindowPos, 
							iWindowWidth, 
							iWindowHeight, 
							NULL, 
							NULL, 
							m_hInstance, 
							NULL);
  if (!m_Window)
    return false;

  // Show and update the window
  ShowWindow(m_Window, cmdShow);
  UpdateWindow(m_Window);

  return true;
}

bool GameEngine::IsKeyDown(int vKey) const
{
	if (GetAsyncKeyState(vKey) < 0) return true;
	else return false;
}

void GameEngine::SetKeyList(const tstring& keyListRef)
{
	if (keyListRef.c_str() != NULL) delete m_KeyListPtr; // clear list if a list already exists

	m_KeyListPtr = (TCHAR*) malloc((keyListRef.size() + 1) * sizeof(TCHAR)); // make place for this amount of keys + 1

	for (int count = 0; count < (int) keyListRef.size() + 1; ++count) 
	{
		TCHAR key = keyListRef.c_str()[count]; 
		m_KeyListPtr[count] = (key > 96 && key < 123)? key-32 : key; // insert the key, coverted to uppercase if supplied character is lowercase
	}
}

void GameEngine::Quit() const
{
	PostMessage(GameEngine::GetWindow(), WM_DESTROY, 0, 0);
}

bool GameEngine::MessageContinue(const tstring& textRef) const
{
	// MessageBox define is undef'd at begin of GameEngine.h
	#ifdef UNICODE						
		return MessageBoxW(GetWindow(), textRef.c_str(), m_TitlePtr->c_str(), MB_ICONWARNING | MB_OKCANCEL) == IDOK;
	#else
		return MessageBoxA(GetWindow(), textRef.c_str(), m_TitlePtr->c_str(), MB_ICONWARNING | MB_OKCANCEL) == IDOK;
	#endif 
}

void GameEngine::MessageBox(const tstring& textRef) const
{
	// MessageBox define is undef'd at begin of GameEngine.h
	#ifdef UNICODE						
		MessageBoxW(GetWindow(), textRef.c_str(), m_TitlePtr->c_str(), MB_ICONEXCLAMATION | MB_OK);
	#else
		MessageBoxA(GetWindow(), textRef.c_str(), m_TitlePtr->c_str(), MB_ICONEXCLAMATION | MB_OK);
	#endif 
}

void GameEngine::MessageBox(int value) const
{
	tstringstream buffer;
	buffer << value;

	MessageBox(buffer.str());
}

void GameEngine::MessageBox(size_t value) const
{
	tstringstream buffer;
	buffer << value;

	MessageBox(buffer.str());
}

void GameEngine::MessageBox(double value) const
{
	tstringstream buffer;
	buffer << value;

	MessageBox(buffer.str());
}

static bool CALLBACK EnumInsertChildrenProc(HWND hwnd, LPARAM lParam)
{
	std::vector<HWND>* rowPtr{ (std::vector<HWND>*) lParam };

	rowPtr->push_back(hwnd); // fill in every element in the vector

	return true;
}

void GameEngine::TabNext(HWND ChildWindow) const
{
	std::vector<HWND> childWindows; 

	EnumChildWindows(m_Window, (WNDENUMPROC) EnumInsertChildrenProc, (LPARAM) &childWindows);

	int position{};
	HWND temp{ childWindows[position] };
	while(temp != ChildWindow) temp = childWindows[++position]; // find the childWindow in the vector

	if (position == childWindows.size() - 1) SetFocus(childWindows[0]);
	else SetFocus(childWindows[position + 1]);
}

void GameEngine::TabPrevious(HWND ChildWindow) const
{	
	std::vector<HWND> childWindows; 

	EnumChildWindows(m_Window, (WNDENUMPROC) EnumInsertChildrenProc, (LPARAM) &childWindows);

	int position{ (int)childWindows.size() - 1 };
	HWND temp{ childWindows[position] };
	while(temp != ChildWindow) temp = childWindows[--position]; // find the childWindow in the vector

	if (position == 0) SetFocus(childWindows[childWindows.size() - 1]);
	else SetFocus(childWindows[position - 1]);
}

SIZE GameEngine::CalculateTextDimensions(const tstring& text, Font* fontPtr) const
{
	SIZE size;
	HDC hdc = GetDC(NULL);
	SelectObject(hdc, fontPtr->GetHandle());    //attach font to hdc

	GetTextExtentPoint32(hdc, text.c_str(), (int) text.size(), &size);

	ReleaseDC(NULL, hdc);

	return size;
}

SIZE GameEngine::CalculateTextDimensions(const tstring& text, Font* fontPtr, RECT rect) const
{
	SIZE size;
	HDC hdc = GetDC(NULL);
	SelectObject(hdc, fontPtr->GetHandle());    //attach font to hdc

	GetTextExtentPoint32(hdc, text.c_str(), (int) text.size(), &size);

	int height = DrawText(hdc, text.c_str(), (int) text.size(), &rect, DT_CALCRECT);

	if (size.cx > rect.right - rect.left)
	{
		size.cx = rect.right - rect.left;
		size.cy = height;
	}

	ReleaseDC(NULL, hdc);

	return size;
}

bool GameEngine::DrawLine(int x1, int y1, int x2, int y2, HDC hDC) const
{
	HPEN hOldPen, hNewPen = CreatePen(PS_SOLID, 1, m_ColDraw);
	hOldPen = (HPEN) SelectObject(hDC, hNewPen);
	MoveToEx(hDC, x1, y1, nullptr);
	LineTo(hDC, x2, y2);
	MoveToEx(hDC, 0, 0, nullptr); // reset the position - sees to it that eg. AngleArc draws from 0,0 instead of the last position of DrawLine
	SelectObject(hDC, hOldPen);
	DeleteObject(hNewPen);
	
	return true;
}

bool GameEngine::DrawLine(int x1, int y1, int x2, int y2) const
{
	if (m_IsDoublebuffering || m_IsPainting) return DrawLine(x1, y1, x2, y2, m_HdcDraw);
	else return false;
}

bool GameEngine::DrawPolygon(const POINT ptsArr[], int count, bool close, HDC hDC) const
{
	HPEN hOldPen, hNewPen = CreatePen(PS_SOLID, 1, m_ColDraw);
	hOldPen = (HPEN) SelectObject(hDC, hNewPen);

	FormPolygon(ptsArr, count, close, hDC);

	SelectObject(hDC, hOldPen);
	DeleteObject(hNewPen);	

	return true;
}

bool GameEngine::DrawPolygon(const POINT ptsArr[], int count, bool close) const
{
	if (m_IsDoublebuffering || m_IsPainting) return DrawPolygon(ptsArr, count, close, m_HdcDraw);
	else return false;
}

bool GameEngine::DrawPolygon(const POINT ptsArr[], int count) const
{
	if (m_IsDoublebuffering || m_IsPainting) return DrawPolygon(ptsArr, count, false, m_HdcDraw);
	else return false;
}

bool GameEngine::FillPolygon(const POINT ptsArr[], int count, bool close, HDC hDC) const
{
	HPEN hOldPen, hNewPen = CreatePen(PS_SOLID, 1, m_ColDraw);
	HBRUSH hOldBrush, hNewBrush = CreateSolidBrush(m_ColDraw);
	hOldPen = (HPEN) SelectObject(hDC, hNewPen);
	hOldBrush = (HBRUSH) SelectObject(hDC, hNewBrush);

	BeginPath(hDC);

	FormPolygon(ptsArr, count, close, hDC);

	EndPath(hDC);
	StrokeAndFillPath(hDC);

	SelectObject(hDC, hOldPen);
	SelectObject(hDC, hOldBrush);

	DeleteObject(hNewPen);
	DeleteObject(hNewBrush);

	return true;
}

bool GameEngine::FillPolygon(const POINT ptsArr[], int count, bool close) const
{
	if (m_IsDoublebuffering || m_IsPainting) return FillPolygon(ptsArr, count, close, m_HdcDraw);
	else return false;
}

bool GameEngine::FillPolygon(const POINT ptsArr[], int count) const
{
	if (m_IsDoublebuffering || m_IsPainting) return FillPolygon(ptsArr, count, false, m_HdcDraw);
	else return false;
}

void GameEngine::FormPolygon(const POINT ptsArr[], int count, bool close, HDC hDC) const
{
	if (!close) Polyline(hDC, ptsArr, count);
	else
	{
		POINT* newPtsArr= new POINT[count+1]; // interesting case: this code does not work with memory allocation at compile time => demo case for dynamic memory use
		for (int i = 0; i < count; i++) newPtsArr[i] = ptsArr[i];
		newPtsArr[count] = ptsArr[0];

		Polyline(hDC, newPtsArr, count+1);

		delete[] newPtsArr;
	}
}

bool GameEngine::DrawRect(int x, int y, int width, int height, HDC hDC) const
{
	HPEN hOldPen, hNewPen = CreatePen(PS_SOLID, 1, m_ColDraw);
	hOldPen = (HPEN) SelectObject(hDC, hNewPen);
	
	POINT pts[4] = {x, y, x + width -1, y, x + width-1, y + height-1, x, y + height-1};
	DrawPolygon(pts, 4, true, hDC);

	SelectObject(hDC, hOldPen);
	DeleteObject(hNewPen);	

	return true;
}

bool GameEngine::DrawRect(int x, int y, int width, int height) const
{
	if (m_IsDoublebuffering || m_IsPainting) return DrawRect(x, y, width, height, m_HdcDraw);
	else return false;
}

bool GameEngine::FillRect(int x, int y, int width, int height, HDC hDC) const
{
	HBRUSH hOldBrush, hNewBrush = CreateSolidBrush(m_ColDraw);
	HPEN hOldPen, hNewPen = CreatePen(PS_SOLID, 1, m_ColDraw);

	hOldBrush = (HBRUSH) SelectObject(hDC, hNewBrush);
	hOldPen = (HPEN) SelectObject(hDC, hNewPen);
	
	Rectangle(hDC, x, y, x + width, y + height);
						
	SelectObject(hDC, hOldPen);
	SelectObject(hDC, hOldBrush);

	DeleteObject(hNewPen);
	DeleteObject(hNewBrush);

	return true;
}


bool GameEngine::FillRect(int x, int y, int width, int height, int opacity) const
{
	HDC tempHdc         = CreateCompatibleDC(m_HdcDraw);
	BLENDFUNCTION blend = {AC_SRC_OVER, 0, (BYTE) opacity, 0};

	RECT dim;
	dim.left = 0;
	dim.top = 0;
	dim.right = width;
	dim.bottom = height;

	HBITMAP hbitmap;       // bitmap handle 
	BITMAPINFO bmi;        // bitmap header 

	// setup bitmap info   
	ZeroMemory(&bmi, sizeof(BITMAPINFO));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = width;
	bmi.bmiHeader.biHeight = height;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;         // four 8-bit components 
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biSizeImage = width * height * 4;

	// create our DIB section and select the bitmap into the dc 
	hbitmap = CreateDIBSection(tempHdc, &bmi, DIB_RGB_COLORS, nullptr, NULL, 0x0);
	SelectObject(tempHdc, hbitmap);

	//SetDCPenColor(tempHdc, RGB(0,0,255));
	//SetDCBrushColor(tempHdc, RGB(0,0,255));
	HBRUSH fillBrush = CreateSolidBrush(m_ColDraw);
	::FillRect(tempHdc, &dim, fillBrush);

	AlphaBlend(m_HdcDraw, x, y, width, height, tempHdc, dim.left, dim.top, dim.right, dim.bottom, blend); 

	DeleteObject(fillBrush);
	DeleteObject(hbitmap);
	DeleteObject(tempHdc);

	return true;
}

bool GameEngine::FillRect(int x, int y, int width, int height) const
{
	if (m_IsDoublebuffering || m_IsPainting) return FillRect(x, y, width, height, m_HdcDraw);
	else return false;
}

bool GameEngine::DrawRoundRect(int x, int y, int width, int height, int radius, HDC hDC) const
{
	HPEN hOldPen, hNewPen = CreatePen(PS_SOLID, 1, m_ColDraw);
	hOldPen = (HPEN) SelectObject(hDC, hNewPen);
	
	BeginPath(hDC);

	RoundRect(hDC, x, y, x + width, y + height, radius, radius);

	EndPath(hDC);
	StrokePath(hDC);

	SelectObject(hDC, hOldPen);
	DeleteObject(hNewPen);	

	return true;
}

bool GameEngine::DrawRoundRect(int x, int y, int width, int height, int radius) const
{
	if (m_IsDoublebuffering || m_IsPainting) return DrawRoundRect(x, y, width, height, radius, m_HdcDraw);
	else return false;
}

bool GameEngine::FillRoundRect(int x, int y, int width, int height, int radius, HDC hDC) const
{
	HBRUSH hOldBrush, hNewBrush = CreateSolidBrush(m_ColDraw);
	HPEN hOldPen, hNewPen = CreatePen(PS_SOLID, 1, m_ColDraw);

	hOldBrush = (HBRUSH) SelectObject(hDC, hNewBrush);
	hOldPen = (HPEN) SelectObject(hDC, hNewPen);
	
	RoundRect(hDC, x, y, x + width, y + height, radius, radius);
						
	SelectObject(hDC, hOldPen);
	SelectObject(hDC, hOldBrush);

	DeleteObject(hNewPen);
	DeleteObject(hNewBrush);

	return true;
}

bool GameEngine::FillRoundRect(int x, int y, int width, int height, int radius) const
{
	if (m_IsDoublebuffering || m_IsPainting) return FillRoundRect(x, y, width, height, radius, m_HdcDraw);
	else return false;
}

bool GameEngine::DrawOval(int x, int y, int width, int height, HDC hDC) const
{
	HPEN hOldPen, hNewPen = CreatePen(PS_SOLID, 1, m_ColDraw);
	hOldPen = (HPEN) SelectObject(hDC, hNewPen);
	
	Arc(hDC, x, y, x + width, y + height, x, y + height/2, x, y + height/2);

	SelectObject(hDC, hOldPen);
	DeleteObject(hNewPen);	

	return true;
}

bool GameEngine::DrawOval(int x, int y, int width, int height) const
{
	if (m_IsDoublebuffering || m_IsPainting) return DrawOval(x, y, width, height, m_HdcDraw);
	else return false;
}

bool GameEngine::FillOval(int x, int y, int width, int height, HDC hDC) const
{
	HBRUSH hOldBrush, hNewBrush = CreateSolidBrush(m_ColDraw);
	HPEN hOldPen, hNewPen = CreatePen(PS_SOLID, 1, m_ColDraw);

	hOldBrush = (HBRUSH) SelectObject(hDC, hNewBrush);
	hOldPen = (HPEN) SelectObject(hDC, hNewPen);
	
	Ellipse(hDC, x, y, x + width, y + height);
						
	SelectObject(hDC, hOldPen);
	SelectObject(hDC, hOldBrush);

	DeleteObject(hNewPen);
	DeleteObject(hNewBrush);

	return true;
}

bool GameEngine::FillOval(int x, int y, int width, int height, int opacity) const
{
	COLORREF color = m_ColDraw;
	if (color == RGB(0, 0, 0)) color = RGB(0, 0, 1);

	HDC tempHdc         = CreateCompatibleDC(m_HdcDraw);

	RECT dim;
	dim.left = 0;
	dim.top = 0;
	dim.right = width;
	dim.bottom = height;

	HBITMAP hbitmap;       // bitmap handle 
	BITMAPINFO bmi;        // bitmap header 

	// setup bitmap info   
	ZeroMemory(&bmi, sizeof(BITMAPINFO));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = width;
	bmi.bmiHeader.biHeight = height;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;         // four 8-bit components 
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biSizeImage = width * height * 4;

	// create our DIB section and select the bitmap into the dc 
	int* dataPtr = nullptr;
	hbitmap = CreateDIBSection(tempHdc, &bmi, DIB_RGB_COLORS, (void**) &dataPtr, NULL, 0x0);
	SelectObject(tempHdc, hbitmap);

	memset(dataPtr, 0, width * height);
	
	HBRUSH hOldBrush, hNewBrush = CreateSolidBrush(color);
	HPEN hOldPen, hNewPen = CreatePen(PS_SOLID, 1, color);

	hOldBrush = (HBRUSH) SelectObject(tempHdc, hNewBrush);
	hOldPen = (HPEN) SelectObject(tempHdc, hNewPen);
	
	Ellipse(tempHdc, 0, 0, width, height);

    for (int count = 0; count < width * height; count++)
    {                   
        if (dataPtr[count] != 0)                                                                            // set alpha channel and premultiply
        {
            //dataPtr[count] = 0x7F7F0000;
            unsigned char* pos = (unsigned char*) &(dataPtr[count]);
            pos[0] = (int) pos[0] * opacity / 255;
            pos[1] = (int) pos[1] * opacity / 255;
            pos[2] = (int) pos[2] * opacity / 255;
            pos[3] = opacity;
        }
    }

	SelectObject(tempHdc, hOldPen);
	SelectObject(tempHdc, hOldBrush);
	
	BLENDFUNCTION blend = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
	AlphaBlend(m_HdcDraw, x, y, width, height, tempHdc, dim.left, dim.top, dim.right, dim.bottom, blend);

	DeleteObject(hNewPen);
	DeleteObject(hNewBrush);
	DeleteObject(hbitmap);
	DeleteObject(tempHdc);

	return true;
}

bool GameEngine::FillOval(int x, int y, int width, int height) const
{
	if (m_IsDoublebuffering || m_IsPainting) return FillOval(x, y, width, height, m_HdcDraw);
	else return false;
}

bool GameEngine::DrawArc(int x, int y, int width, int height, int startDegree, int angle, HDC hDC) const
{
	if (angle == 0) return false;
	if (angle > 360) { DrawOval(x, y, width, height, hDC); }
	else
	{
		HPEN hOldPen, hNewPen = CreatePen(PS_SOLID, 1, m_ColDraw);
		hOldPen = (HPEN) SelectObject(hDC, hNewPen);
		
		POINT ptStart = AngleToPoint(x, y, width, height, startDegree);
		POINT ptEnd = AngleToPoint(x, y, width, height, startDegree + angle);
		
		if (angle > 0) Arc(hDC, x, y, x + width, y + height, ptStart.x, ptStart.y, ptEnd.x, ptEnd.y);
		else Arc(hDC, x, y, x + width, y + height, ptEnd.x, ptEnd.y, ptStart.x, ptStart.y);

		SelectObject(hDC, hOldPen);
		DeleteObject(hNewPen);	
	}

	return true;
}

bool GameEngine::DrawArc(int x, int y, int width, int height, int startDegree, int angle) const
{
	if (m_IsDoublebuffering || m_IsPainting) return DrawArc(x, y, width, height, startDegree, angle, m_HdcDraw);
	else return false;
}

bool GameEngine::FillArc(int x, int y, int width, int height, int startDegree, int angle, HDC hDC) const
{	
	if (angle == 0) return false;
	if (angle > 360) { FillOval(x, y, width, height, hDC); }
	else
	{
		HBRUSH hOldBrush, hNewBrush = CreateSolidBrush(m_ColDraw);
		HPEN hOldPen, hNewPen = CreatePen(PS_SOLID, 1, m_ColDraw);

		hOldBrush = (HBRUSH) SelectObject(hDC, hNewBrush);
		hOldPen = (HPEN) SelectObject(hDC, hNewPen);

		POINT ptStart = AngleToPoint(x, y, width, height, startDegree);
		POINT ptEnd = AngleToPoint(x, y, width, height, startDegree + angle);
		
		if (angle >0) Pie(hDC, x, y, x + width, y + height, ptStart.x, ptStart.y, ptEnd.x, ptEnd.y);
		else Pie(hDC, x, y, x + width, y + height, ptEnd.x, ptEnd.y, ptStart.x, ptStart.y);

		SelectObject(hDC, hOldPen);
		SelectObject(hDC, hOldBrush);

		DeleteObject(hNewPen);
		DeleteObject(hNewBrush);
	}

	return true;
}


bool GameEngine::FillArc(int x, int y, int width, int height, int startDegree, int angle) const
{
	if (m_IsDoublebuffering || m_IsPainting) return FillArc(x, y, width, height, startDegree, angle, m_HdcDraw);
	else return false;
}

POINT GameEngine::AngleToPoint(int x, int y, int width, int height, int angle) const
{
	POINT pt;

	// if necessary adjust angle so that it has a value between 0 and 360 degrees
	if (angle > 360 || angle < -360) angle = angle % 360;
	if (angle < 0) angle += 360;

	// default values for standard angles
	if (angle == 0) { pt.x = x + width; pt.y = y + (int) (height / 2); }
	else if (angle == 90) { pt.x = x + (int) (width / 2); pt.y = y; }
	else if (angle == 180) { pt.x = x; pt.y = y + (int) (height / 2); }
	else if (angle == 270) { pt.x = x + (int) (width / 2); pt.y = y + height; }
	// else calculate non-default values
	else
	{
		// point on the ellipse = "stelsel" of the cartesian equation of the ellipse combined with y = tg(alpha) * x
		// using the equation for ellipse with 0,0 in the center of the ellipse
		double aSquare = pow(width/2.0, 2);
		double bSquare = pow(height/2.0, 2);
		double tangens = tan(angle * M_PI / 180);
		double tanSquare = pow(tangens, 2);

		// calculate x
		pt.x = (long) sqrt( aSquare * bSquare / (bSquare + tanSquare * aSquare));
		if (angle > 90 && angle < 270) pt.x *= -1; // sqrt returns the positive value of the square, take the negative value if necessary

		// calculate y
		pt.y = (long) (tangens * pt.x);
		pt.y *= -1;	// reverse the sign because of inverted y-axis

		// offset the ellipse into the screen
		pt.x += x + (int)(width / 2);
		pt.y += y + (int)(height / 2);
	}

	return pt;
}

int GameEngine::DrawString(const tstring& textRef, int x, int y, int width, int height, HDC hDC) const
{
	HFONT hOldFont;
	COLORREF oldColor;

	if (m_FontDraw != 0)
	{
		hOldFont = (HFONT)SelectObject(hDC, m_FontDraw);

		oldColor = SetTextColor(hDC, m_ColDraw);
		SetBkMode(hDC, TRANSPARENT);

		RECT rc = { x, y, x + width - 1, y + height - 1 };
		int result = DrawText(hDC, textRef.c_str(), -1, &rc, DT_WORDBREAK);

		SetBkMode(hDC, OPAQUE);
		SetTextColor(hDC, oldColor);
		
		SelectObject(hDC, hOldFont);

		return result;
	}
	else
	{
		oldColor = SetTextColor(hDC, m_ColDraw);
		SetBkMode(hDC, TRANSPARENT);

		RECT rc = { x, y, x + width - 1, y + height - 1 };
		int result = DrawText(hDC, textRef.c_str(), -1, &rc, DT_WORDBREAK);

		SetBkMode(hDC, OPAQUE);
		SetTextColor(hDC, oldColor);

		return result;
	}
}

int GameEngine::DrawString(const tstring& textRef, int x, int y, int width, int height) const
{
	if (m_IsDoublebuffering || m_IsPainting) return DrawString(textRef, x, y, width, height, m_HdcDraw);
	else return -1;
}

int GameEngine::DrawString(const tstring& textRef, int x, int y, HDC hDC) const
{
	HFONT hOldFont;
	COLORREF oldColor;

	if (m_FontDraw != 0)
	{
		hOldFont = (HFONT)SelectObject(hDC, m_FontDraw);

		oldColor = SetTextColor(hDC, m_ColDraw);
		SetBkMode(hDC, TRANSPARENT);

		int result = TextOut(hDC, x, y, textRef.c_str(), (int)textRef.size());

		SetBkMode(hDC, OPAQUE);
		SetTextColor(hDC, oldColor);

		SelectObject(hDC, hOldFont);

		return result;
	}
	else
	{
		oldColor = SetTextColor(hDC, m_ColDraw);
		SetBkMode(hDC, TRANSPARENT);

		int result = TextOut(hDC, x, y, textRef.c_str(), (int)textRef.size());

		SetBkMode(hDC, OPAQUE);
		SetTextColor(hDC, oldColor);

		return result;
	}
}

int GameEngine::DrawString(const tstring& textRef, int x, int y) const
{
	if (m_IsDoublebuffering || m_IsPainting) return DrawString(textRef, x, y, m_HdcDraw);
	else return -1;
}

bool GameEngine::DrawBitmap(Bitmap* bitmapPtr, int x, int y, RECT rect, HDC hDC) const
{
	if (!bitmapPtr->Exists()) return false;

	int opacity = bitmapPtr->GetOpacity();

	if (opacity == 0 && bitmapPtr->HasAlphaChannel()) return true; // don't draw if opacity == 0 and opacity is used

	HDC hdcMem = CreateCompatibleDC(hDC);
	HBITMAP hbmOld = (HBITMAP) SelectObject(hdcMem, bitmapPtr->GetHandle());

	if (bitmapPtr->HasAlphaChannel())
	{
		BLENDFUNCTION blender={AC_SRC_OVER, 0, (BYTE) (2.55 * opacity), AC_SRC_ALPHA}; // blend function combines opacity and pixel based transparency
		AlphaBlend(hDC, x, y, rect.right - rect.left, rect.bottom - rect.top, hdcMem, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, blender);
	}
	else TransparentBlt(hDC, x, y, rect.right - rect.left, rect.bottom - rect.top, hdcMem, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, bitmapPtr->GetTransparencyColor());

	SelectObject(hdcMem, hbmOld);
	DeleteDC(hdcMem);

	return true;
}

bool GameEngine::DrawBitmap(Bitmap* bitmapPtr, int x, int y, RECT rect) const
{
	if (m_IsDoublebuffering || m_IsPainting) return DrawBitmap(bitmapPtr, x, y, rect, m_HdcDraw);
	else return false;
}

bool GameEngine::DrawBitmap(Bitmap* bitmapPtr, int x, int y, HDC hDC) const
{
	if (!bitmapPtr->Exists()) return false;

    BITMAP bm;
    GetObject(bitmapPtr->GetHandle(), sizeof(bm), &bm);
	RECT rect = {0, 0, bm.bmWidth, bm.bmHeight};

    return DrawBitmap(bitmapPtr, x, y, rect, hDC);
}

bool GameEngine::DrawBitmap(Bitmap* bitmapPtr, int x, int y) const
{
	if (m_IsDoublebuffering || m_IsPainting) return DrawBitmap(bitmapPtr, x, y, m_HdcDraw);
	else return false;
}

bool GameEngine::DrawSolidBackground(COLORREF color, HDC hDC, RECT rect)
{
	COLORREF oldColor = GetDrawColor();
	SetColor(color);
	FillRect(0, 0, rect.right, rect.bottom, hDC);
	SetColor(oldColor);

	return true;
}

bool GameEngine::DrawSolidBackground(COLORREF color)
{	
	if (m_IsDoublebuffering || m_IsPainting) return DrawSolidBackground(color, m_HdcDraw, m_RectDraw);
	else return false;
}

bool GameEngine::Repaint() const
{
	return InvalidateRect(m_Window, nullptr, true)?true:false;
}

POINT GameEngine::GetLocation() const
{
	RECT info;
	POINT pos;

	GetWindowRect(m_Window, &info);
	pos.x = info.left;
	pos.y = info.top;

	return pos;
}

void GameEngine::SetLocation(int x, int y)
{
	SetWindowPos(m_Window, 0, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	InvalidateRect(m_Window, 0, TRUE);
}

void GameEngine::SetFont(Font* fontPtr)
{
	m_FontDraw = fontPtr->GetHandle();
}

void GameEngine::RunGameLoop(bool value)
{
	m_RunGameLoop = value;
}

LRESULT GameEngine::HandleEvent(HWND hWindow, UINT msg, WPARAM wParam, LPARAM lParam)
{
	RECT		rect;
	HDC         hDC;
	PAINTSTRUCT ps;

	// Double buffering code
	HDC hBufferDC;
	HBITMAP hBufferBmp;
	HBITMAP hOldBmp;

	int width, height;
	static int count = 0;

	// Route Windows messages to game engine member functions
	switch (msg)
	{
		case WM_CREATE:
			// Seed the random number generator
			srand(GetTickCount());
			// Set the game window 
			SetWindow(hWindow);
			// Run user defined functions for start of the game
			m_GamePtr->Start();  
			return 0;

		case WM_PAINT:
			if (m_PaintDoublebuffered)
			{
				// Get window, rectangle and HDC
				hDC = BeginPaint(hWindow, &ps);
				GetClientRect(hWindow, &rect);

				// Double buffering code
				hBufferDC = CreateCompatibleDC(hDC);
				width = GameEngine::GetSingleton()->GetWidth();
				height = GameEngine::GetSingleton()->GetHeight();
				// Create the buffer
				hBufferBmp = CreateCompatibleBitmap(hDC, width, height);
				hOldBmp = (HBITMAP) SelectObject(hBufferDC, hBufferBmp);

				// Do user defined drawing functions on the buffer, parameters added
				// for ease of drawing
				m_HdcDraw = hBufferDC;
				m_RectDraw = rect;

				m_IsPainting = true;
				m_GamePtr->Paint(rect);
				m_IsPainting = false;

				// As a last step copy the memdc to the hdc
				BitBlt(hDC, 0, 0, width, height, hBufferDC, 0, 0, SRCCOPY);

				// Reset the old bmp of the buffer, mainly for show since we kill it anyway
				SelectObject(hBufferDC, hOldBmp);
				// Kill the buffer
				DeleteObject(hBufferBmp);
				DeleteDC(hBufferDC);

				// end paint
				EndPaint(hWindow, &ps);
			}
			else
			{
				m_HdcDraw = BeginPaint(hWindow, &ps);	
				GetClientRect(hWindow, &m_RectDraw);

				m_IsPainting = true;
				m_GamePtr->Paint(m_RectDraw);
				m_IsPainting = false;

				EndPaint(hWindow, &ps);
			}

			return 0;

		case WM_CTLCOLOREDIT:
			return SendMessage((HWND) lParam, WM_CTLCOLOREDIT, wParam, lParam);	// delegate this message to the child window

		case WM_CTLCOLORBTN:
			return SendMessage((HWND) lParam, WM_CTLCOLOREDIT, wParam, lParam);	// delegate this message to the child window

		case WM_LBUTTONDOWN:
			m_GamePtr->MouseButtonAction(true, true, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), wParam);
			return 0;

		case WM_LBUTTONUP:
			m_GamePtr->MouseButtonAction(true, false, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), wParam);
			return 0;

		case WM_RBUTTONDOWN:
			m_GamePtr->MouseButtonAction(false, true, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), wParam);
			return 0;

		case WM_RBUTTONUP:
			m_GamePtr->MouseButtonAction(false, false, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), wParam);
			return 0;
			
		case WM_MOUSEWHEEL:
			m_GamePtr->MouseWheelAction(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), (short) HIWORD(wParam), wParam);
			return 0;

		case WM_MOUSEMOVE:
			m_GamePtr->MouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), wParam);
			return 0;
			
		case WM_SYSCOMMAND:	// trapping this message prevents a freeze after the ALT key is released
			if (wParam == SC_KEYMENU) return 0;			// see win32 API : WM_KEYDOWN
			else break;    

		case WM_DESTROY:
			// User defined code for ending the game
			m_GamePtr->End();
			// Delete the game engine object
			delete GameEngine::GetSingleton();
			
			// End and exit the application
			PostQuitMessage(0);
			return 0;

	}
	return DefWindowProc(hWindow, msg, wParam, lParam);
}

//-----------------------------------------------------------------
// Caller methods
//-----------------------------------------------------------------
bool Caller::AddActionListener(Callable* targetPtr)
{
	return AddListenerObject(targetPtr);
}	

bool Caller::RemoveActionListener(const Callable* targetPtr) 
{
	return RemoveListenerObject(targetPtr);
}

class CallAllActions
{
public:
	CallAllActions(Caller* callerPtr) : m_CallerPtr(callerPtr)
	{}

	void operator()(Callable* callablePtr)
	{
		callablePtr->CallAction(m_CallerPtr);
	}

private:
	Caller* m_CallerPtr;
};

bool Caller::CallListeners()   
{	
	for_each(m_TargetList.begin(), m_TargetList.end(), CallAllActions(this));

	return (m_TargetList.size() > 0);
}

bool Caller::AddListenerObject(Callable* targetPtr) 
{
	vector<Callable*>::iterator pos = find(m_TargetList.begin(), m_TargetList.end(), targetPtr);

	if (pos != m_TargetList.end()) return false;
	
	m_TargetList.push_back(targetPtr);
	return true;
}
	
bool Caller::RemoveListenerObject(const Callable* targetPtr) 
{
	vector<Callable*>::iterator pos = find(m_TargetList.begin(), m_TargetList.end(), targetPtr); // find algorithm from STL

	if (pos == m_TargetList.end()) return false;

	m_TargetList.erase(pos);
	return true;
}

//-----------------------------------------------------------------
// Bitmap methods
//-----------------------------------------------------------------

// set static datamember to zero
int Bitmap::m_Nr = 0;

Bitmap::Bitmap(HBITMAP hBitmap) : m_TransparencyKey(-1), m_Opacity(100), m_Exists(true), m_IsTarga(false), m_HasAlphaChannel(false), m_hBitmap(hBitmap)
{
	// nothing to create
}

Bitmap::Bitmap(const tstring& nameRef, bool createAlphaChannel) : m_hBitmap(0), m_TransparencyKey(-1), m_Opacity(100), m_PixelsPtr(0), m_Exists(false)
{
	// check if the file to load is a targa
	size_t len = nameRef.length(); 

	if (len > 4 && nameRef.substr(len-4) == _T(".tga")) 
	{
		m_IsTarga = true;
		m_HasAlphaChannel = true;
		TargaLoader* targa = new TargaLoader();

		if (targa->Load((TCHAR*) nameRef.c_str()) == 1)
		{
			m_hBitmap = CreateBitmap(targa->GetWidth(), targa->GetHeight(), 1, targa->GetBPP(), (void*)targa->GetImg());
			if (m_hBitmap != 0) m_Exists = true;
		}
		
		delete targa;
	}
	// else load as bitmap
	else 
	{
		m_IsTarga = false;
		m_HasAlphaChannel = createAlphaChannel;
		m_hBitmap = (HBITMAP)LoadImage(GameEngine::GetSingleton()->GetInstance(), nameRef.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		if (m_hBitmap != 0) m_Exists = true;
	}

	if (m_IsTarga || createAlphaChannel) LoadBitInfo();
}

Bitmap::Bitmap(int IDBitmap, const tstring& typeRef, bool createAlphaChannel): m_TransparencyKey(-1), m_Opacity(100), m_Exists(false)
{
	if (typeRef == _T("BITMAP"))
	{	
		m_IsTarga = false;
		m_HasAlphaChannel = createAlphaChannel;

		m_hBitmap = LoadBitmap(GameEngine::GetSingleton()->GetInstance(), MAKEINTRESOURCE(IDBitmap));
		
		if (m_hBitmap != 0) m_Exists = true;
		
		if (createAlphaChannel) LoadBitInfo();
	}
	else if (typeRef == _T("TGA"))
	{
		m_IsTarga = true;
		m_HasAlphaChannel = true;

		tstringstream buffer;
		buffer << "temp\\targa";
		buffer << m_Nr++;
		buffer << ".tga";

		tstring fileName = buffer.str();

		Extract(IDBitmap, _T("TGA"), fileName);

		TargaLoader* targa = new TargaLoader();

		if (targa->Load((TCHAR*) fileName.c_str()) == 1)
		{
			m_hBitmap = CreateBitmap(targa->GetWidth(), targa->GetHeight(), 1, targa->GetBPP(), (void*)targa->GetImg());
			if (m_hBitmap != 0) m_Exists = true;
		}
		
		delete targa;
		
		LoadBitInfo();
	}
}

void Bitmap::LoadBitInfo()
{
	BITMAPINFOHEADER bminfoheader;
	::ZeroMemory(&bminfoheader, sizeof(BITMAPINFOHEADER));
	bminfoheader.biSize        = sizeof(BITMAPINFOHEADER);
	bminfoheader.biWidth       = GetWidth();
	bminfoheader.biHeight      = GetHeight();
	bminfoheader.biPlanes      = 1;
	bminfoheader.biBitCount    = 32;
	bminfoheader.biCompression = BI_RGB;
	
	HDC windowDC = GetWindowDC(GameEngine::GetSingleton()->GetWindow());
	m_PixelsPtr = new unsigned char[this->GetWidth() * this->GetHeight() * 4];
	
	GetDIBits(windowDC, m_hBitmap, 0, GetHeight(), m_PixelsPtr, (BITMAPINFO*)&bminfoheader, DIB_RGB_COLORS); // load pixel info

	// premultiply if it's a targa
	if (m_IsTarga)
	{
		for (int count = 0; count < GetWidth() * GetHeight(); count++)
		{
			if (m_PixelsPtr[count * 4 + 3] < 255)
			{
				m_PixelsPtr[count * 4 + 2] = (unsigned char)((int) m_PixelsPtr[count * 4 + 2] * (int) m_PixelsPtr[count * 4 + 3] / 0xff);
				m_PixelsPtr[count * 4 + 1] = (unsigned char)((int) m_PixelsPtr[count * 4 + 1] * (int) m_PixelsPtr[count * 4 + 3] / 0xff);
				m_PixelsPtr[count * 4] = (unsigned char)((int) m_PixelsPtr[count * 4] * (int) m_PixelsPtr[count * 4 + 3] / 0xff);
			}
		}
			
		SetDIBits(windowDC, m_hBitmap, 0, GetHeight(), m_PixelsPtr, (BITMAPINFO*)&bminfoheader, DIB_RGB_COLORS); // save the pixel info for later manipulation
	}	
	// add alpha channel values of 255 for every pixel if bmp
	else
	{		
		for (int count = 0; count < GetWidth() * GetHeight(); count++)
		{
			m_PixelsPtr[count * 4 + 3] = 255;
		}
	}
	
	SetDIBits(windowDC, m_hBitmap, 0, GetHeight(), m_PixelsPtr, (BITMAPINFO*)&bminfoheader, DIB_RGB_COLORS); // save the pixel info for later manipulation
}

/*
void Bitmap::Premultiply() // Multiply R, G and B with Alpha
{
    //Note that the APIs use premultiplied alpha, which means that the red,
    //green and blue channel values in the bitmap must be premultiplied with
    //the alpha channel value. For example, if the alpha channel value is x,
    //the red, green and blue channels must be multiplied by x and divided by
    //0xff prior to the call.

    unsigned long Index,nPixels;
    unsigned char *bCur;
    short iPixelSize;

	// Set ptr to start of image
    bCur=pImage;

    // Calc number of pixels
    nPixels=width*height;

	// Get pixel size in bytes
    iPixelSize=iBPP/8;

    for(Index=0;Index!=nPixels;Index++)  // For each pixel
    {

        *bCur=(unsigned char)((int)*bCur* (int)*(bCur+3)/0xff);
        *(bCur+1)=(unsigned char)((int)*(bCur+1)* (int)*(bCur+3)/0xff);
        *(bCur+2)=(unsigned char)((int)*(bCur+2)* (int)*(bCur+3)/0xff);

        bCur+=iPixelSize; // Jump to next pixel
    }
}
*/

Bitmap::~Bitmap()
{
	if (HasAlphaChannel())
	{
		delete[] m_PixelsPtr;
		m_PixelsPtr = 0;
	}

	DeleteObject(m_hBitmap);
}

bool Bitmap::Exists() const
{
	return m_Exists;
}

void Bitmap::Extract(WORD id, tstring sType, tstring fileName) const
{
	CreateDirectory(_T("temp\\"), nullptr);

    HRSRC hrsrc = FindResource(NULL, MAKEINTRESOURCE(id), sType.c_str());
    HGLOBAL hLoaded = LoadResource(NULL, hrsrc);
    LPVOID lpLock =  LockResource(hLoaded);
    DWORD dwSize = SizeofResource(NULL, hrsrc);
    HANDLE hFile = CreateFile(fileName.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    DWORD dwByteWritten;
    WriteFile(hFile, lpLock , dwSize , &dwByteWritten , NULL);
    CloseHandle(hFile);
    FreeResource(hLoaded);
} 

HBITMAP Bitmap::GetHandle() const
{
	return m_hBitmap;
}

int Bitmap::GetWidth() const
{
	if (!Exists()) return 0;

    BITMAP bm;

	GetObject(m_hBitmap, sizeof(bm), &bm);

    return bm.bmWidth;
}

int Bitmap::GetHeight() const
{
	if (!Exists()) return 0;

	BITMAP bm;

	GetObject(m_hBitmap, sizeof(bm), &bm);

    return bm.bmHeight;
}

void Bitmap::SetTransparencyColor(COLORREF color) // converts transparency value to pixel-based alpha
{
	m_TransparencyKey = color;

	if (HasAlphaChannel())
	{
		BITMAPINFOHEADER bminfoheader;
		::ZeroMemory(&bminfoheader, sizeof(BITMAPINFOHEADER));
		bminfoheader.biSize        = sizeof(BITMAPINFOHEADER);
		bminfoheader.biWidth       = GetWidth();
		bminfoheader.biHeight      = GetHeight();
		bminfoheader.biPlanes      = 1;
		bminfoheader.biBitCount    = 32;
		bminfoheader.biCompression = BI_RGB;
		
		HDC windowDC = GetWindowDC(GameEngine::GetSingleton()->GetWindow());

		unsigned char* NewPixels = new unsigned char[this->GetWidth() * this->GetHeight() * 4]; // create 32 bit buffer

		for (int count = 0; count < this->GetWidth() * this->GetHeight(); ++count)
		{
			if (RGB(m_PixelsPtr[count * 4 + 2], m_PixelsPtr[count * 4 + 1], m_PixelsPtr[count * 4]) == color) // if the color of this pixel == transparency color
			{
				((int*) NewPixels)[count] = 0; // set all four values to zero, this assumes sizeof(int) == 4 on this system
												// setting values to zero means premultiplying the RGB values to an alpha of 0
			}
			else ((int*) NewPixels)[count] = ((int*) m_PixelsPtr)[count]; // copy all four values from m_PixelsPtr to NewPixels
		}

		SetDIBits(windowDC, m_hBitmap, 0, GetHeight(), NewPixels, (BITMAPINFO*)&bminfoheader, DIB_RGB_COLORS); // insert pixels into bitmap

		delete[] NewPixels; //destroy buffer

		ReleaseDC(GameEngine::GetSingleton()->GetWindow(), windowDC); // release DC
	}
}

COLORREF Bitmap::GetTransparencyColor() const
{
	return m_TransparencyKey;
}

void Bitmap::SetOpacity(int opacity)
{
	if (HasAlphaChannel())
	{
		if (opacity > 100) m_Opacity = 100;
		else if (opacity < 0) m_Opacity = 0;
		else m_Opacity = opacity;
	}
}

int Bitmap::GetOpacity() const
{
	return m_Opacity;
}

bool Bitmap::IsTarga() const
{
	return m_IsTarga;
}

bool Bitmap::HasAlphaChannel() const
{
	return m_HasAlphaChannel;
}

bool Bitmap::SaveToFile(tstring fileName) const
{
	bool result = true;

	HDC hScreenDC = CreateDC(TEXT("DISPLAY"), nullptr, nullptr, nullptr);

	int width = GetWidth();
	int height = GetHeight();

	BYTE* data = new BYTE[width * height * 4];

	BITMAPINFO Bmi;
	memset(&Bmi, 0, sizeof(BITMAPINFO));
	Bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	Bmi.bmiHeader.biWidth = width;
	Bmi.bmiHeader.biHeight = height;
	Bmi.bmiHeader.biPlanes = 1;
	Bmi.bmiHeader.biBitCount = 32;
	Bmi.bmiHeader.biCompression = BI_RGB;
	Bmi.bmiHeader.biSizeImage = width * height * 4;

	GetDIBits(hScreenDC, m_hBitmap, 0, width, data, (BITMAPINFO*)&Bmi, DIB_RGB_COLORS); // load pixel info

	int sz = Bmi.bmiHeader.biSizeImage;

	BITMAPFILEHEADER bfh;
	bfh.bfType = ('M' << 8) + 'B';
	bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	bfh.bfSize = sz + bfh.bfOffBits;
	bfh.bfReserved1 = 0;
	bfh.bfReserved2 = 0;

	HANDLE hFile = ::CreateFile(fileName.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) result = false;
	else
	{
		DWORD dw;
		WriteFile(hFile, &bfh, sizeof(bfh), &dw, 0);
		WriteFile(hFile, &Bmi.bmiHeader, sizeof(BITMAPINFOHEADER), &dw, 0);
		WriteFile(hFile, data, sz, &dw, 0);
		CloseHandle(hFile);
	}

	delete[] data;

	DeleteDC(hScreenDC);

	return result;
}

//-----------------------------------------------------------------
// Audio methods
//-----------------------------------------------------------------

// set static datamember to zero
int Audio::m_Nr = 0;

#pragma warning(disable:4311)
#pragma warning(disable:4312)
Audio::Audio(const tstring& nameRef) : m_Playing(false), m_Paused(false), m_MustRepeat(false), m_hWnd(0), m_Volume(100)
{	
	size_t len = nameRef.length();

	ASSERT(len >= 4, _T("Audio name length must be longer than 4 characters!"));
	if (len < 4) return;

	tstring end = nameRef.substr(len - 4);

	if (end == _T(".mp3") || end == _T(".wav") || end == _T(".mid"))
	{
		tstringstream buffer;
		buffer << _T("audio");
		buffer << m_Nr++;

		m_Alias = buffer.str();
		m_FileName = nameRef;

		Create(nameRef);
	}
}

Audio::Audio(int IDAudio, const tstring& typeRef) : m_Playing(false), m_Paused(false), m_MustRepeat(false), m_hWnd(0), m_Volume(100)
{
	if (typeRef == _T("MP3") || typeRef == _T("WAV") || typeRef == _T("MID"))
	{
		tstringstream buffer;
		buffer << _T("audio");
		buffer << m_Nr++;

		m_Alias = buffer.str();
		m_FileName = tstring(_T("temp\\")) + m_Alias;

		if (typeRef == _T("MP3")) m_FileName += _T(".mp3");
		else if (typeRef == _T("WAV")) m_FileName += _T(".wav");
		else m_FileName += _T(".mid");
			
		Extract(IDAudio, typeRef, m_FileName);

		Create(m_FileName);
	}
}

void Audio::Create(const tstring& nameRef)
{
	TCHAR response[100];

	tstringstream buffer;

	size_t len = nameRef.length();

	ASSERT(len >= 4, _T("Audio name length must be longer than 4 characters!"));
	if (len < 4) return;

	tstring end = nameRef.substr(len - 4);

	if (end == _T(".mp3")) 
	{
		buffer << _T("open \"") + m_FileName + _T("\" type mpegvideo alias ");
		buffer << m_Alias;
	}
	else if (end == _T(".wav")) 
	{
		buffer << _T("open \"") + m_FileName + _T("\" type waveaudio alias ");
		buffer << m_Alias;
	}
	else if (end == _T(".mid")) 
	{
		buffer << _T("open \"") + m_FileName + _T("\" type sequencer alias ");
		buffer << m_Alias;
	}

	int result = mciSendString(buffer.str().c_str(), 0, 0, 0);	
	if (result != 0) return;
	
	buffer.str(_T(""));
	buffer << _T("set ") + m_Alias + _T(" time format milliseconds");
	mciSendString(buffer.str().c_str(), 0, 0, 0);

	buffer.str(_T(""));
	buffer << _T("status ") + m_Alias + _T(" length");
	mciSendString(buffer.str().c_str(), response, 100, 0);

	buffer.str(_T(""));
	buffer << response;
	buffer >> m_Duration;
	
	// Create a window to catch the MM_MCINOTIFY message with
	m_hWnd = CreateWindow(TEXT("STATIC"), TEXT(""), 0, 0, 0, 0, 0, 0, 0, GameEngine::GetSingleton()->GetInstance(), 0);
	SetWindowLongPtr(m_hWnd, GWLA_WNDPROC, (LONG_PTR) AudioProcStatic);	// set the custom message loop (subclassing)
	SetWindowLongPtr(m_hWnd, GWLA_USERDATA, (LONG_PTR) this);			// set this object as the parameter for the Proc
}

void Audio::Extract(WORD id , const tstring& typeRef, const tstring& fileNameRef) const
{
	CreateDirectory(TEXT("temp\\"), nullptr);

    HRSRC hrsrc = FindResource(NULL, MAKEINTRESOURCE(id), typeRef.c_str());
    HGLOBAL hLoaded = LoadResource( NULL, hrsrc);
    LPVOID lpLock =  LockResource(hLoaded);
    DWORD dwSize = SizeofResource(NULL, hrsrc);
    HANDLE hFile = CreateFile(fileNameRef.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    DWORD dwByteWritten;
    WriteFile(hFile, lpLock, dwSize, &dwByteWritten, nullptr);
    CloseHandle(hFile);
    FreeResource(hLoaded);
} 

#pragma warning(default:4311)
#pragma warning(default:4312)

Audio::~Audio()
{
	Stop();

	tstring sendString = tstring(_T("close")) + m_Alias;
	mciSendString(sendString.c_str(), 0, 0, 0);

	// release the window resources if necessary
	if (m_hWnd)
	{
		DestroyWindow(m_hWnd);
		m_hWnd = 0;
	}
}

void Audio::Play(int msecStart, int msecStop)
{
	if (!m_Playing)
	{
		m_Playing = true;
		m_Paused = false;

		if (msecStop == -1) QueuePlayCommand(msecStart);
		else QueuePlayCommand(msecStart, msecStop);
	}	
	else if (m_Paused)
	{
		m_Paused = false;

		QueueResumeCommand();
	}
}

void Audio::Pause()
{
	if (m_Playing && !m_Paused) 
	{
		m_Paused = true;

		QueuePauseCommand();
	}
}

void Audio::Stop()
{
	if (m_Playing)
	{
		m_Playing = false;
		m_Paused = false;

		QueueStopCommand();
	}
}

void Audio::QueuePlayCommand(int msecStart)
{
	tstringstream buffer;
	buffer << _T("play ") + m_Alias + _T(" from ");
	buffer << msecStart;
	buffer << _T(" notify");
	
	QueueCommand(buffer.str());
}

void Audio::QueuePlayCommand(int msecStart, int msecStop)
{
	tstringstream buffer;
	buffer << _T("play ") + m_Alias + _T(" from ");
	buffer << msecStart;
	buffer << _T(" to ");
	buffer << msecStop;
	buffer << _T(" notify");
	
	QueueCommand(buffer.str());
}

void Audio::QueuePauseCommand()
{
	QueueCommand(_T("pause ") + m_Alias);
}

void Audio::QueueResumeCommand()
{
	QueueCommand(_T("resume ") + m_Alias);
}

void Audio::QueueStopCommand()
{
	QueueCommand(_T("stop ") + m_Alias);
}

void Audio::QueueVolumeCommand(int volume)
{
	tstringstream buffer;
	buffer << _T("setaudio ") + m_Alias + _T(" volume to ");
	buffer << volume * 10;
	
	QueueCommand(buffer.str());
}

void Audio::QueueCommand(const tstring& commandRef)
{
	m_CommandQueue.push(commandRef);
}

void Audio::Tick()
{
	if (!m_CommandQueue.empty())
	{
		SendMCICommand(m_CommandQueue.front());
		m_CommandQueue.pop();
	}
}

void Audio::SendMCICommand(const tstring& commandRef) const
{
	int result = mciSendString(commandRef.c_str(), 0, 0, m_hWnd);
}

const tstring& Audio::GetName() const
{
	return m_FileName;
}
	
const tstring& Audio::GetAlias() const
{
	return m_Alias;
}

bool Audio::IsPlaying() const
{
	return m_Playing;
}

bool Audio::IsPaused() const
{
	return m_Paused;
}

void Audio::SwitchPlayingOff()
{
	m_Playing = false;
	m_Paused = false;
}

void Audio::SetRepeat(bool repeat)
{
	m_MustRepeat = repeat;
}

bool Audio::GetRepeat() const
{
	return m_MustRepeat;
}

int Audio::GetDuration() const
{
	return m_Duration;
}

void Audio::SetVolume(int volume)
{
	m_Volume = min(100, max(0, volume));	// values below 0 and above 100 are trimmed to 0 and 100, respectively

	QueueVolumeCommand(volume);
}

int Audio::GetVolume() const
{
	return m_Volume;
}

bool Audio::Exists() const
{
	return m_hWnd?true:false;
}

int Audio::GetType() const
{
	return Caller::Audio;
}

LRESULT Audio::AudioProcStatic(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{	
	#pragma warning(disable: 4312)
	Audio* audio = reinterpret_cast<Audio*>(GetWindowLongPtr(hWnd, GWLA_USERDATA));
	#pragma warning(default: 4312)

	switch (msg)
	{		
	case MM_MCINOTIFY: // message received when an audio file has finished playing - used for repeat function

		if (wParam == MCI_NOTIFY_SUCCESSFUL && audio->IsPlaying()) 
		{
			audio->SwitchPlayingOff();

			if (audio->GetRepeat()) audio->Play();	// repeat the audio
			else audio->CallListeners();			// notify listeners that the audio file has come to an end
		}
	}
	return 0;	
}

//-----------------------------------------------------------------
// TextBox methods
//-----------------------------------------------------------------

#pragma warning(disable:4311)	
#pragma warning(disable:4312)
TextBox::TextBox(const tstring& textRef) : m_X(0), m_Y(0), m_BgColor(RGB(255, 255, 255)), m_ForeColor(RGB(0, 0, 0)), m_BgColorBrush(0), m_Font(0), m_OldFont(0)
{
	// Create the edit box
	m_hWndEdit = CreateWindow(_T("EDIT"), textRef.c_str(), WS_BORDER | WS_CHILD | WS_CLIPSIBLINGS | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL, 0, 0, 0, 0, GameEngine::GetSingleton()->GetWindow(), NULL, GameEngine::GetSingleton()->GetInstance(), nullptr);

	// Set the new WNDPROC for the edit box, and store old one
	m_ProcOldEdit = (WNDPROC) SetWindowLongPtr(m_hWndEdit, GWLA_WNDPROC, (LONG_PTR) EditProcStatic);

	// Set this object as userdata for the static wndproc function of the edit box so that it can call members
	SetWindowLongPtr(m_hWndEdit, GWLA_USERDATA, (LONG_PTR) this);
}

TextBox::TextBox() : m_X(0), m_Y(0), m_BgColor(RGB(255, 255, 255)), m_ForeColor(RGB(0, 0, 0)), m_BgColorBrush(0), m_Font(0), m_OldFont(0)
{
	// Create the edit box
	m_hWndEdit = CreateWindow(_T("EDIT"), _T(""), WS_BORDER | WS_CHILD | WS_CLIPSIBLINGS | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL, 0, 0, 0, 0, GameEngine::GetSingleton()->GetWindow(), NULL, GameEngine::GetSingleton()->GetInstance(), nullptr);
	
	// Set the new WNDPROC for the edit box, and store old one
	m_ProcOldEdit = (WNDPROC) SetWindowLongPtr(m_hWndEdit, GWLA_WNDPROC, (LONG_PTR) EditProcStatic);
	
	// Set this object as userdata for the static wndproc function of the edit box so that it can call members
	SetWindowLongPtr(m_hWndEdit, GWLA_USERDATA, (LONG_PTR) this);
}
#pragma warning(default:4311)
#pragma warning(default:4312)

TextBox::~TextBox()
{
	// release the background brush if necessary
	if (m_BgColorBrush != 0) 
	{
		DeleteObject(m_BgColorBrush);
		m_BgColorBrush = 0;
	}

	// release the font if necessary
	if (m_Font != 0)
	{
		SelectObject(GetDC(m_hWndEdit), m_OldFont);
		DeleteObject(m_Font);
		m_Font = m_OldFont = 0;
	}
		
	// release the window resources
	DestroyWindow(m_hWndEdit);
	m_hWndEdit = NULL;
}

void TextBox::SetBounds(int x, int y, int width, int height)
{
	m_X = x;
	m_Y = y;

	MoveWindow(m_hWndEdit, x, y, width, height, true);
}

RECT TextBox::GetRect() const
{
	RECT rc;

	GetClientRect(m_hWndEdit, &rc);

	rc.left += m_X;
	rc.right += m_X;
	rc.top += m_Y; 
	rc.bottom += m_Y;

	return rc;
}

void TextBox::SetEnabled(bool bEnable)
{
	EnableWindow(m_hWndEdit, bEnable);
}

void TextBox::Update() const
{
	UpdateWindow(m_hWndEdit);
}

void TextBox::Show() const
{
	// Show and update the edit box
	ShowWindow(m_hWndEdit, SW_SHOW);
	UpdateWindow(m_hWndEdit);
}

void TextBox::Hide() const
{
	// Show and update the edit box
	ShowWindow(m_hWndEdit, SW_HIDE);
	UpdateWindow(m_hWndEdit);
}

tstring TextBox::GetText() const
{
	int textLength = (int) SendMessage(m_hWndEdit, (UINT) WM_GETTEXTLENGTH, 0, 0);
	
	TCHAR* bufferPtr = new TCHAR[textLength + 1];

	SendMessage(m_hWndEdit, (UINT) WM_GETTEXT, (WPARAM) textLength + 1, (LPARAM) bufferPtr);

	tstring newString(bufferPtr);

	delete[] bufferPtr;

	return newString;
}

void TextBox::SetText(const tstring& textRef)
{
	SendMessage(m_hWndEdit, WM_SETTEXT, 0, (LPARAM) textRef.c_str());
}

void TextBox::SetFont(const tstring& fontNameRef, bool bold, bool italic, bool underline, int size)
{
	LOGFONT ft;

	//_tcscpy_s(ft.lfFaceName, sizeof(ft.lfFaceName) / sizeof(TCHAR), fontName.c_str());
	for (int teller = 0; teller < (int) fontNameRef.size() && teller < LF_FACESIZE; ++teller)
	{
		ft.lfFaceName[teller] = fontNameRef[teller];
	}

	ft.lfStrikeOut = 0;
	ft.lfUnderline = underline?1:0;
	ft.lfHeight = size;
    ft.lfEscapement = 0;
	ft.lfWeight = bold?FW_BOLD:0;
	ft.lfItalic = italic?1:0;

	// clean up if another custom font was already in place
	if (m_Font != 0) { DeleteObject(m_Font); }

	// create the new font. The WM_CTLCOLOREDIT message will set the font when the textbox is about to redraw
    m_Font = CreateFontIndirect(&ft);

	// redraw the textbox
	InvalidateRect(m_hWndEdit, nullptr, true);
}

void TextBox::SetForecolor( COLORREF color )
{
	m_ForeColor = color;
	
	// redraw the textbox
	InvalidateRect(m_hWndEdit, nullptr, true);
}

void TextBox::SetBackcolor( COLORREF color )
{
	m_BgColor = color;
	
	if (m_BgColorBrush != 0) DeleteObject(m_BgColorBrush);
	m_BgColorBrush = CreateSolidBrush( color );
	
	// redraw the textbox
	InvalidateRect(m_hWndEdit, nullptr, true);
}

COLORREF TextBox::GetForecolor() const
{
	return m_ForeColor;
}

COLORREF TextBox::GetBackcolor() const
{
	return m_BgColor;
}

HBRUSH TextBox::GetBackcolorBrush() const
{
	return m_BgColorBrush;
}

LRESULT TextBox::EditProcStatic(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	#pragma warning(disable: 4312)
	return reinterpret_cast<TextBox*>(GetWindowLongPtr(hWnd, GWLA_USERDATA))->EditProc(hWnd, msg, wParam, lParam);
	#pragma warning(default: 4312)
}

LRESULT TextBox::EditProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{		
	case WM_CTLCOLOREDIT:
		SetBkColor((HDC) wParam, GetBackcolor() );
		SetTextColor((HDC) wParam, GetForecolor() );
		if (m_Font != 0) 
		{
			if (m_OldFont == 0) m_OldFont = (HFONT) SelectObject((HDC) wParam, m_Font);
			else SelectObject((HDC) wParam, m_Font);
		}
		return (LRESULT) GetBackcolorBrush();

	case WM_CHAR: 
		if (wParam == VK_TAB) return 0;
		if (wParam == VK_RETURN) return 0;
		break;

	case WM_KEYDOWN :
		switch (wParam)
		{
		case VK_TAB:
			if (GameEngine::GetSingleton()->IsKeyDown(VK_SHIFT)) GameEngine::GetSingleton()->TabPrevious(hWnd);
			else GameEngine::GetSingleton()->TabNext(hWnd);
			return 0;
		case VK_ESCAPE:
			SetFocus(GetParent(hWnd));
			return 0;
		case VK_RETURN:
			//if (m_Target) result = m_Target->CallAction(this);
			CallListeners();
			break;
		}
	}
	return CallWindowProc(m_ProcOldEdit, hWnd, msg, wParam, lParam);
}



//-----------------------------------------------------------------
// Button methods
//-----------------------------------------------------------------

#pragma warning(disable:4311)
#pragma warning(disable:4312)
Button::Button(const tstring& textRef) : m_X(0), m_Y(0), m_Armed(false), m_Font(0), m_OldFont(0)
{
	// Create the button object
	m_hWndButton = CreateWindow(_T("BUTTON"), textRef.c_str(), WS_BORDER | WS_CHILD | WS_CLIPSIBLINGS | WS_TABSTOP | BS_PUSHBUTTON, 0, 0, 0, 0, GameEngine::GetSingleton()->GetWindow(), nullptr, GameEngine::GetSingleton()->GetInstance(), NULL);

	// Set de new WNDPROC for the button, and store the old one
	m_ProcOldButton = (WNDPROC) SetWindowLongPtr(m_hWndButton, GWLA_WNDPROC, (LONG_PTR) ButtonProcStatic);

	// Store 'this' as data for the Button object so that the static PROC can call the member proc
	SetWindowLongPtr(m_hWndButton, GWLA_USERDATA, (LONG_PTR) this);
}

Button::Button() : m_X(0), m_Y(0), m_Armed(false), m_Font(0), m_OldFont(0)
{
	// Create the button object
	m_hWndButton = CreateWindow(_T("BUTTON"), _T(""), WS_BORDER | WS_CHILD | WS_CLIPSIBLINGS | WS_TABSTOP | BS_PUSHBUTTON, 0, 0, 0, 0, GameEngine::GetSingleton()->GetWindow(), nullptr, GameEngine::GetSingleton()->GetInstance(), NULL);

	// Set de new WNDPROC for the button, and store the old one
	m_ProcOldButton = (WNDPROC) SetWindowLongPtr(m_hWndButton, GWLA_WNDPROC, (LONG_PTR) ButtonProcStatic);

	// Store 'this' as data for the Button object so that the static PROC can call the member proc
	SetWindowLongPtr(m_hWndButton, GWLA_USERDATA, (LONG_PTR) this);
}
#pragma warning(default:4311)
#pragma warning(default:4312)

Button::~Button()
{
	// release the font if necessary
	if (m_Font != 0)
	{
		SelectObject(GetDC(m_hWndButton), m_OldFont);
		DeleteObject(m_Font);
		m_Font = m_OldFont = 0;
	}
		
	// release the window resource
	DestroyWindow(m_hWndButton);
	m_hWndButton = NULL;	
}

void Button::SetBounds(int x, int y, int width, int height)
{
	m_X = x;
	m_Y = y;

	MoveWindow(m_hWndButton, x, y, width, height, true);
}

RECT Button::GetRect() const
{
	RECT rc;

	GetClientRect(m_hWndButton, &rc);
	
	rc.left += m_X;
	rc.right += m_X;
	rc.top += m_Y; 
	rc.bottom += m_Y;

	return rc;
}

void Button::SetEnabled(bool bEnable)
{
	EnableWindow(m_hWndButton, bEnable);
}

void Button::Update() const
{
	UpdateWindow(m_hWndButton);
}

void Button::Show() const
{
	// Show and update the button
	ShowWindow(m_hWndButton, SW_SHOW);
	UpdateWindow(m_hWndButton);
}

void Button::Hide() const
{
	// Show and update the button
	ShowWindow(m_hWndButton, SW_HIDE);
	UpdateWindow(m_hWndButton);
}

tstring Button::GetText() const
{
	int textLength = (int) SendMessage(m_hWndButton, (UINT) WM_GETTEXTLENGTH, 0, 0);
	
	TCHAR* bufferPtr = new TCHAR[textLength + 1];

	SendMessage(m_hWndButton, (UINT) WM_GETTEXT, (WPARAM) textLength + 1, (LPARAM) bufferPtr);

	tstring newString(bufferPtr);

	delete[] bufferPtr;

	return newString;
}

void Button::SetText(const tstring& textRef)
{
	SendMessage(m_hWndButton, WM_SETTEXT, 0, (LPARAM) textRef.c_str());
}

void Button::SetFont(const tstring& fontNameRef, bool bold, bool italic, bool underline, int size)
{
	LOGFONT ft;

	//_tcscpy_s(ft.lfFaceName, sizeof(ft.lfFaceName) / sizeof(TCHAR), fontName.c_str());
	for (int teller = 0; teller < (int) fontNameRef.size() && teller < LF_FACESIZE; ++teller)
	{
		ft.lfFaceName[teller] = fontNameRef[teller];
	}

	ft.lfStrikeOut = 0;
	ft.lfUnderline = underline?1:0;
	ft.lfHeight = size;
    ft.lfEscapement = 0;
	ft.lfWeight = bold?FW_BOLD:0;
	ft.lfItalic = italic?1:0;

	// clean up if another custom font was already in place
	if (m_Font != 0) { DeleteObject(m_Font); }

	// create the new font. The WM_CTLCOLOREDIT message will set the font when the button is about to redraw
    m_Font = CreateFontIndirect(&ft);

	// redraw the button
	InvalidateRect(m_hWndButton, nullptr, true);
}

LRESULT Button::ButtonProcStatic(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	#pragma warning(disable: 4312)
	return reinterpret_cast<Button*>(GetWindowLongPtr(hWnd, GWLA_USERDATA))->ButtonProc(hWnd, msg, wParam, lParam);
	#pragma warning(default: 4312)
}

LRESULT Button::ButtonProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CTLCOLOREDIT:
		if (m_Font != 0) 
		{
			if (m_OldFont == 0) m_OldFont = (HFONT) SelectObject((HDC) wParam, m_Font);
			else SelectObject((HDC) wParam, m_Font);
		}
		return 0;

	case WM_CHAR: 
		if (wParam == VK_TAB) return 0;
		if (wParam == VK_RETURN) return 0;
		break;

	case WM_KEYDOWN :
		switch (wParam)
		{
		case VK_TAB:			
			if (GameEngine::GetSingleton()->IsKeyDown(VK_SHIFT)) GameEngine::GetSingleton()->TabPrevious(hWnd);
			else GameEngine::GetSingleton()->TabNext(hWnd);
			return 0;
		case VK_ESCAPE:
			SetFocus(GetParent(hWnd));
			return 0;
		case VK_SPACE:
			//if (m_Target) result = m_Target->CallAction(this);
			CallListeners();
			break;
		}
		break;
	case WM_LBUTTONDOWN :
	case WM_LBUTTONDBLCLK:					// clicking fast will throw LBUTTONDBLCLK's as well as LBUTTONDOWN's, you need to capture both to catch all button clicks
		m_Armed = true;
		break;
	case WM_LBUTTONUP :
		if (m_Armed)
		{
			RECT rc;
			POINT pt;
			GetWindowRect(hWnd, &rc);
			GetCursorPos(&pt);

			//if (PtInRect(&rc, pt) && m_Target) result = m_Target->CallAction(this);
			if (PtInRect(&rc, pt)) CallListeners();

			m_Armed = false;
		}
	}
	return CallWindowProc(m_ProcOldButton, hWnd, msg, wParam, lParam);
}

//-----------------------------------------------------------------
// Timer methods
//-----------------------------------------------------------------

Timer::Timer(int msec, Callable* targetPtr) : m_IsRunning(false)
{
	m_Delay = msec;

	AddActionListener(targetPtr);
}

Timer::~Timer()
{
	if (m_IsRunning) Stop(); // stop closes the handle

	// no objects to delete
}

void Timer::Start()
{
	if (m_IsRunning == false)
	{
		CreateTimerQueueTimer(&m_TimerHandle, NULL, TimerProcStatic, (void*) this, m_Delay, m_Delay, WT_EXECUTEINTIMERTHREAD);	
		m_IsRunning = true;
	}
}

void Timer::Stop()
{	
	if (m_IsRunning == true)
	{
		DeleteTimerQueueTimer(NULL, m_TimerHandle, NULL);  
		//CloseHandle (m_TimerHandle);		DeleteTimerQueueTimer automatically closes the handle? MSDN Documentation seems to suggest this
		
		m_IsRunning = false;
	}
}

bool Timer::IsRunning() const
{
	return m_IsRunning;
}

void Timer::SetDelay(int msec)
{
	m_Delay = max(msec, 1); // timer will not accept values less than 1 msec

	if (m_IsRunning)
	{
		Stop();
		Start();
	}
}

int Timer::GetDelay() const
{
	return m_Delay;
}

void CALLBACK Timer::TimerProcStatic(void* lpParameter, BOOLEAN TimerOrWaitFired)
{
	Timer* timer = reinterpret_cast<Timer*>(lpParameter);

	//if (timer->m_IsRunning) timer->m_Target->CallAction(timer);
	if (timer->m_IsRunning) timer->CallListeners();
}

//-----------------------------------------------------------------
// Targa loader code
//-----------------------------------------------------------------
 
#define IMG_OK              0x1
#define IMG_ERR_NO_FILE     0x2
#define IMG_ERR_MEM_FAIL    0x4
#define IMG_ERR_BAD_FORMAT  0x8
#define IMG_ERR_UNSUPPORTED 0x40
 
TargaLoader::TargaLoader()
{ 
	pImage = pPalette = pData = nullptr;
	width = height = iBPP = bEnc =0;
	lImageSize = 0;
}
 
TargaLoader::~TargaLoader()
{
	if(pImage)
	{
		delete [] pImage;
		pImage = nullptr;
	}
 
	if(pPalette)
	{
		delete [] pPalette;
		pPalette = nullptr;
	}
 
	if(pData)
	{
		delete [] pData;
		pData = nullptr;
	}
}
 
int TargaLoader::Load(const TCHAR* fileNamePtr)
{
	using namespace std;
	ifstream fIn;
	unsigned long ulSize;
	int iRet;
 
	// Clear out any existing image and palette
	if(pImage)
    {
		delete [] pImage;
		pImage = nullptr;
    }
 
	if(pPalette)
    {
		delete [] pPalette;
		pPalette = nullptr;
    }
 
	// Open the specified file
	//fIn.open(szFilename,ios::binary);
	fIn.open(fileNamePtr,ios::binary);
    
	if(!fIn) return IMG_ERR_NO_FILE;
 
	// Get file size
	fIn.seekg(0,ios_base::end);
	ulSize=(unsigned long) fIn.tellg();
	fIn.seekg(0,ios_base::beg);
 
	// Allocate some space
	// Check and clear pDat, just in case
	if(pData) delete [] pData; 
 
	pData=new unsigned char[ulSize];
 
	if( pData == nullptr)
    {
		fIn.close();
		return IMG_ERR_MEM_FAIL;
	}
 
	// Read the file into memory
	fIn.read((char*)pData,ulSize);
 
	fIn.close();
 
	// Process the header
	iRet=ReadHeader();
 
	if(iRet!=IMG_OK) return iRet;
 
	switch(bEnc)
	{
    case 1: // Raw Indexed
		// Check filesize against header values
        if((lImageSize+18+pData[0]+768)>ulSize) return IMG_ERR_BAD_FORMAT;
 
		// Double check image type field
		if(pData[1]!=1) return IMG_ERR_BAD_FORMAT;
 
		// Load image data
		iRet=LoadRawData();
        
		if(iRet!=IMG_OK) return iRet;
 
		// Load palette
        iRet=LoadTgaPalette();
        
		if(iRet!=IMG_OK) return iRet;
 
       break;
 
    case 2: // Raw RGB
		// Check filesize against header values
		if((lImageSize+18+pData[0])>ulSize) return IMG_ERR_BAD_FORMAT;
 
		// Double check image type field
        if(pData[1]!=0) return IMG_ERR_BAD_FORMAT;
 
		// Load image data
		iRet=LoadRawData();
        
		if(iRet!=IMG_OK) return iRet;
 
		//BGRtoRGB(); // Convert to RGB
		break;
 
    case 9: // RLE Indexed
      	// Double check image type field
        if(pData[1]!=1) return IMG_ERR_BAD_FORMAT;
 
		// Load image data
		iRet=LoadTgaRLEData();
			
		if(iRet!=IMG_OK) return iRet;
 
		// Load palette
		iRet=LoadTgaPalette();
        
		if(iRet!=IMG_OK) return iRet;
 
		break;
	
	case 10: // RLE RGB
       // Double check image type field
       if(pData[1]!=0) return IMG_ERR_BAD_FORMAT;
 
       // Load image data
       iRet=LoadTgaRLEData();
        
	   if(iRet!=IMG_OK) return iRet;
 
       //BGRtoRGB(); // Convert to RGB
       break;
 
	default:
		return IMG_ERR_UNSUPPORTED;
    }
 
	// Check flip bit
	if((pData[17] & 0x20)==0) FlipImg();
 
	// Release file memory
	delete [] pData;
	pData = nullptr;

	return IMG_OK;
}
 
int TargaLoader::ReadHeader() // Examine the header and populate our class attributes
{
	short ColMapStart,ColMapLen;
	short x1,y1,x2,y2;
 
	if(pData == nullptr)
		return IMG_ERR_NO_FILE;
 
	if(pData[1]>1)    // 0 (RGB) and 1 (Indexed) are the only types we know about
		return IMG_ERR_UNSUPPORTED;
 
	bEnc=pData[2];     // Encoding flag  1 = Raw indexed image
                      //                2 = Raw RGB
                      //                3 = Raw greyscale
                      //                9 = RLE indexed
                      //               10 = RLE RGB
                      //               11 = RLE greyscale
                      //               32 & 33 Other compression, indexed
 
	if(bEnc>11)       // We don't want 32 or 33
		return IMG_ERR_UNSUPPORTED;
 
 
	// Get palette info
	memcpy(&ColMapStart,&pData[3],2);
	memcpy(&ColMapLen,&pData[5],2);
 
	// Reject indexed images if not a VGA palette (256 entries with 24 bits per entry)
	if(pData[1]==1) // Indexed
    {
		if(ColMapStart!=0 || ColMapLen!=256 || pData[7]!=24) return IMG_ERR_UNSUPPORTED;
    }
 
	// Get image window and produce width & height values
	memcpy(&x1,&pData[8],2);
	memcpy(&y1,&pData[10],2);
	memcpy(&x2,&pData[12],2);
	memcpy(&y2,&pData[14],2);
 
	width=(x2-x1);
	height=(y2-y1);
 
	if(width<1 || height<1) return IMG_ERR_BAD_FORMAT;
 
	// Bits per Pixel
	iBPP=pData[16];
 
	// Check flip / interleave byte
	if(pData[17]>32) // Interleaved data
		return IMG_ERR_UNSUPPORTED;
 
	// Calculate image size
	lImageSize=(width * height * (iBPP/8));
 
	return IMG_OK;
}
 
int TargaLoader::LoadRawData() // Load uncompressed image data
{
	short iOffset;
 
	if(pImage) // Clear old data if present
		delete [] pImage;
 
	pImage=new unsigned char[lImageSize];
 
	if(pImage == nullptr) return IMG_ERR_MEM_FAIL;
 
	iOffset=pData[0]+18; // Add header to ident field size
 
	if(pData[1]==1) // Indexed images
		iOffset+=768;  // Add palette offset
 
	memcpy(pImage,&pData[iOffset],lImageSize);
 
	return IMG_OK;
}
 
int TargaLoader::LoadTgaRLEData() // Load RLE compressed image data
{
	short iOffset,iPixelSize;
	unsigned char *pCur;
	unsigned long Index=0;
	unsigned char bLength,bLoop;
 
	// Calculate offset to image data
	iOffset=pData[0]+18;
 
	// Add palette offset for indexed images
	if(pData[1]==1) iOffset+=768; 
 
	// Get pixel size in bytes
	iPixelSize=iBPP/8;
 
	// Set our pointer to the beginning of the image data
	pCur=&pData[iOffset];
 
	// Allocate space for the image data
	if(pImage != nullptr) delete [] pImage;
 
	pImage=new unsigned char[lImageSize];
 
	if(pImage == nullptr) return IMG_ERR_MEM_FAIL;
 
	// Decode
	while(Index<lImageSize) 
    {
		if(*pCur & 0x80) // Run length chunk (High bit = 1)
		{
			bLength=*pCur-127; // Get run length
			pCur++;            // Move to pixel data  
 
			// Repeat the next pixel bLength times
			for(bLoop=0;bLoop!=bLength;++bLoop,Index+=iPixelSize)
			memcpy(&pImage[Index],pCur,iPixelSize);
  
			pCur+=iPixelSize; // Move to the next descriptor chunk
		}
		else // Raw chunk
		{
			bLength=*pCur+1; // Get run length
			pCur++;          // Move to pixel data
 
			// Write the next bLength pixels directly
			for(bLoop=0;bLoop!=bLength;++bLoop,Index+=iPixelSize,pCur+=iPixelSize)
			memcpy(&pImage[Index],pCur,iPixelSize);
		}
    }
 
	return IMG_OK;
}
 
int TargaLoader::LoadTgaPalette() // Load a 256 color palette
{
	unsigned char bTemp;
	short iIndex,iPalPtr;
  
	// Delete old palette if present
	if(pPalette)
    {
		delete [] pPalette;
		pPalette = nullptr;
    }
 
	// Create space for new palette
	pPalette=new unsigned char[768];
 
	if(pPalette == nullptr) return IMG_ERR_MEM_FAIL;
 
	// VGA palette is the 768 bytes following the header
	memcpy(pPalette,&pData[pData[0]+18],768);
 
	// Palette entries are BGR ordered so we have to convert to RGB
	for(iIndex=0,iPalPtr=0;iIndex!=256;++iIndex,iPalPtr+=3)
    {
		bTemp=pPalette[iPalPtr];               // Get Blue value
		pPalette[iPalPtr]=pPalette[iPalPtr+2]; // Copy Red to Blue
		pPalette[iPalPtr+2]=bTemp;             // Replace Blue at the end
	}
 
	return IMG_OK;
} 
 
void TargaLoader::BGRtoRGB() // Convert BGR to RGB (or back again)
{
	unsigned long Index,nPixels;
	unsigned char *bCur;
	unsigned char bTemp;
	short iPixelSize;
 
	// Set ptr to start of image
	bCur=pImage;
 
	// Calc number of pixels
	nPixels=width*height;
 
	// Get pixel size in bytes
	iPixelSize=iBPP/8;
 
	for(Index=0;Index!=nPixels;Index++)  // For each pixel
    {
		bTemp=*bCur;      // Get Blue value
		*bCur=*(bCur+2);  // Swap red value into first position
		*(bCur+2)=bTemp;  // Write back blue to last position
 
		bCur+=iPixelSize; // Jump to next pixel
    }
}
 
void TargaLoader::FlipImg() // Flips the image vertically (Why store images upside down?)
{
	unsigned char bTemp;
	unsigned char *pLine1, *pLine2;
	int iLineLen,iIndex;
 
	iLineLen=width*(iBPP/8);
	pLine1=pImage;
	pLine2=&pImage[iLineLen * (height - 1)];
 
	for( ;pLine1<pLine2;pLine2-=(iLineLen*2))
    {
		for(iIndex=0;iIndex!=iLineLen;pLine1++,pLine2++,iIndex++)
		{
			bTemp=*pLine1;
			*pLine1=*pLine2;
			*pLine2=bTemp;       
		}
	}  
}
  
int TargaLoader::GetBPP() const 
{
	return iBPP;
}
 
int TargaLoader::GetWidth() const
{
	return width;
}
 
int TargaLoader::GetHeight() const
{
	return height;
}
 
const unsigned char* TargaLoader::GetImg() const
{
	return pImage;
}
 
const unsigned char* TargaLoader::GetPalette() const
{
	return pPalette;
}


//---------------------------
// HitRegion methods
//---------------------------
HitRegion::HitRegion() : m_HitRegion(0)
{
	// nothing to create
}

HitRegion::~HitRegion()
{
	if (m_HitRegion)
		DeleteObject(m_HitRegion);
}


bool HitRegion::Create(int type, int x, int y, int width, int height)
{
	if (m_HitRegion) DeleteObject(m_HitRegion);

	if (type == HitRegion::Ellipse)
		m_HitRegion = CreateEllipticRgn(x, y, x + width, y + height);
	else
		m_HitRegion = CreateRectRgn(x, y, x + width, y + height);

	return true;
}

bool HitRegion::Create(int type, const POINT* pointsArr, int numberOfPoints)
{
	if (m_HitRegion) DeleteObject(m_HitRegion);

	m_HitRegion = CreatePolygonRgn(pointsArr, numberOfPoints, WINDING);

	return true;
}	

bool HitRegion::Create(int type, const Bitmap* bmpPtr, COLORREF cTransparent, COLORREF cTolerance)
{
	if (!bmpPtr->Exists()) return false;

	HBITMAP hBitmap = bmpPtr->GetHandle();

	if (!hBitmap) return false;

	if (m_HitRegion) DeleteObject(m_HitRegion);

	// for some reason, the BitmapToRegion function has R and B switched. Flipping the colors to get the right result.
	COLORREF flippedTransparent = RGB(GetBValue(cTransparent), GetGValue(cTransparent), GetRValue(cTransparent));
	COLORREF flippedTolerance = RGB(GetBValue(cTolerance), GetGValue(cTolerance), GetRValue(cTolerance));

	m_HitRegion = BitmapToRegion(hBitmap, flippedTransparent, flippedTolerance);

	return (m_HitRegion?true:false);
}	

//	BitmapToRegion :	Create a region from the "non-transparent" pixels of a bitmap
//	Author :		Jean-Edouard Lachand-Robert (http://www.geocities.com/Paris/LeftBank/1160/resume.htm), June 1998
//  Some modifications: Kevin Hoefman, Febr 2007
HRGN HitRegion::BitmapToRegion(HBITMAP hBmp, COLORREF cTransparentColor, COLORREF cTolerance) const
{
	HRGN hRgn = NULL;

	if (hBmp)
	{
		// Create a memory DC inside which we will scan the bitmap content
		HDC hMemDC = CreateCompatibleDC(NULL);

		if (hMemDC)
		{
			// Get bitmap siz
			BITMAP bm;
			GetObject(hBmp, sizeof(bm), &bm);

			// Create a 32 bits depth bitmap and select it into the memory DC
			BITMAPINFOHEADER RGB32BITSBITMAPINFO = {
					sizeof(BITMAPINFOHEADER),	// biSize
					bm.bmWidth,					// biWidth;
					bm.bmHeight,				// biHeight;
					1,							// biPlanes;
					32,							// biBitCount
					BI_RGB,						// biCompression;
					0,							// biSizeImage;
					0,							// biXPelsPerMeter;
					0,							// biYPelsPerMeter;
					0,							// biClrUsed;
					0							// biClrImportant;
			};
			VOID * pbits32;
			HBITMAP hbm32 = CreateDIBSection(hMemDC, (BITMAPINFO *)&RGB32BITSBITMAPINFO, DIB_RGB_COLORS, &pbits32, NULL, 0);

			if (hbm32)
			{
				HBITMAP holdBmp = (HBITMAP)SelectObject(hMemDC, hbm32);

				// Create a DC just to copy the bitmap into the memory D
				HDC hDC = CreateCompatibleDC(hMemDC);

				if (hDC)
				{
					// Get how many bytes per row we have for the bitmap bits (rounded up to 32 bits
					BITMAP bm32;
					GetObject(hbm32, sizeof(bm32), &bm32);
					while (bm32.bmWidthBytes % 4)
						bm32.bmWidthBytes++;

					// Copy the bitmap into the memory D
					HBITMAP holdBmp = (HBITMAP)SelectObject(hDC, hBmp);
					BitBlt(hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, hDC, 0, 0, SRCCOPY);

					// For better performances, we will use the ExtCreateRegion() function to create the
					// region. This function take a RGNDATA structure on entry. We will add rectangles b
					// amount of ALLOC_UNIT number in this structure
					#define ALLOC_UNIT	100
					DWORD maxRects = ALLOC_UNIT;
					HANDLE hData = GlobalAlloc(GMEM_MOVEABLE, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects));
					RGNDATA *pData = (RGNDATA *)GlobalLock(hData);
					pData->rdh.dwSize = sizeof(RGNDATAHEADER);
					pData->rdh.iType = RDH_RECTANGLES;
					pData->rdh.nCount = pData->rdh.nRgnSize = 0;
					SetRect(&pData->rdh.rcBound, MAXLONG, MAXLONG, 0, 0);

					// Keep on hand highest and lowest values for the "transparent" pixel
					BYTE lr = GetRValue(cTransparentColor);
					BYTE lg = GetGValue(cTransparentColor);
					BYTE lb = GetBValue(cTransparentColor);
					BYTE hr = min(0xff, lr + GetRValue(cTolerance));
					BYTE hg = min(0xff, lg + GetGValue(cTolerance));
					BYTE hb = min(0xff, lb + GetBValue(cTolerance));

					// Scan each bitmap row from bottom to top (the bitmap is inverted vertically
					BYTE *p32 = (BYTE *)bm32.bmBits + (bm32.bmHeight - 1) * bm32.bmWidthBytes;
					for (int y = 0; y < bm.bmHeight; y++)
					{
						// Scan each bitmap pixel from left to righ
						for (int x = 0; x < bm.bmWidth; x++)
						{
							// Search for a continuous range of "non transparent pixels"
							int x0 = x;
							LONG *p = (LONG *)p32 + x;
							while (x < bm.bmWidth)
							{
								BYTE b = GetRValue(*p);
								if (b >= lr && b <= hr)
								{
									b = GetGValue(*p);
									if (b >= lg && b <= hg)
									{
										b = GetBValue(*p);
										if (b >= lb && b <= hb)
											// This pixel is "transparent"
											break;
									}
								}
								p++;
								x++;
							}

							if (x > x0)
							{
								// Add the pixels (x0, y) to (x, y+1) as a new rectangle in the regio
								if (pData->rdh.nCount >= maxRects)
								{
									GlobalUnlock(hData);
									maxRects += ALLOC_UNIT;
									hData = GlobalReAlloc(hData, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects), GMEM_MOVEABLE);
									pData = (RGNDATA *)GlobalLock(hData);
								
								}
								RECT *pr = (RECT *)&pData->Buffer;
								SetRect(&pr[pData->rdh.nCount], x0, y, x, y+1);
								if (x0 < pData->rdh.rcBound.left)
									pData->rdh.rcBound.left = x0;
								if (y < pData->rdh.rcBound.top)
									pData->rdh.rcBound.top = y;
								if (x > pData->rdh.rcBound.right)
									pData->rdh.rcBound.right = x;
								if (y+1 > pData->rdh.rcBound.bottom)
									pData->rdh.rcBound.bottom = y+1;
								pData->rdh.nCount++;

								/*
								// On Windows98, ExtCreateRegion() may fail if the number of rectangles is to
								// large (ie: > 4000). Therefore, we have to create the region by multiple steps
								if (pData->rdh.nCount == 2000)
								{
									HRGN h = ExtCreateRegion(nullptr, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects), pData);
									
									// Free the data
									GlobalFree(hData);

									if (hRgn)
									{
										CombineRgn(hRgn, hRgn, h, RGN_OR);
										DeleteObject(h);
									}
									else
										hRgn = h;
									pData->rdh.nCount = 0;
									SetRect(&pData->rdh.rcBound, MAXLONG, MAXLONG, 0, 0);
								}
								*/
							}
						}

						// Go to next row (remember, the bitmap is inverted vertically
						p32 -= bm32.bmWidthBytes;
					}

					// Create or extend the region with the remaining rectangle
					HRGN h = ExtCreateRegion(nullptr, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects), pData);

					if (hRgn)
					{
						CombineRgn(hRgn, hRgn, h, RGN_OR);
						DeleteObject(h);
					}
					else
						hRgn = h;

					// Clean u
					SelectObject(hDC, holdBmp);
					DeleteDC(hDC);
				}

				DeleteObject(SelectObject(hMemDC, holdBmp));
			}

			DeleteDC(hMemDC);
		}
	}

	return hRgn;
}
	
HitRegion* HitRegion::Clone() const
{
	HitRegion* temp = new HitRegion();

	temp->m_HitRegion = CreateRectRgn(0, 0, 10, 10); // create dummy region
	CombineRgn(temp->m_HitRegion, m_HitRegion, 0, RGN_COPY);

	return temp;
}
	
void HitRegion::Move(int x, int y)
{
	OffsetRgn(m_HitRegion, x, y);
}
	
RECT HitRegion::GetDimension() const
{
	RECT boundingbox;
	GetRgnBox(m_HitRegion, &boundingbox);

	return boundingbox;
}

HRGN HitRegion::GetHandle() const
{
	return m_HitRegion;
}

bool HitRegion::HitTest(HitRegion* regPtr) const
{
	HRGN temp = CreateRectRgn(0, 0, 10, 10);			// dummy region
	bool result = (CombineRgn(temp, m_HitRegion, regPtr->m_HitRegion, RGN_AND) != NULLREGION);

	DeleteObject(temp);
	return result;
}
	
bool HitRegion::HitTest(int x, int y) const
{
	return PtInRegion(m_HitRegion, x, y)?true:false;
}
	
POINT HitRegion::CollisionTest(HitRegion* regPtr) const
{
	POINT result;

	HRGN temp = CreateRectRgn(0, 0, 10, 10);			// dummy region
	int overlap = CombineRgn(temp, m_HitRegion, regPtr->m_HitRegion, RGN_AND);

	if (overlap == NULLREGION)
	{
		result.x = -1000000;
		result.y = -1000000;
	}
	else
	{
		RECT boundingbox;
		GetRgnBox(temp, &boundingbox);
		result.x = boundingbox.left + (boundingbox.right - boundingbox.left)/2;
		result.y = boundingbox.top + (boundingbox.bottom - boundingbox.top)/2;
	}

	DeleteObject(temp);
	
	return result;
}


//-----------------------------------------------------------------
// Font methods 
//-----------------------------------------------------------------

Font::Font(const tstring& fontNameRef, bool bold, bool italic, bool underline, int size)
{
	LOGFONT ft;
	ZeroMemory(&ft, sizeof(ft));

	for (int teller = 0; teller < (int) fontNameRef.size() && teller < LF_FACESIZE; ++teller)
	{
		ft.lfFaceName[teller] = fontNameRef[teller];
	}

	ft.lfStrikeOut = 0;
	ft.lfUnderline = underline?1:0;
	ft.lfHeight = size;
    ft.lfEscapement = 0;
	ft.lfWeight = bold?FW_BOLD:0;
	ft.lfItalic = italic?1:0;

    m_hFont = CreateFontIndirect(&ft);
}

Font::~Font()
{
	DeleteObject(m_hFont);
}
	
//-----------------------------------------------------------------
// OutputDebugString functions
//-----------------------------------------------------------------

void OutputDebugString(const tstring& textRef)
{
	OutputDebugString(textRef.c_str());
}
