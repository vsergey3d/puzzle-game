#pragma once
#include <memory>
#include <functional>

#include <QLabel>
#include <QPainter>
#include "clickablelabel.h"

class AnimImage {

public:
	AnimImage(QImage& sprite, uint32_t framesCount, uint32_t frame,
		const std::function<void()>& onClick);
	~AnimImage() = default;

	void setFrame(uint32_t number);
	uint32_t getFrame() const { return frame_; }
	uint32_t getFrameCount() const { return framesCount_; }
    QLabel* getQLabel() { return &label_; }

private:
	uint32_t size_ = 0;
	QImage image_;
	QPainter painter_;
    ClickableLabel label_;
	QPixmap sprite_;
	uint32_t framesCount_ = 0;
	uint32_t frame_ = 0;
	std::function<void()> onClick_;
};

using AnimImagePtr = std::unique_ptr<AnimImage>;

class Animation {

public:
	Animation(AnimImage& target, uint32_t delay, uint32_t duration,
		uint32_t startFrame, uint32_t endFrame);
	~Animation() = default;

	bool update(uint32_t msDelta);

private:
	AnimImage& target_;
	uint32_t delay_ = 0;
	uint32_t duration_ = 0;
	uint32_t elapsed_ = 0;
	uint32_t startFrame_ = 0;
	uint32_t endFrame_ = 0;
};

using AnimationPtr = std::unique_ptr<Animation>;
