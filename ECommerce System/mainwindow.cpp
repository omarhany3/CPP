#include "mainwindow.h"         // For MainWindow class DECLARATION and other class declarations
#include "checkoutdialog.h"     // For CheckoutDialog
#include "orderhistorydialog.h" // For OrderHistoryDialog
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QMessageBox>
#include <QInputDialog>
#include <QFormLayout>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDebug>
#include <string>
#include <sstream>   // <<< ADDED for std::stringstream (used in formatPrice)
#include <iomanip>   // <<< ADDED for std::fixed, std::setprecision (used in formatPrice)
#include <algorithm> // For std::find

using namespace std; // As per your preference

// --- Helper Function Definition ---
// This is the ONLY place formatPrice should be defined.
string formatPrice(float price) {
    stringstream ss;
    ss << fixed << setprecision(2) << price;
    return ss.str();
}

// --- MainWindow Method Definitions ---

// MainWindow Constructor
MainWindow::MainWindow(User* user, vector<Product*>& products, QWidget *parent)
    : QMainWindow(parent),
    // Initialize data members first, in the order of declaration in mainwindow.h
    m_currentUser(user),
    m_currentCustomer(nullptr),
    m_currentAdmin(nullptr),
    m_allProducts(products),
    // Initialize UI member pointers to nullptr, matching declaration order in mainwindow.h
    m_productListWidget(nullptr),
    m_productNameLabel(nullptr),
    m_productTypeLabel(nullptr),
    m_productPriceLabel(nullptr),
    m_productStockLabel(nullptr),
    m_productSpecificLabel1(nullptr),
    m_productSpecificLabel2(nullptr),
    m_logoutButton(nullptr),
    m_addToCartButton(nullptr),
    m_cartTableWidget(nullptr),
    m_editCartButton(nullptr),
    m_deleteCartButton(nullptr),
    m_cartTotalLabel(nullptr),
    m_checkoutButton(nullptr),
    m_viewOrderHistoryButton(nullptr),
    m_adminActionsGroupBox(nullptr),
    m_adminAddProductButton(nullptr),
    m_adminEditProductButton(nullptr),
    m_adminDeleteProductButton(nullptr)
{
    if (!m_currentUser) {
        qCritical() << "MainWindow created with a null user! Defaulting to temporary guest.";
        m_currentUser = new User("ErrorGuest", "", "", true);
    }
    if (m_currentUser && !m_currentUser->isGuest()) {
        if (dynamic_cast<Customer*>(m_currentUser)) {
            m_currentCustomer = static_cast<Customer*>(m_currentUser);
        } else if (dynamic_cast<Admin*>(m_currentUser)) {
            m_currentAdmin = static_cast<Admin*>(m_currentUser);
        }
    }
    setupMainLayout();
    populateProductList();
    updateUserSpecificUI();
}

// MainWindow Destructor
MainWindow::~MainWindow() {
    qDebug() << "MainWindow instance is being destroyed for user:" << (m_currentUser ? QString::fromStdString(m_currentUser->getName()) : "Unknown/ErrorGuest");
    if (m_currentUser && m_currentUser->getName() == "ErrorGuest") { // Clean up temporary guest
        delete m_currentUser;
        m_currentUser = nullptr;
    }
}

void MainWindow::setupMainLayout() {
    QWidget *centralWidget = new QWidget(this);
    this->setCentralWidget(centralWidget);
    QVBoxLayout *mainVLayout = new QVBoxLayout(centralWidget);

    QHBoxLayout *topBarLayout = new QHBoxLayout();
    QLabel* userInfoLabel = new QLabel(this);
    if (m_currentUser) {
        userInfoLabel->setText(QString("Welcome, %1")
                                   .arg(QString::fromStdString(m_currentUser->getName())));
    } else {
        userInfoLabel->setText("Error: No user data available");
    }
    m_logoutButton = new QPushButton("Logout", this);
    connect(m_logoutButton, &QPushButton::clicked, this, &MainWindow::onLogoutButtonClicked);
    topBarLayout->addWidget(userInfoLabel);
    topBarLayout->addStretch();
    topBarLayout->addWidget(m_logoutButton);
    mainVLayout->addLayout(topBarLayout);

    setupCommonUI(mainVLayout);
    setupCustomerUI(mainVLayout);
    setupAdminUI(mainVLayout);

    setWindowTitle("Shop With Me");
    resize(1000, 750);
}

void MainWindow::setupCommonUI(QVBoxLayout* mainLayout) {
    QHBoxLayout* productAreaLayout = new QHBoxLayout();
    QGroupBox* productGroup = new QGroupBox("Available Products", this);
    QVBoxLayout* productGroupLayout = new QVBoxLayout(productGroup);
    m_productListWidget = new QListWidget(productGroup);
    connect(m_productListWidget, &QListWidget::currentItemChanged, this, &MainWindow::onProductSelectedInList);
    productGroupLayout->addWidget(m_productListWidget);
    productAreaLayout->addWidget(productGroup, 1);

    QGroupBox* detailsGroup = new QGroupBox("Product Details", this);
    QFormLayout* detailsFormLayout = new QFormLayout(detailsGroup);
    m_productNameLabel = new QLabel("N/A", detailsGroup);
    m_productTypeLabel = new QLabel("N/A", detailsGroup);
    m_productPriceLabel = new QLabel("N/A", detailsGroup);
    m_productStockLabel = new QLabel("N/A", detailsGroup);
    m_productSpecificLabel1 = new QLabel("N/A", detailsGroup);
    m_productSpecificLabel2 = new QLabel("N/A", detailsGroup);
    detailsFormLayout->addRow("Name:", m_productNameLabel);
    detailsFormLayout->addRow("Type:", m_productTypeLabel);
    detailsFormLayout->addRow("Price:", m_productPriceLabel);
    detailsFormLayout->addRow("Stock:", m_productStockLabel);
    detailsFormLayout->addRow("Spec 1:", m_productSpecificLabel1);
    detailsFormLayout->addRow("Spec 2:", m_productSpecificLabel2);
    productAreaLayout->addWidget(detailsGroup, 1);
    mainLayout->addLayout(productAreaLayout);
}

void MainWindow::setupCustomerUI(QVBoxLayout* mainLayout) {
    m_addToCartButton = new QPushButton("Add to Cart", this);
    connect(m_addToCartButton, &QPushButton::clicked, this, &MainWindow::onAddToCartClicked);
    mainLayout->addWidget(m_addToCartButton);

    QGroupBox* cartGroup = new QGroupBox("Your Shopping Cart", this);
    QVBoxLayout* cartLayout = new QVBoxLayout(cartGroup);
    m_cartTableWidget = new QTableWidget(cartGroup);
    m_cartTableWidget->setColumnCount(5);
    QStringList headers = {"ID", "Name", "Unit Price", "Quantity", "Total"};
    m_cartTableWidget->setHorizontalHeaderLabels(headers);
    m_cartTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_cartTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_cartTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    cartLayout->addWidget(m_cartTableWidget);

    QHBoxLayout* cartActionsLayout = new QHBoxLayout();
    m_editCartButton = new QPushButton("Edit Quantity", cartGroup);
    m_deleteCartButton = new QPushButton("Remove from Cart", cartGroup);
    connect(m_editCartButton, &QPushButton::clicked, this, &MainWindow::onEditCartItemClicked);
    connect(m_deleteCartButton, &QPushButton::clicked, this, &MainWindow::onDeleteCartItemClicked);
    cartActionsLayout->addWidget(m_editCartButton);
    cartActionsLayout->addWidget(m_deleteCartButton);
    cartLayout->addLayout(cartActionsLayout);

    m_cartTotalLabel = new QLabel("Cart Total: 0.00 EGP", cartGroup);
    m_cartTotalLabel->setStyleSheet("font-weight: bold;");
    cartLayout->addWidget(m_cartTotalLabel);

    m_checkoutButton = new QPushButton("Proceed to Checkout", cartGroup);
    connect(m_checkoutButton, &QPushButton::clicked, this, &MainWindow::onCheckoutClicked);
    cartLayout->addWidget(m_checkoutButton);
    mainLayout->addWidget(cartGroup);

    m_viewOrderHistoryButton = new QPushButton("View Order History", this);
    connect(m_viewOrderHistoryButton, &QPushButton::clicked, this, &MainWindow::onViewOrderHistoryClicked);
    mainLayout->addWidget(m_viewOrderHistoryButton);
}

void MainWindow::setupAdminUI(QVBoxLayout* mainLayout) {
    m_adminActionsGroupBox = new QGroupBox("Administrator Product Management", this);
    QHBoxLayout* adminActionsLayout = new QHBoxLayout(m_adminActionsGroupBox);
    m_adminAddProductButton = new QPushButton("Add New Product", m_adminActionsGroupBox);
    connect(m_adminAddProductButton, &QPushButton::clicked, this, &MainWindow::onAdminAddProductClicked);
    adminActionsLayout->addWidget(m_adminAddProductButton);
    m_adminEditProductButton = new QPushButton("Edit Selected Product", m_adminActionsGroupBox);
    connect(m_adminEditProductButton, &QPushButton::clicked, this, &MainWindow::onAdminEditProductClicked);
    adminActionsLayout->addWidget(m_adminEditProductButton);
    m_adminDeleteProductButton = new QPushButton("Delete Selected Product", m_adminActionsGroupBox);
    connect(m_adminDeleteProductButton, &QPushButton::clicked, this, &MainWindow::onAdminDeleteProductClicked);
    adminActionsLayout->addWidget(m_adminDeleteProductButton);
    mainLayout->addWidget(m_adminActionsGroupBox);
}

void MainWindow::updateUserSpecificUI() {
    if (!m_currentUser) {
        qWarning() << "updateUserSpecificUI: m_currentUser is null.";
        if(m_addToCartButton) m_addToCartButton->setVisible(false);
        QWidget* cartGroupW = m_cartTableWidget ? qobject_cast<QWidget*>(m_cartTableWidget->parentWidget()) : nullptr;
        if(cartGroupW) cartGroupW->setVisible(false);
        if(m_checkoutButton) m_checkoutButton->setVisible(false);
        if(m_viewOrderHistoryButton) m_viewOrderHistoryButton->setVisible(false);
        if(m_adminActionsGroupBox) m_adminActionsGroupBox->setVisible(false);
        if(m_logoutButton) m_logoutButton->setText("Login Required");
        return;
    }

    bool isActualCustomer = (m_currentCustomer != nullptr && !m_currentUser->isGuest());
    bool isActualAdmin = (m_currentAdmin != nullptr && !m_currentUser->isGuest());

    if (m_addToCartButton) m_addToCartButton->setVisible(isActualCustomer);
    QWidget* cartGroupWidget = m_cartTableWidget ? qobject_cast<QWidget*>(m_cartTableWidget->parentWidget()) : nullptr;
    if (cartGroupWidget) cartGroupWidget->setVisible(isActualCustomer);
    if (m_checkoutButton) m_checkoutButton->setVisible(isActualCustomer);
    if (m_viewOrderHistoryButton) m_viewOrderHistoryButton->setVisible(isActualCustomer);

    if (m_adminActionsGroupBox) m_adminActionsGroupBox->setVisible(isActualAdmin);
    if(m_logoutButton) m_logoutButton->setText("Logout");

    if (isActualCustomer) {
        updateCartDisplay();
    } else {
        if (m_cartTableWidget) m_cartTableWidget->setRowCount(0);
        if (m_cartTotalLabel) m_cartTotalLabel->setText("Cart Total: 0.00 EGP");
        if (m_checkoutButton) m_checkoutButton->setEnabled(false);
    }
    onProductSelectedInList();
}

void MainWindow::onLogoutButtonClicked() {
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Logout", "Are you sure you want to logout?", QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        qInfo() << "User" << (m_currentUser ? QString::fromStdString(m_currentUser->getName()) : "") << "confirmed logout.";
        emit logoutRequested();
    }
}

void MainWindow::populateProductList() {
    if (!m_productListWidget) { qWarning() << "populateProductList: m_productListWidget is null!"; return; }
    Product* previouslySelectedProduct = getSelectedProductFromList();
    int previouslySelectedId = previouslySelectedProduct ? previouslySelectedProduct->getID() : -1;
    m_productListWidget->clear();
    for (Product* product : m_allProducts) {
        if (product) {
            QString itemText = QString("%1 (%2) - %3 EGP - Stock: %4")
            .arg(QString::fromStdString(product->getName()))
                .arg(QString::fromStdString(product->getType()))
                .arg(QString::fromStdString(formatPrice(product->getPrice())))
                .arg(product->getAmount());
            QListWidgetItem* listItem = new QListWidgetItem(itemText, m_productListWidget);
            listItem->setData(Qt::UserRole, QVariant::fromValue(product->getID()));
            if (product->getID() == previouslySelectedId) {
                m_productListWidget->setCurrentItem(listItem);
            }
        }
    }
    onProductSelectedInList();
}

Product* MainWindow::getSelectedProductFromList() const {
    if (!m_productListWidget) return nullptr;
    QListWidgetItem* currentItem = m_productListWidget->currentItem();
    if (!currentItem) return nullptr;
    int productID = currentItem->data(Qt::UserRole).toInt();
    return findProductById(productID);
}

Product* MainWindow::findProductById(int productID) const {
    for (Product* p : m_allProducts) {
        if (p && p->getID() == productID) return p;
    }
    return nullptr;
}

void MainWindow::onProductSelectedInList() {
    Product* selectedProduct = getSelectedProductFromList();
    displayProductDetails(selectedProduct);
    bool productIsSelected = (selectedProduct != nullptr);
    bool productIsInStock = (selectedProduct != nullptr && selectedProduct->getAmount() > 0);
    bool canAddToCart = (m_currentCustomer != nullptr && m_currentUser && !m_currentUser->isGuest() && productIsSelected && productIsInStock);
    if (m_addToCartButton) m_addToCartButton->setEnabled(canAddToCart);
    bool canAdminModify = (m_currentAdmin != nullptr && m_currentUser && !m_currentUser->isGuest() && productIsSelected);
    if (m_adminEditProductButton) m_adminEditProductButton->setEnabled(canAdminModify);
    if (m_adminDeleteProductButton) m_adminDeleteProductButton->setEnabled(canAdminModify);
    if (m_checkoutButton) m_checkoutButton->setEnabled(m_currentCustomer && !m_currentCustomer->customerCart.empty() && m_currentUser && !m_currentUser->isGuest());
}

void MainWindow::displayProductDetails(Product* product) {
    if (!m_productNameLabel) { qWarning() << "displayProductDetails: UI labels not init."; return; }
    QFormLayout* formLayout = qobject_cast<QFormLayout*>(m_productNameLabel->parentWidget()->layout());
    QLabel* spec1RowLabelWidget = formLayout ? qobject_cast<QLabel*>(formLayout->labelForField(m_productSpecificLabel1)) : nullptr;
    QLabel* spec2RowLabelWidget = formLayout ? qobject_cast<QLabel*>(formLayout->labelForField(m_productSpecificLabel2)) : nullptr;

    if (!product) {
        m_productNameLabel->setText("N/A"); m_productTypeLabel->setText("N/A");
        m_productPriceLabel->setText("N/A"); m_productStockLabel->setText("N/A");
        m_productSpecificLabel1->setText("N/A"); m_productSpecificLabel1->setVisible(false);
        if (spec1RowLabelWidget) { spec1RowLabelWidget->setText("Spec 1:"); spec1RowLabelWidget->setVisible(false); }
        m_productSpecificLabel2->setText("N/A"); m_productSpecificLabel2->setVisible(false);
        if (spec2RowLabelWidget) { spec2RowLabelWidget->setText("Spec 2:"); spec2RowLabelWidget->setVisible(false); }
        return;
    }
    m_productNameLabel->setText(QString::fromStdString(product->getName()));
    m_productTypeLabel->setText(QString::fromStdString(product->getType()));
    m_productPriceLabel->setText(QString::fromStdString(formatPrice(product->getPrice())) + " EGP");
    m_productStockLabel->setText(QString::number(product->getAmount()));
    string spec1Val = product->getSpec1(); string spec2Val = product->getSpec2();
    m_productSpecificLabel1->setText(QString::fromStdString(spec1Val));
    m_productSpecificLabel2->setText(QString::fromStdString(spec2Val));
    bool spec1Visible = true; bool spec2Visible = true;
    if (dynamic_cast<Groceries*>(product)) { if (spec1RowLabelWidget) spec1RowLabelWidget->setText("Prod. Date:"); if (spec2RowLabelWidget) spec2RowLabelWidget->setText("Exp. Date:"); }
    else if (dynamic_cast<Clothes*>(product)) { if (spec1RowLabelWidget) spec1RowLabelWidget->setText("Size:"); if (spec2RowLabelWidget) spec2RowLabelWidget->setText("Made In:"); }
    else if (dynamic_cast<Electronics*>(product)) { if (spec1RowLabelWidget) spec1RowLabelWidget->setText("Brand:"); if (spec2RowLabelWidget) spec2RowLabelWidget->setText("Model:"); }
    else { if (spec1RowLabelWidget) spec1RowLabelWidget->setText("Spec 1:"); if (spec2RowLabelWidget) spec2RowLabelWidget->setText("Spec 2:"); spec1Visible = !spec1Val.empty(); spec2Visible = !spec2Val.empty(); }
    m_productSpecificLabel1->setVisible(spec1Visible); if(spec1RowLabelWidget) spec1RowLabelWidget->setVisible(spec1Visible);
    m_productSpecificLabel2->setVisible(spec2Visible); if(spec2RowLabelWidget) spec2RowLabelWidget->setVisible(spec2Visible);
}

void MainWindow::updateCartDisplay() {
    if (!m_cartTableWidget || !m_cartTotalLabel) { qWarning() << "updateCartDisplay: Cart UI elements null."; return; }
    if (!m_currentCustomer || (m_currentUser && m_currentUser->isGuest())) {
        m_cartTableWidget->setRowCount(0); m_cartTotalLabel->setText("Cart Total: 0.00 EGP"); return;
    }
    m_cartTableWidget->setRowCount(0);
    for (const auto& cartItem : m_currentCustomer->customerCart) {
        if (cartItem.product) {
            int row = m_cartTableWidget->rowCount(); m_cartTableWidget->insertRow(row);
            m_cartTableWidget->setItem(row, 0, new QTableWidgetItem(QString::number(cartItem.product->getID())));
            m_cartTableWidget->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(cartItem.product->getName())));
            m_cartTableWidget->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(formatPrice(cartItem.product->getPrice())) + " EGP"));
            m_cartTableWidget->setItem(row, 3, new QTableWidgetItem(QString::number(cartItem.quantity)));
            float itemTotal = cartItem.product->getPrice() * cartItem.quantity;
            m_cartTableWidget->setItem(row, 4, new QTableWidgetItem(QString::fromStdString(formatPrice(itemTotal)) + " EGP"));
        }
    }
    m_cartTotalLabel->setText(QString("Cart Total: %1 EGP").arg(QString::fromStdString(formatPrice(m_currentCustomer->getCartTotalPrice()))));
    if(m_checkoutButton) m_checkoutButton->setEnabled(m_currentCustomer && !m_currentCustomer->customerCart.empty() && m_currentUser && !m_currentUser->isGuest());
}

void MainWindow::onAddToCartClicked() {
    if (!m_currentCustomer || (m_currentUser && m_currentUser->isGuest())) { QMessageBox::information(this, "Guest Action", "Guests cannot add items to cart."); return; }
    Product* selectedProduct = getSelectedProductFromList();
    if (!selectedProduct) { QMessageBox::warning(this, "No Product", "Select a product."); return; }
    if (selectedProduct->getAmount() <= 0) { QMessageBox::warning(this, "Out of Stock", "Product out of stock."); return; }
    bool ok;
    int quantity = QInputDialog::getInt(this, "Add to Cart", QString("Quantity for %1 (Max: %2):").arg(QString::fromStdString(selectedProduct->getName())).arg(selectedProduct->getAmount()), 1, 1, selectedProduct->getAmount(), 1, &ok);
    if (ok && quantity > 0) {
        string result = m_currentCustomer->addProductToCart(*selectedProduct, quantity);
        QMessageBox::information(this, "Cart Update", QString::fromStdString(result));
        updateCartDisplay(); populateProductList();
    }
}

void MainWindow::onEditCartItemClicked() {
    if (!m_currentCustomer || (m_currentUser && m_currentUser->isGuest())) { QMessageBox::information(this, "Guest Action", "Guests do not have a cart."); return; }
    if (!m_cartTableWidget || m_cartTableWidget->currentRow() < 0) { QMessageBox::warning(this, "No Selection", "Select item in cart."); return; }
    int row = m_cartTableWidget->currentRow(); int id = m_cartTableWidget->item(row, 0)->text().toInt();
    Product* masterProd = findProductById(id);
    if (!masterProd) { QMessageBox::critical(this, "Error", "Product not found."); return; }
    int currentQty = 0; bool found = false;
    for(const auto& item : m_currentCustomer->customerCart) if(item.product && item.product->getID() == id) { currentQty = item.quantity; found = true; break; }
    if (!found) { QMessageBox::warning(this, "Cart Error", "Item not in cart data."); return; }
    int maxNewQty = masterProd->getAmount() + currentQty; bool ok;
    int newQuantity = QInputDialog::getInt(this, "Edit Quantity", QString("New quantity for %1 (0 to remove, max: %2):").arg(QString::fromStdString(masterProd->getName())).arg(maxNewQty), currentQty, 0, maxNewQty, 1, &ok);
    if (ok) {
        string result = m_currentCustomer->editCartItem(*masterProd, newQuantity);
        QMessageBox::information(this, "Cart Update", QString::fromStdString(result));
        updateCartDisplay(); populateProductList();
    }
}

void MainWindow::onDeleteCartItemClicked() {
    if (!m_currentCustomer || (m_currentUser && m_currentUser->isGuest())) { QMessageBox::information(this, "Guest Action", "Guests do not have a cart."); return; }
    if (!m_cartTableWidget || m_cartTableWidget->currentRow() < 0) { QMessageBox::warning(this, "No Selection", "Select item in cart."); return; }
    int row = m_cartTableWidget->currentRow(); int id = m_cartTableWidget->item(row, 0)->text().toInt();
    Product* masterProd = findProductById(id);
    if (!masterProd) { QMessageBox::critical(this, "Error", "Product not found."); return; }
    string result = m_currentCustomer->deleteCartItem(*masterProd);
    QMessageBox::information(this, "Cart Update", QString::fromStdString(result));
    updateCartDisplay(); populateProductList();
}

// --- New Slots for Checkout and Order History ---
void MainWindow::onCheckoutClicked() {
    if (!m_currentCustomer || (m_currentUser && m_currentUser->isGuest())) {
        QMessageBox::information(this, "Checkout", "Please log in as a customer to proceed to checkout.");
        return;
    }
    if (m_currentCustomer->customerCart.empty()) {
        QMessageBox::information(this, "Checkout", "Your cart is empty. Add some products first!");
        return;
    }

    CheckoutDialog checkoutDialog(m_currentCustomer, this);
    if (checkoutDialog.exec() == QDialog::Accepted) {
        updateCartDisplay();
        populateProductList(); // Stock might have changed (though our model deducts on add to cart)
        QMessageBox::information(this, "Order Placed", "Your order has been placed successfully!\nDelivery is scheduled for tomorrow.\nPayment: Cash On Delivery.");
    }
}

void MainWindow::onViewOrderHistoryClicked() {
    if (!m_currentCustomer || (m_currentUser && m_currentUser->isGuest())) {
        QMessageBox::information(this, "Order History", "Please log in as a customer to view order history.");
        return;
    }
    // Pass m_currentUser because OrderHistoryDialog expects a const User*
    OrderHistoryDialog historyDialog(m_currentUser, this);
    historyDialog.exec();
}


// --- Admin Action Slots ---
void MainWindow::onAdminAddProductClicked() {
    if (!m_currentAdmin) return;
    QDialog addDialog(this); addDialog.setWindowTitle("Add New Product");
    QFormLayout form(&addDialog);
    QLineEdit *nameEdit = new QLineEdit(&addDialog);
    QComboBox *catCombo = new QComboBox(&addDialog); catCombo->addItems({"Groceries", "Clothes", "Electronics", "Generic"});
    QLineEdit *amtEdit = new QLineEdit(&addDialog); QLineEdit *priceEdit = new QLineEdit(&addDialog);
    QLineEdit *spec1Edit = new QLineEdit(&addDialog); QLineEdit *spec2Edit = new QLineEdit(&addDialog);
    QLabel *spec1Lbl = new QLabel("Spec 1:", &addDialog); QLabel *spec2Lbl = new QLabel("Spec 2:", &addDialog);
    form.addRow("Name:", nameEdit); form.addRow("Category:", catCombo);
    form.addRow("Amount:", amtEdit); form.addRow("Price (EGP):", priceEdit);
    form.addRow(spec1Lbl, spec1Edit); form.addRow(spec2Lbl, spec2Edit);
    auto updateLabels = [=]() {
        QString type = catCombo->currentText(); bool show = true;
        if (type == "Groceries") { spec1Lbl->setText("Prod. Date:"); spec2Lbl->setText("Exp. Date:"); }
        else if (type == "Clothes") { spec1Lbl->setText("Size:"); spec2Lbl->setText("Made In:"); }
        else if (type == "Electronics") { spec1Lbl->setText("Brand:"); spec2Lbl->setText("Model:"); }
        else { spec1Lbl->setText("Spec 1 (Opt):"); spec2Lbl->setText("Spec 2 (Opt):"); show = false; }
        spec1Lbl->setVisible(show); spec1Edit->setVisible(show);
        spec2Lbl->setVisible(show); spec2Edit->setVisible(show);
        if (type != "Generic") { spec1Lbl->setVisible(true); spec1Edit->setVisible(true); spec2Lbl->setVisible(true); spec2Edit->setVisible(true); }
    };
    connect(catCombo, &QComboBox::currentTextChanged, updateLabels); updateLabels();
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &addDialog);
    connect(buttons, &QDialogButtonBox::accepted, &addDialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &addDialog, &QDialog::reject);
    form.addRow(buttons);
    if (addDialog.exec() == QDialog::Accepted) {
        string name = nameEdit->text().toStdString(); string cat = catCombo->currentText().toStdString();
        bool amtOk, priceOk; int amt = amtEdit->text().toInt(&amtOk); float priceVal = priceEdit->text().toFloat(&priceOk);
        string s1 = spec1Edit->text().toStdString(); string s2 = spec2Edit->text().toStdString();
        if (name.empty() || !amtOk || !priceOk || amt < 0 || priceVal < 0.0f) { QMessageBox::warning(this, "Input Invalid", "Name, Amount, Price required."); return; }
        if (cat != "Generic" && (s1.empty() || s2.empty())) { QMessageBox::warning(this, "Input Invalid", "Spec fields required for non-Generic."); return; }
        Product* newProd = nullptr;
        if (cat == "Groceries") newProd = new Groceries(name, amt, priceVal, s1, s2);
        else if (cat == "Clothes") newProd = new Clothes(name, amt, priceVal, s1, s2);
        else if (cat == "Electronics") newProd = new Electronics(name, amt, priceVal, s1, s2);
        else newProd = new Product(name, cat, amt, priceVal);
        if (newProd) { m_allProducts.push_back(newProd); populateProductList(); QMessageBox::information(this, "Success", "Product added."); }
        else QMessageBox::critical(this, "Error", "Failed to create product.");
    }
}

void MainWindow::onAdminEditProductClicked() {
    if (!m_currentAdmin) return;
    Product* selProd = getSelectedProductFromList();
    if (!selProd) { QMessageBox::information(this, "Edit Product", "Select product to edit."); return; }
    openProductEditDialog(selProd);
}

void MainWindow::openProductEditDialog(Product* prod) {
    if (!prod) { qWarning() << "openProductEditDialog: null product."; return; }
    QDialog editDialog(this); editDialog.setWindowTitle("Edit: " + QString::fromStdString(prod->getName()));
    QFormLayout form(&editDialog);
    QLineEdit *nameEdit = new QLineEdit(QString::fromStdString(prod->getName()), &editDialog);
    QComboBox *catCombo = new QComboBox(&editDialog); catCombo->addItems({"Groceries", "Clothes", "Electronics", "Generic"});
    catCombo->setCurrentText(QString::fromStdString(prod->getType())); catCombo->setEnabled(false);
    QLineEdit *amtEdit = new QLineEdit(QString::number(prod->getAmount()), &editDialog);
    QLineEdit *priceEdit = new QLineEdit(QString::fromStdString(formatPrice(prod->getPrice())), &editDialog);
    QLineEdit *spec1Edit = new QLineEdit(QString::fromStdString(prod->getSpec1()), &editDialog);
    QLineEdit *spec2Edit = new QLineEdit(QString::fromStdString(prod->getSpec2()), &editDialog);
    QLabel *spec1Lbl = new QLabel("Spec 1", &editDialog); QLabel *spec2Lbl = new QLabel("Spec 2", &editDialog);
    form.addRow("Name:", nameEdit); form.addRow("Category (Fixed):", catCombo);
    form.addRow("Amount:", amtEdit); form.addRow("Price (EGP):", priceEdit);
    form.addRow(spec1Lbl, spec1Edit); form.addRow(spec2Lbl, spec2Edit);
    auto updateLabels = [=]() {
        string type = prod->getType(); bool show = true;
        if (type == "Groceries") { spec1Lbl->setText("Prod. Date:"); spec2Lbl->setText("Exp. Date:"); }
        else if (type == "Clothes") { spec1Lbl->setText("Size:"); spec2Lbl->setText("Made In:"); }
        else if (type == "Electronics") { spec1Lbl->setText("Brand:"); spec2Lbl->setText("Model:"); }
        else { spec1Lbl->setText("Spec 1:"); spec2Lbl->setText("Spec 2:"); show = !prod->getSpec1().empty() || !prod->getSpec2().empty(); }
        spec1Lbl->setVisible(show); spec1Edit->setVisible(show);
        spec2Lbl->setVisible(show); spec2Edit->setVisible(show);
    };
    updateLabels();
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, &editDialog);
    connect(buttons, &QDialogButtonBox::accepted, &editDialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &editDialog, &QDialog::reject);
    form.addRow(buttons);
    if (editDialog.exec() == QDialog::Accepted) {
        bool amtOk, priceOk; int amt = amtEdit->text().toInt(&amtOk); float priceVal = priceEdit->text().toFloat(&priceOk);
        string name = nameEdit->text().toStdString(); string s1 = spec1Edit->text().toStdString(); string s2 = spec2Edit->text().toStdString();
        if (name.empty() || !amtOk || !priceOk || amt < 0 || priceVal < 0.0f) { QMessageBox::warning(this, "Input Invalid", "Name, Amount, Price required."); return; }
        if (prod->getType() != "Generic" && (s1.empty() || s2.empty())) { QMessageBox::warning(this, "Input Invalid", "Spec fields required."); return; }
        prod->setName(name); prod->setAmount(amt); prod->setPrice(priceVal);
        prod->setSpec1(s1); prod->setSpec2(s2);
        populateProductList(); displayProductDetails(prod); QMessageBox::information(this, "Success", "Product updated.");
    }
}

void MainWindow::onAdminDeleteProductClicked() {
    if (!m_currentAdmin) return;
    Product* selProd = getSelectedProductFromList();
    if (!selProd) { QMessageBox::information(this, "Delete Product", "Select product to delete."); return; }
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirm Delete", QString("Delete '%1'? This cannot be undone.").arg(QString::fromStdString(selProd->getName())), QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        qInfo() << "Admin deleting ID:" << selProd->getID() << QString::fromStdString(selProd->getName());
        qWarning() << "Product deletion: Removing from customer carts NOT IMPLEMENTED.";

        auto it = std::find(m_allProducts.begin(), m_allProducts.end(), selProd);
        if (it != m_allProducts.end()) {
            m_allProducts.erase(it);
            delete selProd;
            selProd = nullptr;
            populateProductList();
            displayProductDetails(nullptr);
            QMessageBox::information(this, "Success", "Product deleted.");
        } else {
            QMessageBox::critical(this, "Error", "Product not found in master list for deletion.");
        }
    }
}
