//---------------------------
// Includes
//---------------------------
#include "DungeonRoom.h"
#include "GameEngine.h"
#include "GameDefines.h"

DungeonRoom::DungeonRoom(const Vector2 position, const Vector2 size, const Color color)
	: m_Position{ position }
	, m_Size { size }
	, m_Color{ color }
{
}

void DungeonRoom::Move(const Vector2& direction)
{
	m_Position += direction;
}

//---------------------------
// Member functions
//---------------------------
void DungeonRoom::Draw() const
{
	GAME_ENGINE->SetColor(m_Color.GetColor());
	GAME_ENGINE->DrawRect(m_Position.x, m_Position.y, m_Size.x, m_Size.y);
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
