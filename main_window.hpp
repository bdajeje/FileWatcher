#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP

#include <atomic>

#include <QLabel>
#include <QMainWindow>
#include <QThread>

#include "inotify_thread.hpp"

class MainWindow final : public QMainWindow
{
	Q_OBJECT

	public:

		MainWindow(QWidget* parent = nullptr);
		~MainWindow();		

	protected:

		void deleteFileWatcher();

	protected slots:

		void openFile();
		void resetContent();
		void reloadContent();
		void inotifyError(QString message);

	protected:

		QString _filepath;
		QLabel* _title;
		QLabel* _file_content;
		InotifyThread* _file_watcher_thread;
};

#endif // MAIN_WINDOW_HPP
