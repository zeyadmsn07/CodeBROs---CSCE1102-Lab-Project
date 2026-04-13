#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QWidget>

class LoginWidget : public QWidget {
    Q_OBJECT

   public:
    explicit LoginWidget(QWidget* parent = nullptr);

    void showError(const QString& message);
    void clearError();
    void setLoading(bool loading);

   signals:
    void loginRequested(const QString& username, const QString& password);
    void goToRegisterRequested();

   protected:
    void paintEvent(QPaintEvent* event) override;

   private:
    QLineEdit* usernameInput;
    QLineEdit* passwordInput;
    QPushButton* loginButton;
    QPushButton* registerLink;
    QLabel* errorLabel;

    void setupUi();
    void applyStyles();
};

#endif
