#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QFileDialog>
#include <QJsonObject>
#include <QJsonDocument>
#include <QCryptographicHash>
#include <QDateTime>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_choose_clicked();
    void on_choose2_clicked();
    void on_start_clicked();
    void backup(QString dir1, QString dir2);
    void check_json(QString dir, QString &path);
    void traverseFolder1(QString dir, QString dir_, const QString dir2, QJsonObject json);
    void traverseFolder2(QString dir, QString dir_, QJsonObject &json);
    void json_write(QString dir, QJsonObject json);
    void mv(QString file1, QString file2);
    bool check_dir(QString dir);

    QString get_dir();
    QString cal_hash(QString dir);

    QJsonObject json_read(QString dir);

    void on_log_textChanged();

    void on_download_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
