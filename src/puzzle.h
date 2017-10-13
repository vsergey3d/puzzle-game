#pragma once
#include <stdint.h>
#include <memory>

class QGridLayout;

class Puzzle {

public:
	static const auto MinSize = 4u;
	static const auto MaxSize = 10u;

    virtual ~Puzzle() = 0 {}
	virtual void update() = 0;
	virtual void reset(uint32_t size) = 0;
	virtual void turnKnob(uint32_t x, uint32_t y) = 0;
	virtual void undo() = 0;
	virtual void redo() = 0;
	virtual bool hasUndos() const = 0;
	virtual bool hasRedos() const = 0;
	virtual bool isBusy() const = 0;
	virtual bool isSolved() const = 0;
	virtual uint32_t getSpentTimeSec() const = 0;

    virtual QGridLayout* getGrid() const = 0;
};

using PuzzlePtr = std::unique_ptr<Puzzle>;

PuzzlePtr makePuzzle(uint32_t size = Puzzle::MinSize);
