#include "logindialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QDebug>
#include <string>
using namespace std;
// Constructor for the LoginDialog
LoginDialog::LoginDialog(vector<User*>& users, User* guestUserTemplate, QWidget *parent)
    : QDialog(parent),        // Call base QDialog constructor
    m_allUsersRef(users),   // Initialize reference to the global user list
    m_loggedInUser(nullptr),// Initially, no user is logged in
    m_guestUserTemplate(guestUserTemplate) { // Store pointer to the guest user template

    setWindowTitle("Login or Create Account"); // Set window title
    setModal(true); // Make the dialog modal (blocks interaction with other windows)

    // Main vertical layout for the dialog
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Email input field
    QLabel *emailLabel = new QLabel("Email:", this);
    m_emailEdit = new QLineEdit(this);
    m_emailEdit->setPlaceholderText("Enter your email"); // Placeholder text

    // Password input field
    QLabel *passwordLabel = new QLabel("Password:", this);
    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setEchoMode(QLineEdit::Password); // Mask password input
    m_passwordEdit->setPlaceholderText("Enter your password");

    // "Login / Create Account" Button
    m_loginButton = new QPushButton("Login / Create Account", this);
    connect(m_loginButton, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);

    // "Login as Guest" Button
    m_guestButton = new QPushButton("Login as Guest", this);
    connect(m_guestButton, &QPushButton::clicked, this, &LoginDialog::onGuestLoginClicked);

    // Add widgets to the main layout
    mainLayout->addWidget(emailLabel);
    mainLayout->addWidget(m_emailEdit);
    mainLayout->addWidget(passwordLabel);
    mainLayout->addWidget(m_passwordEdit);

    // Horizontal layout for the buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(m_loginButton);
    buttonLayout->addWidget(m_guestButton);
    mainLayout->addLayout(buttonLayout); // Add button layout to the main layout

    setLayout(mainLayout); // Apply the main layout to the dialog
    setMinimumWidth(350);  // Set a reasonable minimum width for the dialog
    m_emailEdit->setFocus(); // Set initial focus to the email field
}

// Returns a pointer to the user who logged in or was created.
User* LoginDialog::getLoggedInUser() const {
    return m_loggedInUser;
}

// Slot executed when the "Login / Create Account" button is clicked.
void LoginDialog::onLoginClicked() {
    string email = m_emailEdit->text().toStdString();
    string password = m_passwordEdit->text().toStdString();

    // Basic input validation
    if (email.empty() || password.empty()) {
        QMessageBox::warning(this, "Input Error", "Email and password cannot be empty.");
        return;
    }

    // Hardcoded Admin credentials check
    if (email == "admin@admin.com" && password == "1234") {
        bool adminFound = false;
        // Check if an admin with this email already exists in the global list
        for (User* u : m_allUsersRef) {
            if (u->getEmail() == email && dynamic_cast<Admin*>(u)) {
                // Admin found, check password (though for this specific admin, it's fixed)
                if (u->getPassword() == password) {
                    m_loggedInUser = u;
                    adminFound = true;
                    break;
                }
            }
        }
        if (!adminFound) {
            // If this specific admin isn't in the list, create and add them.
            // This ensures the admin user object exists.
            Admin* admin = new Admin("Site Admin", email, password); // Name, Email, Password
            m_allUsersRef.push_back(admin); // Add to the global list of users
            m_loggedInUser = admin;         // Set as the logged-in user
        }
        QMessageBox::information(this, "Login Successful", "Welcome, Admin!");
        accept(); // Close the dialog with QDialog::Accepted status
        return;
    }

    // Check for existing customer
    for (User* user : m_allUsersRef) {
        if (user->getEmail() == email) { // Email matches
            if (user->getPassword() == password) { // Password also matches
                m_loggedInUser = user; // Set as logged-in user
                QMessageBox::information(this, "Login Successful", QString("Welcome back, %1!").arg(QString::fromStdString(user->getName())));
                accept(); // Close dialog
                return;
            } else { // Email matches, but password incorrect
                QMessageBox::warning(this, "Login Failed", "Incorrect password for this email.");
                return;
            }
        }
    }

    // If email not found, offer to create a new Customer account
    QString qEmail = QString::fromStdString(email);
    // Use part before '@' as a default name, or the full email if no '@'
    QString defaultName = qEmail.contains('@') ? qEmail.section('@', 0, 0) : qEmail;

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Create Account?",
                                  QString("No account found for '%1'.\nCreate a new account with this email and password?").arg(qEmail),
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        // User chose to create a new account
        Customer* newCustomer = new Customer(defaultName.toStdString(), email, password);
        m_allUsersRef.push_back(newCustomer); // Add new customer to the global list
        m_loggedInUser = newCustomer;         // Set as logged-in user
        QMessageBox::information(this, "Account Created", QString("Account created successfully! Welcome, %1!").arg(defaultName));
        accept(); // Close dialog
    } else {
        // User chose not to create an account
        m_passwordEdit->clear(); // Clear password field for potential re-entry
    }
}

// Slot executed when the "Login as Guest" button is clicked.
void LoginDialog::onGuestLoginClicked() {
    if (m_guestUserTemplate) {
        m_loggedInUser = m_guestUserTemplate; // Use the shared guest user instance
        QMessageBox::information(this, "Guest Login", "You are now browsing as Guest.");
        accept(); // Close dialog
    } else {
        // This case should ideally not be reached if main.cpp initializes G_guestUserInstance
        QMessageBox::critical(this, "Error", "Guest user profile is not available. Please contact support.");
        qCritical() << "Guest user template was null in LoginDialog.";
    }
}
