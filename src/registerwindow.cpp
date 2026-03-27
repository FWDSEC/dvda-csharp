#include "registerwindow.h"
#include "dbaccess.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QMessageBox>
#include <iostream>

RegisterWindow::RegisterWindow(QWidget* parent)
    : QWidget(parent, Qt::Window)
{
    setWindowTitle("DVDA - Register New User");
    setMinimumSize(400, 300);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QLabel* titleLabel = new QLabel("Register New Account");
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont;
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    mainLayout->addWidget(titleLabel);

    mainLayout->addSpacing(10);

    QGroupBox* formGroup = new QGroupBox("User Details");
    QVBoxLayout* formLayout = new QVBoxLayout(formGroup);

    formLayout->addWidget(new QLabel("Username:"));
    m_usernameEdit = new QLineEdit();
    m_usernameEdit->setPlaceholderText("Choose a username");
    formLayout->addWidget(m_usernameEdit);

    formLayout->addWidget(new QLabel("Password:"));
    m_passwordEdit = new QLineEdit();
    m_passwordEdit->setPlaceholderText("Choose a password");
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    formLayout->addWidget(m_passwordEdit);

    formLayout->addWidget(new QLabel("Email:"));
    m_emailEdit = new QLineEdit();
    m_emailEdit->setPlaceholderText("Enter your email");
    formLayout->addWidget(m_emailEdit);

    mainLayout->addWidget(formGroup);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    m_registerBtn = new QPushButton("Register");
    m_cancelBtn = new QPushButton("Cancel");
    btnLayout->addWidget(m_registerBtn);
    btnLayout->addWidget(m_cancelBtn);
    mainLayout->addLayout(btnLayout);

    mainLayout->addStretch();

    connect(m_registerBtn, &QPushButton::clicked, this, &RegisterWindow::onRegisterClicked);
    connect(m_cancelBtn, &QPushButton::clicked, this, &RegisterWindow::onCancelClicked);
}

// ============================================================
// ============================================================
void RegisterWindow::onRegisterClicked()
{
    QString username = m_usernameEdit->text().trimmed();
    QString password = m_passwordEdit->text().trimmed();
    QString email = m_emailEdit->text().trimmed();

    if (username.isEmpty() || password.isEmpty() || email.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter all the fields!");
        return;
    }

    DBAccess db;
    db.openConnection();

    if (db.registerUser(username, password, email)) {
        QMessageBox::information(this, "Success", "Registration successful! You can now login.");
        this->close();
    } else {
        QMessageBox::warning(this, "Error", "Registration failed!");
    }

    db.closeConnection();
}

void RegisterWindow::onCancelClicked()
{
    this->close();
}
