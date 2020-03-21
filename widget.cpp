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
}

Widget::~Widget()
{
    delete ui;
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
            entry.date = xml.readElementText();
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

void Widget::init_logtable(QString logpath)
{
    qDebug() << logpath;

    svnlog.parse_xml_log(logpath.toStdString().c_str());
    int linenum = 0;

    QList<LogEntry>::iterator i;
    QList<LogPath>::iterator j;
    QTableWidgetItem *item;
    for(i=svnlog.logs.begin(); i != svnlog.logs.end(); ++i, linenum++)
    {
        /* 插入行 */
        ui->logTable->insertRow(linenum);

        /* id */
        item = new QTableWidgetItem(i->logid);
        ui->logTable->setItem(linenum, 0, item);

        /* auth */
        item = new QTableWidgetItem(i->auth);
        ui->logTable->setItem(linenum, 1, item);

        /* date */
        item = new QTableWidgetItem(i->date);
        ui->logTable->setItem(linenum, 2, item);

        /* 注释 */
        item = new QTableWidgetItem(i->comment);
        ui->logTable->setItem(linenum, 3, item);

    }
    /* 初始化选中第一行 */
    ui->logTable->selectRow(0);
}


void Widget::on_openButton_clicked()
{
//    QString dir = QFileDialog::getExistingDirectory();
    QString filepath = QFileDialog::getOpenFileName();
    qDebug() << filepath;
    Widget::init_logtable(filepath);
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
        qDebug() << "动作：" << j->action << "  路径：" << j->path;

        ui->filesTable->insertRow(row);

        item = new QTableWidgetItem(j->action);
        ui->filesTable->setItem(row, 0, item);

        item = new QTableWidgetItem(j->path);
        ui->filesTable->setItem(row, 1, item);
    }
}

void Widget::on_filesTable_itemDoubleClicked(QTableWidgetItem *current)
{
    int log_select_row = ui->logTable->selectedItems()[0]->row();
    QString path = ui->filesTable->item(current->row(), 1)->text();
    QString svn = ui->logTable->item(log_select_row, 0)->text();

    qDebug() << path << "|" << rootpath + path << " " << svn;

    QProcess process(this);
    QString cmd = "svn diff -c " + svn + " " + rootpath + path;
    qDebug() << cmd;
    process.startDetached(cmd);
}
