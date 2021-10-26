//
// Controle de bomba e nivel da caixa dágua //
// 

#include <LiquidCrystal.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <DHT.h>
#include <DS1307.h>

// Define uma instancia do oneWire para comunicacao com o sensor / Porta do pino de sinal do DS18B20
    OneWire oneWire(10);
    DallasTemperature sensors(&oneWire);
    DeviceAddress (Aq);
    
// Sensor DHT
    DHT dht(A0,DHT22); 
     
//Define os pinos que serão utilizados para ligação ao display
    LiquidCrystal lcd(7, 6, 5, 4, 3, 2);
    
//Modulo RTC DS1307 ligado as portas A4 e A5 do Arduino 
    DS1307 rtc(A4, A5);
    
// Constantes (portas) para reles e alarme e backlight e botoes e Ultrasom
    const int Rl1 = 8; //RL1 (D8)
    const int Rl2 = 9; //RL2 (D9)
    const int Bt1 = 11; // Botao 01 (D11)
    const int Vai = 12; // Ultrasom (D12)
    const int Volta = 13; // Ultrasom (D13)
    const int Al1 = 15; // alarme (A1)
    const int Bl1 = 16; // BackLight (A2)
    const int Bt2 = 17; // Botao 02 (A3)
    const int Bb1 = 0; // Status Bomba (A4)
    const int Hb = 1; // HeartBeat (01 - Tx)

// constantes e variaveis de nivel
    const int offset = 19;
    int litros;
    float percentual;
    String bomba;

// Variaveis temperatura
    int T_quente;// Temp caixa aqua quente sensor Aq
    int Tambint, Umidade; // Temp interna e umidade

// Variaveis do sistema
    int tela=1;   // Determina a tela exibida
    int tela_ant; // tela anterior 
    
//********************************************************************

void setup()
{
 // Define Entradas e saidas
    pinMode(Rl1, OUTPUT);digitalWrite(Rl1,HIGH);// Rele1 ativo em Low
    pinMode(Rl2, OUTPUT);digitalWrite(Rl2,HIGH); // Rele2 ativo em Low
    pinMode(Al1, OUTPUT);digitalWrite(Al1,LOW); // Alarme ativo em High
    pinMode(Bl1, OUTPUT);digitalWrite(Bl1,LOW); //Backlight ativo em Low
    pinMode(Bb1, INPUT_PULLUP); //DEFINE Bb1 COMO ENTRADA (status bomba)  
    pinMode(Bt1, INPUT); //DEFINE Bt1 COMO ENTRADA
    pinMode(Bt2, INPUT); //DEFINE Bt2 COMO ENTRADA
    pinMode(Hb, OUTPUT); //HeartBeat
  
//Define o número de colunas e linhas do LCD
    lcd.begin(16, 2);
// Define pinos ultra 
    pinMode(Vai, OUTPUT);
    pinMode(Volta, INPUT);

 // inicializa DHT
    dht.begin(); 
 
// Inicializa DallasTemp
    sensors.begin();
    sensors.getAddress(Aq, 0);
    
//Aciona o relogio
     rtc.halt(false);
//As linhas abaixo setam a data e hora do modulo
rtc.setDOW(SUNDAY);      //Define o dia da semana
rtc.setTime(19, 00, 0);     //Define o horario
rtc.setDate(10, 10, 2021);   //Define o dia, mes e ano
//Definicoes do pino SQW/Out
  rtc.setSQWRate(SQW_RATE_1);
  rtc.enableSQW(true);

}

//********************************************************************

void loop()
{
digitalWrite(Hb,!digitalRead(Hb)); // HeartBeat

// Leitura dos botoes
  botao(); 
// Leitura de Volume cx daqua e ações
  volume();
// leitura das temperaturas
  temperaturas();
  
// Chama telas
    if (tela_ant!=tela){
      lcd.clear();
    }
    tela_ant=tela;             
switch (tela) {
  case 1:
    Tela_1(); // Chama Tela_1 Caixa e bomba
    break;
  case 2:
    Tela_2(); // Chama Tela_2 Temperatura
    break;
  case 3:
    Tela_3(); // Chama Tela_3 Relogio (Faz a leitura direto da rotina)
    break;
  //default:
    // comando(s)
   // break;
}

// Tempo de cada ciclo
 // delay(100);
}

//********************************************************************

// Rotina de leitura do ultrasom e conversão em litros
void volume()
{
  long duracao;
  int h;
    digitalWrite(Vai, LOW);
    delayMicroseconds(2);
    digitalWrite(Vai, HIGH);
    delayMicroseconds(10);
    digitalWrite(Vai, LOW);
    duracao = pulseIn(Volta, HIGH);
//Converte tempo em cm e em litros
    h = duracao/58;
    if (h<58)
    {
      litros=(h-offset)* 33;
      litros=2210-litros;
    }
    else if (h>=58)
    {
      litros=(h-offset-37)* 20;
      litros=940-litros;
    }
  // Arredonda litros
  litros=litros/10;
  litros=litros*10;

  // printa Altura
 // lcd.setCursor(13, 1);
 // lcd.print("   ");
 // lcd.setCursor(14, 1);
 // lcd.print(h);

 // chama alarme
   if (litros<350) {
    alarmebuzz();
    alarmedisplay();
    tela=1;
   }
 
// Liga ou desliga a bomba
  if (litros <= 1750) digitalWrite(Rl1, LOW); //Liga Bomba
  else if (litros >= 2050) digitalWrite(Rl1, HIGH); //Desiga Bomba

// Leitura status bomba
  bomba="Desl";
  if (digitalRead(Bb1) == LOW) bomba="Lig";

// chama alarme de bomba
  if ((litros <= 1750)&&(digitalRead(Bb1) == HIGH)){
    alarmedisplay();
    tela=1;
  }
  
}
//********************************************************************

void botao() // Le os botoes
{
// Leitura e ação botao 01
          if (digitalRead(Bt1) == HIGH){ //SE A LEITURA DO Botao1 FOR IGUAL A HIGH, FAZ
                digitalWrite(Bl1, HIGH); // Apaga BL 
            }
            
// Leitura e ação botao 02
          if ((digitalRead(Bt2) == HIGH)&&(digitalRead(Bl1) == HIGH)) digitalWrite(Bl1, LOW);// Acende BL //Se tela apagada, acende
              
          else if (digitalRead(Bt2) == HIGH)
                { //SE A LEITURA DO Botao2 FOR IGUAL A HIGH, FAZ
                digitalWrite(Bl1, LOW);// Acende BL
                tela=tela+1;
                }
                if (tela>3){
                  tela=1;
                }
          

}
//********************************************************************

void temperaturas()
{
   // Leitura Temp Dallas
          sensors.requestTemperatures();
          T_quente = sensors.getTempC(Aq);
    // Leitura do DHT
          Umidade = dht.readHumidity(); //Le o valor da umidade
          Tambint = dht.readTemperature(); //Le o valor da temperatura interna
}

//********************************************************************
void alarmebuzz()
{
      digitalWrite(Al1,HIGH);
      delay(300);
      digitalWrite(Al1,LOW);
    
}

//********************************************************************
void alarmedisplay()
{
      digitalWrite(Bl1, HIGH);// Apaga BL
      delay(100);
      digitalWrite(Bl1, LOW);// Acende BL
}


//********************************************************************

void Tela_1() // volume caixa e status bomba
{

//Printa litros
      lcd.setCursor(5,0);
      lcd.print("Litros");
      lcd.setCursor(0,0);
      lcd.print("    ");
      lcd.setCursor(0,0);
      lcd.print(litros);

      //Printa percentual
      lcd.setCursor(12,0);
      lcd.print("    ");
      lcd.setCursor(12,0);
      percentual=litros/2210.00;
      percentual=percentual*100;
      lcd.print(percentual,0);
      lcd.print("%");

        //Printa Status Bomba
      lcd.setCursor(0,1);
      lcd.print("Bomba:");
      lcd.setCursor(7,1);
      lcd.print("    ");
      lcd.setCursor(7,1);
      lcd.print(bomba);
      
}

void Tela_2() // Temperatura e umidade
{
//lcd.clear();
 
/* Printa Temp
        lcd.setCursor(0, 0);
        lcd.print("    ");
        lcd.setCursor(0, 0);
        lcd.print(T_quente);
        */

//Printa DHT
            lcd.setCursor(0, 0);
            lcd.print("Ti");
            lcd.setCursor(3, 0);
            lcd.print("   ");
            lcd.setCursor(3, 0);
            lcd.print(Tambint);

            lcd.setCursor(0, 1);
            lcd.print("Umidade:");
            lcd.setCursor(10, 1);
            lcd.print("   ");
            lcd.setCursor(10, 1);
            lcd.print(Umidade);
            lcd.print ("%");
}

void Tela_3() // Relogio e temp caixa agua quente
{
  //Mostra as informações
            lcd.setCursor(0, 0);
            lcd.print(rtc.getTimeStr(FORMAT_SHORT));
           // lcd.setCursor(5, 0);
            //lcd.print("   ");
            lcd.setCursor(7, 0);
            lcd.print(rtc.getDateStr(FORMAT_SHORT));

            lcd.setCursor(0, 1);
            lcd.print("Agua quente:");
            lcd.setCursor(12, 1);
            lcd.print("   ");
            lcd.setCursor(12, 1);
            lcd.print(T_quente);
  /*
  Serial.print("Hora : ");
  Serial.print(rtc.getTimeStr());
  Serial.print(" ");
  Serial.print("Data : ");
  Serial.print(rtc.getDateStr(FORMAT_SHORT));
  Serial.print(" ");
  Serial.println(rtc.getDOWStr(FORMAT_SHORT));
  */
}
