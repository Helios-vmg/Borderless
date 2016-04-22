/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#include "ImageViewerApplication.h"
#include "MainWindow.h"
#include "RotateDialog.h"
#include "Misc.h"
#include "OptionsDialog.h"
#include <QShortcut>
#include <QMessageBox>
#include <sstream>
#include <cassert>

ImageViewerApplication::ImageViewerApplication(int argc, char **argv, const QString &unique_name):
		SingleInstanceApplication(argc, argv, unique_name),
		do_not_save(false){
	this->restore_settings();
	this->setQuitOnLastWindowClosed(!this->settings->get_keep_application_in_background());
	this->new_instance(this->args);
	if (!this->windows.size() && !this->settings->get_keep_application_in_background())
		throw NoWindowsException();

	connect(this->desktop(), SIGNAL(resized(int)), this, SLOT(resolution_change(int)));
	connect(this->desktop(), SIGNAL(workAreaResized(int)), this, SLOT(work_area_change(int)));
}

ImageViewerApplication::~ImageViewerApplication(){
	this->save_settings();
	this->windows.clear();
}

void ImageViewerApplication::new_instance(const QStringList &args){
	sharedp_t p(new MainWindow(*this, args));
	if (!p->is_null())
		this->add_window(p);
}

void ImageViewerApplication::add_window(sharedp_t p){
	connect(p.get(), SIGNAL(closing(MainWindow *)), this, SLOT(window_closing(MainWindow *)));
	p->show();
	p->raise();
	p->activateWindow();
	p->setFocus();
	this->windows[(uintptr_t)p.get()] = p;
}

void ImageViewerApplication::window_closing(MainWindow *window){
	auto it = this->windows.find((uintptr_t)window);
	if (it == this->windows.end())
		return;
	this->windows.erase(it);
}

std::shared_ptr<DirectoryIterator> ImageViewerApplication::request_directory(const QString &path){
	auto clean = path;
	std::shared_ptr<DirectoryIterator> ret;
	if (!check_and_clean_path(clean))
		return ret;
	for (auto &p : this->listings){
		if (*p.first == clean){
			ret.reset(new DirectoryIterator(*p.first));
			p.second++;
			return ret;
		}
	}
	auto list = new DirectoryListing(clean);
	if (!*list){
		delete list;
		return ret;
	}
	this->listings.push_back(std::make_pair(list, 1));
	ret.reset(new DirectoryIterator(*list));
	return ret;
}

void ImageViewerApplication::release_directory(std::shared_ptr<DirectoryIterator> it){
	if (!it)
		return;
	bool found = false;
	size_t i = 0;
	for (auto &p : this->listings){
		if (p.first == it->get_listing()){
			found = true;
			break;
		}
		i++;
	}
	if (!found)
		return;
	auto *pair = &this->listings[i];
	if (!--pair->second){
		delete pair->first;
		this->listings.erase(this->listings.begin() + i);
	}
}

#define SAVE_SETTING(x) tree.set_value(#x, this->x)

void ImageViewerApplication::save_current_state(){
}

void ImageViewerApplication::save_current_windows(){
}

void ImageViewerApplication::save_settings(bool with_state){
}

void ImageViewerApplication::restore_current_state(){
}

void ImageViewerApplication::restore_current_windows(){
}

std::shared_ptr<QMenu> ImageViewerApplication::build_context_menu(MainWindow *caller){
	this->context_menu_last_requester = caller;
	std::shared_ptr<QMenu> ret(new QMenu);
	auto initial = ret->actions().size();
	caller->build_context_menu(*ret);
	if (ret->actions().size() != initial)
		ret->addSeparator();
	ret->addAction("Options...", this, SLOT(show_options()), this->shortcuts.get_current_sequence(show_options_command));
	ret->addAction("Quit", this, SLOT(quit_slot()), this->shortcuts.get_current_sequence(quit_command));
	return ret;
}

void ImageViewerApplication::set_option_values(){
	this->setQuitOnLastWindowClosed(!this->settings->get_keep_application_in_background());
}

void ImageViewerApplication::show_options(){
	OptionsDialog dialog(*this);
	dialog.exec();
}

void ImageViewerApplication::quit_and_discard_state(){
	this->save_settings(false);
	this->quit();
}

#define RESTORE_SETTING(key) tree->get_value(this->key, key##_setting)

void ImageViewerApplication::restore_settings(){
}

void ImageViewerApplication::resolution_change(int screen){
	for (auto &i : this->windows)
		i.second->resolution_change(screen);
}

void ImageViewerApplication::work_area_change(int screen){
	for (auto &i : this->windows)
		i.second->work_area_change(screen);
}

void ImageViewerApplication::minimize_all(){
	for (auto &p : this->windows)
		p.second->minimize_slot();
}

void ImageViewerApplication::options_changed(const std::vector<ShortcutTriple> &new_shortcuts){
	this->shortcuts.update(new_shortcuts);
	this->propagate_shortcuts();
}

void ImageViewerApplication::propagate_shortcuts(){
	for (auto &w : this->windows)
		w.second->setup_shortcuts();
}