#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <string>
#include <vector>
#include <iostream> // For User::printUserDetails, etc.
// <iomanip> // Moved to mainwindow.cpp (for formatPrice definition)
// <sstream> // Moved to mainwindow.cpp (for formatPrice definition)
#include <QDateTime> // For QDate, QDateTime (used in Order struct)

// Forward declarations for Qt UI elements
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QListWidget;
class QLabel;
class QPushButton;
class QTableWidget;
class QVBoxLayout;
class QComboBox;
class QGroupBox;

// =================================================================================
// Data Class Definitions (User, Product, Order etc.)
// =================================================================================

class Product;

struct OrderedItem {
    int productId;
    std::string productName;
    int quantity;
    float pricePerItem;
    float itemTotalPrice;
    OrderedItem(int id, std::string name, int qty, float price)
        : productId(id), productName(name), quantity(qty), pricePerItem(price) {
        itemTotalPrice = pricePerItem * quantity;
    }
};

struct Order {
    int orderId;
    int customerId;
    std::string customerName;
    std::vector<OrderedItem> items;
    float grandTotal;
    QDateTime orderTimestamp;
    QDate deliveryDate;
    std::string deliveryTimeSlot;
    std::string deliveryAddress;
    std::string contactNumber;
    std::string paymentMethod;
    std::string orderStatus;
    static int nextOrderId;
    Order() : orderId(nextOrderId++), customerId(-1), grandTotal(0.0f), paymentMethod("Cash On Delivery"), orderStatus("Placed") {}
};

struct CartItem {
    Product* product;
    int quantity;
};

class User {
protected:
    int id;
    std::string name;
    std::string type;
    std::string email;
    std::string password;
    bool m_isGuest;
    static int nextID;
public:
    User(std::string n, std::string e, std::string p, bool isGuest = false)
        : name(n), email(e), password(p), m_isGuest(isGuest) {
        id = nextID++;
        if (isGuest) { type = "Guest"; }
        else { type = "User"; }
    }
    virtual ~User() {}
    int getID() const { return id; }
    std::string getName() const { return name; }
    void setName(const std::string& newName) { name = newName; }
    std::string getEmail() const { return email; }
    std::string getPassword() const { return password; }
    std::string getType() const { return type; }
    bool isGuest() const { return m_isGuest; }
    virtual void printUserDetails() const { // Inline definition is fine here
        std::cout << "User ID: " << id << ", Name: " << name << ", Type: " << type << std::endl;
    }
};

class Admin : public User {
public:
    Admin(std::string n, std::string e, std::string p) : User(n, e, p, false) { type = "Admin"; }
    void printUserDetails() const override; // Declaration only
};

class Customer : public User {
public:
    std::vector<CartItem> customerCart;
    Customer(std::string n, std::string e, std::string p) : User(n, e, p, false) { type = "Customer"; }
    void printUserDetails() const override; // Declaration only
    std::string addProductToCart(Product& productToAdd, int quantity); // Declaration only
    std::string editCartItem(Product& productToEdit, int newQuantity); // Declaration only
    std::string deleteCartItem(Product& productToDelete); // Declaration only
    float getCartTotalPrice() const; // Declaration only
    void clearCart() { customerCart.clear(); } // Inline definition is fine
};

class Product {
protected:
    int id;
    static int nextID;
public:
    std::string name;
    std::string type;
    int amount;
    float price;
    Product(std::string n, std::string t, int a, float p) : name(n), type(t), amount(a), price(p) { id = nextID++; }
    virtual ~Product() {}
    int getID() const { return id; }
    std::string getName() const { return name; }
    void setName(const std::string& newName) { name = newName; }
    std::string getType() const { return type; }
    void setType(const std::string& newType) { type = newType; }
    int getAmount() const { return amount; }
    void setAmount(int newAmount) { amount = newAmount; }
    float getPrice() const { return price; }
    void setPrice(float newPrice) { price = newPrice; }
    virtual void printProductDetails() const; // Declaration only
    virtual std::string getSpec1() const { return ""; } // Inline definition is fine
    virtual void setSpec1(const std::string& s1) { (void)s1; } // Inline definition is fine
    virtual std::string getSpec2() const { return ""; } // Inline definition is fine
    virtual void setSpec2(const std::string& s2) { (void)s2; } // Inline definition is fine
};

class Groceries : public Product {
private: std::string prodDate, expDate;
public:
    Groceries(std::string n, int a, float p, std::string dop, std::string exd)
        : Product(n, "Groceries", a, p), prodDate(dop), expDate(exd) {}
    std::string getProdDate() const { return prodDate; } std::string getExpDate() const { return expDate; }
    void printProductDetails() const override; // Declaration only
    std::string getSpec1() const override { return prodDate; } void setSpec1(const std::string& s1) override { prodDate = s1; }
    std::string getSpec2() const override { return expDate; }  void setSpec2(const std::string& s2) override { expDate = s2; }
};
class Clothes : public Product {
private: std::string size, madeIn;
public:
    Clothes(std::string n, int a, float p, std::string s, std::string m)
        : Product(n, "Clothes", a, p), size(s), madeIn(m) {}
    std::string getSize() const { return size; } std::string getMadeIn() const { return madeIn; }
    void printProductDetails() const override; // Declaration only
    std::string getSpec1() const override { return size; } void setSpec1(const std::string& s1) override { size = s1; }
    std::string getSpec2() const override { return madeIn; } void setSpec2(const std::string& s2) override { madeIn = s2; }
};
class Electronics : public Product {
private: std::string brand, model;
public:
    Electronics(std::string n, int a, float p, std::string b, std::string m)
        : Product(n, "Electronics", a, p), brand(b), model(m) {}
    std::string getBrand() const { return brand; } std::string getModel() const { return model; }
    void printProductDetails() const override; // Declaration only
    std::string getSpec1() const override { return brand; } void setSpec1(const std::string& s1) override { brand = s1; }
    std::string getSpec2() const override { return model; } void setSpec2(const std::string& s2) override { model = s2; }
};

class MainWindow : public QMainWindow {
    Q_OBJECT // This macro is necessary for Qt's meta-object system (signals, slots, etc.)
public:
    explicit MainWindow(User* user, std::vector<Product*>& products, QWidget *parent = nullptr);
    ~MainWindow();
signals:
    void logoutRequested();
private slots:
    void onProductSelectedInList();
    void onAddToCartClicked();
    void onEditCartItemClicked();
    void onDeleteCartItemClicked();
    void onLogoutButtonClicked();
    void onAdminAddProductClicked();
    void onAdminEditProductClicked();
    void onAdminDeleteProductClicked();
    void onCheckoutClicked();
    void onViewOrderHistoryClicked();
private:
    // Data members first (logical grouping, helps with -Wreorder if init list matches)
    User* m_currentUser;
    Customer* m_currentCustomer;
    Admin* m_currentAdmin;
    std::vector<Product*>& m_allProducts;

    // UI Elements
    QListWidget *m_productListWidget;
    QLabel *m_productNameLabel;
    QLabel *m_productTypeLabel;
    QLabel *m_productPriceLabel;
    QLabel *m_productStockLabel;
    QLabel *m_productSpecificLabel1;
    QLabel *m_productSpecificLabel2;
    QPushButton *m_logoutButton;

    // Customer-specific UI
    QPushButton *m_addToCartButton;
    QTableWidget *m_cartTableWidget;
    QPushButton *m_editCartButton;
    QPushButton *m_deleteCartButton;
    QLabel *m_cartTotalLabel;
    QPushButton *m_checkoutButton;
    QPushButton *m_viewOrderHistoryButton;

    // Admin-specific UI
    QGroupBox   *m_adminActionsGroupBox;
    QPushButton *m_adminAddProductButton;
    QPushButton *m_adminEditProductButton;
    QPushButton *m_adminDeleteProductButton;

    // UI Setup helper methods
    void setupMainLayout();
    void setupCommonUI(QVBoxLayout* contentLayout);
    void setupCustomerUI(QVBoxLayout* contentLayout);
    void setupAdminUI(QVBoxLayout* contentLayout);
    void updateUserSpecificUI();
    void populateProductList();
    void displayProductDetails(Product* product);
    void updateCartDisplay();
    Product* getSelectedProductFromList() const;
    Product* findProductById(int productID) const;
    void openProductEditDialog(Product* productToEdit);
};

// Declaration for the global helper function formatPrice.
// Its definition will be in mainwindow.cpp.
std::string formatPrice(float price);

#endif // MAINWINDOW_H
