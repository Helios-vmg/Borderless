/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#ifndef SHORTCUTINFO_H
#define SHORTCUTINFO_H

#include <QKeySequence>

struct ShortcutInfo {
	const char *display_name;
	const char *internal_name;
	const char *default_sequences_str[4];
	QKeySequence default_sequences_seq[4];
	size_t sequences_count;
	ShortcutInfo() {}
	ShortcutInfo(const char *display_name, const char *internal_name) : display_name(display_name), internal_name(internal_name), sequences_count(0) {}
	ShortcutInfo(const char *display_name, const char *internal_name, const char *s1) : display_name(display_name), internal_name(internal_name), sequences_count(0) {
		*this << s1;
	}
	ShortcutInfo(const char *display_name, const char *internal_name, const char *s1, const char *s2) : display_name(display_name), internal_name(internal_name), sequences_count(0) {
		*this << s1 << s2;
	}
	ShortcutInfo &operator<<(const char *s) {
		this->sequences_count %= 4;
		this->default_sequences_str[this->sequences_count] = s;
		this->default_sequences_seq[this->sequences_count] = QKeySequence((QString)s);
		this->sequences_count++;
		return *this;
	}
};

#endif
