#ifndef INOTIFY_THREAD_HPP
#define INOTIFY_THREAD_HPP

#include <atomic>

#include <QThread>

class InotifyThread final : public QThread
{
	Q_OBJECT

	public:

		InotifyThread() = default;
		~InotifyThread();

		void init();
		void startWatching(const QString& filepath);

		void run() override;
		void stop();

	private:

		void watch();
		void removeWatch();

	signals:

		void inotifyError(QString);
		void fileDeleted();
		void contentUpdated();

	private:

		QString _target_event_name;

		int _inotify_fd {-1};
		int _watch_file_descriptor {-1};

		std::condition_variable _wait_condition;
		std::mutex _wait_mutex;

		std::atomic<bool> _continue;
};

#endif // INOTIFY_THREAD_HPP
