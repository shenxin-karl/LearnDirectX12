#pragma once

class ITick {
public:
	virtual void beginTick() {}
	virtual void tick() {}
	virtual void endTick() {}
};

