#pragma once

//-----------------------------------------------------
// Include Files
//-----------------------------------------------------
#include "DungeonRoom.h"
#include "DungeonGenerator.h"
#include <vector>

//-----------------------------------------------------
// Dungeon Class									
//-----------------------------------------------------
class Dungeon final : public std::enable_shared_from_this<Dungeon>
{
public:
	Dungeon() = default;	// Constructor
	~Dungeon() = default;	// Destructor

	// -------------------------
	// Copy/move constructors and assignment operators
	// -------------------------    
	Dungeon(const Dungeon& other)					= delete;
	Dungeon(Dungeon&& other) noexcept				= delete;
	Dungeon& operator=(const Dungeon& other)		= delete;
	Dungeon& operator=(Dungeon&& other)	noexcept	= delete;

	//-------------------------------------------------
	// Member functions						
	//-------------------------------------------------
	void GenerateDungeon();
	void Update();
	void SetKeyCount(int count) { m_NrKeys = count; }
	DungeonGenerator& GetGenerator() { return m_Generator; }
	bool PickUpKeyInRoom(int roomIdx);
	bool UseKeyInRoom(int roomIdx);
	void SetNeedAllKeys(bool needAllKeys);

	int GetStartRoom() const;
	int GetEndRoom() const;
	Vector2 GetRoomPositionFromIndex(int roomIdx) const;
	const std::vector<int>& GetRoomConnectionsFromIndex(int roomIdx) const;
	bool IsRoomLocked(int roomIdx) const;
	bool IsSolved() const;

	void Draw() const;

private:
	//-------------------------------------------------
	// Private member functions								
	//-------------------------------------------------
	void GenerateKeysAndLockedRooms();

	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
	DungeonGenerator m_Generator{};

	std::vector<DungeonRoom> m_Rooms{};
	bool m_HasAddedKeys{};
	int m_NrKeys{};
	bool m_NeedAllKeys{};
};
