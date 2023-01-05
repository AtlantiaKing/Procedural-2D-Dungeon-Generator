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
	void SetSeed(int seed);
	void SetInitialRadius(int radius);
	void SetInitialRoomCount(int roomCount);
	void SetRoomBounds(int minSize, int maxSize);
	void SetGenerationState(bool isSlowlyGenerating);
	void SetRoomSizeThreshold(int size);
	void Update();

	void Draw() const;
	int GetInitialRoomCount() const;
	int GetInitialRadius() const;

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
