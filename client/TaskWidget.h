#pragma once

#include <QComboBox>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QWidget>
#include "Task.h"
#include <vector>

class TaskWidget : public QWidget {
    Q_OBJECT
public:
    explicit TaskWidget(QWidget* parent = nullptr);

signals:
    void backRequested();

private slots:
    void onTaskSelected(int index);
    void onSubmit();

private:
    QComboBox*      selector;
    QLabel*         descLabel;
    QLabel*         hintLabel;
    QPushButton*    hintBtn;
    QPlainTextEdit* answerEditor;
    QPushButton*    submitBtn;
    QLabel*         feedbackLabel;
    QPushButton*    backBtn;

    std::vector<Task> tasks;
    bool hintVisible = false;

    void buildUi();
};