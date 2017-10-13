#include "animation.h"
#include <algorithm>
#include <cassert>
#include <QObject>

AnimImage::AnimImage(QImage& sprite, uint32_t framesCount, uint32_t frame,
	const std::function<void()>& onClick) :
    size_(sprite.height()),
	image_(size_, size_, QImage::Format_ARGB32),
    sprite_(QPixmap::fromImage(sprite)),
	framesCount_(framesCount),
	onClick_(onClick) {

	setFrame(frame);
    label_.setFixedSize(size_, size_);
	QObject::connect(&label_, &ClickableLabel::clicked, [this]() {
		this->onClick_();
	});
}

void AnimImage::setFrame(uint32_t number) {

	assert(number < framesCount_);
	frame_ = number;
	QPainter painter;
	painter.begin(&image_);
	painter.eraseRect(0, 0, image_.width(), image_.height());
    painter.drawImage(0, 0, sprite_.toImage(), size_ * frame_, 0, size_, size_);
	painter.end();
	label_.setPixmap(QPixmap::fromImage(image_));
}

Animation::Animation(AnimImage& target, uint32_t delay, uint32_t duration,
	uint32_t startFrame, uint32_t endFrame) :
	target_(target),
	delay_(delay),
	duration_(duration),
	startFrame_(startFrame),
	endFrame_(endFrame) {
}

bool Animation::update(uint32_t msDelta) {

	auto factor = (elapsed_ > delay_) ? ((elapsed_ - delay_) / (float)duration_) : 0.0f;
	if (factor > 1.0f) {
		target_.setFrame(endFrame_);
		return true;
	}
	auto curFrame = (endFrame_ > startFrame_) ?
		uint32_t((endFrame_ - startFrame_) * factor) + startFrame_ :
		uint32_t((startFrame_ - endFrame_) * (1.0f - factor)) + endFrame_;

	target_.setFrame(curFrame);
	elapsed_ += msDelta;
	return false;
}
