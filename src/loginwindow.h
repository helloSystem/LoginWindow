#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QWidget>
#include <QLineEdit>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QtNetwork/QtNetwork>
#include <QComboBox>

class LoginWindow : public QWidget
{
Q_OBJECT

public:
    LoginWindow();

public slots:
    void checkCredentials();
    int startSession();


private:
    QLineEdit *m_usernameEdit;
    QLineEdit *m_passwordEdit;
    QComboBox *m_sessionComboBox;

    void shake();
    bool checkCredentialsWithPam(QString username, QString password);
};

#endif // LOGINWINDOW_H
