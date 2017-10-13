#include "puzzle.h"
#include "common.h"
#include "command.h"
#include "animation.h"
#include <cassert>
#include <vector>
#include <algorithm>
#include <algorithm>
#include <random>
#include <chrono>
#include <QGridLayout>

using namespace std::chrono;

using TimePoint = time_point<system_clock>;

static const auto AnimationDelay = 150u;
static const auto AnimationDuration = 250u;

static const auto KnobStartFrame = 0u;
static const auto KnobMiddleFrame = 6u;
static const auto KnobEndFrame = 12u;

static const auto LockStartFrame = 0u;
static const auto LockEndFrame = 6u;

class PuzzleImpl : public Puzzle {

public:
    PuzzleImpl(uint32_t size);
	virtual ~PuzzleImpl() = default;
	void update() override;
	void reset(uint32_t size) override;
	void turnKnob(uint32_t x, uint32_t y) override;
	void undo() override;
	void redo() override;
    bool hasUndos() const override { return !undos_.empty(); }
    bool hasRedos() const override { return !redos_.empty(); }
    bool isBusy() const override { return !animations_.empty();}
	bool isSolved() const override;
	uint32_t getSpentTimeSec() const override;
    QGridLayout* getGrid() const override { return grid_.get(); }

private:
	void turnKnobAction(uint32_t x, uint32_t y);
	void updateKnob(uint32_t index, uint32_t delay);
	void updateLock(uint32_t index, uint32_t delay);
	void generateField();
	void rebuild();

	struct Knob {

		AnimImagePtr image = nullptr;
		bool checked = false;
	};

	struct Lock {

		AnimImagePtr image = nullptr;
		bool locked = true;
	};

	class TurnKnobCommand : public Command {

	public:
		TurnKnobCommand(PuzzleImpl& owner, uint32_t x, uint32_t y) :
			owner_(owner), x_(x), y_(y) {}
		virtual ~TurnKnobCommand() = default;

		void doAction() override { owner_.turnKnobAction(x_, y_); }
		void undoAction() override { owner_.turnKnobAction(x_, y_); }

	private:
		PuzzleImpl& owner_;
		uint32_t x_ = 0u;
		uint32_t y_ = 0u;
	};

    std::unique_ptr<QGridLayout> grid_;
	QImage knobSprite_;
	QImage lockSprite_;
	uint32_t size_ = MinSize;
	std::vector<Lock> locks_;
	std::vector<Knob> knobs_;
	std::vector<CommandPtr> undos_;
	std::vector<CommandPtr> redos_;
	std::vector<AnimationPtr> animations_;
	TimePoint startTime_ = system_clock::now();
	TimePoint lastFrameTime_ = system_clock::now();
	milliseconds spentTime_ = milliseconds::zero();
};

PuzzleImpl::PuzzleImpl(uint32_t size) :
    knobSprite_(":/icons/knob.png"),
	lockSprite_(":/icons/lock.png") {

	reset(size);
}

void PuzzleImpl::update() {

	auto curTime = system_clock::now();
	auto dt = duration_cast<milliseconds>(curTime - lastFrameTime_);
	lastFrameTime_ = curTime;

	for (size_t i = 0; i < animations_.size();) {
		auto& anim = animations_[i];
		if (anim->update(dt.count())) {
			animations_[i] = std::move(animations_.back());
			animations_.pop_back();
		} else {
			++i;
		}
	}
	if (!isSolved())
		spentTime_ += dt;
}

void PuzzleImpl::reset(uint32_t size) {

	assert(size >= MinSize);
	assert(size <= MaxSize);

	animations_.clear();
	undos_.clear();
	redos_.clear();
	locks_.clear();
	knobs_.clear();

	size_ = size;
	locks_.resize(size_);
	rebuild();

	startTime_ = system_clock::now();
	spentTime_ = milliseconds::zero();
}

void PuzzleImpl::turnKnob(uint32_t x, uint32_t y) {

	assert(x < size_);
	assert(y < size_);

	if (isBusy())
		return;

	redos_.clear();

	auto command = std::make_unique<TurnKnobCommand>(*this, x, y);
	command->doAction();
	undos_.push_back(std::move(command));
}

void PuzzleImpl::undo() {

	if (isBusy() || undos_.empty())
		return;

	redos_.push_back(std::move(undos_.back()));
	undos_.pop_back();
	redos_.back()->undoAction();
}

void PuzzleImpl::redo() {

	if (isBusy() || redos_.empty())
		return;

	undos_.push_back(std::move(redos_.back()));
	redos_.pop_back();
	undos_.back()->doAction();
}

bool PuzzleImpl::isSolved() const {

	if (isBusy())
		return false;

	return std::find_if(locks_.begin(), locks_.end(), [](const auto& lock) {
		return lock.locked;
	}) == locks_.end();
}

uint32_t PuzzleImpl::getSpentTimeSec() const {

	return duration_cast<seconds>(spentTime_).count();
}

static uint32_t difference(uint32_t v0, uint32_t v1) {

	return (v0 > v1) ? (v0 - v1) : (v1 - v0);
}

void PuzzleImpl::turnKnobAction(uint32_t x, uint32_t y) {

	assert(x < size_);
	assert(y < size_);
	// start from center knob
	auto centerIndex = y * size_ + x;
	updateKnob(centerIndex, 0);
	// iterate through current row and column (excluding center element)
	for (auto i = 0u; i < size_; ++i) {
		auto horizIndex = y * size_ + i;
		if (horizIndex != centerIndex)
			updateKnob(horizIndex, difference(x, i) * AnimationDelay);
		auto vertIndex = i * size_ + x;
		if (vertIndex != centerIndex)
			updateKnob(vertIndex, difference(y, i) * AnimationDelay);
	}
	auto lockDelay = AnimationDelay * (std::max(
		std::max(difference(x, 0), difference(x, size_ - 1)),
		std::max(difference(y, 0), difference(y, size_ - 1))) - 1);
	// check each column and update locks
	for (auto ix = 0u; ix < size_; ++ix) {
		auto unlocked = true;
		for (auto iy = 0u; iy < size_; ++iy)
			unlocked = unlocked && knobs_[iy * size_ + ix].checked;
		auto& lock = locks_[ix];
		if (lock.locked != (!unlocked))
			updateLock(ix, lockDelay);
	}
}

void PuzzleImpl::updateKnob(uint32_t index, uint32_t delay) {

	assert(index < knobs_.size());
	auto& knob = knobs_[index];
	const auto startFrame = knob.checked ? KnobStartFrame : KnobMiddleFrame;
	const auto endFrame = knob.checked ? KnobMiddleFrame : KnobEndFrame;
	animations_.push_back(std::make_unique<Animation>(
		*knob.image.get(), delay, AnimationDuration, startFrame, endFrame));
	knob.checked = !knob.checked;
}

void PuzzleImpl::updateLock(uint32_t index, uint32_t delay) {

	assert(index < locks_.size());
	auto& lock = locks_[index];
	const auto startFrame = lock.locked ? LockStartFrame : LockEndFrame;
	const auto endFrame = lock.locked ? LockEndFrame : LockStartFrame;
	animations_.push_back(std::make_unique<Animation>(
		*lock.image.get(), delay, AnimationDuration, startFrame, endFrame));
	lock.locked = !lock.locked;
}

void clearLayout(QGridLayout* layout) {

	QLayoutItem* item = nullptr;

    while ((item = layout->takeAt(0))) {
		auto widget = item->widget();
        if (widget) {
            widget->hide();
			delete_qt_forced(widget);
        } else
			delete_qt_forced(item);
    }
}

void PuzzleImpl::rebuild() {

    if(grid_.get())
        clearLayout(grid_.get());

    grid_ = std::make_unique<QGridLayout>();
    grid_->setSpacing(0);

	generateField();
	
    auto sz = knobSprite_.height();
    grid_->addItem(make_qt_owned<QSpacerItem>(sz, sz * 2,
		QSizePolicy::Minimum, QSizePolicy::Expanding), 0, 0, 1, size_ + 2);
    grid_->addItem(make_qt_owned<QSpacerItem>(sz, sz * 2,
		QSizePolicy::Minimum, QSizePolicy::Expanding), size_ + 2, 0, 1, size_ + 2);
 	grid_->addItem(make_qt_owned<QSpacerItem>(sz * 2, sz,
		QSizePolicy::Expanding, QSizePolicy::Minimum), 1, 0, size_ + 1, 1);
	grid_->addItem(make_qt_owned<QSpacerItem>(sz * 2, sz,
		QSizePolicy::Expanding, QSizePolicy::Minimum), 1, size_ + 1, size_ + 1, 1);
}

void PuzzleImpl::generateField() {

	auto engine = std::default_random_engine(
		system_clock::to_time_t(system_clock::now()));
	const auto distrBool = std::uniform_int_distribution<>(0, 1);
	const auto distrSize = std::uniform_int_distribution<>(0, size_ - 1);
	const auto knobSpriteFrames = knobSprite_.width() / knobSprite_.height();
	const auto lockSpriteFrames = lockSprite_.width() / lockSprite_.height();

	knobs_.resize(size_ * size_);
	locks_.resize(size_);

	for (auto ix = 0u; ix < size_; ++ix) {
		auto& lock = locks_[ix];
		lock.locked = true;
		// insert lock
		lock.image = std::make_unique<AnimImage>(lockSprite_,
			lockSpriteFrames, LockStartFrame, []() {});
		grid_->addWidget(lock.image->getQLabel(), 0 + 1, ix + 1);
		// generate random knobs values
		auto unlocked = true;
		for (auto iy = 0u; iy < size_; ++iy) {
			auto& knob = knobs_[iy * size_ + ix];
			knob.checked = distrBool(engine);
			unlocked = unlocked && knob.checked;
		}
		// prevent unlocked locks
		if (unlocked) {
			auto randIndex = distrSize(engine) * size_ + ix;
			knobs_[randIndex].checked = !knobs_[randIndex].checked;
		}
		// insert knobs
		for (auto iy = 0u; iy < size_; ++iy) {
			auto& knob = knobs_[iy * size_ + ix];
			knob.image = std::make_unique<AnimImage>(knobSprite_, knobSpriteFrames,
				knob.checked ? KnobStartFrame : KnobMiddleFrame,
				[this, ix, iy]() { this->turnKnob(ix, iy); });
			grid_->addWidget(knob.image->getQLabel(), iy + 1 + 1, ix + 1);
		}
	}
}

PuzzlePtr makePuzzle(uint32_t size) {
    return std::make_unique<PuzzleImpl>(size);
}
