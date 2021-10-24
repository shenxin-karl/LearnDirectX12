#pragma once
class GameTimer;
class ITick {
public:
	virtual void beginTick() {}
	virtual void tick(GameTimer &dt) {}
	virtual void endTick() {}
};

