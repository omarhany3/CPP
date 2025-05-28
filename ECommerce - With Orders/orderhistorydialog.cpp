#include "orderhistorydialog.h" // Defines OrderHistoryDialog, includes mainwindow.h (for User, Order, formatPrice declaration)
#include <QVBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>      // For QPushButton
#include <QDebug>           // For qDebug, qCritical
#include <vector>           // For std::vector (used with G_allOrders)
// mainwindow.h (included via orderhistorydialog.h) should provide QDate, QDateTime, formatPrice declaration.

// Reference to the global order list (must be defined in main.cpp)
extern std::vector<Order> G_allOrders;

OrderHistoryDialog::OrderHistoryDialog(const User* customer, QWidget *parent)
    : QDialog(parent), m_customer(customer), m_ordersTableWidget(nullptr) { // Initialize m_ordersTableWidget

    if (!m_customer) {
        qCritical() << "OrderHistoryDialog initialized with a null customer! Dialog will be unusable.";
        // Optionally, disable widgets or close the dialog immediately
        return;
    }

    setWindowTitle("Your Order History - " + QString::fromStdString(m_customer->getName()));
    setModal(true);
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    m_ordersTableWidget = new QTableWidget(this); // Create the table widget
    m_ordersTableWidget->setColumnCount(8); // Order ID, Date Placed, Delivery Date, Time Slot, Contact, Total, Status, Items
    QStringList headers = {"Order ID", "Date Placed", "Delivery Date", "Time Slot", "Contact #", "Total (EGP)", "Status", "Items"};
    m_ordersTableWidget->setHorizontalHeaderLabels(headers);
    m_ordersTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); // Stretch all columns
    m_ordersTableWidget->horizontalHeader()->setSectionResizeMode(7, QHeaderView::Interactive); // Allow "Items" column to be resized or set a fixed width
    m_ordersTableWidget->setColumnWidth(7, 200); // Example fixed width for items summary

    m_ordersTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_ordersTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_ordersTableWidget->setWordWrap(true); // Allow word wrap for items column
    m_ordersTableWidget->setTextElideMode(Qt::ElideNone); // Prevent eliding text in items column

    QPushButton* closeButton = new QPushButton("Close", this);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept); // accept() closes the dialog

    mainLayout->addWidget(m_ordersTableWidget);
    mainLayout->addWidget(closeButton);

    setLayout(mainLayout);
    populateOrderHistory(); // Populate data after UI is set up
    resize(900, 500);       // Adjusted size for more columns
}

void OrderHistoryDialog::populateOrderHistory() {
    if (!m_customer || !m_ordersTableWidget) {
        qWarning() << "populateOrderHistory: Customer or table widget is null.";
        return;
    }

    m_ordersTableWidget->setRowCount(0); // Clear existing rows

    for (const auto& order : G_allOrders) {
        if (order.customerId == m_customer->getID()) { // Filter orders for the current customer
            int row = m_ordersTableWidget->rowCount();
            m_ordersTableWidget->insertRow(row);

            m_ordersTableWidget->setItem(row, 0, new QTableWidgetItem(QString::number(order.orderId)));
            m_ordersTableWidget->setItem(row, 1, new QTableWidgetItem(order.orderTimestamp.toString("yyyy-MM-dd hh:mm ap")));
            m_ordersTableWidget->setItem(row, 2, new QTableWidgetItem(order.deliveryDate.toString("yyyy-MM-dd")));
            m_ordersTableWidget->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(order.deliveryTimeSlot)));
            m_ordersTableWidget->setItem(row, 4, new QTableWidgetItem(QString::fromStdString(order.contactNumber))); // New Contact Number column
            m_ordersTableWidget->setItem(row, 5, new QTableWidgetItem(QString::fromStdString(formatPrice(order.grandTotal)) + " EGP"));
            m_ordersTableWidget->setItem(row, 6, new QTableWidgetItem(QString::fromStdString(order.orderStatus)));

            // Create a summary string for items
            QString itemsSummaryStr;
            for (size_t i = 0; i < order.items.size(); ++i) {
                const auto& item = order.items[i];
                itemsSummaryStr += QString::fromStdString(item.productName) + " (Qty: " + QString::number(item.quantity) + ")";
                if (i < order.items.size() - 1) {
                    itemsSummaryStr += ", ";
                }
            }
            QTableWidgetItem* itemsCell = new QTableWidgetItem(itemsSummaryStr);
            // itemsCell->setTextAlignment(Qt::AlignLeft | Qt::AlignTop); // Align text for readability
            m_ordersTableWidget->setItem(row, 7, itemsCell); // New Items column
        }
    }
    m_ordersTableWidget->resizeRowsToContents(); // Adjust row height if text wraps
}
