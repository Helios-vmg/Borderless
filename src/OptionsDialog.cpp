/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#include "OptionsDialog.h"
#include "ImageViewerApplication.h"
#include <cassert>
#include <algorithm>
#include <QScreen>

ShortcutListModel::ShortcutListModel(const ApplicationShortcuts &shortcuts): items(shortcuts.get_current_shortcuts()){
	this->sort();
}

bool ShortcutTriple_order(const ShortcutTriple &a, const ShortcutTriple &b){
	return a.display_name < b.display_name;
}

void ShortcutListModel::sort(){
	std::sort(this->items.begin(), this->items.end(), ShortcutTriple_order);
}

// Given a range [first, last) and a predicate f such that for al
// first <= x < y !f(x) and for all y <= z < last f(z), find_all() returns y,
// or last if it does not exist.
template<class It, class F>
It find_all(It first, It last, const F &f){
	auto n = last - first;
	while (n > 0){
		auto n2 = n / 2;
		auto mid = first + n2;
		if (!f(*mid)){
			first = ++mid;
			n -= n2 + 1;
		}else
			n = n2;
	}
	return (first);
}

void ShortcutListModel::add_new_item(const ApplicationShortcuts &shortcuts, const QString &command, const QKeySequence &sequence){
	if (this->sequence_already_exists(command))
		return;
	auto info = shortcuts.get_shortcut_info(command);
	if (!info)
		return;
	auto it = find_all(this->items.begin(), this->items.end(),
		[=](const ShortcutTriple &a){
			return info->display_name < a.display_name;
		});
	auto position = it - this->items.begin();
	ShortcutTriple triple;
	triple.command = command;
	triple.display_name = info->display_name;
	triple.sequence = sequence;
	this->beginInsertRows(QModelIndex(), position, position);
	this->items.insert(it, triple);
	this->endInsertRows();
	emit item_inserted_at(position);
}

bool ShortcutListModel::sequence_already_exists(const QKeySequence &seq) const{
	for (auto &i : this->items)
		if (i.sequence == seq)
			return true;
	return false;
}

QModelIndex ShortcutListModel::create_index(int row){
	return this->createIndex(row, 0);
}

const ShortcutTriple &ShortcutListModel::get_index(const QModelIndex &index) const{
	return this->items[index.row()];
}

ShortcutTriple ShortcutListModel::remove_index(const QModelIndex &index){
	auto i = index.row();
	auto ret = this->items[i];
	this->beginRemoveRows(index.parent(), i, i);
	this->items.erase(this->items.begin() + i);
	this->endRemoveRows();
	return ret;
}

void ShortcutListModel::replace_all(const std::vector<ShortcutTriple> &new_shortcuts){
	if (this->items.size()){
		this->beginRemoveRows(QModelIndex(), 0, this->items.size() - 1);
		this->items.clear();
		this->endRemoveRows();
	}
	if (new_shortcuts.size()){
		this->beginInsertRows(QModelIndex(), 0, new_shortcuts.size() - 1);
		this->items = new_shortcuts;
		this->sort();
		this->endInsertRows();
	}
}

QModelIndex ShortcutListModel::index(int row, int column, const QModelIndex &) const{
	return this->createIndex(row, column);
}

QModelIndex ShortcutListModel::parent(const QModelIndex &) const{
	return QModelIndex();
}

int ShortcutListModel::rowCount(const QModelIndex &parent) const{
	if (parent.row() >= 0 || parent.column() >= 0)
		return 0;
	return this->items.size();
}

int ShortcutListModel::columnCount(const QModelIndex &parent) const{
	if (parent.row() >= 0 || parent.column() >= 0)
		return 0;
	return 2;
}

QVariant ShortcutListModel::data(const QModelIndex &index, int role) const{
	if (index.row() < 0 || (size_t)index.row() >= this->items.size() || index.column() < 0 || index.column() > 1 || role != Qt::DisplayRole)
		return QVariant();
	switch (index.column()){
		case 0:
			return this->items[index.row()].display_name;
		case 1:
			return this->items[index.row()].sequence.toString();
	}
	assert(false);
	return QVariant();
}

class ShortcutListHeaderModel : public QAbstractItemModel{
public:
	QModelIndex index(int row, int column, const QModelIndex &) const{
		return this->createIndex(row, column);
	}
	QModelIndex parent(const QModelIndex &) const{
		return QModelIndex();
	}
	int rowCount(const QModelIndex &) const{
		return 0;
	}
	int columnCount(const QModelIndex &parent) const{
		if (parent.row() >= 0 || parent.column() >= 0)
			return 0;
		return 2;
	}
	QVariant data(const QModelIndex &, int) const{
		return QVariant();
	}
	QVariant headerData(int section, Qt::Orientation, int role) const{
		if (section < 0 || section > 1 || role != Qt::DisplayRole)
			return QVariant();
		switch (section){
			case 0:
				return "Action";
			case 1:
				return "Shortcut";
		}
		assert(false);
		return QVariant();
	}
};

static ShortcutListHeaderModel slhm;

OptionsDialog::OptionsDialog(ImageViewerApplication &app):
		ui(new Ui_OptionsDialog),
		app(&app),
		no_shortcut_changes(true){
	this->setModal(true);
	this->ui->setupUi(this);

	this->options = app.get_option_values();
	
	this->setup_command_input();
	this->setup_shortcuts_list_view();
	this->setup_general_options();
	this->setup_signals();
	auto geom = this->screen()->availableGeometry();
	this->resize(geom.size() / 2);
}

void OptionsDialog::setup_command_input(){
	auto &infos = this->app->get_shortcuts().get_shortcut_infos();
	std::vector<std::pair<QString, QString> > v;
	for (auto &p : infos){
		v.push_back(std::make_pair((QString)p.second.display_name, (QString)p.second.internal_name));
	}
	std::sort(v.begin(), v.end());
	for (auto &i : v){
		this->ui->command_input->addItem(i.first, i.second);
	}
	this->ui->command_input->setCurrentIndex(-1);
}

void OptionsDialog::setup_shortcuts_list_view(){
	this->sl_model.reset(new ShortcutListModel(this->app->get_shortcuts()));
	this->sl_selmodel.reset(new SimpleItemSelectionModel(*this->sl_model));
	auto lv = this->ui->shortcuts_list_view;
	lv->setModel(this->sl_model.get());
	lv->header()->setModel(&slhm);
	lv->resizeColumnToContents(0);
	lv->setSelectionModel(this->sl_selmodel.get());
}

void OptionsDialog::setup_general_options(){
	this->ui->center_when_displayed_cb->setChecked(this->options->get_center_when_displayed());
	this->ui->use_checkerboard_pattern_cb->setChecked(this->options->get_use_checkerboard_pattern());
	this->ui->clamp_to_edges_cb->setChecked(this->options->get_clamp_to_edges());
	this->ui->save_state_on_exit_cb->setChecked(this->options->get_save_state_on_exit());
    this->ui->keep_application_running_cb->setChecked(this->options->get_keep_application_in_background());
	this->ui->clamp_strength_spinbox->setValue(this->options->get_clamp_strength());
	this->ui->zoom_mode_for_new_windows_cb->set_selected_item(this->options->get_zoom_mode_for_new_windows());
	this->ui->fullscreen_zoom_mode_for_new_windows_cb->set_selected_item(this->options->get_fullscreen_zoom_mode_for_new_windows());
	this->ui->resize_windows_cb->setChecked(this->options->get_resize_windows_on_monitor_change());
}

void OptionsDialog::setup_signals(){
	connect(this->ui->key_sequence_input, SIGNAL(editingFinished()), this, SLOT(sequence_entered()));
	connect(this->sl_model.get(), SIGNAL(item_inserted_at(size_t)), this, SLOT(item_inserted_into_shortcut_model(size_t)));
	connect(
		this->sl_selmodel.get(),
		SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
		this,
		SLOT(selected_shortcut_changed(const QItemSelection &, const QItemSelection &))
	);
	connect(this->ui->add_button, SIGNAL(clicked(bool)), this, SLOT(add_button_clicked(bool)));
	connect(this->ui->remove_button, SIGNAL(clicked(bool)), this, SLOT(remove_button_clicked(bool)));
	connect(this->ui->reset_button, SIGNAL(clicked(bool)), this, SLOT(reset_button_clicked(bool)));
}

bool OptionsDialog::sequence_exists_in_model(const QKeySequence &seq){
	return this->sl_model->sequence_already_exists(seq);
}

void OptionsDialog::sequence_entered(){
	auto seq = this->ui->key_sequence_input->keySequence();
	this->ui->add_button->setEnabled(!this->sequence_exists_in_model(seq));
}

void OptionsDialog::selected_shortcut_changed(const QItemSelection &selected, const QItemSelection &){
	this->ui->remove_button->setEnabled(!!selected.indexes().size());
}

void OptionsDialog::add_button_clicked(bool){
	auto seq = this->ui->key_sequence_input->keySequence();
	if (this->ui->command_input->currentIndex() < 0)
		return;
	auto command = this->ui->command_input->currentData();
	auto str_command = command.toString();
	this->sl_model->add_new_item(this->app->get_shortcuts(), str_command, seq);
	this->no_shortcut_changes = false;
}

void OptionsDialog::remove_button_clicked(bool){
	auto index = this->ui->shortcuts_list_view->currentIndex();
	auto item = this->sl_model->remove_index(index);
	auto &combo = this->ui->command_input;
	int found = -1;
	for (auto i = combo->count(); i--;){
		auto data = combo->itemData(i);
		auto command = data.toString();
		if (command == item.command){
			found = i;
			break;
		}
	}
	assert(found >= 0);
	combo->setCurrentIndex(found);
	this->ui->key_sequence_input->setKeySequence(item.sequence);
	this->sequence_entered();
	this->no_shortcut_changes = false;
	this->ui->shortcuts_list_view->setCurrentIndex(index);
}

void OptionsDialog::item_inserted_into_shortcut_model(size_t index){
	auto lv = this->ui->shortcuts_list_view;
	lv->setCurrentIndex(this->sl_model->create_index((int)index));
	this->ui->add_button->setEnabled(false);
}

std::shared_ptr<MainSettings> OptionsDialog::build_options(){
	auto ret = std::make_shared<MainSettings>();
	ret->set_center_when_displayed(this->ui->center_when_displayed_cb->isChecked());
	ret->set_use_checkerboard_pattern(this->ui->use_checkerboard_pattern_cb->isChecked());
	ret->set_clamp_to_edges(this->ui->clamp_to_edges_cb->isChecked());
	ret->set_save_state_on_exit(this->ui->save_state_on_exit_cb->isChecked());
	ret->set_keep_application_in_background(this->ui->keep_application_running_cb->isChecked());
	ret->set_clamp_strength(this->ui->clamp_strength_spinbox->value());
	ret->set_zoom_mode_for_new_windows(this->ui->zoom_mode_for_new_windows_cb->get_selected_item());
	ret->set_fullscreen_zoom_mode_for_new_windows(this->ui->fullscreen_zoom_mode_for_new_windows_cb->get_selected_item());
	ret->set_resize_windows_on_monitor_change(this->ui->resize_windows_cb->isChecked());
	return ret;
}

void OptionsDialog::accept(){
	auto options = this->build_options();
	if (*options != *this->options)
		this->app->set_option_values(*options);
	if (!this->no_shortcut_changes)
		this->app->options_changed(this->sl_model->get_items());
	this->hide();
}

void OptionsDialog::reject(){
	this->hide();
}

void OptionsDialog::reset_button_clicked(bool){
	auto defaults = this->app->get_shortcuts().get_default_shortcuts();
	this->sl_model->replace_all(defaults);
	this->ui->shortcuts_list_view->resizeColumnToContents(0);
	this->no_shortcut_changes = false;
}
