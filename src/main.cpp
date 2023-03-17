#include <Wire.h>
#include <SSD1306.h>
#include <Keypad.h>
#include <melody_player.h>
#include <melody_factory.h>
#include "Open_Sans_Regular_32.h"
#include "Open_Sans_Regular_24.h"
#include "Open_Sans_Regular_16.h"
#include <Preferences.h>

#define SDA 4
#define SCL 15
#define BUZZER_PIN 21

const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns

char keys[ROWS][COLS] = {
        {'1', '2', '3', 'A'},
        {'4', '5', '6', 'B'},
        {'7', '8', '9', 'C'},
        {'*', '0', '#', 'D'}
};

// For ESP32 Microcontroller
byte rowPins[ROWS] = {32, 33, 25, 26};
byte colPins[COLS] = {27, 14, 12, 13};

Preferences preferences;

SSD1306 display(0x3c, SDA, SCL);
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
MelodyPlayer player(BUZZER_PIN, 0, LOW);

char texto[20] = "?x?=??";

int op1;
int op2;
int num;
int numDigitos;
int maxDigitos;

int puntos = 0;
int record = 0;

void nuevaPrueba() {
    op1 = random(1, 11);
    op2 = random(1, op1 == 10 ? 10 : 11);
    maxDigitos = op1 * op2 < 10 ? 1 : 2;
    if (op1 * op2 > 100) maxDigitos++;
    numDigitos = 0;
    num = 0;

    Serial.printf("%d x %d = %d\n", op1, op2, op1 * op2);
}

void dibujarPuntos() {
    char texto2[100];
    display.setFont(Open_Sans_Regular_16);
    sprintf(texto2, "Ptos %d / %d", puntos, record);
    display.drawString(0, 44, texto2);
}

void ganar() {
    const char* winMelody = "start:d=4,o=6,b=200: 8c, 8e, 4g";

    puntos++;
    display.clear();
    display.setFont(Open_Sans_Regular_32);
    display.drawString(0, 0, "¡Bien!");
    dibujarPuntos();
    display.display();

    Melody melody = MelodyFactory.loadRtttlString(winMelody);
    player.play(melody);
}

void perder() {
    const char* loseMelody = "start:d=4,o=4,b=100: 1c";
    const char* recordMelody = "start:d=4,o=4,b=300: 8c,8e,8g,4c5,8p,8g,4c5";
    int resultado = texto[4] - '0';
    if (numDigitos == 2) {
        resultado = resultado * 10 + texto[5] - '0';
    }
    char texto2[20];

    sprintf(texto, "%dx%d<>%d", op1, op2, resultado);
    sprintf(texto2, "%dx%d=%d", op1, op2, op1 * op2);

    display.invertDisplay();
    display.clear();
    display.setFont(Open_Sans_Regular_32);
    display.drawString(0, 0, texto);
    dibujarPuntos();
    display.display();

    Melody melody = MelodyFactory.loadRtttlString(loseMelody);
    player.play(melody);

    for (int i = 0; i < 3; i++) {
        display.invertDisplay();
        display.clear();
        display.setFont(Open_Sans_Regular_32);
        display.drawString(0, 0, texto);
        dibujarPuntos();
        display.display();
        delay(750);
        display.normalDisplay();
        display.clear();
        display.setFont(Open_Sans_Regular_32);
        display.drawString(0, 0, texto2);
        dibujarPuntos();
        display.display();
        delay(750);
    }

    display.setFont(Open_Sans_Regular_24);
    sprintf(texto2, "Ptos. %d", puntos);
    display.clear();
    display.drawString(0, 0, texto2);
    if (puntos > record) {
        display.drawString(0, 32, "¡Record!");
        display.display();
        record = puntos;
        melody = MelodyFactory.loadRtttlString(recordMelody);
        /*player.play(melody);
        preferences.begin("app", false);
        preferences.putInt("record", record);
        preferences.end();*/
    } else {
        display.setFont(Open_Sans_Regular_16);
        sprintf(texto2, "Récord: %d", record);
        display.drawString(0, 44, texto2);
        display.display();
    }

    delay(1000);
    display.setFont(Open_Sans_Regular_32);

    puntos = 0;
}

void comprobar() {
    if (op1 * op2 == num) {
        ganar();
    } else {
        perder();
    }
    nuevaPrueba();
}

void setup() {
    Serial.begin(115200);
    pinMode(16, OUTPUT);
    digitalWrite(16, HIGH);

    if (keypad.getKey() == 'C') {
        /*preferences.begin("app", false);
        preferences.clear();
        preferences.end();
        Serial.print("Borradas preferencias");*/
    }

    /*preferences.begin("app", true);
    record = preferences.getInt("record", 0);
    Serial.print("Cargadas preferencias");
    preferences.end();*/

    display.init();
    //display.invertDisplay();

    randomSeed(analogRead(0));
    nuevaPrueba();

    const char* startMelody = "aadams:d=4,o=5,b=180:8c,f,8a,f,8c,b4,2g,8f,e,8g,e,8e4,a4,2f,8c,f,8a,f,8c,b4,2g,8f,e,8c,d,8e,1f,8c,8d,8e,8f,1p,8d,8e,8f#,8g,1p,8d,8e,8f#,8g,p,8d,8e,8f#,8g,p,8c,8d,8e,8f";

    display.clear();
    display.setFont(Open_Sans_Regular_16);
    display.drawString(0, 0, "¡Bienvenido");
    display.drawString(0, 22, "a las tablas de");
    display.drawString(0, 44, "multiplicar!");
    display.display();

    Melody melody = MelodyFactory.loadRtttlString(startMelody);
    player.playAsync(melody);
    while (keypad.getKey() == 0 && player.isPlaying()) {
        delay(10);
    }
    player.stop();
    while (keypad.getKey() != 0) {
        delay(10);
    }
}

const char* click = "start:d=32,o=4,b=800: g,p";
Melody clickMelody = MelodyFactory.loadRtttlString(click);

void dibujarTexto() {
    display.clear();
    display.setFont(Open_Sans_Regular_32);
    if (numDigitos == 0) {
        sprintf(texto, "%dx%d=?", op1, op2);
    } else {
        sprintf(texto, "%dx%d=%d", op1, op2, num);
    }
    if (maxDigitos > 1 && num < 10) strcat(texto, "?");
    if (maxDigitos > 2 && num < 100) strcat(texto, "?");
    display.drawString(0, 0, texto);
    dibujarPuntos();
    display.display();
}

void loop() {
    char key;

    dibujarTexto();

    while ((key = keypad.getKey()) == 0) delay(50);

    Serial.println(key);

    //if (key == '#' && numDigitos > 0) comprobar();
    if (key == '*' && numDigitos > 0) {
        numDigitos--;
        num = num / 10;
    }
    if (key >= '0' && key <= '9' && numDigitos < maxDigitos) {
        if (numDigitos != 0 || key != '0') {
            num = num * 10 + key - '0';
            numDigitos++;
            player.play(clickMelody);
            if (numDigitos == maxDigitos) {
                dibujarTexto();
                delay(200);
                comprobar();
                return;
            }
        }
    }
    while (keypad.getKey() == key) delay(50);
}