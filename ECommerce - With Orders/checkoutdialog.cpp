#include "checkoutdialog.h" // Includes mainwindow.h (for Customer, Order, formatPrice)
#include <QVBoxLayout>
#include <QFormLayout>
#include <QTextEdit>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QPushButton>      // <<< CRUCIAL INCLUDE for using QPushButton methods
#include <QMessageBox>
#include <QDate>
#include <QDateTime>
#include <QDebug>
#include <vector>           // For std::vector (G_allOrders)
#include <string>           // For std::string conversions

// Reference to the global order list (must be defined in main.cpp)
extern std::vector<Order> G_allOrders;
// formatPrice is declared in mainwindow.h, which is included via checkoutdialog.h

CheckoutDialog::CheckoutDialog(Customer* customer, QWidget *parent)
    : QDialog(parent), m_customer(customer), m_cartTotal(0.0f) {

    if (!m_customer) {
        qCritical() << "CheckoutDialog initialized with a null customer! Dialog will be unusable.";
        // Optionally, disable parts of the dialog or close it.
        // For now, proceeding might lead to crashes if m_customer is accessed.
        return;
    }

    setWindowTitle("Complete Your Order");
    setModal(true); // Make dialog modal
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Order Summary Section
    QLabel *summaryLabel = new QLabel("<b>Order Summary:</b>", this);
    m_orderSummaryTextEdit = new QTextEdit(this);
    m_orderSummaryTextEdit->setReadOnly(true);
    m_orderSummaryTextEdit->setMinimumHeight(100); // Provide some space for summary

    // Total Price Display
    m_totalPriceLabel = new QLabel("<b>Total: 0.00 EGP</b>", this); // Default text, updated by populateOrderSummary

    // Delivery and Payment Information Section (using QFormLayout)
    QFormLayout *detailsLayout = new QFormLayout();
    m_addressLineEdit = new QLineEdit(this);
    m_addressLineEdit->setPlaceholderText("Enter your full delivery address");
    m_contactLineEdit = new QLineEdit(this);
    m_contactLineEdit->setPlaceholderText("Enter your contact phone number");

    m_deliveryTimeComboBox = new QComboBox(this);
    m_deliveryTimeComboBox->addItems({ // Available delivery time slots
        "9:00 AM - 12:00 PM", "12:00 PM - 3:00 PM",
        "3:00 PM - 6:00 PM",  "6:00 PM - 9:00 PM"
    });

    QDate deliveryDate = QDate::currentDate().addDays(1); // Delivery is for tomorrow
    // If a fixed date is needed for testing: QDate deliveryDate(2025, 5, 12);
    m_deliveryDateLabel = new QLabel(deliveryDate.toString("dddd, MMMM d, yyyy"), this);
    m_paymentMethodLabel = new QLabel("Cash On Delivery", this); // Payment method is fixed

    detailsLayout->addRow("Delivery Address:", m_addressLineEdit);
    detailsLayout->addRow("Contact Number:", m_contactLineEdit);
    detailsLayout->addRow("Preferred Delivery Time:", m_deliveryTimeComboBox);
    detailsLayout->addRow("Delivery Date:", m_deliveryDateLabel);
    detailsLayout->addRow("Payment Method:", m_paymentMethodLabel);

    // Dialog Buttons (Place Order, Cancel)
    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    QPushButton* okButton = m_buttonBox->button(QDialogButtonBox::Ok); // Get the OK button
    if (okButton) { // Check if the button was successfully retrieved
        okButton->setText("Place Order"); // Customize OK button text
    }
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &CheckoutDialog::onPlaceOrderClicked);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    // Add all sections to the main layout
    mainLayout->addWidget(summaryLabel);
    mainLayout->addWidget(m_orderSummaryTextEdit);
    mainLayout->addWidget(m_totalPriceLabel);
    mainLayout->addLayout(detailsLayout);
    mainLayout->addWidget(m_buttonBox);

    setLayout(mainLayout);      // Apply the layout to the dialog
    populateOrderSummary();     // Fill summary and total price based on cart
    setMinimumWidth(450);       // Set a decent minimum width
    m_addressLineEdit->setFocus(); // Set initial focus to address field
}

// Populates the order summary text edit and total price label.
void CheckoutDialog::populateOrderSummary() {
    if (!m_customer || !m_orderSummaryTextEdit || !m_totalPriceLabel) {
        qWarning() << "populateOrderSummary: Customer or UI elements are null.";
        return;
    }

    m_orderSummaryTextEdit->clear();
    m_cartTotal = 0.0f; // Reset cart total
    // Using HTML for a richer summary display
    QString summaryHtml = "<table width='100%' style='border-collapse: collapse;'>";
    summaryHtml += "<thead><tr>"
                   "<th style='text-align:left; border-bottom:1px solid #ccc; padding: 4px;'>Product</th>"
                   "<th style='text-align:right; border-bottom:1px solid #ccc; padding: 4px;'>Qty</th>"
                   "<th style='text-align:right; border-bottom:1px solid #ccc; padding: 4px;'>Price</th>"
                   "<th style='text-align:right; border-bottom:1px solid #ccc; padding: 4px;'>Subtotal</th>"
                   "</tr></thead><tbody>";

    for (const auto& cartItem : m_customer->customerCart) {
        if (cartItem.product) {
            float itemSubtotal = cartItem.product->getPrice() * cartItem.quantity;
            m_cartTotal += itemSubtotal;
            summaryHtml += QString("<tr><td style='padding: 4px;'>%1</td>"
                                   "<td align='right' style='padding: 4px;'>%2</td>"
                                   "<td align='right' style='padding: 4px;'>%3 EGP</td>"
                                   "<td align='right' style='padding: 4px;'>%4 EGP</td></tr>")
                               .arg(QString::fromStdString(cartItem.product->getName()))
                               .arg(cartItem.quantity)
                               .arg(QString::fromStdString(formatPrice(cartItem.product->getPrice())))
                               .arg(QString::fromStdString(formatPrice(itemSubtotal)));
        }
    }
    summaryHtml += "</tbody></table>";
    m_orderSummaryTextEdit->setHtml(summaryHtml);
    m_totalPriceLabel->setText(QString("<b>Total: %1 EGP</b>").arg(QString::fromStdString(formatPrice(m_cartTotal))));
}

// Slot for when the "Place Order" button is clicked.
void CheckoutDialog::onPlaceOrderClicked() {
    if (!m_customer) {
        QMessageBox::critical(this, "Error", "Customer data is missing. Cannot place order.");
        return;
    }
    if (m_customer->customerCart.empty()) {
        QMessageBox::warning(this, "Empty Cart", "Your cart is empty. Please add items before placing an order.");
        return;
    }

    // Retrieve delivery details from input fields
    QString address = m_addressLineEdit->text().trimmed();
    QString contact = m_contactLineEdit->text().trimmed();
    QString timeSlot = m_deliveryTimeComboBox->currentText();

    // Basic validation for delivery details
    if (address.isEmpty()) {
        QMessageBox::warning(this, "Input Required", "Please enter your delivery address.");
        m_addressLineEdit->setFocus(); return;
    }
    if (contact.isEmpty()) {
        QMessageBox::warning(this, "Input Required", "Please enter your contact number.");
        m_contactLineEdit->setFocus(); return;
    }

    // Create the Order object
    Order newOrder; // Order ID is auto-incremented by its static member
    newOrder.customerId = m_customer->getID();
    newOrder.customerName = m_customer->getName();
    newOrder.orderTimestamp = QDateTime::currentDateTime();
    newOrder.deliveryDate = QDate::currentDate().addDays(1); // Delivery is for tomorrow
    // If a fixed date is needed for testing: newOrder.deliveryDate = QDate(2025, 5, 12);
    newOrder.deliveryTimeSlot = timeSlot.toStdString();
    newOrder.deliveryAddress = address.toStdString();
    newOrder.contactNumber = contact.toStdString();
    // newOrder.paymentMethod = "Cash On Delivery"; // Set by default constructor
    // newOrder.orderStatus = "Placed";           // Set by default constructor

    // Populate ordered items from the cart
    for (const auto& cartItem : m_customer->customerCart) {
        if (cartItem.product) {
            newOrder.items.emplace_back( // Use emplace_back for efficiency
                cartItem.product->getID(), cartItem.product->getName(),
                cartItem.quantity, cartItem.product->getPrice()
                );
        }
    }
    newOrder.grandTotal = m_cartTotal; // Total calculated in populateOrderSummary

    G_allOrders.push_back(newOrder); // Add the new order to the global list
    m_customer->clearCart();         // Clear the customer's cart after order is placed

    qInfo() << "Order placed successfully. Order ID:" << newOrder.orderId
            << "by Customer ID:" << newOrder.customerId << " (" << QString::fromStdString(newOrder.customerName) << ")";

    accept(); // Close the dialog with QDialog::Accepted status, indicating success
}
