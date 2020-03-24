#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "QList"
#include "QTableWidgetItem"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

QString utcDateFormatLocalTime(QString &sdate);

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

/* 需要继承信号和槽机制，所以这里继承QObject类 */
class Svnlog: public QObject {
    Q_OBJECT        //凡是QObject类（不管是直接子类还是间接子类），都应该在第一行代码写上Q_OBJECT。不管是不是使用信号槽，都应该添加这个宏。

private:
    bool currentWorkDirHaveSvnInfo();
    bool getSvnRootPath();
    bool setSvnUrl(const QString &input_url);
    bool makeSvnLogFile();
public:
    QList <LogEntry> logs;
    QString rootpath;
    QString svnUrl;
    QString logFilePath;
    QString msg;
    bool showAllLog = false;

    void parse_xml_log(const char *filename);
    void print_svnlog();
    bool init_log(const QString &url);
    void destory_log();
    LogEntry *get_logentry_by_id(QString &id);
    void updateSvnMsg(QString msg) {
        msg = msg;
        emit svnMsgChange(msg); // emit关键字发送这个信号
    }
signals:
    //定义自己的信号
    void svnMsgChange(QString &msg);
};

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
    Svnlog svnlog;

    void show_notice_msg(const QString &msg);
    void clear_log_table();
    void update_logtable(QList <LogEntry> &logs);
    void search_log_and_show(const QString &keyword);

private slots:
    void on_openButton_clicked();

    void on_logTable_currentItemChanged(QTableWidgetItem *current, QTableWidgetItem *previous);

    void on_filesTable_itemDoubleClicked(QTableWidgetItem *item);

    void on_showAllCheckBox_stateChanged(int arg1);

    void on_findButton_clicked();

    void on_search_key_word_returnPressed();

private:
    Ui::Widget *ui;

public slots:
    void updateSvnMsgOnUI(QString &msg)
    {
        show_notice_msg(msg);
    }
};
#endif // WIDGET_H
