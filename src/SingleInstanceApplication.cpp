/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#include "SingleInstanceApplication.h"
#include "GenericException.h"
#include <QtNetwork/QLocalSocket>
#include <QDataStream>
#include <exception>
#include "Misc.h"
#include "MainWindow.h"
#include <QProcess>
#include <fstream>

//#define DISABLE_SINGLE_INSTANCE

#ifdef WIN32
#include <Windows.h>

void allow_set_foreground_window(qint64 pid){
	auto ret = AllowSetForegroundWindow((DWORD)(qulonglong)pid);
	ret = ret;
}
#else
void allow_set_foreground_window(qulonglong){
}
#endif

SingleInstanceApplication::SingleInstanceApplication(int &argc, char **argv, const QString &unique_name):
		QApplication(argc, argv),
		running(false),
		unique_name(unique_name){
#ifndef DISABLE_SINGLE_INSTANCE
	this->args = this->arguments();
	bool success = false;
	for (int tries = 0; tries < 5 && !success; tries++){
		this->shared_memory.reset(new QSharedMemory);
		this->shared_memory->setKey(unique_name);
		for (; tries < 5 && !success; tries++){
			if (this->shared_memory->attach()){
				QLocalSocket socket(this);
				this->running = true;
				qint64 server_pid;
				if (this->communicate_with_server(socket, server_pid, this->arguments()))
					allow_set_foreground_window(server_pid);
				else{
					this->clear_shared_memory();
					break;
				}
				throw ApplicationAlreadyRunningException();
			}
			success = this->shared_memory->create(1);
		}
	}

	if (!success)
		throw GenericException("Unable to allocate shared memory.");

	this->local_server.reset(new QLocalServer(this));
	connect(this->local_server.get(), SIGNAL(newConnection()), this, SLOT(receive_message()));
	for (int i = 0; i < 2 && !this->local_server->listen(unique_name); i++)
		this->clear_shared_memory();
#endif
}

void SingleInstanceApplication::clear_shared_memory(){
#ifndef WIN32
	QFile file("/tmp/" + unique_name);
	if (file.exists())
		std::cerr << "Shared memory exists!\n";
	file.remove();
#endif
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
	union{
		unsigned char buf[sizeof(qint64)];
		qint64 pid;
	} u;
	for (int i = 0; i < response.size(); i++)
		u.buf[i] = response[i];
	server_pid = u.pid;
	return true;
}

bool SingleInstanceApplication::communicate_with_server(QLocalSocket &socket, QByteArray &response, const QByteArray &msg){
	if (!this->running)
		return false;
	socket.connectToServer(this->unique_name, QIODevice::ReadWrite);
	if (!socket.waitForConnected(this->timeout)){
		qDebug() << socket.errorString().toLatin1();
		return false;
	}
	socket.write(msg);
	if (!socket.waitForBytesWritten(this->timeout)){
		qDebug() << socket.errorString().toLatin1();
		return false;
	}
	if (!socket.waitForReadyRead(this->timeout)){
		qDebug() << socket.errorString().toLatin1();
		return false;
	}
	response = socket.readAll();
	return true;
}
