#pragma once
#include <memory>

class Command {

public:
	virtual ~Command() = 0 {};
	virtual void doAction() = 0;
	virtual void undoAction() = 0;
};

using CommandPtr = std::unique_ptr<Command>;
