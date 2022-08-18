#pragma once
#include <string>

namespace core {

struct IAsset {
	virtual const std::string &getAssetName() const = 0;
	virtual const std::string &getAssetPath() const = 0;
	virtual void saveMeta() const = 0;
};

}
