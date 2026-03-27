#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void onViewExpenses();
    void onAddExpense();
    void onClearExpenses();
    void onExportCsv();
    void onProfile();
    void onLogout();

private:
    QLabel* m_userLabel;
    QTableWidget* m_expenseTable;
    QPushButton* m_viewBtn;
    QPushButton* m_addBtn;
    QPushButton* m_clearBtn;
    QPushButton* m_exportBtn;
    QPushButton* m_profileBtn;
    QPushButton* m_logoutBtn;
};

#endif // MAINWINDOW_H
