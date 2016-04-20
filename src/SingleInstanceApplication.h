/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#ifndef SINGLEINSTANCEAPPLICATION_H
#define SINGLEINSTANCEAPPLICATION_H

#include <QApplication>
#include <QSharedMemory>
#include <QtNetwork/QLocalServer>
#include <QByteArray>
#include <QStringList>
#include <memory>
#include "DirectoryListing.h"
#include <map>
#include <vector>
#include <utility>
#include <exception>

class MainWindow;

class ApplicationAlreadyRunningException : public std::exception{};

class SingleInstanceApplication : public QApplication{
	Q_OBJECT

	bool running;
	QString unique_name;
	QSharedMemory shared_memory;
	std::shared_ptr<QLocalServer> local_server;

	static const int timeout = 1000;

	bool send_message(const QString &s){
		return this->send_message(s.toUtf8());
	}
	bool communicate_with_server(QLocalSocket &socket, qint64 &server_pid, const QStringList &list);
	bool communicate_with_server(QLocalSocket &socket, QByteArray &response, const QByteArray &msg);

protected:
	QStringList args;
	virtual void new_instance(const QStringList &args) = 0;

public:
	//May throw ApplicationAlreadyRunningException.
	explicit SingleInstanceApplication(int argc, char **argv, const QString &unique_name);
	bool is_running() const{
		return this->running;
	}

private:

//signals:
//	void message_available(QString message);

public slots:
	void receive_message();

};

#endif // SINGLEINSTANCEAPPLICATION_H
