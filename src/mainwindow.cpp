#include "mainwindow.h"
#include "sessionmanager.h"
#include "loginwindow.h"
#include "dbaccess.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>
#include <QHeaderView>
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QInputDialog>
#include <QDateTime>
#include <iostream>

MainWindow::MainWindow(QWidget* parent)
    : QWidget(parent)
{
    // Check if user is logged in
    if (!SessionManager::isLoggedIn()) {
        this->hide();
        LoginWindow* login = new LoginWindow();
        login->show();
        this->close();
        return;
    }

    setWindowTitle("DVDA - Expense Tracker");
    setMinimumSize(700, 500);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QHBoxLayout* headerLayout = new QHBoxLayout();
    QLabel* titleLabel = new QLabel("DVDA - Expense Tracker");
    QFont titleFont;
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    headerLayout->addWidget(titleLabel);

    headerLayout->addStretch();
    m_userLabel = new QLabel("Logged in as: " + SessionManager::getUsername());
    headerLayout->addWidget(m_userLabel);
    mainLayout->addLayout(headerLayout);

    mainLayout->addSpacing(10);

    m_expenseTable = new QTableWidget(0, 4);
    m_expenseTable->setHorizontalHeaderLabels({"Item", "Price", "Date", "Time"});
    m_expenseTable->horizontalHeader()->setStretchLastSection(true);
    m_expenseTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_expenseTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    mainLayout->addWidget(m_expenseTable);

    QHBoxLayout* btnLayout = new QHBoxLayout();

    m_viewBtn = new QPushButton("View Expenses");
    m_addBtn = new QPushButton("Add Expense");
    m_clearBtn = new QPushButton("Clear All");
    m_exportBtn = new QPushButton("Export CSV");
    m_profileBtn = new QPushButton("Profile");
    m_logoutBtn = new QPushButton("Logout");

    btnLayout->addWidget(m_viewBtn);
    btnLayout->addWidget(m_addBtn);
    btnLayout->addWidget(m_clearBtn);
    btnLayout->addWidget(m_exportBtn);
    btnLayout->addWidget(m_profileBtn);
    btnLayout->addWidget(m_logoutBtn);

    mainLayout->addLayout(btnLayout);

    // Connect signals
    connect(m_viewBtn, &QPushButton::clicked, this, &MainWindow::onViewExpenses);
    connect(m_addBtn, &QPushButton::clicked, this, &MainWindow::onAddExpense);
    connect(m_clearBtn, &QPushButton::clicked, this, &MainWindow::onClearExpenses);
    connect(m_exportBtn, &QPushButton::clicked, this, &MainWindow::onExportCsv);
    connect(m_profileBtn, &QPushButton::clicked, this, &MainWindow::onProfile);
    connect(m_logoutBtn, &QPushButton::clicked, this, &MainWindow::onLogout);
}

// ============================================================
// The email value is concatenated directly into the SQL query.
// By entering a UNION SELECT payload, an attacker can extract
// data from any table in the database.
// ============================================================
void MainWindow::onViewExpenses()
{
    // The default value comes from the session file, but the user
    QString sessionEmail = SessionManager::getEmail();
    bool ok;
    QString email = QInputDialog::getText(this, "View Expenses",
        "Email to filter by:", QLineEdit::Normal, sessionEmail, &ok);
    if (!ok) return;

    DBAccess db;
    db.openConnection();

    QVector<QVariantMap> results = db.viewExpenses(email);

    m_expenseTable->setRowCount(0);
    for (const auto& row : results) {
        int r = m_expenseTable->rowCount();
        m_expenseTable->insertRow(r);
        m_expenseTable->setItem(r, 0, new QTableWidgetItem(row["item"].toString()));
        m_expenseTable->setItem(r, 1, new QTableWidgetItem(row["price"].toString()));
        m_expenseTable->setItem(r, 2, new QTableWidgetItem(row["date"].toString()));
        m_expenseTable->setItem(r, 3, new QTableWidgetItem(row["time"].toString()));
    }

    db.closeConnection();
}

// ============================================================
// ============================================================
void MainWindow::onAddExpense()
{
    // Simple inline dialog for adding an expense
    bool ok;
    QString item = QInputDialog::getText(this, "Add Expense", "Item:", QLineEdit::Normal, "", &ok);
    if (!ok || item.isEmpty()) return;

    QString price = QInputDialog::getText(this, "Add Expense", "Price:", QLineEdit::Normal, "", &ok);
    if (!ok || price.isEmpty()) return;

    QString date = QInputDialog::getText(this, "Add Expense", "Date (YYYY-MM-DD):",
                                          QLineEdit::Normal,
                                          QDateTime::currentDateTime().toString("yyyy-MM-dd"), &ok);
    if (!ok || date.isEmpty()) return;

    QString email = SessionManager::getEmail();
    QString time = QDateTime::currentDateTime().toString("HH:mm:ss");

    DBAccess db;
    db.openConnection();

    if (db.addExpenses(date, item, price, email, time)) {
        QMessageBox::information(this, "Success", "Data saved successfully");
    } else {
        QMessageBox::warning(this, "Error", "Failed to save expense");
    }

    db.closeConnection();
}

// ============================================================
// ============================================================
void MainWindow::onClearExpenses()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Caution", "Do you want to clear all your expenses?",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        DBAccess db;
        db.openConnection();

        QString email = SessionManager::getEmail();

        if (db.clearExpenses(email)) {
            QMessageBox::information(this, "Success", "Expenses cleared successfully");
            m_expenseTable->setRowCount(0);
        } else {
            QMessageBox::warning(this, "Error", "Failed to clear expenses");
        }

        db.closeConnection();
    }
}

// ============================================================
// Malicious formulas like =CMD(...) will execute in Excel
// ============================================================
void MainWindow::onExportCsv()
{
    if (m_expenseTable->rowCount() == 0) {
        QMessageBox::warning(this, "Error", "Click View Expenses before doing this");
        return;
    }

    QString downloadPath = QDir::homePath() + "/Downloads/expenses.csv";

    QFile file(downloadPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Could not write CSV file");
        return;
    }

    QTextStream out(&file);

    // Write header
    for (int c = 0; c < m_expenseTable->columnCount(); c++) {
        if (c > 0) out << ",";
        out << m_expenseTable->horizontalHeaderItem(c)->text();
    }
    out << "\n";

    for (int r = 0; r < m_expenseTable->rowCount(); r++) {
        for (int c = 0; c < m_expenseTable->columnCount(); c++) {
            if (c > 0) out << ",";
            out << "\"" << m_expenseTable->item(r, c)->text() << "\"";
        }
        out << "\n";
    }

    file.close();
    QMessageBox::information(this, "Success", "Exported! Check your Downloads folder");
    std::cout << "[DVDA] CSV exported to: " << downloadPath.toStdString() << std::endl;
}

void MainWindow::onProfile()
{
    QString username = SessionManager::getUsername();
    QString email = SessionManager::getEmail();
    QMessageBox::information(this, "Profile",
                              "Username: " + username + "\nEmail: " + email);
}

void MainWindow::onLogout()
{
    SessionManager::clearSession();
    this->hide();
    LoginWindow* login = new LoginWindow();
    login->setAttribute(Qt::WA_DeleteOnClose);
    login->show();
    this->close();
}
