#ifndef REGISTERWINDOW_H
#define REGISTERWINDOW_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>

class RegisterWindow : public QWidget
{
    Q_OBJECT

public:
    explicit RegisterWindow(QWidget* parent = nullptr);

private slots:
    void onRegisterClicked();
    void onCancelClicked();

private:
    QLineEdit* m_usernameEdit;
    QLineEdit* m_passwordEdit;
    QLineEdit* m_emailEdit;
    QPushButton* m_registerBtn;
    QPushButton* m_cancelBtn;
};

#endif // REGISTERWINDOW_H
