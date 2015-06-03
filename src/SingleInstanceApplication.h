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
