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

#include "SingleInstanceApplication.h"
#include <QtNetwork/QLocalSocket>
#include <QDataStream>
#include <exception>
#include "Misc.h"
#include "MainWindow.h"
#include <QProcess>
#include <fstream>

#ifdef WIN32
#include <Windows.h>

void allow_set_foreground_window(qint64 pid){
	auto ret = AllowSetForegroundWindow((DWORD)(qulonglong)pid);
	ret = ret;
}
#else
void allow_set_foreground_window(qulonglong pid){
}
#endif

SingleInstanceApplication::SingleInstanceApplication(int argc, char *argv[], const QString &unique_name):
		QApplication(argc, argv),
		unique_name(unique_name),
		running(false){
	this->args = this->arguments();
	this->shared_memory.setKey(unique_name);
	bool success;
	for (int tries = 0; tries < 5; tries++){
		if (this->shared_memory.attach()){
			QLocalSocket socket(this);
			this->running = true;
			qint64 server_pid;
			if (this->communicate_with_server(socket, server_pid, this->arguments()))
				allow_set_foreground_window(server_pid);
			throw ApplicationAlreadyRunningException();
		}
		success = this->shared_memory.create(1);
		if (success)
			break;
	}

	if (!success)
		throw std::exception("Unable to allocate shared memory.");

	this->local_server.reset(new QLocalServer(this));
	connect(this->local_server.get(), SIGNAL(newConnection()), this, SLOT(receive_message()));
	this->local_server->listen(unique_name);
}

void SingleInstanceApplication::receive_message(){
	auto socket = this->local_server->nextPendingConnection();
	if (!socket->waitForReadyRead(this->timeout))
		return;

	auto byte_msg = socket->readAll();
	{
		qint64 pid = this->applicationPid();
		socket->write(QByteArray((const char *)&pid, sizeof(pid)));
		socket->waitForBytesWritten(this->timeout);
	}
	socket->waitForDisconnected(this->timeout);
	this->new_instance(to_QStringList(byte_msg));
}

bool SingleInstanceApplication::communicate_with_server(QLocalSocket &socket, qint64 &server_pid, const QStringList &list){
	QByteArray response;
	if (!this->communicate_with_server(socket, response, to_QByteArray(list)))
		return false;
	server_pid = 0;
	unsigned char buf[sizeof(server_pid)];
	for (int i = 0; i < response.size(); i++)
		buf[i] = response[i];
	server_pid = *(quint64 *)buf;
	return true;
}

bool SingleInstanceApplication::communicate_with_server(QLocalSocket &socket, QByteArray &response, const QByteArray &msg){
	if (!this->running)
		return false;
	socket.connectToServer(this->unique_name, QIODevice::ReadWrite);
	if (!socket.waitForConnected(this->timeout)){
		qDebug(socket.errorString().toLatin1());
		return false;
	}
	socket.write(msg);
	if (!socket.waitForBytesWritten(this->timeout)){
		qDebug(socket.errorString().toLatin1());
		return false;
	}
	if (!socket.waitForReadyRead(this->timeout)){
		qDebug(socket.errorString().toLatin1());
		return false;
	}
	response = socket.readAll();
	return true;
}
