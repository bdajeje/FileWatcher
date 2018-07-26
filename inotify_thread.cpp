#include "inotify_thread.hpp"

#include <QDir>
#include <QFileInfo>

#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

namespace {
	 void             /* Display information from inotify_event structure */
	 displayInotifyEvent(struct inotify_event *i)
	 {
			 printf("    wd =%2d; ", i->wd);
			 if (i->cookie > 0)
					 printf("cookie =%4d; ", i->cookie);

			 printf("mask = ");
			 if (i->mask & IN_ACCESS)        printf("IN_ACCESS ");
			 if (i->mask & IN_ATTRIB)        printf("IN_ATTRIB ");
			 if (i->mask & IN_CLOSE_NOWRITE) printf("IN_CLOSE_NOWRITE ");
			 if (i->mask & IN_CLOSE_WRITE)   printf("IN_CLOSE_WRITE ");
			 if (i->mask & IN_CREATE)        printf("IN_CREATE ");
			 if (i->mask & IN_DELETE)        printf("IN_DELETE ");
			 if (i->mask & IN_DELETE_SELF)   printf("IN_DELETE_SELF ");
			 if (i->mask & IN_IGNORED)       printf("IN_IGNORED ");
			 if (i->mask & IN_ISDIR)         printf("IN_ISDIR ");
			 if (i->mask & IN_MODIFY)        printf("IN_MODIFY ");
			 if (i->mask & IN_MOVE_SELF)     printf("IN_MOVE_SELF ");
			 if (i->mask & IN_MOVED_FROM)    printf("IN_MOVED_FROM ");
			 if (i->mask & IN_MOVED_TO)      printf("IN_MOVED_TO ");
			 if (i->mask & IN_OPEN)          printf("IN_OPEN ");
			 if (i->mask & IN_Q_OVERFLOW)    printf("IN_Q_OVERFLOW ");
			 if (i->mask & IN_UNMOUNT)       printf("IN_UNMOUNT ");
			 printf("\n");

			 if (i->len > 0)
					 printf("        name = %s\n", i->name);
	 }
}

InotifyThread::~InotifyThread()
{
	removeWatch();

	if(_inotify_fd >= 0)
		(void) ::close(_inotify_fd);
}

void InotifyThread::init()
{
	_inotify_fd = inotify_init();
	if(_inotify_fd < 0)
		emit inotifyError(strerror(errno));
}

void InotifyThread::run()
{
	while(_continue.load())
	{
		std::unique_lock<std::mutex> lock(_wait_mutex);
		_wait_condition.wait(lock, [&]() { return _watch_file_descriptor >= 0 || !_continue.load(); });

		if(_watch_file_descriptor < 0)
			continue;

		watch();
	}
}

void InotifyThread::removeWatch()
{
	if(_watch_file_descriptor < 0)
		return;

	(void) inotify_rm_watch(_inotify_fd, _watch_file_descriptor);
}

void InotifyThread::startWatching(const QString& filepath)
{
	removeWatch();

	QFileInfo file_info(filepath);
	QString target_directory = file_info.dir().absolutePath();
	_watch_file_descriptor = inotify_add_watch(_inotify_fd, target_directory.toStdString().data(), IN_MODIFY /*| IN_DELETE | IN_CREATE*/ /*IN_ALL_EVENTS*/);
	if(_watch_file_descriptor < 0)
	{
		emit inotifyError(QString("Unable to read file events: ") + strerror(errno));
		return;
	}

	// \todo Kate changes the file event name I don't know why ... Sublime does that correctly
//	_target_event_name = "." + file_info.fileName() + ".";
	_target_event_name = file_info.fileName();

	_wait_condition.notify_one();
}

void InotifyThread::watch()
{ std::cout << "watch" << std::endl;
	constexpr auto event_size = sizeof (struct inotify_event);
	constexpr auto buffer_length = 1024 * ( event_size + 16 );
	char buffer[buffer_length];
	int i {0};

	int length = read(_inotify_fd, buffer, buffer_length);
	if(length <= 0)
		emit inotifyError(QString("Can't read events:  ") + strerror(errno));

	while(i < length)
	{
		struct inotify_event* event = (struct inotify_event*) &buffer[i];
displayInotifyEvent(event);

		if(event->len && QString(event->name).startsWith(_target_event_name))
		{
			if(event->mask & IN_DELETE)
				emit fileDeleted();
			else if(event->mask & IN_CREATE || event->mask & IN_MODIFY)
				emit contentUpdated();
			}

		i += event_size + event->len;
	}

}

void InotifyThread::stop()
{ std::cout << "exit notify thread" << std::endl;
	_continue.store(false);
	removeWatch();
	_wait_condition.notify_one();
}
