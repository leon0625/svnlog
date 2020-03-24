#include "widget.h"
#include "ui_widget.h"
#include "QXmlStreamReader"
#include "QFile"
#include "QDebug"
#include "QMessageBox"
#include "QList"
#include <QHeaderView>  //表头视图类
#include <QFileDialog>
#include <QProcess>
#include <QTimeZone>
#include <QThread>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    //设置表格控件特性
    //每次选中整行
    ui->logTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->filesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    //为方便删除按钮操作，把选中模式设为单选，即每次只选中一行，而不能选中多行
    ui->logTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->filesTable->setSelectionMode(QAbstractItemView::SingleSelection);
    //设置末尾一列自动拉伸
    ui->logTable->horizontalHeader()->setStretchLastSection(true);
    ui->filesTable->horizontalHeader()->setStretchLastSection(true);

    //设置表格为不可编辑状态
    ui->logTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->filesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    //设置表格不显示前面的行号
    ui->logTable->verticalHeader()->setHidden(true);
    ui->filesTable->verticalHeader()->setHidden(true);

    //关联信号槽
    QObject::connect(&svnlog, SIGNAL(svnMsgChange(QString &)), this, SLOT(updateSvnMsgOnUI(QString &)));
}

Widget::~Widget()
{
    delete ui;
}

/* 2020-12-02T12:23:22.123132 ==> 2020-12-02 20:23:22  */
QString utcDateFormatLocalTime(QString &sdate)
{
    QString sdate_no_ms = sdate.split(".")[0];
    QDateTime date = QDateTime::fromString(sdate_no_ms, "yyyy-MM-dd'T'hh:mm:ss");
    int offset = date.timeZone().offsetFromUtc(date);
    date = date.addSecs(offset);
    //qDebug() << date.toString("yyyy-MM-dd hh:mm:ss");
    return date.toString("yyyy-MM-dd hh:mm:ss");
}

void Svnlog::parse_xml_log(const char *filename)
{
    QFile file(filename);

    if(!file.open(QFile::ReadOnly|QFile::Text))
    {
        QMessageBox::information(NULL, QString("打开文件失败"), file.errorString());
        return ;
    }
    LogEntry entry;

    //构建QXmlStreamReader对象
    QXmlStreamReader xml(&file);
    while (!xml.atEnd())
    {
        xml.readNext();
        if(xml.isStartElement() && xml.name() == "logentry")
        {
            //解析一条记录
            entry.logid = xml.attributes().value("revision").toString();
        }
        else if(xml.name() == "author")
        {
            entry.auth = xml.readElementText();
        }
        else if(xml.name() == "date")
        {
            QString utcDate = xml.readElementText();
            entry.date = utcDateFormatLocalTime(utcDate);
        }
        else if(xml.name() == "paths")
        {

        }
        else if(xml.name() == "path")
        {
            LogPath path;
            path.action = xml.attributes().value("action").toString();
            path.path = xml.readElementText();
            entry.paths.append(path);
        }
        else if(xml.name() == "msg")
            entry.comment = xml.readElementText();

        if(xml.isEndElement() && xml.name() == "logentry")
        {
            logs.append(entry);
            entry.paths.clear();
        }
    }

    if (xml.hasError())
    {
        qDebug()<<xml.errorString();
          // do error handling
    }
    file.close();
}

bool Svnlog::currentWorkDirHaveSvnInfo()
{
    QProcess process;

    qDebug() << QDir::currentPath();
    process.start("svn ");
}

bool Svnlog::setSvnUrl(const QString &input_url)
{
    if(input_url.length())
    {
        svnUrl = input_url;
        return true;
    }
    else
    {
        QProcess process;
        QString cmd;
        qDebug() << QDir::currentPath();

        updateSvnMsg("获取当前路径的svn信息中...");

        cmd = "svn info --show-item url";
        qDebug() << cmd;
        process.start(cmd);
        process.waitForFinished();
        QString output = process.readAllStandardOutput().data();

        if(output.length() > 0)
        {
            qDebug() << "repos-root-url: " << output;
            svnUrl = output.trimmed();
            updateSvnMsg(svnUrl);
            return true;
        }
        else
        {
            msg = process.readAllStandardError().data();
            updateSvnMsg(msg);
            qDebug() << "svn info --show-item repos-root-url [error]: " << msg;
            return false;
        }
    }
}

bool Svnlog::getSvnRootPath()
{
    QProcess process;
    QString cmd;
    qDebug() << QDir::currentPath();

    updateSvnMsg("获取svn root url...");
    cmd = "svn info --show-item repos-root-url " + svnUrl;
    qDebug() << cmd;
    process.start(cmd);
    process.waitForFinished();
    QString output = process.readAllStandardOutput().data();

    if(output.length() > 0)
    {
        qDebug() << "repos-root-url: " << output;
        rootpath = output.trimmed();
        updateSvnMsg(rootpath);
        return true;
    }
    else
    {
        msg = process.readAllStandardError().data();
        updateSvnMsg(msg);
        qDebug() << "svn info --show-item repos-root-url [error]: " << msg;
    }
    return false;
}

bool Svnlog::makeSvnLogFile()
{
    QProcess process;
    QString cmd;
    char filepath[] = "/tmp/temp.svnlog.txt";

    updateSvnMsg("获取svn日志中...(日志较多时可能卡顿)");

    if(!svnUrl.length())
    {
        qDebug() << "svnUrl len is 0";
        return false;
    }

    cmd = QString::asprintf("svn log -v --xml %s %s", showAllLog ? "" : "-l 100", svnUrl.toStdString().c_str());
    qDebug() << cmd;
    process.setStandardOutputFile(filepath);
    process.start(cmd);
    process.waitForFinished();
    if(process.exitCode() == 0)
    {
        logFilePath = filepath;
        return true;
    }
    else
    {
        qDebug() << process.exitCode();
        msg = process.readAllStandardError().data();
        updateSvnMsg(msg);
        qDebug() << msg;
        return false;
    }
}

void Svnlog::destory_log()
{
    for(auto entry:logs)
    {
        entry.paths.clear();
    }
    logs.clear();
}

bool Svnlog::init_log(const QString &url)
{
    destory_log();

    if(!setSvnUrl(url))
        return false;

    if(!getSvnRootPath())
        return false;

    qDebug() << rootpath;

    if(!makeSvnLogFile())
        return false;

    parse_xml_log(logFilePath.toStdString().c_str());

    return true;
}

void Svnlog::print_svnlog(void)
{
    //打印出来
    QList<LogEntry>::iterator i;
    QList<LogPath>::iterator j;

    for(i=logs.begin(); i != logs.end(); ++i)
    {
        qDebug() << "ID: " <<  i->logid;
        qDebug() << "作者: " << i->auth;
        qDebug() << "日期："  << i->date;
        qDebug() << "注释：" << i->comment;
        for(j=i->paths.begin(); j != i->paths.end(); ++j)
        {
            qDebug() << "动作：" << j->action << "  路径：" << j->path;
        }
        qDebug() << "===================";
    }
}

void Widget::clear_log_table()
{
    /* 清除表格中数据 */
    ui->logTable->setRowCount(0);
    ui->filesTable->clearContents();
    // ui->logTable->clear();   //这个函数会清除表头

    ui->labelLogNum->setText("当前显示了" + QString::number(0) + "条日志");
}

void Widget::update_logtable(QList <LogEntry> &logs)
{
    int linenum = 0;

    QTableWidgetItem *item;
    for(auto i=logs.begin(); i != logs.end(); ++i, linenum++)
    {
        /* 插入行 */
        ui->logTable->insertRow(linenum);

        /* id */
        item = new QTableWidgetItem(i->logid);
        ui->logTable->setItem(linenum, 0, item);
        ui->logTable->item(linenum, 0)->setTextAlignment(Qt::AlignCenter);  //居中

        /* auth */
        item = new QTableWidgetItem(i->auth);
        ui->logTable->setItem(linenum, 1, item);
        ui->logTable->item(linenum, 1)->setTextAlignment(Qt::AlignCenter);  //居中

        /* date */
        item = new QTableWidgetItem(i->date);
        ui->logTable->setItem(linenum, 2, item);

        /* 注释 */
        item = new QTableWidgetItem(i->comment);
        ui->logTable->setItem(linenum, 3, item);

    }

    ui->labelLogNum->setText("当前显示了" + QString::number(linenum) + "条日志");
    /* 初始化选中第一条日志 */
    ui->logTable->selectRow(0);
}

void Widget::show_notice_msg(const QString &msg)
{
    ui->commentBrowser->append(msg);
    ui->commentBrowser->repaint();      //可以使其立即更新
}


void Widget::on_openButton_clicked()
{
    //QString dir = QFileDialog::getExistingDirectory();
    //QString filepath = QFileDialog::getOpenFileName();
    //qDebug() << dir;
    ui->commentBrowser->clear();
    if(svnlog.init_log(ui->LineEditSvnPath->text()))
    {
        clear_log_table();
        update_logtable(svnlog.logs);

        ui->LineEditSvnPath->setText(svnlog.svnUrl);
    }
}

LogEntry *Svnlog::get_logentry_by_id(QString &id)
{
    //打印出来
    QList<LogEntry>::iterator i;
    QList<LogPath>::iterator j;
    LogEntry *finded = NULL;

    for(i=logs.begin(); i != logs.end(); ++i)
    {
        if(id == i->logid)
        {
            finded = &*i;
            qDebug() << i->logid << i->comment;
            return finded;
        }
    }

    return finded;
}

void Widget::on_logTable_currentItemChanged(QTableWidgetItem *current, QTableWidgetItem *previous)
{
    /* 当日志条目表格清空时，这里会传入空指针 */
    if(!current)
    {
        return;
    }

    /* 注释框内显示注释 */
    ui->commentBrowser->clear();
    QString id = ui->logTable->item(current->row(), 0)->text();
    LogEntry *finded = svnlog.get_logentry_by_id(id);
    ui->commentBrowser->append(finded->comment);

    /* 显示修改的文件 */
    /* 先删除所有行 */
    ui->filesTable->setRowCount(0);
    ui->filesTable->clearContents();
    /* 添加新的文件 */
    QList<LogPath>::iterator j;
    QTableWidgetItem *item;
    int row = 0;
    for(j=finded->paths.begin(); j != finded->paths.end(); ++j, row++)
    {
        // qDebug() << "动作：" << j->action << "  路径：" << j->path;

        ui->filesTable->insertRow(row);

        item = new QTableWidgetItem(j->action);
        ui->filesTable->setItem(row, 0, item);
        ui->filesTable->item(row, 0)->setTextAlignment(Qt::AlignCenter);  //居中

        item = new QTableWidgetItem(j->path);
        ui->filesTable->setItem(row, 1, item);
    }
}

void Widget::on_filesTable_itemDoubleClicked(QTableWidgetItem *current)
{
    int log_select_row = ui->logTable->selectedItems()[0]->row();
    QString path = ui->filesTable->item(current->row(), 1)->text();
    QString svn = ui->logTable->item(log_select_row, 0)->text();

    qDebug() << path << "|" << svnlog.rootpath + path << " " << svn;

    QProcess process(this);
    QString cmd = "svn diff -c " + svn + " " + svnlog.rootpath + path;
    qDebug() << cmd;
    process.startDetached(cmd);
}

void Widget::on_showAllCheckBox_stateChanged(int arg1)
{
    qDebug() << arg1;
    svnlog.showAllLog = arg1;
}

bool keyword_in_log(const LogEntry &log, const QString &keyword)
{
    //查找注释
    if(-1 != log.comment.indexOf(keyword))
        return true;

    //查找作者
    if(-1 != log.auth.indexOf(keyword))
        return true;

    //查找文件名
    for(auto path : log.paths)
    {
        if(-1 != path.path.indexOf(keyword))
            return true;
    }

    return false;
}

/* 查找日志，空格分隔关键字 */
void Widget::search_log_and_show(const QString &keyword)
{
    QStringList key_words =  keyword.split(" ");
    QList<LogEntry> search_logs;
    bool found = true;

    for(auto log : svnlog.logs)
    {
        for(auto key:key_words)
        {
            if(false == keyword_in_log(log, key))
            {
                found = false;
                break;
            }
        }
        if(found)
        {
            search_logs.append(log);
        }
        found = true;
    }

    clear_log_table();
    update_logtable(search_logs);
}

void Widget::on_findButton_clicked()
{
    QString input = ui->search_key_word->text();
    search_log_and_show(input);
}

/* 输入框回车事件 */
void Widget::on_search_key_word_returnPressed()
{
    QString input = ui->search_key_word->text();
    search_log_and_show(input);
}
