#pragma once
#include "puzzle.h"
#include "scores.h"
#include <memory>
#include <stdint.h>
#include <QWidget>
#include <QLabel>

class QGridLayout;
class QBoxLayout;
class QLineEdit;
class QPushButton;

class GameWidget : public QWidget {

    Q_OBJECT
public:
    explicit GameWidget(uint32_t size, QWidget* parent = nullptr);

private:
	QBoxLayout* mainLayout_ = nullptr;
	QLineEdit* timer_ = nullptr;
	QPushButton* undoBtn_ = nullptr;
	QPushButton* redoBtn_ = nullptr;
	PuzzlePtr puzzle_ = nullptr;
	ScoresPtr scores_ = nullptr;
	bool isFinished_ = false;

private slots:
    void newGame();
    void showScores();
    void onTimer();
};
