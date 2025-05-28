#include "mainwindow.h"   // For MainWindow, User, Product, etc. class DECLARATIONS
#include "logindialog.h"    // For the LoginDialog class
#include <QApplication>     // For the Qt Application
#include <vector>           // For std::vector
#include <iostream>         // For std::cout (debug)
#include <iomanip>          // For std::setprecision in Product::printProductDetails
#include <sstream>          // For std::stringstream in Product::printProductDetails
#include <QDebug>           // For qDebug, qInfo, qWarning, qCritical
#include <QDate>            // For QDate (used in Order)
#include <QDateTime>        // For QDateTime (used in Order)

// --- Global Data ---
std::vector<User*> G_allRegisteredUsers;
User* G_guestUserInstance = nullptr;
std::vector<Order> G_allOrders;


// --- Static Member Variable Definitions ---
int User::nextID = 1;
int Product::nextID = 1;
int Order::nextOrderId = 1;

// --- Method Implementations for Classes Declared in mainwindow.h ---

// Admin Method Definitions
void Admin::printUserDetails() const {
    std::cout << "=== Admin Account ===" << std::endl;
    User::printUserDetails();
}

// Customer Method Definitions
void Customer::printUserDetails() const {
    std::cout << "=== Customer Account ===" << std::endl;
    User::printUserDetails();
}

std::string Customer::addProductToCart(Product& productToAdd, int quantity) {
    if (quantity <= 0) {
        return "Error: Quantity to add must be positive.";
    }
    if (quantity > productToAdd.getAmount()) {
        return "Error: Not enough stock. Available: " + std::to_string(productToAdd.getAmount());
    }
    for (auto& item : customerCart) {
        if (item.product && item.product->getID() == productToAdd.getID()) {
            item.quantity += quantity;
            productToAdd.setAmount(productToAdd.getAmount() - quantity);
            return "Quantity updated for '" + productToAdd.getName() + "' in the cart. Stock updated.";
        }
    }
    customerCart.push_back({&productToAdd, quantity});
    productToAdd.setAmount(productToAdd.getAmount() - quantity);
    return "'" + productToAdd.getName() + "' added to cart. Stock updated.";
}

std::string Customer::editCartItem(Product& productToEdit, int newQuantity) {
    for (size_t i = 0; i < customerCart.size(); ++i) {
        if (customerCart[i].product && customerCart[i].product->getID() == productToEdit.getID()) {
            int oldQuantityInCart = customerCart[i].quantity;
            int stockChange = oldQuantityInCart - newQuantity;
            int totalEffectivelyAvailableForThisItem = productToEdit.getAmount() + oldQuantityInCart;

            if (newQuantity > 0) {
                if (newQuantity > totalEffectivelyAvailableForThisItem) {
                    return "Error: New quantity (" + std::to_string(newQuantity) + ") exceeds total available stock for '" + productToEdit.getName() +
                           "'. Max possible for cart: " + std::to_string(totalEffectivelyAvailableForThisItem);
                }
                customerCart[i].quantity = newQuantity;
                productToEdit.setAmount(productToEdit.getAmount() + stockChange);
                return "Quantity of '" + productToEdit.getName() + "' updated to " + std::to_string(newQuantity) + ". Stock updated.";
            } else {
                productToEdit.setAmount(productToEdit.getAmount() + oldQuantityInCart);
                std::string name = customerCart[i].product->getName();
                customerCart.erase(customerCart.begin() + i);
                return "'" + name + "' removed from cart due to zero/negative quantity. Stock restored.";
            }
        }
    }
    return "Error: Product not found in cart for editing.";
}

std::string Customer::deleteCartItem(Product& productToDelete) {
    for (size_t i = 0; i < customerCart.size(); ++i) {
        if (customerCart[i].product && customerCart[i].product->getID() == productToDelete.getID()) {
            int quantityInCart = customerCart[i].quantity;
            productToDelete.setAmount(productToDelete.getAmount() + quantityInCart);
            std::string name = customerCart[i].product->getName();
            customerCart.erase(customerCart.begin() + i);
            return "'" + name + "' removed from cart. Stock restored.";
        }
    }
    return "Error: Product not found in cart for deletion.";
}

float Customer::getCartTotalPrice() const {
    float total = 0.0f;
    for (const auto& item : customerCart) {
        if (item.product) {
            total += item.product->getPrice() * item.quantity;
        }
    }
    return total;
}

// Product and Derived Classes Method Definitions
void Product::printProductDetails() const {
    std::stringstream priceStream;
    priceStream << std::fixed << std::setprecision(2) << price;

    std::cout << "Product ID: " << id << ", Name: " << name << ", Type: " << type
              << ", Amount: " << amount << ", Price: $" << priceStream.str() << std::endl;
    if (!getSpec1().empty()) {
        std::cout << "  Spec 1: " << getSpec1() << std::endl;
    }
    if (!getSpec2().empty()) {
        std::cout << "  Spec 2: " << getSpec2() << std::endl;
    }
}

void Groceries::printProductDetails() const {
    Product::printProductDetails();
}

void Clothes::printProductDetails() const {
    Product::printProductDetails();
}

void Electronics::printProductDetails() const {
    Product::printProductDetails();
}


// --- Main Application Entry Point ---
int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    G_guestUserInstance = new User("Guest", "guest@shop.com", "", true);

    std::vector<Product*> allProducts;
    allProducts.push_back(new Groceries("Organic Milk", 50, 42.99f, "2025-07-01", "2025-07-15"));
    allProducts.push_back(new Groceries("Artisan Bread", 30, 4.49f, "2025-07-10", "2025-07-13"));
    allProducts.push_back(new Clothes("Cotton T-Shirt (Red)", 100, 319.99f, "L", "Vietnam"));
    allProducts.push_back(new Clothes("Denim Jeans (Blue)", 60, 500.99f, "32W/30L", "Mexico"));
    allProducts.push_back(new Electronics("Wireless Mouse Pro", 25, 3499.99f, "Logitech", "MX Master 3S"));
    allProducts.push_back(new Electronics("4K IPS Monitor", 15, 6999.99f, "Dell", "U2723QE"));
    allProducts.push_back(new Product("Generic Mug", "Accessory", 99.99f, 9.99f));

    User* currentUser = nullptr;
    int finalExitCode = 0;

    while (true) {
        LoginDialog loginDialog(G_allRegisteredUsers, G_guestUserInstance);
        int loginResult = loginDialog.exec();

        if (loginResult == QDialog::Accepted) {
            currentUser = loginDialog.getLoggedInUser();
            if (!currentUser) {
                qCritical() << "LoginDialog accepted but no user was returned. Critical error. Exiting.";
                finalExitCode = 1;
                break;
            }
            qInfo() << "User logged in:" << QString::fromStdString(currentUser->getName())
                    << "(" << QString::fromStdString(currentUser->getType()) << ")";
        } else {
            qInfo() << "Login process ended without a successful login. Exiting application.";
            finalExitCode = 0;
            break;
        }

        MainWindow mainWindow(currentUser, allProducts);
        QObject::connect(&mainWindow, &MainWindow::logoutRequested, &mainWindow, &QMainWindow::close);
        mainWindow.show();
        (void)a.exec();

        qDebug() << "MainWindow closed. Current user was:" << (currentUser ? QString::fromStdString(currentUser->getName()) : "N/A");
    }

    qInfo() << "Application shutting down. Cleaning up resources...";
    for (Product* p : allProducts) {
        delete p;
    }
    allProducts.clear();

    for (User* u : G_allRegisteredUsers) {
        if (u != G_guestUserInstance) {
            delete u;
        }
    }
    G_allRegisteredUsers.clear();
    if (G_guestUserInstance) {
        delete G_guestUserInstance;
        G_guestUserInstance = nullptr;
    }

    qInfo() << "Cleanup complete. Application finished with exit code:" << finalExitCode;
    return finalExitCode;
}
