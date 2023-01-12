//---------------------------
// Includes
//---------------------------
#include "DungeonRoom.h"
#include "GameEngine.h"
#include "GameDefines.h"
#include "Camera.h"

//---------------------------
// Constructor & Destructor
//---------------------------
DungeonRoom::DungeonRoom(const Vector2& position, const Vector2& size, const Color& color)
	: m_Position{ position }
	, m_Size { size }
	, m_Color{ color }
{
}

//---------------------------
// Member functions
//---------------------------
void DungeonRoom::Move(const Vector2& direction)
{
	m_Position += direction;
}

void DungeonRoom::SetColor(const Color& color)
{
	m_Color = color;
}

void DungeonRoom::SetRoomType(DungeonRoomType type)
{
	m_RoomType = type;
}

void DungeonRoom::AddConnection(int roomIdx)
{
	m_ConnectedRooms.push_back(roomIdx);
}

void DungeonRoom::Draw(bool debugRender) const
{
	if (debugRender)
	{
		// Render in gray
		GAME_ENGINE->SetColor(RGB(127, 127, 127));
	}
	else
	{
		// Render in room color
		GAME_ENGINE->SetColor(m_Color.GetColor());
	}
	
	// Draw the room
	const Vector2 scaledPosition{ CAMERA->ScalePoint(m_Position) };
	const int scaledSizeX{ CAMERA->ScaleSize(m_Size.x) };
	const int scaledSizeY{ CAMERA->ScaleSize(m_Size.y) };
	GAME_ENGINE->DrawRect(scaledPosition.x, scaledPosition.y, scaledSizeX, scaledSizeY);

	if (debugRender) return;

	switch (m_RoomType)
	{
	case DungeonRoomType::KeyRoom:
	{
		// Render in brown
		GAME_ENGINE->SetColor(RGB(102, 51, 0));

		// Draw a box
		const int scaledBoxSize{ CAMERA->ScaleSize(15) };
		GAME_ENGINE->FillRect(scaledPosition.x + scaledSizeX / 2 - scaledBoxSize / 2, scaledPosition.y + scaledSizeY / 2 - scaledBoxSize / 2, scaledBoxSize, scaledBoxSize);
		break;
	}
	case DungeonRoomType::LockedRoom:
	{
		// Render in brown
		GAME_ENGINE->SetColor(RGB(102, 51, 0));

		// Draw a thick line around the room
		GAME_ENGINE->FillRect(scaledPosition.x + 1, scaledPosition.y, scaledSizeX - 2, scaledSizeY - 2);

		// Render in black
		GAME_ENGINE->SetColor(RGB(0, 0, 0));

		GAME_ENGINE->FillRect(scaledPosition.x + 5, scaledPosition.y + 4, scaledSizeX - 11, scaledSizeY - 11);
		break;
	}
	}
}

bool DungeonRoom::IsOverlapping(const DungeonRoom& other) const
{
	return m_Position.x < other.m_Position.x + other.m_Size.x && m_Position.x + m_Size.x > other.m_Position.x &&
		m_Position.y + m_Size.y > other.m_Position.y && m_Position.y < other.m_Position.y + other.m_Size.y;
}

Vector2 DungeonRoom::GetPosition() const
{
	return m_Position;
}

Vector2 DungeonRoom::GetSize() const
{
	return m_Size;
}

DungeonRoom::DungeonRoomType DungeonRoom::GetRoomType() const
{
	return m_RoomType;
}

const std::vector<int>& DungeonRoom::GetConnections() const
{
	return m_ConnectedRooms;
}

bool DungeonRoom::HasKey() const
{
	return m_RoomType == DungeonRoomType::KeyRoom;
}

bool DungeonRoom::IsLocked() const
{
	return m_RoomType == DungeonRoomType::LockedRoom;
}
