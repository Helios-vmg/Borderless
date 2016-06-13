#include "ClangErrorMessage.hpp"

ClangErrorMessage::ClangErrorMessage(QWidget * parent) : QDialog(parent) {
	ui.setupUi(this);
}

ClangErrorMessage::~ClangErrorMessage() {
	
}

void ClangErrorMessage::set_error_message(const QString &msg){
	this->ui.plainTextEdit->setPlainText(msg);
}
