/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#include "MainWindow.h"
#include "ui_mainwindow.h"
#include <QDir>

void MainWindow::restore_state(){
}

#define SAVE_SETTING(x) ret->set_value(#x, this->x)

void MainWindow::save_state() const{
}

void MainWindow::reset_settings(){
}
