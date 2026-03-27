#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>

class LoginWindow : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWindow(QWidget* parent = nullptr);

private slots:
    void onLoginClicked();
    void onRegisterClicked();
    void onConfigureClicked();

private:
    bool isBeingDebugged();

    QLineEdit* m_usernameEdit;
    QLineEdit* m_passwordEdit;
    QLineEdit* m_serverEdit;
    QPushButton* m_loginBtn;
    QPushButton* m_registerBtn;
    QPushButton* m_configureBtn;
};

#endif // LOGINWINDOW_H
