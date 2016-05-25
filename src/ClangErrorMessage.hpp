#pragma once
#include <QWidget>
#include "ui_ClangErrorMessage.h"

class ClangErrorMessage : public QDialog {
	Q_OBJECT

public:
	ClangErrorMessage(QWidget * parent = Q_NULLPTR);
	~ClangErrorMessage();

	void set_error_message(const QString &);

private:
	Ui::ClangErrorMessage ui;
};
