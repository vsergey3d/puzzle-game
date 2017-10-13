#pragma once
#include "scores.h"
#include <QDialog>

class QTableView;
class QStandardItemModel;

class ScoreDialog : public QDialog {

    Q_OBJECT
public:
    ScoreDialog(Scores& scores, QWidget *parent = nullptr);
    ~ScoreDialog() = default;

private:
	QTableView* view_ = nullptr;
	QStandardItemModel* model_ = nullptr;
};
