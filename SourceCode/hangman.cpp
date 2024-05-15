#include "hangman.h"
#include <QVariant>
#include <QDebug>
#include <QMessageBox>
#include <QRadioButton>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QLabel>
#include <QMenuBar>
#include <QValidator>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QPainter>
#include <QCheckBox>
#include <memory>

Hangman::Hangman(QWidget *parent) : QMainWindow(parent) {

    //Layout
    centralWid = new QGroupBox(this);
    mainLay = new QHBoxLayout();
    centralWid->setLayout(mainLay);

    pixLab = new QLabel(this);
    mainLay->addWidget(pixLab);
    butLay = new QGridLayout();
    mainLay->addLayout(butLay);

    this->setCentralWidget(centralWid);
    this->setFixedSize(650,640);
    this->setWindowTitle("Hangman V1.2");

    //Bild
    this->initPics();


    pix = QPixmap(400,640);
    pix.fill();
    bool s = pix.load("./pic/Hintergrund.png");
    if(!s) qDebug() << "Fehler beim Laden des Bildes";
    pixLab->setPixmap(pix);


    //Buttons initialisieren und ins eigene Layout einfügen
    int pos = 65; // Pos für den richtigen Buchstaben (65=A)
    char letter;
    QString name;
    for(int i=0;i<26;i++) {  // Von A bis Z
        letter=pos++;
        name=letter;
        // Neuer Button und Eigenschaften
        but[i] = new PushButton(name, this);
        but[i]->setFixedSize(50,50);
        but[i]->setFont(QFont("Arial",20));
        but[i]->setShortcut(QKeySequence(letter));  // Tastatureingabe

        connect(but[i],SIGNAL(myclicked(QString)),this,SLOT(buttonClicked(QString)));
    }
    // Button ins Layout einfügen
    pos=0;  //pos zeigt auf aktuellen Button
    for(int i=0;i<7;i++) {
        for(int j=0;j<4;j++) {
            if(pos>=26) {
                break;
            }
            butLay->addWidget(but[pos],i,j);
            pos++;
        }
    }

    //Initialisierung der Attribute
    ratewort = "";
    fehler = 0;
    FEHLERMAX = 0;
    //modus = QString("");

    //Fehleranzeige
    mistakes = new QLabel(tr("Fehler: %1/%2").arg(fehler).arg(FEHLERMAX),this);
    butLay->addWidget(mistakes,7,0,1,2);  // Unterhalb der Buttons

    //Aktueller Stand hinzufügen
    cryptLab = new QLabel("Lösungswort: ",this);
    cryptedWord = new QTextEdit("",this);
    cryptedWord->setReadOnly(true);
    cryptedWord->setFrameStyle(QFrame::NoFrame);
    cryptedWord->setFont(QFont("Arial",10));
    //cryptedWord->setWordWrap(true);
    butLay->addWidget(cryptLab,8,0,1,2);  // Label für aktuellen Stand
    butLay->addWidget(cryptedWord,9,0,4,4);  // aktueller Stand beim Raten

    // Menü hinzufügen
    this->createMenu();
    this->enableAllLetters(false);


}

Hangman::~Hangman()
{

}

void Hangman::buttonClicked(QString name) {
    /* Prüft, ob der geratene Buchstabe im Wort vorkommt und erhöht den
     * Fehlercounter oder ersetzt den Buchstaben.
     *
     * Input: name: enthält den geratenen Buchstaben.
     */

    bool win = true, wrong=true;
    QString word = cryptedWord->toPlainText();
    for(int i=0;i<ratewort.length();i++) {  // Ersetze die erratenen Buchstaben
        if(QVariant(ratewort.at(i)).toString()==name) { word.replace(i,1,name); wrong=false; }
        // Wenn an einer Stelle noch ein + steht ist das Wort noch nicht erraten.
        if(QVariant(word.at(i)).toString()=="+") win = false;
    }
    this->cryptedWord->setText(word);
    //Überprüfen, ob gewonnen wurde...
    if(win) { gewonnen(); return; }

    // Wurde ein Fehler gemacht, wird der Fehlercounter aktualisiert und das neue Bild gesetzt.
    if(wrong) {
        ++fehler;
        QString str = "";
        str.append(tr("Fehler: %1/%2").arg(fehler).arg(FEHLERMAX));
        mistakes->setText(str);

        //Aktuelles Bild laden
        if(modus==easy) {
            pix.load(picsEasy[fehler-1]);
        }
        else if(modus==middle) {
            pix.load(picsMiddle[fehler-1]);
        }
        else if(modus==strong) {
            pix.load(picsStrong[fehler-1]);
        }
        else {
            throw "Hangman::ButtonClicked: Unknown Modus";
        }

        pixLab->setPixmap(pix);

        if(fehler==(FEHLERMAX)) { verloren(); }
    }

}

void Hangman::checkTxt(QString txt) {
    // Ersetzt bei der Eingabe des Suchworts die Umlaute durch die Schreibweise mit e am Ende (z.B. ü -> ue).
    int size = txt.size();
    int cursorPos = wordLine->cursorPosition();

    txt.replace("Ü","UE");
    txt.replace("Ä","AE");
    txt.replace("Ö","OE");
    txt.replace("ü","ue");
    txt.replace("ä","ae");
    txt.replace("ö","oe");

    // Da die Fkt nach jedem Buchstaben aufgerufen wird, muss der Cursor verschoben werden,
    // wenn ein Umlaut ersetzt wurde.
    wordLine->setText(txt);
    if((txt.size()-1)==size) wordLine->setCursorPosition(cursorPos+1);
    else wordLine->setCursorPosition(cursorPos);
}

void Hangman::createMenu() {
    // Erzeugt das Menue des Hauptfensters.

    spiel = new QMenu("Spiel",this);
    spiel->addAction("Neues Spiel", QKeySequence(tr("F2","Neues Spiel|F2")),
                     this, SLOT(newGameDialog()) );
    spiel->addSeparator();
    spiel->addAction("Beenden", QKeySequence(tr("Ctrl+C","Neues Spiel|Beenden")),
                     this, SLOT(close()));

    menuBar()->addMenu(spiel);
    this->setMenuBar(menuBar());
}

void Hangman::crypt(QString word) {
    // Ersetzt Buchstaben des Ratewortes durch + um die Positionen und Anzahl der Buchstaben anzugeben.

    QString crWord ="";  // Crypted word
    int num_let=0;  // Anzahl der Buchstaben (keine Leerzeichen).
    for(int i=0;i<word.length();i++) {
        if(word.at(i)==' ') {
            crWord.append(" ");
        }
        else{
            crWord.append("+");
            num_let++;
        }
    }
    crWord.append(QString("\n(%1 Buchstaben)").arg(QVariant(num_let).toString()));
    this->cryptedWord->setText(crWord);
}

void Hangman::enableAllLetters(bool bo) {
    // Aktiviert (bo==true) oder deaktiviert die Buttons mit den Buchstaben.

    for(int i=0;i<26;i++) {
        but[i]->setEnabled(bo);
    }
}

void Hangman::gewonnen() {
    this->enableAllLetters(false);
    QMessageBox::information(0,"Gewonnen","Glückwunsch! Sie haben gewonnen!!",QMessageBox::Ok);
}

void Hangman::initPics() {
    // Füllt die Arrays mit den Links zu den Bildern.

    bground = QString("./pic/Hintergrund.png");

    picsEasy[0] = QString("./pic/middle_1.png");
    picsEasy[1] = QString("./pic/box.png");
    picsEasy[2] = QString("./pic/middle_3.png");
    picsEasy[3] = QString("./pic/middle_4.png");
    picsEasy[4] = QString("./pic/middle_5.png");
    picsEasy[5] = QString("./pic/middle_6.png");
    picsEasy[6] = QString("./pic/middle_7.png");
    picsEasy[7] = QString("./pic/middle_8.png");
    picsEasy[8] = QString("./pic/middle_9.png");
    picsEasy[9] = QString("./pic/middle_10.png");
    picsEasy[10] = QString("./pic/middle_11.png");
    picsEasy[11] = QString("./pic/gameOver.png");

    picsMiddle[0] = QString("./pic/box.png");
    picsMiddle[1] = QString("./pic/middle_3.png");
    picsMiddle[2] = QString("./pic/middle_4.png");
    picsMiddle[3] = QString("./pic/middle_5.png");
    picsMiddle[4] = QString("./pic/middle_7.png");
    picsMiddle[5] = QString("./pic/middle_9.png");
    picsMiddle[6] = QString("./pic/middle_11.png");
    picsMiddle[7] = QString("./pic/gameOver.png");

    picsStrong[0] = QString("./pic/box.png");
    picsStrong[1] = QString("./pic/middle_4.png");
    picsStrong[2] = QString("./pic/middle_11.png");
    picsStrong[3] = QString("./pic/gameOver.png");

}

void Hangman::newGame() {
    //Das Ratewort verschlüsseln lassen und setzen. Außerdem das Bild setzen.

    fehler=0;
    QString str="";
    str.append(tr("Fehler: %1/%2").arg(fehler).arg(FEHLERMAX));
    mistakes->setText(str);

    pix.load(bground);

    pixLab->setPixmap(pix);
}

void Hangman::newGameDialog() {
    // Dialog, um neues Spiel zu starten.

    auto newGameDialog = std::make_unique<QDialog>();
    //QDialog *newGameDialog = new QDialog(this);
    QRadioButton *easy, *middle, *strong;
    QGroupBox *radioGroup;
    QHBoxLayout *radioLay;
    QGridLayout *newGameLay;
    QLabel *wordLab;
    QDialogButtonBox *dialButBox;
    QPushButton *go, *cancel;
    QCheckBox *check;

    //RadioButtons mit Box
    easy = new QRadioButton(tr("Leicht (%1 Fehler)").arg(EASY), newGameDialog.get());
    middle = new QRadioButton(tr("Mittel (%1 Fehler)").arg(MIDDLE), newGameDialog.get());
    strong = new QRadioButton(tr("Schwer (%1 Fehler)").arg(STRONG), newGameDialog.get());
    middle->setChecked(true);
    radioGroup = new QGroupBox("Schwierigkeit",newGameDialog.get());
    radioLay = new QHBoxLayout();
    radioGroup->setLayout(radioLay);

    radioLay->addWidget(easy);
    radioLay->addWidget(middle);
    radioLay->addWidget(strong);

    //Eingabefeld
    wordLab = new QLabel("Suchwort (keine Sonderzeichen): ",newGameDialog.get());
    wordLine = new QLineEdit(newGameDialog.get());
    wordLine->setMaxLength(99);
    wordLine->setEchoMode(QLineEdit::Password);
    QRegularExpression re("[a-z ü ä ö A-Z Ü Ä Ö]*");  // Erlaubte Zeichen
    QValidator *val = new QRegularExpressionValidator(re, newGameDialog.get());
    wordLine->setValidator(val);  // Nur erlaubte Zeichen dürfen eingetragen werden.

    //CheckBox
    check = new QCheckBox("Wort anzeigen",newGameDialog.get());
    check->setChecked(false);

    //Buttons
    go = new QPushButton("Start",newGameDialog.get());
    cancel = new QPushButton("Abbrechen",newGameDialog.get());
    dialButBox = new QDialogButtonBox(newGameDialog.get());
    dialButBox->addButton(go,QDialogButtonBox::AcceptRole);
    dialButBox->addButton(cancel,QDialogButtonBox::RejectRole);

    //Layout und anzeigen
    newGameLay = new QGridLayout(newGameDialog.get());
    newGameLay->addWidget(wordLab,0,0);
    newGameLay->addWidget(wordLine,0,1,1,2);
    newGameLay->addWidget(check,1,0,1,3,Qt::AlignRight);
    newGameLay->addWidget(radioGroup,2,0,1,3);
    newGameLay->addWidget(dialButBox,3,0,1,3,Qt::AlignRight);

    //connect
    connect(go,SIGNAL(clicked()),newGameDialog.get(),SLOT(accept()));
    connect(cancel,SIGNAL(clicked()),newGameDialog.get(),SLOT(reject()));
    connect(wordLine,SIGNAL(textChanged(QString)),this,SLOT(checkTxt(QString)));
    connect(check,SIGNAL(toggled(bool)),this,SLOT(showWord(bool)));

    newGameDialog->setLayout(newGameLay);
    wordLine->setFocus();

    int ret = newGameDialog->exec();

    // Dialog erneut aufrufen, solange mit leerem Wort gestartet werden soll.
    while(((wordLine->text()=="")&&(ret==QDialog::Accepted))) ret=newGameDialog->exec();

    //Anzahl der maximalen Fehler
    if(easy->isChecked()) { FEHLERMAX = EASY; modus = Modes::easy; }
    else if(middle->isChecked()) { FEHLERMAX = MIDDLE; modus = Modes::middle; }
    else if(strong->isChecked()) { FEHLERMAX = STRONG; modus = Modes::strong; }
    else { FEHLERMAX = MIDDLE; }  // Nur als Backup

    if(ret==QDialog::Accepted) {
        // Spiel starten
        this->ratewort = wordLine->text();
        this->ratewort = ratewort.toUpper();
        this->crypt(ratewort);
        for(int i=0;i<26;i++) but[i]->setEnabled(true);
        this->newGame();
    }
}

void Hangman::showWord(bool b) {
    // Klartext oder Punkte bei Eingabe des Rateworts?

    int cursorPos = wordLine->cursorPosition();
    b ? wordLine->setEchoMode(QLineEdit::Normal) : wordLine->setEchoMode(QLineEdit::Password);
    wordLine->setFocus();
    wordLine->setCursorPosition(cursorPos);
}

void Hangman::verloren() {
    this->enableAllLetters(false);
    QMessageBox::information(0,"Verloren","Sie haben verloren! Das Lösungswort lautet:\n"
                               +ratewort,QMessageBox::Ok);
}


// Klasse PushButton_________________________________________________________

PushButton::PushButton(QString name, QWidget *parent) : QPushButton(name,parent) {
    this->name = name;
    connect(this,SIGNAL(clicked()),this,SLOT(click()));
}

void PushButton::click() {
    emit myclicked(this->name);
    this->setEnabled(false);
}

