#include "TaskWidget.h"
#include "TaskLoader.h"
#include "TaskValidator.h"
#include <QFont>
#include <QVBoxLayout>

TaskWidget::TaskWidget(QWidget* parent)
    : QWidget(parent)
{
    buildUi();
}

void TaskWidget::buildUi()
{
    selector = new QComboBox(this);

    descLabel = new QLabel("Select a task above.", this);
    descLabel->setWordWrap(true);
    descLabel->setObjectName("descLabel");

    hintBtn = new QPushButton("Show Hint", this);
    hintBtn->setObjectName("hintBtn");

    hintLabel = new QLabel("", this);
    hintLabel->setObjectName("hintLabel");
    hintLabel->setWordWrap(true);
    hintLabel->hide();

    answerEditor = new QPlainTextEdit(this);
    answerEditor->setFont(QFont("Courier New", 11));
    answerEditor->setPlaceholderText("Type your answer here...");
    answerEditor->setMinimumHeight(80);

    submitBtn = new QPushButton("[ SUBMIT ]", this);
    submitBtn->setObjectName("submitBtn");
    submitBtn->setMinimumHeight(38);

    feedbackLabel = new QLabel("", this);
    feedbackLabel->setObjectName("feedbackLabel");
    feedbackLabel->setWordWrap(true);
    feedbackLabel->hide();

    backBtn = new QPushButton("Back to Dashboard", this);
    backBtn->setObjectName("backBtn");
    backBtn->setFlat(true);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(40, 30, 40, 30);
    layout->setSpacing(12);
    layout->addWidget(selector);
    layout->addWidget(descLabel);
    layout->addWidget(hintBtn);
    layout->addWidget(hintLabel);
    layout->addWidget(answerEditor);
    layout->addWidget(submitBtn);
    layout->addWidget(feedbackLabel);
    layout->addStretch();
    layout->addWidget(backBtn);

    tasks = TaskLoader::load("data/tasks.json");
    for (auto& t : tasks)
        selector->addItem(QString::fromStdString(t.title));

    if (!tasks.empty())
        onTaskSelected(0);

    connect(selector,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &TaskWidget::onTaskSelected);

    connect(hintBtn, &QPushButton::clicked, [this]() {
        hintVisible = !hintVisible;
        hintLabel->setVisible(hintVisible);
        hintBtn->setText(hintVisible ? "Hide Hint" : "Show Hint");
    });

    connect(submitBtn, &QPushButton::clicked,
            this, &TaskWidget::onSubmit);

    connect(backBtn, &QPushButton::clicked,
            [this]() { emit backRequested(); });

    setStyleSheet(
        "QComboBox {"
        "  background: rgba(0,0,0,0.60);"
        "  color: #ffffff;"
        "  border: 1px solid rgba(57,255,20,0.25);"
        "  border-radius: 3px;"
        "  padding: 6px 10px;"
        "  font-family: 'Courier New';"
        "  font-size: 13px;"
        "}"
        "QLabel#descLabel {"
        "  color: #cccccc;"
        "  font-size: 13px;"
        "  font-family: 'Courier New';"
        "}"
        "QLabel#hintLabel {"
        "  color: rgba(57,255,20,0.60);"
        "  font-size: 12px;"
        "  font-family: 'Courier New';"
        "  font-style: italic;"
        "}"
        "QPlainTextEdit {"
        "  background: rgba(0,0,0,0.60);"
        "  color: #ffffff;"
        "  border: 1px solid rgba(57,255,20,0.20);"
        "  border-radius: 3px;"
        "  font-family: 'Courier New';"
        "  font-size: 13px;"
        "  padding: 6px;"
        "}"
        "QPushButton#submitBtn {"
        "  background: transparent;"
        "  color: #39ff14;"
        "  border: 1px solid #39ff14;"
        "  border-radius: 3px;"
        "  font-family: 'Courier New';"
        "  font-size: 13px;"
        "  letter-spacing: 2px;"
        "}"
        "QPushButton#submitBtn:hover {"
        "  background: rgba(57,255,20,0.10);"
        "}"
        "QPushButton#hintBtn {"
        "  background: transparent;"
        "  color: rgba(57,255,20,0.60);"
        "  border: 1px solid rgba(57,255,20,0.30);"
        "  border-radius: 3px;"
        "  font-family: 'Courier New';"
        "  font-size: 12px;"
        "  min-height: 32px;"
        "}"
        "QPushButton#hintBtn:hover { color: #39ff14; border-color: #39ff14; }"
        "QLabel#feedbackLabel {"
        "  font-family: 'Courier New';"
        "  font-size: 13px;"
        "  font-weight: bold;"
        "}"
        "QPushButton#backBtn {"
        "  color: rgba(57,255,20,0.40);"
        "  font-family: 'Courier New';"
        "  font-size: 11px;"
        "  border: none;"
        "}"
        "QPushButton#backBtn:hover { color: #39ff14; }"
    );
}

void TaskWidget::onTaskSelected(int index)
{
    if (index < 0 || index >= (int)tasks.size()) return;

    auto& t = tasks[index];
    descLabel->setText(QString::fromStdString(t.description));
    hintLabel->setText(QString::fromStdString(t.hint));
    answerEditor->clear();
    feedbackLabel->hide();
    hintLabel->hide();
    hintVisible = false;
    hintBtn->setText("Show Hint");
}

void TaskWidget::onSubmit()
{
    int idx = selector->currentIndex();
    if (idx < 0 || idx >= (int)tasks.size()) return;

    std::string answer = answerEditor->toPlainText().toStdString();
    auto result = TaskValidator::validate(tasks[idx], answer);

    feedbackLabel->show();
    if (result.passed) {
        feedbackLabel->setText("Correct!");
        feedbackLabel->setStyleSheet(
            "color: #39ff14; font-family: 'Courier New'; font-size: 13px;");
    } else {
        feedbackLabel->setText(
            QString::fromStdString(result.feedback));
        feedbackLabel->setStyleSheet(
            "color: #ff5555; font-family: 'Courier New'; font-size: 12px;");
    }
}