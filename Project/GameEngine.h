//-----------------------------------------------------------------
// Game Engine Object
// C++ Header - GameEngine.h - version v7_01
// Copyright 2006-2021 Kevin Hoefman - kevin.hoefman@howest.be
//
// New features: 
//		- developed from ResearchEngine 6_01
//
// New features: 
//		- updated mouse coordinates to account for multiple displays
//
//-----------------------------------------------------------------

#pragma once

//-----------------------------------------------------------------
// Include Files
//-----------------------------------------------------------------
#define _WIN32_WINNT 0x0A00				// Windows 10
#define WIN32_LEAN_AND_MEAN
#define _WINSOCKAPI_
#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>
#include <tchar.h>
#include <Mmsystem.h>					// winmm.lib header, used for playing sound
#undef MessageBox

#include "AbstractGame.h"					// base for all games
#include "GameDefines.h"				// common header files and defines / macros

#include <vector>						// using std::vector for tab control logic
#include <queue>						// using std::queue for event system
#include <algorithm>
using namespace std;

//-----------------------------------------------------------------
// Pragma Library includes
//-----------------------------------------------------------------
#pragma comment(lib, "msimg32.lib")		// used for transparency
#pragma comment(lib, "winmm.lib")		// used for sound

//-----------------------------------------------------------------
// GameEngine Defines
//-----------------------------------------------------------------
#define KEYBCHECKRATE 60

//-----------------------------------------------------------------
// GameEngine Forward Declarations
//-----------------------------------------------------------------
class Bitmap;
class SoundWave;
class Midi;
class HitRegion;
class Font;

//-----------------------------------------------------------------
// GameEngine Class
//-----------------------------------------------------------------
class GameEngine
{
private:
	// singleton implementation : private constructor + static pointer to game engine object
	GameEngine();
	static GameEngine*  m_GameEnginePtr;

public:
	// Destructor
	virtual ~GameEngine();

	// Disabling copy/move constructors and assignment operators   
	GameEngine(const GameEngine& other) = delete;
	GameEngine(GameEngine&& other) noexcept = delete;
	GameEngine& operator=(const GameEngine& other) = delete;
	GameEngine& operator=(GameEngine&& other) noexcept = delete;

	// Static methods
	static GameEngine*  GetSingleton();

	// General Methods
	void		SetGame(AbstractGame* gamePtr);
	bool		Run(HINSTANCE hInstance, int cmdShow);
	bool        ClassRegister(int cmdShow);
	bool		SetGameValues(const tstring& titleRef, WORD icon, WORD smallIcon, int width, int height);
	bool		SetWindowRegion(const HitRegion* regionPtr); 
	bool		HasWindowRegion() const; 
	bool		GoFullscreen();		
	bool		GoWindowedMode();		
	bool		IsFullscreen() const;		
	void		ShowMousePointer(bool value) const;	

	LRESULT     HandleEvent(HWND hWindow, UINT msg, WPARAM wParam, LPARAM lParam);
	bool		IsKeyDown(int vKey) const;
	void		Quit(void) const;
	void		MessageBox(double value) const;
	void		MessageBox(int value) const;
	void		MessageBox(size_t value) const;
	void		MessageBox(const tstring& textRef) const;
	bool		MessageContinue(const tstring& textRef) const;
	void		RunGameLoop(bool value);
	void		TabNext(HWND ChildWindow) const;
	void		TabPrevious(HWND ChildWindow) const;

	// Text Dimensions
	SIZE		CalculateTextDimensions(const tstring& text, Font* fontPtr) const;
	SIZE		CalculateTextDimensions(const tstring& text, Font* fontPtr, RECT rect) const;

	// Draw Methods
	bool		DrawLine(int x1, int y1, int x2, int y2) const;
	bool		DrawLine(int x1, int y1, int x2, int y2, HDC hDC) const;
	bool		DrawPolygon(const POINT ptsArr[], int count) const;
	bool		DrawPolygon(const POINT ptsArr[], int count, bool close) const;
	bool		DrawPolygon(const POINT ptsArr[], int count, bool close, HDC hDC) const;
	bool		FillPolygon(const POINT ptsArr[], int count) const;
	bool		FillPolygon(const POINT ptsArr[], int count, bool close) const;
	bool		FillPolygon(const POINT ptsArr[], int count, bool close, HDC hDC) const;
	bool		DrawRect(int x, int y, int width, int height) const;
	bool		DrawRect(int x, int y, int width, int height, HDC hDC) const;
	bool		FillRect(int x, int y, int width, int height) const;
	bool		FillRect(int x, int y, int width, int height, int opacity) const;
	bool		FillRect(int x, int y, int width, int height, HDC hDC) const;
	bool		DrawRoundRect(int x, int y, int width, int height, int radius) const;
	bool		DrawRoundRect(int x, int y, int width, int height, int radius, HDC hDC) const;
	bool		FillRoundRect(int x, int y, int width, int height, int radius) const;
	bool		FillRoundRect(int x, int y, int width, int height, int radius, HDC hDC) const;
	bool		DrawOval(int x, int y, int width, int height) const;
	bool		DrawOval(int x, int y, int width, int height, HDC hDC) const;
	bool		FillOval(int x, int y, int width, int height) const;
	bool		FillOval(int x, int y, int width, int height, int opacity) const;
	bool		FillOval(int x, int y, int width, int height, HDC hDC) const;
	bool		DrawArc(int x, int y, int width, int height, int startDegree, int angle) const;
	bool		DrawArc(int x, int y, int width, int height, int startDegree, int angle, HDC hDC) const;
	bool		FillArc(int x, int y, int width, int height, int startDegree, int angle) const;
	bool		FillArc(int x, int y, int width, int height, int startDegree, int angle, HDC hDC) const;
	int			DrawString(const tstring& textRef, int x, int y) const;
	int			DrawString(const tstring& textRef, int x, int y, HDC hDC) const;
	int			DrawString(const tstring& textRef, int x, int y, int width, int height) const;
	int			DrawString(const tstring& textRef, int x, int y, int width, int height, HDC hDC) const;
	bool		DrawBitmap(Bitmap* bitmapPtr, int x, int y) const;
	bool		DrawBitmap(Bitmap* bitmapPtr, int x, int y, HDC hDC) const;
	bool		DrawBitmap(Bitmap* bitmapPtr, int x, int y, RECT rect) const;
	bool		DrawBitmap(Bitmap* bitmapPtr, int x, int y, RECT rect, HDC hDC) const;
	bool		DrawSolidBackground(COLORREF color);
	bool		DrawSolidBackground(COLORREF color, HDC hDC, RECT rect);
	void		SetColor(COLORREF color) {m_ColDraw = color;}
	COLORREF	GetDrawColor() const {return m_ColDraw;}
	void		SetFont(Font* fontPtr);
	bool		Repaint() const;

	// Accessor Methods
	HINSTANCE	GetInstance()	const	{ return m_hInstance; }
	HWND		GetWindow()		const	{ return m_Window; }
	tstring&	GetTitle()		const	{ return *m_TitlePtr; }
	WORD		GetIcon()		const	{ return m_Icon; }
	WORD		GetSmallIcon()	const	{ return m_SmallIcon; }
	int			GetWidth()		const	{ return m_Width; }
	int			GetHeight()		const	{ return m_Height; }
	int			GetFrameDelay() const	{ return m_FrameDelay; }
	POINT		GetLocation()	const;

	// Public Mutator Methods	
	void		SetTitle(const tstring& titleRef);							// SetTitle automatically sets the window class name to the same name as the title 
	void		SetLocation(int x, int y);
	void		SetKeyList(const tstring& keyListRef);
	void		SetIcon(WORD icon)						{ m_Icon = icon; }
	void		SetSmallIcon(WORD smallIcon)			{ m_SmallIcon = smallIcon; }
	void		SetWidth(int width)						{ m_Width = width; }
	void		SetHeight(int height)					{ m_Height = height; }
	void		SetFrameRate(double frameRate)			{ m_FrameDelay = (int)(1000 / frameRate); }
	void		SetPaintDoublebuffered(bool value)		{ m_PaintDoublebuffered = value; }

	// Keyboard monitoring thread method
	virtual		DWORD		KeybThreadProc();
	
private:
	// Private Mutator Methods	
	void		SetInstance(HINSTANCE hInstance)		{ m_hInstance = hInstance; }
	void		SetWindow(HWND hWindow)					{ m_Window = hWindow; }

	// Private Draw Methods
	void		FormPolygon(const POINT ptsArr[], int count, bool close, HDC hDC)	const;
	POINT		AngleToPoint(int x, int y, int width, int height, int angle)		const;

	// Member Variables
	HINSTANCE           m_hInstance{};
	HWND                m_Window{};
	tstring*            m_TitlePtr{};
	WORD                m_Icon{}, m_SmallIcon{};
	int                 m_Width{}, m_Height{};
	int                 m_FrameDelay{};
	bool				m_RunGameLoop{};
	HANDLE				m_hKeybThread{};
	DWORD				m_dKeybThreadID{};
	bool				m_KeybRunning{};
	TCHAR*				m_KeyListPtr{};
	unsigned int		m_KeybMonitor{};
	AbstractGame*		m_GamePtr{};
	bool				m_PaintDoublebuffered{};
	bool				m_Fullscreen{};

	// Draw assistance variables
	HDC					m_HdcDraw{};
	RECT				m_RectDraw{};
	bool				m_IsPainting{}, m_IsDoublebuffering{};
	COLORREF			m_ColDraw{};
	HFONT				m_FontDraw{};

	// Fullscreen assistance variable
	POINT				m_OldLoc{};

	// Window Region assistance variable
	HitRegion*			m_WindowRegionPtr{};
};

//------------------------------------------------------------------------------------------------
// Callable Interface
//
// Interface implementation for classes that can be called by "caller" objects
//------------------------------------------------------------------------------------------------
class Caller;	// forward declaration

class Callable
{
public:
	virtual ~Callable() {}						// virtual destructor for polymorphism
	virtual void CallAction(Caller* callerPtr) = 0;
};

//------------------------------------------------------------------------------------------------
// Caller Base Class
//
// Base Clase implementation for up- and downcasting of "caller" objects: TextBox, Button, Timer, Audio and Video
//------------------------------------------------------------------------------------------------
class Caller
{
public:
	virtual ~Caller() {}				// do not delete the targets!

	// Disabling copy/move constructors and assignment operators   
	Caller(const Caller& other) = delete;
	Caller(Caller&& other) noexcept = delete;
	Caller& operator=(const Caller& other) = delete;
	Caller& operator=(Caller&& other) noexcept = delete;

	static const int TextBox	= 0;
	static const int Button		= 1;
	static const int Timer		= 2;
	static const int Audio		= 3;
	static const int Video		= 4;

	virtual int GetType() const = 0;

	virtual bool AddActionListener(Callable* targetPtr);			// public interface method, call is passed on to private method
	virtual bool RemoveActionListener(const Callable* targetPtr);	// public interface method, call is passed on to private method

protected:
	Caller() {}								// constructor only for derived classes
	vector<Callable*> m_TargetList;

	virtual bool CallListeners();			// placing the event code in a separate method instead of directly in the windows messaging
											// function allows inheriting classes to override the event code. 

private:
	bool AddListenerObject(Callable* targetPtr);
	bool RemoveListenerObject(const Callable* targetPtr);
};

//--------------------------------------------------------------------------
// Timer Class
//
// This timer is a kernel timer, it will only work on Windows 2000 and higher
//--------------------------------------------------------------------------

class Timer : public Caller
{
public:
	Timer(int msec, Callable* targetPtr); // constructor automatically adds 2nd parameter to the list of listener objects

	virtual ~Timer();

	// Disabling copy/move constructors and assignment operators   
	Timer(const Timer& other) = delete;
	Timer(Timer&& other) noexcept = delete;
	Timer& operator=(const Timer& other) = delete;
	Timer& operator=(Timer&& other) noexcept = delete;

	int		GetType()			const {return Caller::Timer;}
	void	Start();
	void	Stop();
	bool	IsRunning()			const;
	void	SetDelay(int msec);
	int		GetDelay()			const;

private:	
	// -------------------------
	// Datamembers
	// -------------------------
	HANDLE	m_TimerHandle;
	bool	m_IsRunning;
	int		m_Delay;

	// -------------------------
	// Handler functions
	// -------------------------	
	static void CALLBACK TimerProcStatic(void* lpParameter, BOOLEAN TimerOrWaitFired); // proc will call CallListeners()
};

//-----------------------------------------------------------------
// TextBox Class
//-----------------------------------------------------------------

class TextBox : public Caller
{
public:
	TextBox(const tstring& textRef);
	TextBox();

	virtual ~TextBox();

	// Disabling copy/move constructors and assignment operators   
	TextBox(const TextBox& other) = delete;
	TextBox(TextBox&& other) noexcept = delete;
	TextBox& operator=(const TextBox& other) = delete;
	TextBox& operator=(TextBox&& other) noexcept = delete;

	int			GetType() const {return Caller::TextBox;}
	void		SetBounds(int x, int y, int width, int height);
	tstring		GetText()													const;
	void		SetText(const tstring& textRef);
	void		SetFont(const tstring& fontNameRef, bool bold, bool italic, bool underline, int size);
	void		SetBackcolor(COLORREF color);
	void		SetForecolor(COLORREF color);
	COLORREF	GetForecolor()												const;
	COLORREF	GetBackcolor()												const;
	HBRUSH		GetBackcolorBrush()											const;
	RECT		GetRect()													const;
	void		SetEnabled(bool bEnable);
	void		Update(void)												const;
	void		Show()														const;
	void		Hide()														const;

private:
	// -------------------------
	// Datamembers
	// -------------------------
	int			m_X, m_Y;
	HWND		m_hWndEdit;
	WNDPROC		m_ProcOldEdit;
	COLORREF	m_BgColor, m_ForeColor;
	HBRUSH		m_BgColorBrush;
	HFONT		m_Font, m_OldFont;

	// -------------------------
	// Handler functions
	// -------------------------	
	static LRESULT CALLBACK EditProcStatic(HWND hWindow, UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT EditProc(HWND hWindow, UINT msg, WPARAM wParam, LPARAM lParam);
};

//-----------------------------------------------------------------
// Button Class
//-----------------------------------------------------------------
class Button : public Caller
{
public:
	Button(const tstring& textRef);
	Button();

	virtual ~Button();

	// Disabling copy/move constructors and assignment operators   
	Button(const Button& other) = delete;
	Button(Button&& other) noexcept = delete;
	Button& operator=(const Button& other) = delete;
	Button& operator=(Button&& other) noexcept = delete;

	int		GetType() const {return Caller::Button;}
	void	SetBounds(int x, int y, int width, int height);
	tstring GetText() const;
	void	SetText(const tstring& textRef);
	void	SetFont(const tstring& fontNameRef, bool bold, bool italic, bool underline, int size);
	RECT	GetRect() const;
	void	SetEnabled(bool bEnable);
	void	Update(void) const;
	void	Show() const;
	void	Hide() const;

private:
	// -------------------------
	// Datamembers
	// -------------------------
	int			m_X, m_Y;
	HWND		m_hWndButton;
	WNDPROC		m_ProcOldButton;
	bool		m_Armed;
	COLORREF	m_BgColor, m_ForeColor;
	HFONT		m_Font, m_OldFont;

	// -------------------------
	// Handler functions
	// -------------------------	
	static LRESULT CALLBACK ButtonProcStatic(HWND hWindow, UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT ButtonProc(HWND hWindow, UINT msg, WPARAM wParam, LPARAM lParam);
};

//-----------------------------------------------------------------
// CheckBox Class
//-----------------------------------------------------------------
class CheckBox : public Caller
{
public:
	CheckBox(bool isChecked = false);

	virtual ~CheckBox();

	// Disabling copy/move constructors and assignment operators   
	CheckBox(const CheckBox& other) = delete;
	CheckBox(CheckBox&& other) noexcept = delete;
	CheckBox& operator=(const CheckBox& other) = delete;
	CheckBox& operator=(CheckBox&& other) noexcept = delete;

	int		GetType() const { return Caller::Button; }
	void	SetBounds(int x, int y, int size);
	RECT	GetRect() const;
	void	SetEnabled(bool bEnable);
	void	Update(void) const;
	void	Show() const;
	void	Hide() const;
	bool	IsChecked() const;

private:
	// -------------------------
	// Datamembers
	// -------------------------
	int			m_X, m_Y;
	HWND		m_hWndButton;
	WNDPROC		m_ProcOldButton;
	bool		m_Armed;
	bool		m_IsChecked;
	COLORREF	m_BgColor, m_ForeColor;

	// -------------------------
	// Handler functions
	// -------------------------	
	static LRESULT CALLBACK ButtonProcStatic(HWND hWindow, UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT ButtonProc(HWND hWindow, UINT msg, WPARAM wParam, LPARAM lParam);
};

//-----------------------------------------------------------------
// Audio Class
//-----------------------------------------------------------------
class Audio : public Caller
{
	// Static Proc function has to be able to call SwitchPlayingOff(), but nobody else should
	//friend LRESULT Audio::AudioProcStatic(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

public:
	Audio(const tstring& nameRef);
	Audio(int IDAudio, const tstring& typeRef);

	virtual ~Audio();

	// Disabling copy/move constructors and assignment operators   
	Audio(const Audio& other) = delete;
	Audio(Audio&& other) noexcept = delete;
	Audio& operator=(const Audio& other) = delete;
	Audio& operator=(Audio&& other) noexcept = delete;

	const tstring&	GetName()					const;
	const tstring&	GetAlias()					const;
	int				GetDuration()				const;
	bool			IsPlaying()					const;
	bool			IsPaused()					const;
	void			SetRepeat(bool repeat);
	bool			GetRepeat()					const;
	bool			Exists()					const;
	int				GetVolume()					const;
	int				GetType()					const;

	// these methods are called to instruct the object. The methods that perform the actual sendstrings are private.
	void Play(int msecStart = 0, int msecStop = -1);
	void Pause();
	void Stop();
	void SetVolume(int volume);

	// commands are queued and sent through a Tick() which should be called by the main thread (mcisendstring isn't thread safe) 
	// has the additional benefit of creating a delay between mcisendstring commands
	void Tick();

private:	
	// -------------------------
	// Datamembers
	// -------------------------
	static int	m_Nr;
	tstring		m_FileName;
	tstring		m_Alias;
	bool		m_Playing, m_Paused;
	bool		m_MustRepeat;
	HWND		m_hWnd;
	int			m_Duration;
	int			m_Volume;

	// -------------------------
	// Member functions
	// -------------------------			
	void Create(const tstring& nameRef);
	void Extract(WORD id , const tstring& typeRef, const tstring& fileNameRef) const;
	void SwitchPlayingOff();		

	// private mcisendstring command methods and command queue datamember
	queue<tstring> m_CommandQueue;
	void QueuePlayCommand(int msecStart);
	void QueuePlayCommand(int msecStart, int msecStop);
	void QueuePauseCommand();
	void QueueResumeCommand();
	void QueueStopCommand();
	void QueueVolumeCommand(int volume);		// add a m_Volume datamember? What volume does the video start on by default?
	void QueuePositionCommand(int x, int y);
	void QueueCommand(const tstring& commandRef);
	void SendMCICommand(const tstring& commandRef) const;
			
	// -------------------------
	// Handler functions
	// -------------------------	
	static LRESULT CALLBACK AudioProcStatic(HWND hWindow, UINT msg, WPARAM wParam, LPARAM lParam);
};

//-----------------------------------------------------------------
// Bitmap Class
//-----------------------------------------------------------------

class Bitmap
{
public:
	Bitmap(HBITMAP hBitmap);		
	Bitmap(const tstring& nameRef, bool createAlphaChannel = true);						
	Bitmap(int IDBitmap, const tstring& typeRef, bool createAlphaChannel = true);

	virtual ~Bitmap();

	// Disabling copy/move constructors and assignment operators   
	Bitmap(const Bitmap& other) = delete;
	Bitmap(Bitmap&& other) noexcept = delete;
	Bitmap& operator=(const Bitmap& other) = delete;
	Bitmap& operator=(Bitmap&& other) noexcept = delete;

	bool		Exists()								const;
	HBITMAP		GetHandle()								const;
	int			GetWidth()								const;
	int			GetHeight()								const;
	void		SetTransparencyColor(COLORREF color);
	COLORREF	GetTransparencyColor()					const;
	void		SetOpacity(int);
	int			GetOpacity()							const;
	bool		IsTarga()								const;
	bool		HasAlphaChannel()						const;
	bool		SaveToFile(tstring fileName)			const;
	
private:	
	// -------------------------
	// Datamembers
	// -------------------------
	static int		m_Nr;
	HBITMAP			m_hBitmap;
	COLORREF		m_TransparencyKey;
	int				m_Opacity;
	bool			m_IsTarga;
	unsigned char*	m_PixelsPtr; 
	bool			m_Exists;
	bool			m_HasAlphaChannel;

	// -------------------------
	// Methods
	// -------------------------
	void LoadBitInfo(); 
	void Extract(WORD id, tstring sType, tstring fileName) const;
};

//-----------------------------------------------------------------
// TGA Loader Class - 16/11/04 Codehead - original name TGAImg
//-----------------------------------------------------------------
class TargaLoader
{
public:
	TargaLoader();
	~TargaLoader();
	int Load(const TCHAR* fileNamePtr);
	int GetBPP() const;
	int GetWidth() const;
	int GetHeight() const;
	const unsigned char* GetImg() const;       // Return a pointer to image data
	const unsigned char* GetPalette() const;   // Return a pointer to VGA palette
 
private:
	short int width,height,iBPP;
	unsigned long lImageSize;
	char bEnc;
	unsigned char *pImage, *pPalette, *pData;
   
	// Internal workers
	int ReadHeader();
	int LoadRawData();
	int LoadTgaRLEData();
	int LoadTgaPalette();
	void BGRtoRGB();
	void FlipImg();
};

//-----------------------------------------------------------------
// HitRegion Class								
//-----------------------------------------------------------------
class HitRegion
{
public: 
	//---------------------------
	// Constructor(s)
	//---------------------------
	HitRegion();						// Default constructor. Geef de waarden van de nieuwe region op via de Create( ) methode.

	//---------------------------
	// Destructor
	//---------------------------
	virtual ~HitRegion();	

	//---------------------------
	// Disabling copy/move constructors and assignment operators   
	//---------------------------
	HitRegion(const HitRegion& other) = delete;
	HitRegion(HitRegion&& other) noexcept = delete;
	HitRegion& operator=(const HitRegion& other) = delete;
	HitRegion& operator=(HitRegion&& other) noexcept = delete;

	//---------------------------
	// General Methods
	//---------------------------
	bool Create(int type, int x, int y, int width, int height);	// Use this create to form a rectangular or elliptic hitregion

	bool Create(int type, const POINT* pointsArr, int numberOfPoints);	// Use this create to form a polygonal hitregion
	
	bool Create(int type, const Bitmap* bmpPtr, COLORREF cTransparent = RGB(255, 0, 255), COLORREF cTolerance = 0); // Use this create to create a hitregion from a bitmap

	HitRegion* Clone() const;						// Makes a new HitRegion object with the same data as this hitregion object, and returns a pointer value to it

	bool HitTest(HitRegion* regPtr) const;			// Returns true if the regions overlap, false if they don't
	bool HitTest(int x, int y) const;				// Returns true if the point that corresponds to the given x- and y-values is in the region, false if not

	POINT CollisionTest(HitRegion* regPtr) const;	// Returns {-1000000, -1000000} if the regions don't overlap, and the center point of the overlapping region if they do overlap
													//		CollisionTest is useful to determine the hitting point of two forms that barely touch
	
	RECT GetDimension() const;					// Returns the position + width and height of the smallest rectangle that encloses this region (in case of a rectangular region: the region itself) 			

	HRGN GetHandle() const;						// Returns the handle of the region (Win32 stuff)

	void Move(int x, int y);				// Moves the complete region according to the given displacement over x and y

	static const int Ellipse = 0;			// Class constants to specify what kind of region needs to be created (similar to enum, see 2nd year)
	static const int Rectangle = 1;
	static const int Polygon = 2;
	static const int FromBitmap = 3;

private:
	//---------------------------
	// Datamembers
	//---------------------------
	HRGN m_HitRegion;						// The region data is stored by means of a Win32 "region" resources (not for 1st semester)

	//---------------------------
	// Private methods
	//---------------------------
	HRGN BitmapToRegion(HBITMAP hBmp, COLORREF cTransparentColor, COLORREF cTolerance) const;	
};

//-----------------------------------------------------------------
// Font Class
//-----------------------------------------------------------------
class Font
{
public:
	Font(const tstring& fontNameRef, bool bold, bool italic, bool underline, int size);
	virtual ~Font();

	HFONT GetHandle() {return m_hFont;}

private:
	HFONT m_hFont;
};


//-----------------------------------------------------------------
// Extra OutputDebugString functions
//-----------------------------------------------------------------
void OutputDebugString(const tstring& textRef);

//-----------------------------------------------------------------
// Windows Procedure Declarations
//-----------------------------------------------------------------
DWORD WINAPI		KeybThreadProc (GameEngine* gamePtr);
LRESULT CALLBACK	WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);