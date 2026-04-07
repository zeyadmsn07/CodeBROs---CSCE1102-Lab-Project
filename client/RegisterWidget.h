#ifndef REGISTERWIDGET_H
#define REGISTERWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

class RegisterWidget : public QWidget {
    Q_OBJECT

public:
    explicit RegisterWidget(QWidget* parent = nullptr);

    void showError(const QString& msg);
    void clearError();
    void setLoading(bool on);

signals:
    void registerRequested(const QString& username,
                           const QString& password);
    void goToLoginRequested();

protected:
    void paintEvent(QPaintEvent* e) override;

private:
    QLineEdit*   usernameInput;
    QLineEdit*   passwordInput;
    QLineEdit*   confirmInput;
    QPushButton* registerBtn;
    QPushButton* backLink;
    QLabel*      errorLabel;

    void buildUi();
    void applyStyles();
    bool validate();
};

#endif
