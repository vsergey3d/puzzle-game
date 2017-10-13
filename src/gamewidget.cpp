#include "gamewidget.h"
#include "common.h"
#include "clickablelabel.h"
#include "scoredialog.h"
#include <QGridLayout>
#include <QPushButton>
#include <QBoxLayout>
#include <QMessageBox>
#include <QInputDialog>
#include <QTimer>
#include <QTime>
#include <QDir>

GameWidget::GameWidget(uint32_t size, QWidget *parent) : QWidget(parent) {

    puzzle_ = makePuzzle(size);
    scores_ = makeScores("scores");
    scores_->load();

    mainLayout_ = make_qt_owned<QBoxLayout>(QBoxLayout::TopToBottom);

    auto newGameBtn = make_qt_owned<QPushButton>(tr("new game"), this);
    connect(newGameBtn, SIGNAL(clicked()), this, SLOT(newGame()));
    newGameBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    undoBtn_ = make_qt_owned<QPushButton>(tr("<"), this);
    connect(undoBtn_, &QPushButton::clicked, [this]() { puzzle_->undo(); });
    undoBtn_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	undoBtn_->setFixedWidth(40);

    redoBtn_ = make_qt_owned<QPushButton>(tr(">"), this);
    connect(redoBtn_,  &QPushButton::clicked, [this]() { puzzle_->redo(); });
    redoBtn_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	redoBtn_->setFixedWidth(40);

    timer_ = make_qt_owned<QLineEdit>(tr("timer"), this);
    timer_->setReadOnly(true);
	timer_->setAlignment(Qt::AlignCenter);
    timer_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	timer_->setFixedWidth(70);

    auto scoreBtn = make_qt_owned<QPushButton>(tr("score"), this);
    connect(scoreBtn, SIGNAL(clicked()), this, SLOT(showScores()));
    scoreBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    auto btnLayout = make_qt_owned<QBoxLayout>(QBoxLayout::LeftToRight);
    btnLayout->addWidget(newGameBtn);
    btnLayout->addWidget(undoBtn_);
    btnLayout->addWidget(redoBtn_);
    btnLayout->addWidget(timer_);
    btnLayout->addWidget(scoreBtn);
    btnLayout->addStretch(1);

    mainLayout_->addLayout(btnLayout);
    mainLayout_->addLayout(puzzle_->getGrid());
    setLayout(mainLayout_);
    adjustSize();
    resize(minimumSizeHint());

    auto timer = make_qt_owned<QTimer>(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(onTimer()));
    timer->start(20);
}

void GameWidget::newGame() {

	auto ok = false;
    auto size = QInputDialog::getInt(this, tr("Enter new grid size"),
		tr("Grid size"), Puzzle::MinSize, Puzzle::MinSize, Puzzle::MaxSize, 1,
		&ok,  Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    if(ok) {
        puzzle_->reset(size);
        mainLayout_->addLayout(puzzle_->getGrid());
        resize(minimumSizeHint());
		isFinished_ = false;
    }
}

void GameWidget::showScores() {

    ScoreDialog dialog(*scores_.get());
    dialog.exec();
}

void GameWidget::onTimer() {

	if (isFinished_) {
		redoBtn_->setEnabled(false);
		undoBtn_->setEnabled(false);
		return;
	}
	puzzle_->update();

	timer_->setText(QString::fromStdString(formatTimeMSec(puzzle_->getSpentTimeSec())));
	redoBtn_->setEnabled(puzzle_->hasRedos() && !puzzle_->isBusy());
	undoBtn_->setEnabled(puzzle_->hasUndos() && !puzzle_->isBusy());

	if (puzzle_->isSolved()) {
		isFinished_ = true;

		auto ok = false;
		auto text = QInputDialog::getText(this, tr("You won!"), tr("Your name:"),
			QLineEdit::Normal, QDir::home().dirName(), &ok,
			Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

		if (ok && !text.isEmpty()) {
			scores_->addRecord(puzzle_->getSpentTimeSec(), text.toStdString().c_str());
			scores_->save();
		}
		showScores();
	}
}
