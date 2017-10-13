#pragma once
#include <stdint.h>
#include <memory>

class Scores {

public:
	static const auto MaxRecordsCount = 10u;
	static const auto MaxNameLength = 27u;

	struct Record {

		uint32_t seconds = 0;
		char name[MaxNameLength + 1] = {0,};
	};

	virtual ~Scores() = 0 {};
	virtual void addRecord(uint32_t seconds, const char* name) = 0;
	virtual uint32_t getRecordsCount() const = 0;
	virtual const Record& getRecord(uint32_t index) const = 0;
	virtual bool save() = 0;
	virtual bool load() = 0;
};

using ScoresPtr = std::unique_ptr<Scores>;

ScoresPtr makeScores(const char* fileName);
