#include <Wire.h>
#include <SSD1306.h>
#include <Keypad.h>
#include <melody_player.h>
#include <melody_factory.h>
#include "Open_Sans_Regular_32.h"
#include "Open_Sans_Regular_24.h"
#include "Open_Sans_Regular_16.h"
#include "Open_Sans_Regular_10.h"
#include "Open_Sans_Light_8.h"
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

char texto[20];

int op1;
int op2;
int num;
int numDigitos;
int maxDigitos;

int puntos = 0;
int record = 0;

int tablas[10];
int tiempo = 0;
int tipo = 0;
int tipoActual = 0;
int configurando = 0;
int secuencia = 0;
int tiempoRestante = 0;

void nuevaPrueba() {
    tiempoRestante = tiempo;

    int solucion;
    if (tipo == 3) {
        tipoActual = random(2);
    } else if (tipo == 4) {
        tipoActual = random(3);
    } else {
        tipoActual = tipo;
    }
    switch (tipoActual) {
        // A x B == C
        case 0:
            // A: Seleccionar una tabla permitida al azar
            do {
                op1 = random(1, 11);
            } while (tablas[op1 - 1] == 1);
            // B: Elegir un número del 1 al 10, salvo para
            // la tabla del 10, que el máximo es 9
            op2 = random(1, op1 == 10 ? 10 : 11);
            // C es la solución a averiguar
            solucion = op1 * op2;
            break;
        case 1:
            // A: Seleccionar una tabla permitida al azar
            do {
                op1 = random(1, 11);
            } while (tablas[op1 - 1] == 1);
            // B: es la solución a averiguar. Un número del 1 al 10,
            // salvo para la tabla del 10, que el máximo es 9
            solucion = random(1, op1 == 10 ? 10 : 11);
            // C: Resultado de multiplicar A por B
            op2 = op1 * solucion;
            break;
        default:
            // A: Es la solución a averiguar. Un número del 1 al 10
            do {
                solucion = random(1, 11);
            } while (tablas[solucion - 1] == 1);
            // B: Elegir un número del 1 al 10, salvo para
            // la tabla del 10, que el máximo es 9
            op1 = random(1, solucion == 10 ? 10 : 11);
            // C: Resultado de multiplicar A por B
            op2 = op1 * solucion;
            break;
    }
    maxDigitos = solucion < 10 ? 1 : 2;
    if (solucion >= 100) maxDigitos++;

    numDigitos = 0;
    num = 0;
}

void procesaSecuencia(char key) {
    switch (secuencia) {
        case 0:
            secuencia = (key == 'A') ? 1 : 0;
            break;
        case 1:
            secuencia = (key == 'C') ? 2 : 0;
            break;
        case 2:
            secuencia = (key == 'A') ? 3 : 0;
            break;
        case 3:
            secuencia = (key == 'B') ? 4 : 0;
            break;
        case 4:
            configurando = (key == 'A') ? 1 : 0;
            secuencia = 0;
            break;
        default:
            secuencia = 0;
    }
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

void compruebaDelay(int delayOrig) {
    for (int j = 0; j < delayOrig / 50; j++) {
        char key = keypad.getKey();
        if (key != 0) procesaSecuencia(key);
        delay(50);
    }
}

void perder() {
    const char* loseMelody = "start:d=4,o=4,b=100: 1c";
    const char* recordMelody = "start:d=4,o=4,b=300: 8c,8e,8g,4c5,8p,8g,4c5";

    char texto2[20];

    switch (tipoActual) {
        case 0:
            sprintf(texto, "%dx%d<>%d", op1, op2, num);
            sprintf(texto2, "%dx%d=%d", op1, op2, op1 * op2);
            break;
        case 1:
            sprintf(texto, "%dx%d<>%d", op1, num, op2);
            sprintf(texto2, "%dx%d=%d", op1, op2 / op1, op2);
            break;
        case 2:
            sprintf(texto, "%dx%d<>%d", num, op1, op2);
            sprintf(texto2, "%dx%d=%d", op2 / op1, op1, op2);
            break;
    }

    if (tiempoRestante < 0) {
        strcpy(texto, texto2);
    }

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
        compruebaDelay(750);
        display.normalDisplay();
        display.clear();
        display.setFont(Open_Sans_Regular_32);
        display.drawString(0, 0, texto2);
        dibujarPuntos();
        display.display();
        compruebaDelay(750);
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
        player.play(melody);
        preferences.begin("app", false);
        preferences.putInt("record", record);
        preferences.end();
    } else {
        display.setFont(Open_Sans_Regular_16);
        sprintf(texto2, "Récord: %d", record);
        display.drawString(0, 44, texto2);
        display.display();
    }

    compruebaDelay(1000);

    display.setFont(Open_Sans_Regular_32);

    puntos = 0;
}

void comprobar() {
    int haGanado;
    switch (tipoActual) {
        case 1:
            haGanado = op1 * num == op2;
            break;
        case 2:
            haGanado = num * op1 == op2;
            break;
        default:
            haGanado = op1 * op2 == num;
            break;
    }
    if (haGanado) {
        ganar();
    } else {
        perder();
    }
    nuevaPrueba();
}

void setup() {
    pinMode(16, OUTPUT);
    digitalWrite(16, HIGH);

    if (keypad.getKey() == 'A') configurando = 1;

    preferences.begin("app", false);
    record = preferences.getInt("record", 0);
    tiempo = preferences.getInt("tiempo", 0);
    tipo = preferences.getInt("tipo", 0);
    for (int i = 0; i < 10; i++) {
        char nombre[9] = "tablas";
        itoa(i + 1, &nombre[6], 10);
        tablas[i] = preferences.getInt(nombre, 0);
    }
    preferences.end();

    display.init();
    digitalWrite(16, HIGH);
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

void dibujarTiempo() {
    if (tiempo == 0) return;
    int altura = 64 * tiempoRestante / tiempo;
    display.fillRect(124, 64 - altura, 4, altura);
}

void dibujarTexto() {
    display.clear();
    display.setFont(Open_Sans_Regular_32);

    switch (tipoActual) {
        case 0:
            if (numDigitos == 0) {
                sprintf(texto, "%dx%d=?", op1, op2);
            } else {
                sprintf(texto, "%dx%d=%d", op1, op2, num);
            }
            if (maxDigitos > 1 && num < 10) strcat(texto, "?");
            if (maxDigitos > 2 && num < 100) strcat(texto, "?");
            break;
        case 1:
            if (numDigitos == 0) {
                sprintf(texto, "%dx?", op1);
            } else {
                sprintf(texto, "%dx%d", op1, num);
            }
            if (maxDigitos > 1 && num < 10) strcat(texto, "?");
            if (maxDigitos > 2 && num < 100) strcat(texto, "?");
            sprintf(&texto[strlen(texto)], "=%d", op2);
            break;
        case 2:
            if (numDigitos == 0) {
                strcpy(texto, "?");
            } else {
                sprintf(texto, "%d", num);
            }
            if (maxDigitos > 1 && num < 10) strcat(texto, "?");
            if (maxDigitos > 2 && num < 100) strcat(texto, "?");
            sprintf(&texto[strlen(texto)], "x%d=%d", op1, op2);
            break;
    }
    display.drawString(0, 0, texto);
    dibujarPuntos();
    dibujarTiempo();
    display.display();
}

void dibujarConfiguracion() {
    display.clear();
    display.setFont(Open_Sans_Regular_16);
    char cadena[20];
    for (int i = 0; i < 10; i++) {
        itoa(i + 1, cadena, 10);
        if (tablas[i] == 0) display.drawString(12 * i, 0, cadena);
    }
    switch (tipo) {
        case 0:
            display.drawString(0, 16, "AxB = ?");
            break;
        case 1:
            display.drawString(0, 16, "Ax? = C");
            break;
        case 2:
            display.drawString(0, 16, "?xB = C");
            break;
        case 3:
            display.drawString(0, 16, "Ax? = ?");
            break;
        case 4:
            display.drawString(0, 16, "?x? = ?");
            break;
    }
    if (tiempo == 0) {
        display.drawString(64, 16, "-- s.");
    } else {
        sprintf(cadena, "%d s.", tiempo / 1000);
        display.drawString(64, 16, cadena);
    }
    display.setFont(Open_Sans_Regular_10);
    display.drawString(0, 36, "A] Tipo B] Tiempo C] Salir");
    display.drawString(0, 48, "D] Guardar *] Récord a 0");
    display.display();
}



void loop() {
    char key;
    int ultAltura = -1;

    if (configurando == 0) {
        dibujarTexto();
        while ((key = keypad.getKey()) == 0) {
            if (tiempo > 0) {
                int altura = 64 * tiempoRestante / tiempo;
                if (altura != ultAltura) {
                    dibujarTexto();
                    ultAltura = altura;
                }
                tiempoRestante -= 50;
                if (tiempoRestante < 0) break;
            }
            delay(50);
        }

        if (tiempoRestante < 0) {
            perder();
            nuevaPrueba();
            return;
        }

        if (key == '*' && numDigitos > 0) {
            numDigitos--;
            num = num / 10;
        } else if (key >= '0' && key <= '9' && numDigitos < maxDigitos) {
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
        procesaSecuencia(key);
        while (keypad.getKey() == key) delay(50);
    } else {
        dibujarConfiguracion();
        while ((key = keypad.getKey()) == 0) delay(50);
        if (key >= '0' && key <= '9') {
            int indice = key - '0' - 1;
            if (indice < 0) indice = 9;
            // Impedir quitar todas
            int cuenta = 0;
            for (int i = 0; i < 10; i++) {
                cuenta += tablas[i];
            }
            if (cuenta < 9 || tablas[indice] == 1) {
                tablas[indice] = 1 - tablas[indice];
            }
        } else if (key == 'A') {
            tipo = (tipo + 1) % 5;
        } else if (key == 'B') {
            tiempo = tiempo == 0 ? 10000 : tiempo - 1000;
            tiempoRestante = tiempo;
        } else if (key == 'C') {
            configurando = 0;
        } else if (key == 'D') {
            preferences.begin("app", false);
            preferences.putInt("tipo", tipo);
            preferences.putInt("tiempo", tiempo);
            preferences.putInt("record", record);
            for (int i = 0; i < 10; i++) {
                char nombre[9] = "tablas";
                itoa(i + 1, &nombre[6], 10);
                preferences.putInt(nombre, tablas[i]);
            }
            preferences.end();
            configurando = 0;
        } else if (key == '*') {
            record = 0;
        }
        while (keypad.getKey() == key) delay(50);
    }
}
