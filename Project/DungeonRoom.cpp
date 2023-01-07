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
DungeonRoom::DungeonRoom(const Vector2 position, const Vector2 size, const Color color)
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
	GAME_ENGINE->DrawRect(scaledPosition.x, scaledPosition.y, CAMERA->ScaleSize(m_Size.x), CAMERA->ScaleSize(m_Size.y));
}

bool DungeonRoom::IsOverlapping(const DungeonRoom& other) const
{
    // If one rectangle is on left side of other or if one rectangle is on top side of other
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
