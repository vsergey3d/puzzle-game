#include "scoredialog.h"
#include "common.h"
#include <QTableView>
#include <QStandardItemModel>
#include <QHBoxLayout>

enum Cols {

	Name,
	Time,
	Count
};

ScoreDialog::ScoreDialog(Scores& scores, QWidget* parent) :
	QDialog(parent, Qt::WindowTitleHint | Qt::WindowCloseButtonHint) {

    model_ = make_qt_owned<QStandardItemModel>(scores.getRecordsCount(), Cols::Count, this);
    model_->setHeaderData(Cols::Name, Qt::Horizontal, tr("Name"));
    model_->setHeaderData(Cols::Time, Qt::Horizontal, tr("Time"));

    for(auto i = 0u; i < scores.getRecordsCount(); ++i) {
       const auto& rec = scores.getRecord(i);

	   auto nameItem = make_qt_owned<QStandardItem>(QString::fromStdString(rec.name));
	   nameItem->setFlags(Qt::ItemIsEnabled);
       model_->setItem(i, Cols::Name,nameItem);

	   auto timeItem = make_qt_owned<QStandardItem>(QString::fromStdString(formatTimeMSec(rec.seconds)));
	   timeItem->setFlags(Qt::ItemIsEnabled);
	   model_->setItem(i, Cols::Time, timeItem);
    }

    view_ = make_qt_owned<QTableView>(this);
    view_->setModel(model_);

	auto topLayout = make_qt_owned<QVBoxLayout>();
    topLayout->addWidget(view_, 1);
    setLayout(topLayout);

    adjustSize();
    setWindowTitle("Score table");
	setFixedSize(350, 496);
}
