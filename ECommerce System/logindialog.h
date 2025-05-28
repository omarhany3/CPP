#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <vector>
#include "mainwindow.h"
using namespace std;

// Forward declarations for Qt UI elements
class QLineEdit;
class QPushButton;
class User; // Already included via mainwindow.h but good practice for clarity

class LoginDialog : public QDialog {
    Q_OBJECT // Macro for classes defining signals or slots

public:
    // Constructor takes a reference to the global list of registered users
    // and a pointer to the shared guest user instance.
    explicit LoginDialog(vector<User*>& users, User* guestUserTemplate, QWidget *parent = nullptr);

    // Returns a pointer to the User object that successfully logged in or was created.
    // Returns nullptr if login was cancelled or failed critically.
    User* getLoggedInUser() const;

private slots:
    // Slot to handle the "Login / Create Account" button click.
    void onLoginClicked();
    // Slot to handle the "Login as Guest" button click.
    void onGuestLoginClicked();

private:
    // UI Elements
    QLineEdit *m_emailEdit;     // Input field for email
    QLineEdit *m_passwordEdit;  // Input field for password
    QPushButton *m_loginButton; // Button for login/account creation
    QPushButton *m_guestButton; // Button for guest login

    // Data
    // Reference to the global list of users (managed in main.cpp).
    // This allows the dialog to check existing users and add new ones.
    vector<User*>& m_allUsersRef;
    // Pointer to the user who successfully logs in or is created by this dialog.
    User* m_loggedInUser;
    // Pointer to the shared guest user instance (created in main.cpp).
    User* m_guestUserTemplate;
};

#endif // LOGINDIALOG_H
