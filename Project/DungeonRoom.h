#pragma once

//-----------------------------------------------------
// Include Files
//-----------------------------------------------------
#include "DataTypes.h"

//-----------------------------------------------------
// DungeonRoom Class									
//-----------------------------------------------------
class DungeonRoom final
{
public:
	enum class DungeonRoomType
	{
		Room,
		KeyRoom,
		LockedRoom,
		Start,
		End
	};

	DungeonRoom() = default;									// Constructors
	DungeonRoom(const Vector2& position, const Vector2& size, const Color& color);
	~DungeonRoom()	= default;									// Destructor

	//-------------------------------------------------
	// Member functions						
	//-------------------------------------------------
	void Move(const Vector2& direction);
	void SetColor(const Color& color);
	void SetRoomType(DungeonRoomType type);
	void AddConnection(int roomIdx);

	void Draw(bool debugRender = false) const;
	bool IsOverlapping(const DungeonRoom& other) const;
	Vector2 GetPosition() const;
	Vector2 GetSize() const;
	DungeonRoomType GetRoomType() const;
	const std::vector<int>& GetConnections() const;
	bool HasKey() const;
	bool IsLocked() const;

private:
	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
	Vector2 m_Position{};
	Vector2 m_Size{};
	Color m_Color{};
	std::vector<int> m_ConnectedRooms{};
	DungeonRoomType m_RoomType{ DungeonRoomType::Room };
};
