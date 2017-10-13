#pragma once
#include <QLabel>

class ClickableLabel : public QLabel {

    Q_OBJECT
public:
    explicit ClickableLabel(QWidget* parent = nullptr,
		Qt::WindowFlags f = Qt::WindowFlags());
    virtual ~ClickableLabel() = default;

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent*);

};
