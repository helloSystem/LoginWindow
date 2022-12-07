#include "loginwindow.h"

#include <security/pam_appl.h>

#include <QApplication>
#include <QDesktopWidget>
#include <QDialogButtonBox>
#include <QHostInfo>
#include <QComboBox>
#include <QSettings>
#include <QMessageBox>
#include <QPainter>
#include <QWindow>
#include <QX11Info>
#include <X11/Xlib.h>

LoginWindow::LoginWindow()
{


    // Create a widget for each screen
    const auto screens = QApplication::desktop()->screenCount();
    for (int i = 0; i < screens; ++i) {

        QWidget* widget = new QWidget();
        // Set the background image, scaling it to fit the screen
        // Note: A background-image does not scale with the size of the widget. To provide a "skin" or background
        // that scales along with the widget size, one must use border-image
        widget->setStyleSheet("border-image: url(/usr/local/share/slim/themes/default/background.jpg) 0 0 0 0 stretch stretch;");
        widget->setAutoFillBackground(true);
        widget->setWindowFlags(Qt::FramelessWindowHint);
        widget->setFixedSize(QApplication::desktop()->screenGeometry(i).size());

        widget->show();
    }

    // Set the window title
    setWindowTitle("LoginWindow");

    // Create a form layout for the login window
    QFormLayout *layout = new QFormLayout(this);

    // Centered computer icon from the theme
    QLabel *icon = new QLabel(this);
    icon->setPixmap(QIcon::fromTheme("computer").pixmap(128, 128));
    QString computerIcon = "/usr/local/share/icons/elementary-xfce/devices/128/computer-hello.png";
    if (QFile::exists(computerIcon)) {
        icon->setPixmap(QPixmap(computerIcon));
    }

    icon->setAlignment(Qt::AlignCenter);
    layout->addRow(icon);


    // Hostname label
    QLabel *title = new QLabel(QHostInfo::localHostName());
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("QLabel { color : grey; }");
    layout->addRow(title);

    // Add vertical spacing
    layout->addItem(new QSpacerItem(0, 20));

    // Create a line edit for the username
    m_usernameEdit = new QLineEdit();

    // Create a line edit for the password
    m_passwordEdit = new QLineEdit();
    m_passwordEdit->setEchoMode(QLineEdit::Password);

    // Add the username and password fields to the form layout, with labels
    layout->addRow("Username:", m_usernameEdit);
    layout->addRow("Password:", m_passwordEdit);

    // Add a dropdown menu for the desktop environment
    m_sessionComboBox = new QComboBox();
    QStringList genericDataLocations = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    // Check which one contains /xsessions
    for (int i = 0; i < genericDataLocations.size(); ++i) {
        if (QDir(genericDataLocations.at(i)).exists("xsessions")) {
            // Get the list of .desktop files in /xsessions
            QStringList desktopFiles = QDir(genericDataLocations.at(i) + "/xsessions").entryList(QStringList("*.desktop"));
            // Add the desktop files to the dropdown menu
            for (int j = 0; j < desktopFiles.size(); ++j) {
                m_sessionComboBox->addItem(desktopFiles.at(j));
                // Set the Name= value from the desktop file as the item text
                QSettings desktopFile(genericDataLocations.at(i) + "/xsessions/" + desktopFiles.at(j), QSettings::IniFormat);
                m_sessionComboBox->setItemText(j, desktopFile.value("Desktop Entry/Name").toString());
                // Set full path to the desktop file as the item data
                m_sessionComboBox->setItemData(j, genericDataLocations.at(i) + "/xsessions/" + desktopFiles.at(j));
                // Set full path to the desktop file as the tooltip
                m_sessionComboBox->setItemData(j, genericDataLocations.at(i) + "/xsessions/" + desktopFiles.at(j), Qt::ToolTipRole);
            }
        }
    }

    // Add the language dropdown menu to the form layout, with a label
    layout->addRow("Session:", m_sessionComboBox);

    // Add as much vertical spacing as possible
    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);

    // Rename the OK button to "Log In"
    buttonBox->button(QDialogButtonBox::Ok)->setText("Login");

    // Add Reboot button
    QPushButton *rebootButton = new QPushButton();
    rebootButton->setIcon(QIcon::fromTheme("system-reboot-symbolic"));
    rebootButton->setToolTip("Reboot");
    buttonBox->addButton(rebootButton, QDialogButtonBox::ActionRole);

    // Add Shutdown button
    QPushButton *shutdownButton = new QPushButton();
    shutdownButton->setIcon(QIcon::fromTheme("system-shutdown-symbolic"));
    shutdownButton->setToolTip("Shutdown");
    buttonBox->addButton(shutdownButton, QDialogButtonBox::ActionRole);

    // Make the OK button the default button
    // This is needed so that the user can press Enter to log in and the button is shown as the default button
    buttonBox->button(QDialogButtonBox::Ok)->setDefault(true);

    // Get the bacckground qss of the OK button
    QString okButtonBackground = buttonBox->button(QDialogButtonBox::Ok)->styleSheet();
    // Animation for the OK button

    // Add the button box to the form layout
    layout->addRow(buttonBox);

    // If the user presses the enter key, the "Log In" button is clicked
    connect(m_passwordEdit, SIGNAL(returnPressed()), buttonBox->button(QDialogButtonBox::Ok), SLOT(animateClick()));

    // Connect the clicked signal of the submit button to a slot that checks the login credentials
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(checkCredentials()));

    // Set padding around the form layout
    layout->setContentsMargins(50, 50, 50, 50);

    // Set window size
    setFixedSize(360, 420);

    // Set the window to be centered on the screen
    int x = (QApplication::desktop()->width() - this->width()) / 2;
    int y = (QApplication::desktop()->height() - this->height()) / 2;
    move(x, y);

    // Set the window to be on top of all other windows
    setWindowFlags(Qt::WindowStaysOnTopHint);

    // No window decorations but keep the drop shadow by using Qt.ToolTip
    setWindowFlags(Qt::ToolTip);

    // Set window opacity to 0.9
    setWindowOpacity(0.9);


}

void LoginWindow::checkCredentials() {
    QString username = m_usernameEdit->text();
    QString password = m_passwordEdit->text();
    int result = checkCredentialsWithPam(username, password);
    if (result == true) {
        qDebug() << "PAM authentication successful";
        close();

        startSession();

    } else {
        qDebug() << "PAM authentication failed";
        shake();
        m_passwordEdit->clear();
    }
}

int LoginWindow::startSession() {

    // Get the path to the selected desktop file from the item data of the selected item
    QString desktopFile = m_sessionComboBox->itemData(m_sessionComboBox->currentIndex()).toString();

    // Message box saying this still needs to be implemented
    QMessageBox::information(this, " ", QString("PAM says the credentials are valid. Now what?\n\nStarting %1 using the entered credentials is not implemented yet.\n\n"
                                                              "Do you know how to do this with minimal code, ideally using existing command line tools?").arg(desktopFile), QMessageBox::Ok);
    exit(0);

    // Get the Exec= value from the desktop file
    QSettings qSettings(desktopFile, QSettings::IniFormat);
    QString exec = qSettings.value("Desktop Entry/Exec").toString();

    if (exec.isEmpty()) {
        qCritical() << "No Exec= value found in" << desktopFile;
        return 1;
    }

    // Start the session
    QProcess process;
    process.setProgram("login");

    // Get username and password from the line edits
    QString username = m_usernameEdit->text();
    QString password = m_passwordEdit->text();

    process.setArguments(QStringList() << "-f" << username << "-p" << password << "-c" << exec);

    process.start();

    if (!process.waitForStarted()) {
        qCritical() << "Failed to start" << process.program();
        return 1;
    }

    // Wait for the process to finish
    process.waitForFinished();

    if (process.exitCode() != 0) {
        qCritical() << "Session exited with code" << process.exitCode();
        return 1;
    }

    return process.exitCode();
}

/*
 * In this example, the LoginWindow::checkCredentials() function uses the pam_start and pam_authenticate functions
 * to start a PAM session and authenticate the user with the given username and password.
 * If the username and password combination is valid, the pam_authenticate function will return PAM_SUCCESS,
 * indicating that the user was successfully authenticated. Otherwise, it will return a non-zero error code
 * indicating that the authentication failed.
 * Note that this  uses a C++11 lambda function to implement the pam_conv structure that is passed
 * to the pam_start function. This allows the password to be passed to the PAM conversation function as a C++ string,
 * rather than as a raw C string. You may need to add the -std=c++11 flag to your compiler options to use this syntax.
 * You may also need to link your Qt project with the pam library, typically by adding -lpam to the LIBS variable.
 */
bool LoginWindow::checkCredentialsWithPam(QString username, QString password)
{
    // Set up the PAM conversation structure
    struct pam_conv conv = {};
    conv.conv = [](int num_msg, const struct pam_message **msg, struct pam_response **resp, void *appdata_ptr) -> int {
        // This function is called by PAM to prompt for the user's password
        // We simply provide the password from the 'password' argument to this function
        QString *password = static_cast<QString*>(appdata_ptr);
        *resp = (struct pam_response*)calloc(num_msg, sizeof(struct pam_response));
        for (int i = 0; i < num_msg; i++) {
            (*resp)[i].resp = strdup(password->toUtf8().constData());
            (*resp)[i].resp_retcode = 0;
        }
        return PAM_SUCCESS;
    };
    conv.appdata_ptr = &password;

    // Start the PAM session and authenticate the user with the given username and password
    pam_handle_t *pamh = nullptr;
    int ret = pam_start("login", username.toUtf8().constData(), &conv, &pamh);
    if (ret != PAM_SUCCESS)
        return false;
    ret = pam_authenticate(pamh, 0);
    if (ret != PAM_SUCCESS)
        return false;

    // End the PAM session and return the result
    pam_end(pamh, ret);
    return (ret == PAM_SUCCESS);
}

void LoginWindow::shake()
{
    // Create a property animation for the login window's position
    QPropertyAnimation *animation = new QPropertyAnimation(this, "pos");

    // Set the duration of the animation to be 100 milliseconds
    animation->setDuration(100);

    // Set the loop count of the animation to be 5
    animation->setLoopCount(5);

    // Set the easing curve of the animation to be a QEasingCurve::OutElastic curve
    animation->setEasingCurve(QEasingCurve::OutElastic);

    // Set the key values of the animation to be a series of positions
    // that alternate between the window's current position and a position offset to the right by a few pixels
    QPointF startPos = pos();
    QPointF endPos = startPos;
    endPos.rx() += 15;
    animation->setKeyValueAt(0, startPos);
    animation->setKeyValueAt(0.5, endPos);
    animation->setKeyValueAt(1, startPos);

    // Start the animation
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

