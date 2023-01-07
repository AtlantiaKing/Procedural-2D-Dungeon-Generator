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
class Dungeon final
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
	DungeonGenerator& GetGenerator() { return m_Generator; };

	void Draw() const;

private:
	//-------------------------------------------------
	// Private member functions								
	//-------------------------------------------------


	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
	std::vector<DungeonRoom> m_Rooms{};
	DungeonGenerator m_Generator{};
};
