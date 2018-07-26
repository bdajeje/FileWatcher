#include "main_window.hpp"

#include <QApplication>
#include <QAction>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QMenuBar>
#include <QVBoxLayout>
#include <QScrollArea>

// \tmp to delete
#include <iostream>

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent)
{
	QWidget *widget = new QWidget;
	setCentralWidget(widget);

	_title = new QLabel;
	_title->setAlignment(Qt::AlignHCenter);

	auto scroll_area = new QScrollArea;
	_file_content = new QLabel(scroll_area);
	scroll_area->setWidget(_file_content);
	scroll_area->setWidgetResizable(true);
	scroll_area->setMinimumSize(300, 300);
	_file_content->setMinimumSize(100, 100);
	_file_content->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	_file_content->setWordWrap(true);
	_file_content->setAutoFillBackground( true );

	QVBoxLayout *layout = new QVBoxLayout;
	layout->setMargin(5);
	layout->addWidget(_title);
	layout->addWidget(scroll_area);
	widget->setLayout(layout);

	auto open_file_action = new QAction(tr("&Open"), this);
	open_file_action->setShortcuts(QKeySequence::Open);
	open_file_action->setStatusTip(tr("Open a file to watch"));
	connect(open_file_action, &QAction::triggered, this, &MainWindow::openFile);

	auto quit_action = new QAction(tr("&Quit"), this);
	quit_action->setShortcuts(QKeySequence::Close);
	quit_action->setStatusTip(tr("Exit application"));
	connect(quit_action, &QAction::triggered, this, &MainWindow::close);

	auto file_menu = menuBar()->addMenu(tr("&File"));
	file_menu->addAction(open_file_action);
	file_menu->addAction(quit_action);

	resetContent();	

	_file_watcher_thread = new InotifyThread;
	connect(_file_watcher_thread, SIGNAL(inotifyError(QString)), this, SLOT(inotifyError(QString)));
	connect(_file_watcher_thread, SIGNAL(contentUpdated()), this, SLOT(reloadContent()));
	connect(_file_watcher_thread, SIGNAL(fileDeleted()), this, SLOT(resetContent()));
	_file_watcher_thread->init();
	_file_watcher_thread->start();
}

MainWindow::~MainWindow()
{
	_file_watcher_thread->stop();
	_file_watcher_thread->deleteLater();
}

void MainWindow::openFile()
{
	auto filepath = QFileDialog::getOpenFileName(this, tr("Open file"));
	if(filepath.isEmpty() || !QFile::exists(filepath))
		return;

	_filepath = filepath;
	QFile _file(_filepath);
	if(!_file.open(QIODevice::OpenModeFlag::ReadOnly))
	{
		QMessageBox::critical(this, "Error", "Can't open selected file.");
		return;
	}

	_title->setText(_filepath);
	reloadContent();
	_file_watcher_thread->startWatching(_filepath);
}

void MainWindow::inotifyError(QString message)
{
	QMessageBox::critical(this, "Error", message);
}

void MainWindow::resetContent()
{
	_title->setText(tr("No file selected"));
	_file_content->setText(tr("Content of your file will be displayed here."));
	_file_content->setAlignment(Qt::AlignCenter);
}

void MainWindow::reloadContent()
{ std::cout << "Reloading content" << std::endl;
	QFile file(_filepath);
	if(!file.open(QIODevice::OpenModeFlag::ReadOnly))
	{
		QMessageBox::critical(this, "Error", QString("Can't read target file."));
		return;
	}

	_file_content->setAlignment(Qt::AlignTop);
	_file_content->setText(file.readAll());
	_file_content->resize(sizeHint());
}
