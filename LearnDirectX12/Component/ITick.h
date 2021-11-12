#pragma once

namespace com {

class GameTimer;
class ITick {
public:
	virtual void beginTick(GameTimer &tick) {};
	virtual void tick(GameTimer &tick) {}
	virtual void endTick(GameTimer &tick) {}
	virtual ~ITick() = default;
};

}