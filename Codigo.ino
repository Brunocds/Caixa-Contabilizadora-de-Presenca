#include <Keypad.h> //Biblioteca teclado matricial
#include <LiquidCrystal.h> //Biblioteca LCD
#include <MFRC522.h> //Biblioteca para o uso do módulo RFID
#include <SPI.h> //Biblioteca para comunicação SPI (utilizada pelo sensor RFID e módulo SD)
#include <SD.h> //Biblioteca para o uso do módulo SD

const byte qtdLinhas = 4; //Quantidade de linhas do teclado
const byte qtdColunas = 4; //Quantidade de colunas do teclado

//Setagem dos pinos utilizados pelo display LCD
const int rs = 2;
const int enable = 3;
const int d4 = 4;
const int d5 = 5;
const int d6 = 6;
const int d7 = 7;

//Determina o pino do buzzer
#define BZ 24
#define tempobuzzer 1500

//Determina os pinos utilizados pelo módulo RFID
#define SS_PIN A3
#define RST_PIN A0

//Determina o pino utilizado pelo módulo SD
#define chipselectSD A7

//Construção da matriz de caracteres do teclado matricial
char matriz_teclas[qtdLinhas][qtdColunas] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte PinosqtdLinhas[qtdLinhas] = {A8, A9, A10, A11}; //Pinos utilizados pelas linhas do teclado matricial
byte PinosqtdColunas[qtdColunas] = {A12, 44, 46, 48}; //Pinos utilizados pelas colunas do teclado matricial

//Criação do objeto teclado matricial
Keypad meuteclado = Keypad( makeKeymap(matriz_teclas), PinosqtdLinhas, PinosqtdColunas, qtdLinhas, qtdColunas);

//Criação do objeto display LCD
LiquidCrystal lcd(rs, enable, d4, d5, d6, d7);

//Criação do objeto sensor RFID
MFRC522 rfid(SS_PIN, RST_PIN);

//Criação do objeto responsável por ler/escrever utilizando o módulo SD
File myFile;

//função que lê o ID do cartão que é aproximado do módulo RFID e retorna uma string
String ler_cartao()
{
  rfid.PICC_ReadCardSerial();
  String cardID = "";
  for (byte i = 0; i < rfid.uid.size; i++)
  {
    cardID.concat(String(rfid.uid.uidByte[i] < 0x10 ? "0" : ""));
    cardID.concat(String(rfid.uid.uidByte[i], HEX));
  }
  return cardID;
}

typedef struct Timer
{
  unsigned long start;//Armazena o tempo de quando foi iniciado o timer
  unsigned long timeout;//Tempo após o start para o estouro
};

unsigned long Now (void)
{
  return millis();//Retorna os milisegundos depois do reset
}

char TimerExpired (struct Timer *timer)
{
  //Verifica se o timer estourou
  if ( Now() > timer->start + timer->timeout )
    return true;

  return false;
}

void TimerStart (struct Timer *timer)
{
  timer->start = Now();//Reseta o timer gravando o horário atual
}

//Rotinas menu inicial
Timer MenuInicial1 = {0, 2500}; //Muda o display lcd inicial para opção A
Timer MenuInicial2 = {0, 5000}; //Muda o display lcd inicial para opção B
Timer LeTeclado1 = {0, 10}; //Le o teclado a cada 10ms

//Rotinas menu cadastrar RA
Timer MenuCadRA1 = {0, 2500};
Timer MenuCadRA2 = {0, 5000};
Timer LeCartao2 = {5, 10};
Timer LeTeclado4 = {0, 10};

//Rotinas do menu cadastrar RA para entrar com RA
Timer MenuEntrarCadRA1 = {0, 2500};
Timer MenuEntrarCadRA2 = {0, 5000};
Timer MenuEntrarCadRA3 = {0, 7500};
Timer LeTeclado5 = {0, 10};

//Rotinas menu chamada
Timer MenuChamada1 = {0, 2500}; //Muda o display lcd para avisar para encostar a carteirinha
Timer MenuChamada2 = {0, 5000}; //Muda o display lcd para mostrar como entrar com RA
Timer MenuChamada3 = {0, 7500}; //Muda o display lcd para mostrar como finalizar a chamada
Timer LeCartao = {5, 10}; //Le o cartão a cada 10ms
Timer LeTeclado2 = {0, 10}; //Le o teclado a cada 10ms

//Rotinas menu chamada para entrar com RA
Timer MenuEntrarRA1 = {0, 2500};
Timer MenuEntrarRA2 = {0, 5000};
Timer MenuEntrarRA3 = {0, 7500};
Timer LeTeclado3 = {0, 10};

//Observação sobre os menus: para não utilizar delay e os fazer travar, fiz um esquema simples. Eu deixo cada opção como um múltiplo tempo
//anterior, exemplo: se quero exibir 2 opções a cada 2s, deixo a primeira com 2s e a segunda com 4s, se tivesse uma terceira ela teria 6s.
//A sacada pra esse timer é ir resetando as opções a cada chamada, na opção 1 eu reseto o tempo da 1, na opção 2 eu reseto o tempo da 1 e da 2,
//na opção 3 eu reseto o tempo da 1, da 2 e da 3. É complexo de entender, mas assim funciona. O princípio de funcionamento é uma forma de não deixar
//em regime permanente, em que a primeira opção oscilando a cada 1s ficaria o tempo todo na tela não deixando a com 2s aparecer. Resetar a de 1s faz
//com que a de 2s apareça.

void setup() {
  //Comandos de inicialização:
  Serial.begin(9600); //inicia comunicação com a serial do computador (serve para debugar utilizando o monitor serial)
  SPI.begin(); //inicia a comunicação SPI (é utilizado pelo módulo SD e RFID)
  lcd.begin(16, 2); //inicia o display LCD
  rfid.PCD_Init(); //inicia o módulo RFID

  //inicia o módulo SD
  SD.begin(chipselectSD);

  //cria os diretórios caso não existam
  if(!SD.exists("/DADOS")) SD.mkdir("/DADOS");
  if(!SD.exists("/LISTAS")) SD.mkdir("/LISTAS");
}

void loop() {
  //Menu inicial
  char tecla_pressionada; //tecla utilizada para navegar no primeiro menu
  while (1)
  {

    if (TimerExpired(&MenuInicial1)) //Muda o display lcd inicial para opção A
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Selecione o modo");
      lcd.setCursor(0, 1);
      lcd.print("A-cadastrar RA");
      TimerStart(&MenuInicial1);
    }

    if (TimerExpired(&MenuInicial2)) //Muda o display lcd inicial para opção B
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Selecione o modo");
      lcd.setCursor(0, 1);
      lcd.print("B-iniciar lista");
      TimerStart(&MenuInicial1);
      TimerStart(&MenuInicial2);
    }

    if (TimerExpired(&LeTeclado1)) //lê a tecla e direciona para a opção desejada
    {
      tecla_pressionada = meuteclado.getKey();
      TimerStart(&LeTeclado1);
      break;
    }

  }
  switch (tecla_pressionada)
  {
    case 'A': //caso para cadastrar o RA

      while (1)
      {
        if (TimerExpired(&MenuCadRA1))
        {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Aproxime a");
          lcd.setCursor(0, 1);
          lcd.print("carteirinha");
          TimerStart(&MenuCadRA1);
        }

        if (TimerExpired(&MenuCadRA2))
        {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Digite C para");
          lcd.setCursor(0, 1);
          lcd.print("finalizar");
          TimerStart(&MenuCadRA1);
          TimerStart(&MenuCadRA2);
        }

        if (TimerExpired(&LeTeclado4))
        {
          tecla_pressionada = meuteclado.getKey();
          if (tecla_pressionada == 'C') //se pressionado C, avisa no display e finaliza o cadastro. Se não pressionado, nada acontece.
          {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Cadastro");
            lcd.setCursor(0, 1);
            lcd.print("finalizado!");
            delay(2500);
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Voltando ao");
            lcd.setCursor(0, 1);
            lcd.print("menu inicial");
            delay(2500);
            TimerStart(&LeTeclado4);
            break;
          }
          TimerStart(&LeTeclado4);
        }

        if (TimerExpired(&LeCartao2)) //lê a tecla e direciona para a opção desejada
        {
          if (rfid.PICC_IsNewCardPresent())
          {
            tone(BZ,800,tempobuzzer);
            //Mostrando que leu no display LCD e armazenando o ID da carteirinha
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Carteirinha lida");
            lcd.setCursor(0, 1);
            lcd.print("com sucesso!");
            String cardID = ler_cartao();
            cardID += ".txt";
            delay(2500);
            String diretorio = "/DADOS/"; //ate este momento o cardID contem o ID da carteirinha e a extensão txt (ex: CC02953F.txt). aqui eu acrescento o diretório para ficar /DADOS/CC02953F.txt por exemplo.
            diretorio += cardID;
            //Verifica se a carteirinha ja foi cadastrada
            if (SD.exists(diretorio))
            {
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("Carteirinha ja");
              lcd.setCursor(0, 1);
              lcd.print("possui cadastro");
              delay(2500);
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("Deseja alterar?");
              lcd.setCursor(0, 1);
              lcd.print("Sim:'C' Nao:'*'");
              char tecladotemp;
              do
              {
                tecladotemp = meuteclado.waitForKey();
                if ((tecladotemp != 'C') && (tecladotemp != '*'))
                {
                  lcd.clear();
                  lcd.setCursor(0, 0);
                  lcd.print("Voce deve teclar");
                  lcd.setCursor(0, 1);
                  lcd.print("'C' ou '*'");
                  delay(2500);
                  lcd.clear();
                  lcd.setCursor(0, 0);
                  lcd.print("Deseja alterar?");
                  lcd.setCursor(0, 1);
                  lcd.print("Sim:'C' Nao:'*'");
                }
              }
              while ((tecladotemp != 'C') && (tecladotemp != '*'));
              if (tecladotemp == 'C')
              {
                String RA = "";

                while (1)
                {
                  if (TimerExpired(&MenuEntrarCadRA1)) //Muda o display lcd para avisar para entrar com o RA
                  {
                    lcd.clear();
                    lcd.setCursor(0, 0);
                    lcd.print("Digite seu RA:");
                    lcd.setCursor(0, 1);
                    lcd.print(RA);
                    TimerStart(&MenuEntrarCadRA1);
                  }

                  if (TimerExpired(&MenuEntrarCadRA2)) //Muda o display lcd para mostrar como enviar o RA
                  {
                    lcd.clear();
                    lcd.setCursor(0, 0);
                    lcd.print("C- enviar");
                    lcd.setCursor(0, 1);
                    lcd.print(RA);
                    TimerStart(&MenuEntrarCadRA1);
                    TimerStart(&MenuEntrarCadRA2);
                  }

                  if (TimerExpired(&MenuEntrarCadRA3)) //Muda o display lcd para mostrar como apagar um caractere
                  {
                    lcd.clear();
                    lcd.setCursor(0, 0);
                    lcd.print("*-apaga caracter");
                    lcd.setCursor(0, 1);
                    lcd.print(RA);
                    TimerStart(&MenuEntrarCadRA1);
                    TimerStart(&MenuEntrarCadRA2);
                    TimerStart(&MenuEntrarCadRA3);
                  }

                  if (TimerExpired(&LeTeclado5)) //Le o teclado matricial, e ao comando da letra C envia o RA para a lista de presença
                  {
                    char digito = meuteclado.getKey();
                    if (digito != 'C')
                    {
                      if (digito == '*') //apaga um caractere quando '*' for pressionado e printa no LCD
                      {
                        int lastindex = RA.length() - 1;
                        RA.setCharAt(lastindex, ' ');
                        lcd.setCursor(0, 1);
                        lcd.print(RA);
                        RA.trim();
                        TimerStart(&LeTeclado5);
                      }
                      else if (digito) //acrescenta um caractere (se pressionado) à variavel RA e printa no LCD
                      {
                        RA += digito;
                        lcd.setCursor(0, 1);
                        lcd.print(RA);
                        TimerStart(&LeTeclado5);
                      }
                    }
                    else
                    {
                      lcd.clear();
                      lcd.setCursor(0, 0);
                      lcd.print("RA da carteirinh");
                      lcd.setCursor(0, 1);
                      lcd.print("foi alterado");
                      delay(2500);
                      SD.remove(diretorio);
                      myFile = SD.open(diretorio, FILE_WRITE);
                      myFile.print(RA);
                      myFile.close();
                      TimerStart(&LeTeclado5);
                      break;
                    }
                  }
                }
              }
              else
              {
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("RA da carteirinh");
                lcd.setCursor(0, 1);
                lcd.print("nao foi alterado");
                delay(2500);
              }
            }
            else
            {
              String RA = "";

              while (1)
              {
                if (TimerExpired(&MenuEntrarCadRA1)) //Muda o display lcd para avisar para entrar com o RA
                {
                  lcd.clear();
                  lcd.setCursor(0, 0);
                  lcd.print("Digite seu RA:");
                  lcd.setCursor(0, 1);
                  lcd.print(RA);
                  TimerStart(&MenuEntrarCadRA1);
                }

                if (TimerExpired(&MenuEntrarCadRA2)) //Muda o display lcd para mostrar como enviar o RA
                {
                  lcd.clear();
                  lcd.setCursor(0, 0);
                  lcd.print("C- enviar");
                  lcd.setCursor(0, 1);
                  lcd.print(RA);
                  TimerStart(&MenuEntrarCadRA1);
                  TimerStart(&MenuEntrarCadRA2);
                }

                if (TimerExpired(&MenuEntrarCadRA3)) //Muda o display lcd para mostrar como apagar um caractere
                {
                  lcd.clear();
                  lcd.setCursor(0, 0);
                  lcd.print("*-apaga caracter");
                  lcd.setCursor(0, 1);
                  lcd.print(RA);
                  TimerStart(&MenuEntrarCadRA1);
                  TimerStart(&MenuEntrarCadRA2);
                  TimerStart(&MenuEntrarCadRA3);
                }

                if (TimerExpired(&LeTeclado5)) //Le o teclado matricial, e ao comando da letra C envia o RA para a lista de presença
                {
                  char digito = meuteclado.getKey();
                  if (digito != 'C')
                  {
                    if (digito == '*') //apaga um caractere quando '*' for pressionado e printa no LCD
                    {
                      int lastindex = RA.length() - 1;
                      RA.setCharAt(lastindex, ' ');
                      lcd.setCursor(0, 1);
                      lcd.print(RA);
                      RA.trim();
                      TimerStart(&LeTeclado5);
                    }
                    else if (digito) //acrescenta um caractere (se pressionado) à variavel RA e printa no LCD
                    {
                      RA += digito;
                      lcd.setCursor(0, 1);
                      lcd.print(RA);
                      TimerStart(&LeTeclado5);
                    }
                  }
                  else
                  {
                    lcd.clear();
                    lcd.setCursor(0, 0);
                    lcd.print("RA cadastrado no");
                    lcd.setCursor(0, 1);
                    lcd.print("banco de dados");
                    delay(2500);
                    myFile = SD.open(diretorio, FILE_WRITE);
                    myFile.print(RA);
                    myFile.close();
                    TimerStart(&LeTeclado5);
                    break;
                  }
                }
              }
            }
          }
          TimerStart(&LeCartao2);
        }
      }
      break;
    case 'B': //caso para fazer a lista de presença
      String nomelista = "LP-"; //cria a variável que armazenará a lista de presença
      String data = "";
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Insira a data");
      lcd.setCursor(0, 1);
      lcd.print("de hoje");
      delay(2500);
      while (1)
      {
        if (TimerExpired(&MenuEntrarRA1)) //Muda o display lcd para mostrar o formato de data a se entrar
        {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Data: DD#MM");
          lcd.setCursor(0, 1);
          lcd.print(data);
          TimerStart(&MenuEntrarRA1);
        }

        if (TimerExpired(&MenuEntrarRA2)) //Muda o display lcd para mostrar como enviar a data
        {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("C- enviar");
          lcd.setCursor(0, 1);
          lcd.print(data);
          TimerStart(&MenuEntrarRA1);
          TimerStart(&MenuEntrarRA2);
        }

        if (TimerExpired(&MenuEntrarRA3)) //Muda o display lcd para mostrar como apagar um caractere
        {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("*-apaga caracter");
          lcd.setCursor(0, 1);
          lcd.print(data);
          TimerStart(&MenuEntrarRA1);
          TimerStart(&MenuEntrarRA2);
          TimerStart(&MenuEntrarRA3);
        }

        if (TimerExpired(&LeTeclado3)) //Le o teclado matricial, e ao comando da letra C envia a data
        {
          char digito = meuteclado.getKey();
          if (digito != 'C')
          {
            if (digito == '*') //apaga um caractere quando '*' for pressionado e printa no LCD
            {
              int lastindex = data.length() - 1;
              data.setCharAt(lastindex, ' ');
              lcd.setCursor(0, 1);
              lcd.print(data);
              data.trim();
              TimerStart(&LeTeclado3);
            }
            else if (digito) //acrescenta um caractere (se pressionado) à variavel data e printa no LCD
            {
              data += digito;
              lcd.setCursor(0, 1);
              lcd.print(data);
              TimerStart(&LeTeclado3);
            }
          }
          else
          {
            if (data.length() != 5 || isDigit(data.charAt(2))) //verifica se a data esta no formato válido
            {
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("Formato invalido");
              lcd.setCursor(0, 1);
              lcd.print("Digite: DD#MM");
              data = "";
              delay(2500);
            }
            else
            {
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("Data inserida!");
              lcd.setCursor(0, 1);
              lcd.print("Iniciando lista");
              delay(2000);
              String dia = "";
              String mes = "";
              dia += data.charAt(0);  
              dia += data.charAt(1);
              mes += data.charAt(3);
              mes += data.charAt(4);
              nomelista += dia;
              nomelista += '_';
              nomelista += mes;
              break;
            }
          }
        }
      }

      nomelista += ".txt";
      String diretoriolista = "/LISTAS/"; //ate este momento a 'nomelista' contem a data a extensão txt (ex: LP-01_12.txt). aqui eu acrescento o diretório para ficar /LISTAS/LP-01_12.txt por exemplo.
      diretoriolista += nomelista;
      if(!SD.exists(diretoriolista))
      {
      myFile = SD.open(diretoriolista, FILE_WRITE);
      myFile.println("Lista dos RAs dos alunos que estiveram presentes:");
      myFile.close();
      }
      while (1)
      {

        if (TimerExpired(&MenuChamada1)) //Muda o display lcd para avisar para encostar a carteirinha
        {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Aproxime a sua");
          lcd.setCursor(0, 1);
          lcd.print("carteirinha");
          TimerStart(&MenuChamada1);
        }

        if (TimerExpired(&MenuChamada2)) //Muda o display lcd para mostrar como entrar com RA
        {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Para entrar com");
          lcd.setCursor(0, 1);
          lcd.print("um RA tecle D");
          TimerStart(&MenuChamada1);
          TimerStart(&MenuChamada2);
        }

        if (TimerExpired(&MenuChamada3)) //Muda o display lcd para mostrar como finalizar a chamada
        {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Para finalizar");
          lcd.setCursor(0, 1);
          lcd.print("a lista tecle C");
          TimerStart(&MenuChamada1);
          TimerStart(&MenuChamada2);
          TimerStart(&MenuChamada3);
        }

        if (TimerExpired(&LeCartao)) //rotina que lê o cartão, compara com o banco de dados e contabiliza a presença no SD
        {
          if (rfid.PICC_IsNewCardPresent()) {
            //Mostrando que leu no display LCD
            tone(BZ,800,tempobuzzer);
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Carteirinha lida");
            lcd.setCursor(0, 1);
            lcd.print("com sucesso!");
            String cardID = ler_cartao();
            delay(2500);

            //Verificando se o RA foi cadastrado e lendo o RA dentro do arquivo
            cardID += ".txt";
            cardID.toUpperCase();
            String RA = "";
            String diretorio = "/DADOS/"; //ate este momento o cardID contem o ID da carteirinha e a extensão txt (ex: CC02953F.txt). aqui eu acrescento o diretório para ficar /DADOS/CC02953F.txt por exemplo.
            diretorio += cardID;
            if (SD.exists(diretorio))
            {
              myFile = SD.open(diretorio);
              while (myFile.available())
              {
                RA += (char)myFile.read();
              }
              myFile.close();
              myFile = SD.open(diretoriolista, FILE_WRITE);
              myFile.println(RA);
              myFile.close();
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("RA cadastrado");
              lcd.setCursor(0, 1);
              lcd.print("na lista");
              delay(2500);
            }
            else //informa que a carteirinha não está cadastrada
            {
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("Carteirinha nao");
              lcd.setCursor(0, 1);
              lcd.print("cadastrada");
              delay(2500);
            }
          }
          TimerStart(&LeCartao);
        }

        if (TimerExpired(&LeTeclado2)) //rotina utilizada para finalizar a lista de presença ou entrar com novo RA
        {
          char tempteclado = meuteclado.getKey();
          if (tempteclado == 'C')
          {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Lista de chamada");
            lcd.setCursor(0, 1);
            lcd.print("finalizada!");
            delay(2500);
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Voltando ao");
            lcd.setCursor(0, 1);
            lcd.print("menu inicial");
            delay(2500);
            TimerStart(&LeTeclado2);
            break;
          }
          if (tempteclado == 'D')
          {
            String RA = "";

            while (1)
            {

              if (TimerExpired(&MenuEntrarRA1)) //Muda o display lcd para avisar para entrar com o RA
              {
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Digite seu RA:");
                lcd.setCursor(0, 1);
                lcd.print(RA);
                TimerStart(&MenuEntrarRA1);
              }

              if (TimerExpired(&MenuEntrarRA2)) //Muda o display lcd para mostrar como enviar o RA
              {
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("C- enviar");
                lcd.setCursor(0, 1);
                lcd.print(RA);
                TimerStart(&MenuEntrarRA1);
                TimerStart(&MenuEntrarRA2);
              }

              if (TimerExpired(&MenuEntrarRA3)) //Muda o display lcd para mostrar como apagar um caractere
              {
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("*-apaga caracter");
                lcd.setCursor(0, 1);
                lcd.print(RA);
                TimerStart(&MenuEntrarRA1);
                TimerStart(&MenuEntrarRA2);
                TimerStart(&MenuEntrarRA3);
              }

              if (TimerExpired(&LeTeclado3)) //Le o teclado matricial, e ao comando da letra C envia o RA para a lista de presença
              {
                char digito = meuteclado.getKey();
                if (digito != 'C')
                {
                  if (digito == '*') //apaga um caractere quando '*' for pressionado e printa no LCD
                  {
                    int lastindex = RA.length() - 1;
                    RA.setCharAt(lastindex, ' ');
                    lcd.setCursor(0, 1);
                    lcd.print(RA);
                    RA.trim();
                    TimerStart(&LeTeclado3);
                  }
                  else if (digito) //acrescenta um caractere (se pressionado) à variavel RA e printa no LCD
                  {
                    RA += digito;
                    lcd.setCursor(0, 1);
                    lcd.print(RA);
                    TimerStart(&LeTeclado3);
                  }
                }
                else
                {
                  lcd.clear();
                  lcd.setCursor(0, 0);
                  lcd.print("RA cadastrado");
                  lcd.setCursor(0, 1);
                  lcd.print("na lista");
                  delay(2500);
                  myFile = SD.open(diretoriolista, FILE_WRITE);
                  myFile.println(RA);
                  myFile.close();
                  TimerStart(&LeTeclado3);
                  break;
                }
              }
            }
          }
          TimerStart(&LeTeclado2);
        }
      }
      break;
    default:
      //
      break;
  }
}
/*
  Funções uteis:

  LCD: lcd.setCursor(0, 0)
      lcd.print("Digite seu RA:")
      lcd.clear()
  SD:  myFile.available()
      myFile.read()
      myFile = SD.open("file.txt")
      myFile.close()
  TECLADO: char tecla_pressionada = meuteclado.waitForKey()
           char tecla_pressionada = meuteclado.getKey()
*/
