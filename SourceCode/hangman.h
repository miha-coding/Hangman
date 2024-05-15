#ifndef HANGMAN_H
#define HANGMAN_H

#include <QMainWindow>
#include <QPushButton>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMenu>
#include <QPixmap>
#include <QTextEdit>
#include <QLineEdit>

const int EASY = 12;
const int MIDDLE = 8;
const int STRONG = 4;

//Klasse PushButton (für die kleine Klasse lohnt sich keine eigene Datei :) )
class PushButton : public QPushButton {
    Q_OBJECT
private:
    QString name;
public:
    PushButton(QString name, QWidget *parent);
public slots:
    void click();
signals:
    void myclicked(QString name);
};

//Klasse Hangman
class Hangman : public QMainWindow
{
    Q_OBJECT

private:
    //Attribute für das Layout
    PushButton *but[26];  // Buttons mit den Buchstaben
    QGridLayout *butLay;
    QHBoxLayout *mainLay;
    QGroupBox *centralWid;  // Links das Bild, rechts die Buchstaben

    //Das Bild
    QPixmap pix;  // Bilder mit aktuellem Stand
    QLabel *pixLab;
    QString bground;
    QString picsEasy[13];  // Jeweils Dateinamen
    QString picsMiddle[8]; // zu den entsprechenden
    QString picsStrong[4]; // Bildern für den jeweiligen Schwierigkeitsgrad

    //Menü
    QMenu *spiel;

    //Attribute für das Spiel
    int fehler, FEHLERMAX;
    QLabel *mistakes;
    QLabel *cryptLab;  // Verstecktes Wort zum Raten
    QTextEdit *cryptedWord;
    enum Modes { easy, middle, strong};
    Modes modus;
    QString ratewort;

    //Dialog
    QLineEdit *wordLine;

public:
    Hangman(QWidget *parent = nullptr);
    // Only one instance needed (no assignment or copy)
    Hangman(Hangman& h) = delete;
    Hangman(Hangman&& h) = delete;
    Hangman& operator=(Hangman& h) = delete;
    Hangman& operator=(Hangman&& h) = delete;
    ~Hangman();
public slots:
    void buttonClicked(QString name);
    void checkTxt(QString txt);
    void createMenu();
    void crypt(QString word);
    void enableAllLetters(bool bo);
    void gewonnen();
    void initPics();
    void newGame();
    void newGameDialog();
    void showWord(bool b);
    void verloren();

};



#endif // HANGMAN_H
