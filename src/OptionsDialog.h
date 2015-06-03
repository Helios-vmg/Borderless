/*

Copyright (c) 2015, Helios
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
	OptionsPack options;

	void setup_command_input();
	void setup_shortcuts_list_view();
	void setup_signals();
	bool sequence_exists_in_model(const QKeySequence &);
	void setup_general_options();
	OptionsPack build_options();
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
