#include <Keypad.h>

// --arrumando o teclado
const byte ROWS = 4;
const byte COLS = 4;

// um mapeamento em matriz, com coordenadas
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {2, 3, 4, 5};
byte colPins[COLS] = {6, 7, 8, 9};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);



// as portas dos 7 segmentos do meu display do a ate o g
int segmentos[7] = {10, 11, 12, 13, A0, A1, A2};




// dizer onde a porta do display e a do led estao conectadas
int display = A4;        
int ledOverflow = A5;

 


// dizendo qual a acombinacao de luz de cada numero no display
// uma matriz, mas cada linha e um numero com os seus elementos
const byte numeros[10][7] = {
  {1,1,1,1,1,1,0}, // 0
  {0,1,1,0,0,0,0}, // 1
  {1,1,0,1,1,0,1}, // 2
  {1,1,1,1,0,0,1}, // 3
  {0,1,1,0,0,1,1}, // 4
  {1,0,1,1,0,1,1}, // 5
  {1,0,1,1,1,1,1}, // 6
  {1,1,1,0,0,0,0}, // 7
  {1,1,1,1,1,1,1}, // 8
  {1,1,1,1,0,1,1}  // 9

};


int A = -1;
int B = -1;
int estado = 0;         // 0: esperando A, 1: esperando B
int resultado = 0;



void setup() {
    //config bits por segund para serial
  Serial.begin(9600);   

  // configurar as portas como de output
  for (int i = 0; i < 7; i++) {
    pinMode(segmentos[i], OUTPUT);
  }
 


  pinMode(display, OUTPUT);        // configura a porta principal do display(que é anodo comum) como de saida
  digitalWrite(display, HIGH);     // para manter o display ativado no comeco
 

  pinMode(ledOverflow, OUTPUT); // configurar a porta do led over como de saida
  digitalWrite(ledOverflow, LOW);   // manter o led off no comeco
}





void loop() {
  char key = keypad.getKey();       // para pegar as entradas do teclado

  if (key && key >= '0' && key <= '9'){ // se existe, se e maior ou igual a e se e menor ou igual a 9
    int num = key - '0';        //??????????????


    if (estado == 0) {      // para receber o A
      A = num;
      Serial.print("A = ");
      Serial.println(A);

      estado = 1;
    }

    else if (estado == 1) {     // pra recebr B
      B = num;
      Serial.print("B = ");
      Serial.println(B);


      calcularSomaBinaria();

      estado = 0;
    }
   
  }


  atualizarDisplay();


  // para zerar tud
  if (key == 'A'){
    apagar();
    resultado = 0;
    digitalWrite(ledOverflow, LOW); 
  }

  // para dar um delay e nao ficar tiltando e dando  lag
  delay(50);
}








void calcularSomaBinaria() {
  if (B > A) {                  // fazr a inversao se o A for menor
    int temp = A;
    A = B;
    B = temp;
    Serial.println("B virou A");
  }


  int bitsA[5] = {0};   // vetor de 4 bits para o A
  int bitsB[5] = {0};   // vetor de 4 bitas para o B

  for (int i = 0; i < 5; i++) {
    bitsA[i] = (A >> i) & 1;    
    bitsB[i] = (B >> i) & 1;
  }

  int soma[6] = {0};    // vetor de 5 bits para guardar o numero e o overflow dele
  int carry = 0;

  for (int i = 0; i < 5; i++) {
    int total = bitsA[i] + bitsB[i] + carry;            // é em bit por bit, simula a soma das colunas da direita para a esqudrd
    soma[i] = total % 2;                        // soma[i] sera o resto por 2 na divisao inteira, 
                                                //ex: 0%2 da 0 0carry, e 1%2 da 1 0carry, 2%2 da 0 1carry, e 3%2 da 1 1carry | é o jeto para ter so 1 ou 0 aqui
    carry = total / 2;                                  // guarda o vai 1 caso a soma passar de 1, no caso se ela der 2 ou 3
  }                                                     
                                                        // preenche -----> da esq para a direit
                                                    // total pode ser 0 1 2 3
                                                    // 

  soma[5] = carry;      // ultima posicao a esquerda, é o carry

  resultado = 0;
  for (int i = 0; i < 6; i++) {
    if (soma[i]) {                  // desloca o i para a esquerda i vezes
      resultado += (1 << i);            // e como fazer 2^i so que apenas com os que soma[i] for 1
    }                                   // no meu vetor soma[] é [5]    [4]     [3]     [2]     [1]   [0]
                                        //        so para o overflow     2^4    2^3     2^2     2^1   2^0
  }

 
 
 
 
 
  Serial.print("Resultado binário: ");
  for (int i = 5; i >= 0; i--) {
    Serial.print(soma[i]);              // percorrer os 5 bits do resultado direito, da direta para a esquerda7
                                        // imprime da esquerda para direita um numero que o comeco e na direita
                                        // porque no caso eu guardo o num em binario de tras para frente
                                        // <----------
  }


  Serial.print("Resultado Decimal: ");
  Serial.println(resultado);

  if (resultado > 9) {          // excede os 4 bits
    Serial.println("--- O Resultado > 9. LED de overflow [ON].");
    digitalWrite(ledOverflow, HIGH);
  } else {
    digitalWrite(ledOverflow, LOW);
  }
}





// como esse display 7 segmentos é anodo, o low ascende e o high apaga
// mas o led é normal mesmo
void apagar(){
  for (int i = 0; i < 7; i++) {
    digitalWrite(segmentos[i], HIGH);
  }
  digitalWrite(ledOverflow, LOW);
}



//como ele é display 7 segmentos anodo, low ascende
// entao tem que trocar a definicao de segmentos acesos de
// cada um deles cim o not, que ai vai dar o inverso
void acenderDisplay(int numero) {
  for (int i = 0; i < 7; i++) {
    digitalWrite(segmentos[i], !numeros[numero][i]);
  }
}



// atualiza o display com o número calculado
void atualizarDisplay() {
  if (resultado <= 9) {
    acenderDisplay(resultado);
  } else {
    // apos verificar o resultado, se for maior que 9 ativa o modo mostrar unidades
    acenderDisplay(resultado % 10);     // mostrar a unidade, ja queo bit de overflow sera o 1 da dezena
  }
}