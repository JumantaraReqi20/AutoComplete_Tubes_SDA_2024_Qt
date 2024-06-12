#include <QApplication>
#include <QWidget>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QStringList>
#include <QFile>
#include <QTextStream>
#include <algorithm>
#include <unordered_map>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <QKeyEvent>
#include <QFileDialog>
#include <QDebug>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>
#include <QMessageBox>
#include <QSpacerItem>
#include <QHBoxLayout>
#include <QIcon>

using namespace std;

// Struktur TrieNode
struct TrieNode {
    unordered_map<char, TrieNode*> children;
    bool isEndOfWord = false;
};

// Prosedur untuk menyisipkan kata ke dalam node
void insertWord(TrieNode*& root, const  string& word) {
    TrieNode* node = root;
    for (char c : word) {
        if (!node->children.count(c)) {
            node->children[c] = new TrieNode();
        }
        node = node->children[c];
    }
    node->isEndOfWord = true;
}

// Prosedur untuk mencari node terakhir
TrieNode* findNode(TrieNode* root, const string& prefix) {
    TrieNode* node = root;
    for (char c : prefix) {
        if (!node->children.count(c)) return nullptr;
        node = node->children[c];
    }
    return node;
}

// Prosedur untuk membangun kata
void buildWords(TrieNode* node, std::vector<string>& results, string prefix) {
    if (node->isEndOfWord) {
        results.push_back(prefix);
    }
    for (const auto& pair : node->children) {
        buildWords(pair.second, results, prefix + pair.first);
    }
}

// Fitur Autocomplete
vector<string> autocomplete(TrieNode* root, const string& prefix) {
    vector<string> results;
    TrieNode* node = findNode(root, prefix);
    if (node) {
        buildWords(node, results, prefix);
    }
    return results;
}

// Sisip dari File
void insertCompactTrieFromFile(TrieNode* root, const string& filename) {
    ifstream file(filename);
    string word;
    if (file.is_open()) {
        while (getline(file, word)) {
            insertWord(root, word);
        }
        file.close();
    }
}

class LandingPage : public QWidget {
    Q_OBJECT
public:
    LandingPage(QWidget* parent = nullptr) : QWidget(parent) {
        createWidgets();
        connectSignalsSlots();
    }

signals:
    void startButtonClicked();

private:
    QLabel* headerLabel;
    QLabel* logoLabel;
    QLabel* creditsLabel;
    QLabel* descriptionLabel;
    QPushButton* startButton;

    void createWidgets() {
        QVBoxLayout *mainLayout = new QVBoxLayout(this);

        // Untuk Header
        headerLabel = new QLabel("A7 TEAM PROJECT", this);
        headerLabel->setAlignment(Qt::AlignCenter);
        headerLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #ffffff; margin-top: 20px;");
        mainLayout->addWidget(headerLabel);

        // Tambahin Spacer biar konsisten jaraknya
        QSpacerItem* topSpacer = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);
        mainLayout->addItem(topSpacer);

        // Logo
        logoLabel = new QLabel(this);
        QPixmap logoPixmap("C:/POLITEKNIK NEGERI BANDUNG/SEMESTER 2/SDA/Praktik/Tugas Besar/Source Code Qt/AutonoteA7/logo.png"); // NOTE: JANGAN LUPA GANTI PATH LOGONYA YA MASSE :D
        logoLabel->setPixmap(logoPixmap);
        logoLabel->setScaledContents(true);
        logoLabel->setFixedSize(320, 320); // Ukurannya 300x300 biar center, tadi nemu setup ini setelah iseng ajah.
        logoLabel->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(logoLabel);

        // Tambahin Spacer lagi :D
        QSpacerItem* bottomSpacer = new QSpacerItem(10, 10, QSizePolicy::Minimum, QSizePolicy::Expanding);
        mainLayout->addItem(bottomSpacer);

        // Kredit dikit gaksii
        creditsLabel = new QLabel("AREL x RECII\n Tugas Besar SDA 2024", this);
        creditsLabel->setAlignment(Qt::AlignCenter);
        creditsLabel->setStyleSheet("font-size: 18px; color: #ffffff; margin-top; 10px;");
        mainLayout->addWidget(creditsLabel);

        // Deskripsi singkat aplikasi
        descriptionLabel = new QLabel("Autonote adalah aplikasi catatan dengan fitur autocomplete yang memudahkan kamu mencatat dengan cepat dan efisien.", this);
        descriptionLabel->setAlignment(Qt::AlignCenter);
        descriptionLabel->setWordWrap(true);
        descriptionLabel->setStyleSheet("font-size: 16px; color: #ffffff;");
        mainLayout->addWidget(descriptionLabel);

        // Tambahin Spacer biar konsisten jaraknya
        QSpacerItem* startSpacer = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);
        mainLayout->addItem(startSpacer);

        // Start button
        startButton = new QPushButton("START", this);
        startButton->setFont(QFont("Arial", 14, QFont::Bold));
        startButton->setStyleSheet("background-color: #4CAF50; color: white; border: none; padding: 10px; border-radius: 5px;");
        mainLayout->addWidget(startButton);

        // Centering all widgets
        mainLayout->setAlignment(Qt::AlignHCenter);
        setLayout(mainLayout);

        setFixedSize(400, 600);
    }

    void connectSignalsSlots() {
        connect(startButton, &QPushButton::clicked, this, &LandingPage::onStartButtonClicked);
    }

public slots:
    void onStartButtonClicked() {
        // Hide landing page and show main autocomplete widget
        hide();
        emit startButtonClicked();
    }
};

class AutoCompleteWidget : public QWidget {
    Q_OBJECT

public:
    // Konstruktor untuk menginisialisasi widget AutoComplete
    AutoCompleteWidget(QWidget* parent = nullptr) : QWidget(parent) {
        // Membuat layout vertikal dan widget UI
        QVBoxLayout* layout = new QVBoxLayout(this);
        QLabel* label = new QLabel("Ketikkan Sesuatu:");
        lineEdit = new QLineEdit();
        listWidget = new QListWidget();
        QPushButton *saveButton = new QPushButton("Save");
        saveButton -> setStyleSheet(
            "QPushButton {"
            "    background-color: #1abc9c;"
            "    color: #ecf0f1;"
            "    border: 1px solid #16a085;"
            "    border-radius: 5px;"
            "    padding: 10px;"
            "    font-size: 18px;"
            "    transition: background-color 0.3s ease, color 0.3s ease;"
            "}"
            "QPushButton:hover {"
            "    background-color: #16a085;"
            "    color: #fff;"
            "}"
            );

        QPushButton *resetButton = new QPushButton("Reset");
        resetButton -> setStyleSheet(
            "QPushButton {"
            "    background-color: #1abc9c;"
            "    color: #ecf0f1;"
            "    border: 1px solid #16a085;"
            "    border-radius: 5px;"
            "    padding: 10px;"
            "    font-size: 18px;"
            "    transition: background-color 0.3s ease, color 0.3s ease;"
            "}"
            "QPushButton:hover {"
            "    background-color: #922B21;"
            "    color: #fff;"
            "}"
            );

        // Menambahkan widget ke layout
        layout->addWidget(label);
        layout->addWidget(lineEdit);
        layout->addWidget(listWidget);
        layout->addWidget(saveButton);
        layout->addWidget(resetButton);

        // Memuat data ke dalam Trie
        root = new TrieNode();
        // insertCompactTrieFromFile(root, "D:/POLBAN/Semester 2/Struktur Data dan Algoritma/Big Project/AutoComplete_Tubes_SDA_2024/source/kata-dasar.txt");
        // insertCompactTrieFromFile(root, "D:/POLBAN/Semester 2/Struktur Data dan Algoritma/Big Project/AutoComplete_Tubes_SDA_2024/source/kota.txt");
        // insertCompactTrieFromFile(root, "D:/POLBAN/Semester 2/Struktur Data dan Algoritma/Big Project/AutoComplete_Tubes_SDA_2024/source/namaOrang.txt");

        insertCompactTrieFromFile(root, "C:/POLITEKNIK NEGERI BANDUNG/SEMESTER 2/SDA/Praktik/Tugas Besar/Source Code Qt/AutonoteA7/source/kata-dasar.txt");
        insertCompactTrieFromFile(root, "C:/POLITEKNIK NEGERI BANDUNG/SEMESTER 2/SDA/Praktik/Tugas Besar/Source Code Qt/AutonoteA7/source/kota");
        insertCompactTrieFromFile(root, "C:/POLITEKNIK NEGERI BANDUNG/SEMESTER 2/SDA/Praktik/Tugas Besar/Source Code Qt/AutonoteA7/source/namaOrang.txt");

        // Menghubungkan perubahan teks pada lineEdit ke fungsi autocomplete
        connect(lineEdit, &QLineEdit::textChanged, this, &AutoCompleteWidget::onTextChanged);
        connect(saveButton, &QPushButton::clicked, this, &AutoCompleteWidget::onSaveButtonClicked);
        connect(lineEdit, &QLineEdit::returnPressed, this, &AutoCompleteWidget::onReturnPressed);
        connect(resetButton, &QPushButton::clicked, this, &AutoCompleteWidget::onResetButtonClicked);

        // Memastikan tombol save dapat digunakan
        saveButton->setEnabled(true);

        // Menerapkan efek bayangan pada line edit
        QGraphicsDropShadowEffect* shadowEffect = new QGraphicsDropShadowEffect(this);
        shadowEffect->setBlurRadius(10);
        shadowEffect->setOffset(2, 2);
        lineEdit->setGraphicsEffect(shadowEffect);

        // Menerapkan efek bayangan pada tombol save
        QGraphicsDropShadowEffect* buttonShadowEffect = new QGraphicsDropShadowEffect(this);
        buttonShadowEffect->setBlurRadius(10);
        buttonShadowEffect->setOffset(2, 2);
        saveButton->setGraphicsEffect(buttonShadowEffect);

        // Mengatur ukuran tetap untuk jendela utama
        setFixedSize(400, 600);
    }

protected:
    // Fungsi untuk menangani event penekanan tombol
    void keyPressEvent(QKeyEvent* event) override {
        // Jika tombol spasi ditekan
        if (event->key() == Qt::Key_Space) {
            if (listWidget->currentRow() != -1) {
                QString selectedWord = listWidget->currentItem()->text();
                typedWords.push_back(selectedWord);
                lineEdit->clear();
                listWidget->clear();
                displayTypedWords();
            }
        }
        // Jika tombol panah atas ditekan
        else if (event->key() == Qt::Key_Up) {
            if (listWidget->currentRow() > 0) {
                listWidget->setCurrentRow(listWidget->currentRow() - 1);
            }
        }
        // Jika tombol panah bawah ditekan
        else if (event->key() == Qt::Key_Down) {
            if (listWidget->currentRow() < listWidget->count() - 1) {
                listWidget->setCurrentRow(listWidget->currentRow() + 1);
            }
        }
        QWidget::keyPressEvent(event);
    }

private slots:
    // Fungsi untuk menangani perubahan teks pada lineEdit
    void onTextChanged(const QString& text) {
        std::string prefix = text.toStdString();
        std::transform(prefix.begin(), prefix.end(), prefix.begin(), ::tolower);
        std::vector<std::string> results = autocomplete(root, prefix);
        std::sort(results.begin(), results.end());

        listWidget->clear();
        for (const auto& result : results) {
            listWidget->addItem(QString::fromStdString(result));
        }
    }

    // Fungsi untuk menangani event return pada lineEdit
    void onReturnPressed() {
        if (listWidget->currentRow() != -1) {
            QString selectedWord = listWidget->currentItem()->text();
            typedWords.push_back(selectedWord);
            lineEdit->clear();
            listWidget->clear();
            displayTypedWords();
        }
    }

    // Fungsi untuk menangani klik tombol save
    void onSaveButtonClicked() {
        qDebug() << "Tombol Save Diklik.";
        QString fileName = QFileDialog::getSaveFileName(this, "Save File", "", "Text Files (.txt);;All Files ()");
        if (fileName.isEmpty()) {
            qDebug() << "Tidak ada file yang dipilih :(";
            return;
        }

        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
            QMessageBox::critical(this, "Save File", "Gagal menyimpan file!");
            return;
        }

        QTextStream out(&file);
        for (const auto& word : typedWords) {
            out << word << " ";
        }
        file.close();
        QMessageBox::information(this, "Save File", "File berhasil disimpan!");

        lineEdit->clear();
        typedWords.clear();
        displayTypedWords();
    }

    //Fungsi untuk menangani klik tombol reset
    void onResetButtonClicked(){
        lineEdit -> clear();
        typedWords.clear();
        displayTypedWords();
    }

private:
    // Fungsi untuk menampilkan kata yang telah diketik
    void displayTypedWords() {
        QString displayText;
        for (const auto& word : typedWords) {
            displayText += word + " ";
        }
        lineEdit->setPlaceholderText(displayText);
    }

    QLineEdit* lineEdit;               // Widget untuk input teks
    QListWidget* listWidget;           // Widget untuk menampilkan daftar hasil autocomplete
    TrieNode* root;                    // Root dari struktur data Trie
    vector<QString> typedWords;        // Vector untuk menyimpan kata-kata yang telah diketik
};

// Fungsi utama
int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    // Icon aplikasi
    QIcon appIcon("C:/POLITEKNIK NEGERI BANDUNG/SEMESTER 2/SDA/Praktik/Tugas Besar/Source Code Qt/AutonoteA7/logo.png");
    app.setWindowIcon(appIcon);

    AutoCompleteWidget window;
    window.setWindowTitle("Autonote");

    // Mengatur stylesheet untuk tampilan widget (ala ala CSS awikwok :v)
    QString styleSheet = R"(
        QWidget {
            background-color: #2c3e50;
            color: #ecf0f1;
            font-family: Arial, Helvetica, sans-serif;
        }
        QLineEdit {
            background-color: #34495e;
            color: #ecf0f1;
            border: 1px solid #1abc9c;
            border-radius: 5px;
            padding: 5px;
            font-size: 18px;
        }
        QListWidget {
            background-color: #34495e;
            color: #ecf0f1;
            border: 1px solid #1abc9c;
            border-radius: 5px;
            padding: 5px;
            font-size: 16px;
        }
        QListWidget::item {
            padding: 5px;
            margin: 2px 0;
        }
        QListWidget::item:selected {
            background-color: #1abc9c;
            color: #2c3e50;
        }
        QLabel {
            font-size: 20px;
            margin-bottom: 10px;
            text-align: center;
        }
    )";

    // Menerapkan stylesheet ke aplikasi
    app.setStyleSheet(styleSheet);

    LandingPage landingPage;
    AutoCompleteWidget autoCompleteWidget;
    landingPage.show();
    QObject::connect(&landingPage, &LandingPage::startButtonClicked, &autoCompleteWidget, &AutoCompleteWidget::show);
    return app.exec();
}

#include "main.moc"
