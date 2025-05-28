#ifndef ORDERHISTORYDIALOG_H
#define ORDERHISTORYDIALOG_H

#include <QDialog>
#include "mainwindow.h"  // For User, Order, and formatPrice declaration

// Forward declarations for Qt classes used as pointers or references
class QTableWidget;
// User is fully defined by including mainwindow.h

class OrderHistoryDialog : public QDialog {
    Q_OBJECT // <<< THIS MACRO IS ESSENTIAL

public:
    // Constructor takes the customer whose order history is to be displayed.
    explicit OrderHistoryDialog(const User* customer, QWidget *parent = nullptr);

private:
    // UI Elements
    QTableWidget *m_ordersTableWidget; // Table to display the list of orders

    // Data
    const User* m_customer; // Pointer to the customer (const as we are only viewing)

    // Helper method to populate the order history table.
    void populateOrderHistory();
};

#endif // ORDERHISTORYDIALOG_H
