#include "scores.h"
#include <vector>
#include <fstream>
#include <algorithm>
#include <cassert>

bool operator < (const Scores::Record& left, const Scores::Record& right) {

	return left.seconds < right.seconds;
}

class ScoresImpl : public Scores {

public:
	ScoresImpl(const char* fileName) : fileName_(fileName) {}
	virtual ~ScoresImpl() = default;
	void addRecord(uint32_t seconds, const char* name) override;
	uint32_t getRecordsCount() const override { return records_.size(); }
	const Record& getRecord(uint32_t index) const override;
	bool save() override;
	bool load() override;

private:
	std::vector<Record> records_;
	std::string fileName_;
};

void ScoresImpl::addRecord(uint32_t seconds, const char* name) {

	assert(seconds > 0);
	assert(name && strlen(name));

	Record record;
	record.seconds = seconds;
	strncpy(record.name, name, MaxNameLength);
	records_.emplace_back(record);
	std::sort(records_.begin(), records_.end());
	records_.resize(std::min(records_.size(), (size_t)MaxRecordsCount));
}

const Scores::Record& ScoresImpl::getRecord(uint32_t index) const {

	assert(index < records_.size());
	return records_[index];
}

bool ScoresImpl::save() {

	std::ofstream file(fileName_, std::ios::binary);
	if (!file.is_open())
		return false;
	
	auto count = records_.size();
	file.write((const char*)&count, sizeof(count));
	file.write((const char*)&records_[0], records_.size() * sizeof(Record));
	return true;
}

bool ScoresImpl::load() {

	std::ifstream file(fileName_, std::ios::binary);
	if (!file.is_open())
		return false;
	
	size_t count = 0;
	file.read((char*)&count, sizeof(count));
	records_.resize(count);
	if (records_.size() > 0)
		file.read((char*)&records_[0], records_.size() * sizeof(Record));
	
	return true;
}

ScoresPtr makeScores(const char* fileName) {

	return std::make_unique<ScoresImpl>(fileName);
}
