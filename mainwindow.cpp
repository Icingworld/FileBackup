#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_choose_clicked()
{
    ui->dir1->setText(get_dir());
}


void MainWindow::on_choose2_clicked()
{
    ui->dir2->setText(get_dir());
}


void MainWindow::on_start_clicked()
{
    if (!check_dir(ui->dir1->text()))
    {
        ui->dir1->setFocus();
        ui->log->append("路径" + ui->dir1->text() + "不存在！");
        return;
    }
    if (!check_dir(ui->dir2->text()))
    {
        ui->dir2->setFocus();
        ui->log->append("路径" + ui->dir2->text() + "不存在！");
        return;
    }
    ui->log->append("准备备份...");
    backup(ui->dir1->text(), ui->dir2->text());
}

QString MainWindow::get_dir()
{
    QString srcDirPath;
    srcDirPath = QFileDialog::getExistingDirectory(this, "choose src Directory","/");
    if (srcDirPath.isEmpty())
    {
        return "Error";
    }
    else
    {
        return srcDirPath;
    }
}

bool MainWindow::check_dir(QString dir)
{
    QFile file(dir);
    if (file.exists()) {
        return true;
    } else {
        return false;
    }
}

void MainWindow::backup(QString dir1, QString dir2)
{
    // 计算dir2的全部hash
    QString path;
    ui->log->append("源文件夹：" + dir1);
    ui->log->append("目标文件夹：" + dir2);
    ui->log->append("正在计算目标文件夹......");
    check_json(dir2, path);
    QJsonObject json = json_read(path);
    if (dir2.right(1) != "/")
    {
        dir2 += "/";
    }
    //遍历，开始备份
    ui->log->append("正在备份...");
    traverseFolder1(dir1, dir1, dir2, json);
    //备份完成
    ui->log->append("备份完成！");
    QFile::remove(path);
}

void MainWindow::check_json(QString dir, QString &path)
{
    if (dir.right(1) != "/")
    {
        dir += "/";
    }
    path = dir + "hash.json";
    if (!check_dir(path))
    {
        QJsonObject jsonObject;
        traverseFolder2(dir, dir, jsonObject);
        json_write(path, jsonObject);
    }
}

void MainWindow::traverseFolder1(const QString dir, QString dir_, const QString dir2, QJsonObject json)
{
    QDir directory(dir_);
    QFileInfoList list = directory.entryInfoList(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs);
    for (int i = 0; i < list.size(); ++i) {
        QFileInfo fileInfo = list.at(i);
        if (fileInfo.isFile()) {
            QString new_path = fileInfo.filePath().right(fileInfo.filePath().length() - dir.length() - 1);
            QString hash = cal_hash(fileInfo.filePath());
            qDebug() << new_path;
            if (json.contains(new_path))
            {
                //有该文件名
                if (hash == json[new_path].toString())
                {
                    //内容也相同
                    qDebug() << new_path + "已存在";
                    ui->log->append(new_path + "已存在");
                    continue;
                } else {
                    //内容不同
                    //修改名字
                    QFileInfo fileInfo_new(new_path);
                    QString oldName = fileInfo_new.baseName();
                    QDateTime currentTime = QDateTime::currentDateTime();
                    QString strCurrentTime = currentTime.toString("yyyy-MM-dd-hh-mm-ss");
                    QString newName = oldName + "-" + strCurrentTime;
                    QString newPath = newName + "." + fileInfo_new.suffix();
                    mv(fileInfo.filePath(), dir2 + newPath);
                }
            } else {
                //没有该文件名
                //直接备份
                mv(fileInfo.filePath(), dir2 + new_path);
            }
        }
        if (fileInfo.isDir()) {
            traverseFolder1(dir, fileInfo.filePath(), dir2, json);
        }
    }
}

void MainWindow::mv(QString file1, QString file2)
{
    QFileInfo fileInfo(file2);
    QString new_path = fileInfo.filePath().left(fileInfo.filePath().length() - fileInfo.fileName().length() - 1);
    QDir dir(new_path);
    if(!dir.exists())
    {
        qDebug()<<"目录不存在";
        dir.mkdir(new_path);
    }
    bool a = QFile::copy(file1, file2);
    qDebug() << "将" << file1 << "备份到" << file2 << a;
    ui->log->append( "将" + file1 + "备份到" + file2);
}

void MainWindow::traverseFolder2(const QString dir, QString dir_, QJsonObject &json)
{
    QDir directory(dir_);
    QFileInfoList list = directory.entryInfoList(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs);
    for (int i = 0; i < list.size(); ++i) {
        QFileInfo fileInfo = list.at(i);
        if (fileInfo.isFile()) {
            QString new_path = fileInfo.filePath().right(fileInfo.filePath().length() - dir.length());
            json.insert(new_path, cal_hash(fileInfo.filePath()));
        }
        if (fileInfo.isDir()) {
            traverseFolder2(dir, fileInfo.filePath(), json);
        }
    }
}

QString MainWindow::cal_hash(QString dir)
{
    QFile file(dir);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open file";
    }
    QCryptographicHash hash(QCryptographicHash::Sha1);
    if (!hash.addData(&file)) {
        qDebug() << "Failed to add data to hash";
        file.close();
    }
    file.close();
    QByteArray hashValue = hash.result();
    ui->log->append("文件:" + file.fileName() + "   哈希值:" +hashValue.toHex());
    //qDebug() << "Hash value:" << hashValue.toHex();
    return hashValue.toHex();
}

void MainWindow::json_write(QString dir, QJsonObject json)
{
    QJsonDocument jsonDoc(json);
    QFile file(dir);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Failed to write";
        return;
    }
    file.write(jsonDoc.toJson());
    file.close();
}

QJsonObject MainWindow::json_read(QString dir)
{
    QFile file(dir);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open file";
    }
    QByteArray jsonData = file.readAll();
    file.close();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
    QJsonObject jsonObject = jsonDoc.object();
    return jsonObject;
}

void MainWindow::on_log_textChanged()
{
    ui->log->moveCursor(QTextCursor::End);
}


void MainWindow::on_download_clicked()
{
    QString dir = ui->dir2->text();
    if (dir.right(1) != "/")
    {
        dir += "/";
    }
    QString logfile = dir + "log.txt";
    QFile file(logfile);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Failed to write";
        return;
    }
    file.write(ui->log->toPlainText().toUtf8());
    file.close();
    ui->log->append("下载到log.txt！");
}

