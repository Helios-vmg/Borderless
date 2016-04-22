/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#ifndef OPTIONS_DIALOG_H
#define OPTIONS_DIALOG_H

#include "ui_OptionsDialog.h"
#include "Shortcuts.h"
#include "ImageViewerApplication.h"
#include <memory>

class ShortcutListModel : public QAbstractItemModel{
	Q_OBJECT
	std::vector<ShortcutTriple> items;
public:
	ShortcutListModel(const ApplicationShortcuts &shortcuts);
	QModelIndex index(int row, int column, const QModelIndex &parent) const;
	QModelIndex parent(const QModelIndex &idx) const;
	int rowCount(const QModelIndex &parent) const;
	int columnCount(const QModelIndex &parent) const;
	QVariant data(const QModelIndex &index, int role) const;
	const std::vector<ShortcutTriple> &get_items() const{
		return this->items;
	}
	void sort();
	void add_new_item(const ApplicationShortcuts &shortcuts, const QString &command, const QKeySequence &sequence);
	bool sequence_already_exists(const QKeySequence &) const;
	QModelIndex create_index(int row);
	const ShortcutTriple &get_index(const QModelIndex &index) const;
	ShortcutTriple remove_index(const QModelIndex &index);
	void replace_all(const std::vector<ShortcutTriple> &);
signals:
	void item_inserted_at(size_t);
public slots:
};

class SimpleItemSelectionModel : public QItemSelectionModel{
public:
	SimpleItemSelectionModel(QAbstractItemModel &model) : QItemSelectionModel(&model){}
};

class OptionsDialog : public QDialog{
	Q_OBJECT
	std::shared_ptr<Ui_OptionsDialog> ui;
	std::shared_ptr<ShortcutListModel> sl_model;
	std::shared_ptr<SimpleItemSelectionModel> sl_selmodel;
	ImageViewerApplication *app;
	bool no_changes;
	std::shared_ptr<MainSettings> options;

	void setup_command_input();
	void setup_shortcuts_list_view();
	void setup_signals();
	bool sequence_exists_in_model(const QKeySequence &);
	void setup_general_options();
	std::shared_ptr<MainSettings> build_options();
public:
	OptionsDialog(ImageViewerApplication &app);

signals:

public slots:
	void sequence_entered();
	void selected_shortcut_changed(const QItemSelection &selected, const QItemSelection &deselected);
	void add_button_clicked(bool);
	void remove_button_clicked(bool);
	void reset_button_clicked(bool);
	void item_inserted_into_shortcut_model(size_t);
	void accept();
	void reject();
};

#endif
