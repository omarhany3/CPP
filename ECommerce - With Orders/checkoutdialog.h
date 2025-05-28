#ifndef CHECKOUTDIALOG_H
#define CHECKOUTDIALOG_H

#include <QDialog>
// mainwindow.h should provide Order, Customer, CartItem, Product, formatPrice declaration
#include "mainwindow.h"

// Forward declarations for Qt classes used as pointers or references
class QTextEdit;
class QLabel;
class QLineEdit;
class QComboBox;
class QDialogButtonBox;
// No need to forward declare Customer if mainwindow.h is included and provides it.

class CheckoutDialog : public QDialog {
    Q_OBJECT

public:
    // Constructor takes the customer placing the order.
    // Assumes mainwindow.h (included above) defines Customer.
    explicit CheckoutDialog(Customer* customer, QWidget *parent = nullptr);
    // Order is now added to G_allOrders directly, so no getter needed here.

private slots:
    // Slot for when the "Place Order" button (OK button in QDialogButtonBox) is clicked.
    void onPlaceOrderClicked();

private:
    // UI Elements
    QTextEdit *m_orderSummaryTextEdit;  // Displays items in the cart
    QLabel *m_totalPriceLabel;          // Shows the total price
    QLabel *m_paymentMethodLabel;       // Fixed to "Cash On Delivery"
    QLabel *m_deliveryDateLabel;        // Shows calculated delivery date
    QLineEdit *m_addressLineEdit;       // Input for delivery address
    QLineEdit *m_contactLineEdit;       // Input for contact number
    QComboBox *m_deliveryTimeComboBox;  // Dropdown for preferred delivery time
    QDialogButtonBox *m_buttonBox;      // Holds "Place Order" and "Cancel" buttons

    // Data
    Customer* m_customer; // Pointer to the customer placing the order
    float m_cartTotal;    // Stores the calculated cart total

    // Helper method to populate the order summary and total price.
    void populateOrderSummary();
};

#endif // CHECKOUTDIALOG_H
