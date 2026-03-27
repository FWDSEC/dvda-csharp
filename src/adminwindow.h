#ifndef ADMINWINDOW_H
#define ADMINWINDOW_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>

class AdminWindow : public QWidget
{
    Q_OBJECT

public:
    explicit AdminWindow(QWidget* parent = nullptr);

private slots:
    void onFtpUpload();

private:
    QPushButton* m_ftpBtn;
    QLabel* m_statusLabel;

    // Visible with `strings` command on the binary
    static void uploadToFtp(const QString& ftpServer, const QString& username,
                            const QString& password, const QString& filename);
};

#endif // ADMINWINDOW_H
