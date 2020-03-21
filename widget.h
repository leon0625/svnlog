#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "QList"
#include "QTableWidgetItem"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class LogPath{
public:
    QString action;
    QString path;
};

class LogEntry{
public:
    QString logid;
    QString auth;
    QString date;
    QString comment;
    QList <LogPath> paths;
};

class Svnlog{
public:
    QList <LogEntry> logs;

    void parse_xml_log(const char *filename);
    void print_svnlog();
    LogEntry *get_logentry_by_id(QString &id);
    QString utcDateFormatLocalTime(QString &sdate);
};

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
    Svnlog svnlog;

    char *logpath;
    char *rootpath;
    void init_logtable(QString path);

private slots:
    void on_openButton_clicked();

    void on_logTable_currentItemChanged(QTableWidgetItem *current, QTableWidgetItem *previous);

    void on_filesTable_itemDoubleClicked(QTableWidgetItem *item);


private:
    Ui::Widget *ui;
};
#endif // WIDGET_H
